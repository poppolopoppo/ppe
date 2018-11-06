#include "stdafx.h"

#include "HAL/Windows/WindowsPlatformProcess.h"

#ifdef PLATFORM_WINDOWS

#include "Diagnostic/Logger.h"
#include "Container/Vector.h"
#include "IO/Format.h"
#include "IO/FormatHelpers.h"
#include "IO/String.h"
#include "IO/StringBuilder.h"
#include "HAL/PlatformMemory.h"
#include "HAL/PlatformMisc.h"

#include "HAL/Windows/DbgHelpWrapper.h"
#include "HAL/Windows/LastError.h"

#include <Psapi.h>
#include <shellapi.h>
#include <TlHelp32.h>

// #TODO put this in Core.Application/HAL
// http://developer.download.nvidia.com/devzone/devcenter/gamegraphics/files/OptimusRenderingPolicies.pdf
// The following line is to favor the high performance NVIDIA GPU if there are multiple GPUs
// Has to be .exe module to be correctly detected.
extern "C" { _declspec(dllexport) u32 NvOptimusEnablement = 0x00000001; }

// And the AMD equivalent
// Also has to be .exe module to be correctly detected.
extern "C" { _declspec(dllexport) u32 AmdPowerXpressRequestHighPerformance = 0x00000001; }

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static bool GIsFirstInstance = false;
static ::HANDLE GNamedMutex = NULL;
//----------------------------------------------------------------------------
template <typename _Char>
struct TBasicNullTerminatedStr_ {
    _Char Buffer[MAX_PATH + 1];
    TBasicNullTerminatedStr_(const TBasicStringView<_Char>& str) {
        Assert(str.size() <= MAX_PATH);
        FPlatformMemory::Memcpy(Buffer, str.data(), str.SizeInBytes());
        Buffer[str.size()] = L'\0';
    }
    operator const wchar_t* () const {
        return (&Buffer[0]);
    }
};
using FNullTerminatedStr_ = TBasicNullTerminatedStr_<char>;
using FWNullTerminatedStr_ = TBasicNullTerminatedStr_<wchar_t>;
//----------------------------------------------------------------------------
static void ReleaseNamedMutex_(void) {
    if (GNamedMutex) {
        ::ReleaseMutex(GNamedMutex);
        GNamedMutex = NULL;
    }
}
//----------------------------------------------------------------------------
static bool MakeNamedMutex_(const char* name) {
    char szFileName[MAX_PATH + 1];
    if (nullptr == name) {
        Verify(::GetModuleFileNameA(NULL, szFileName, MAX_PATH + 1));
        name = szFileName;
    }

    GNamedMutex = ::CreateMutexA(NULL, true, name);

    if (GNamedMutex	&& ::GetLastError() != ERROR_ALREADY_EXISTS) {
        // We're the first instance!
        return true;
    }
    else {
        // Still need to release it in this case, because it gave us a valid copy
        ReleaseNamedMutex_();
        // There is already another instance of the game running.
        return false;
    }
}
//----------------------------------------------------------------------------
static bool OpenWebURL_(const FWStringView& url) {
    LOG(HAL, Info, L"OpenWebURL({0})", url);

    FWString browserOpenCommand;

    // First lookup the program Id for the default browser.
    FWString progId;
    if (FWindowsPlatformMisc::QueryRegKey(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\Shell\\Associations\\UrlAssociations\\http\\UserChoice", L"Progid", &progId)) {
        // If we found it, then lookup it's open shell command in the classes registry.
        const FWString browserRegPath = progId + L"\\shell\\open\\command";
        FWindowsPlatformMisc::QueryRegKey(HKEY_CLASSES_ROOT, browserRegPath.c_str(), NULL, &browserOpenCommand);
    }

    // If we failed to find a default browser using the newer location, revert to using shell open command for the HTTP file association.
    if (browserOpenCommand.empty())
        FWindowsPlatformMisc::QueryRegKey(HKEY_CLASSES_ROOT, L"http\\shell\\open\\command", NULL, &browserOpenCommand);

    // If we have successfully looked up the correct shell command, then we can create a new process using that command
    // we do this instead of shell execute due to security concerns.  By starting the browser directly we avoid most issues.
    if (not browserOpenCommand.empty()) {
        // If everything has gone to plan, the shell command should be something like this:
        // "C:\Program Files (x86)\Mozilla Firefox\firefox.exe" -osint -url "%1"

        // If anything failed to parse right, don't continue down this path, just use shell execute.

        if (browserOpenCommand.gsub(L"%1", url) == 0) {
            // If we fail to detect the placement token we append the URL to the arguments.
            // This is for robustness, and to fix a known error case when using Internet Explorer 8.
            browserOpenCommand.append(L" \"");
            browserOpenCommand.append(url);
            browserOpenCommand.append(L"\"");
        }

        // Now that we have the shell open command to use, run the shell command in the open process with any and all parameters.
        if (FWindowsPlatformProcess::ExecDetachedProcess(browserOpenCommand))
            return true;
    }

    // If all else fails just do a shell execute and let windows sort it out.  But only do it if it's an
    // HTTP or HTTPS address.  A malicious address could be problematic if just passed directly to shell execute.
    if (StartsWithI(url, L"http://") || StartsWithI(url, L"https://")) {
        const FWString urlNullTerminated(url);
        const ::HINSTANCE code = ::ShellExecuteW(NULL, L"open", urlNullTerminated.data(), NULL, NULL, SW_SHOWNORMAL);
        if (intptr_t(code) > 32) // https://docs.microsoft.com/en-us/windows/desktop/api/shellapi/nf-shellapi-shellexecutea
            return true;

        LOG(HAL, Error, L"failed open web url : {0}", url);
    }

    return false;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FWindowsPlatformProcess::OnProcessStart() {
#   if USE_PPE_MEMORY_DEBUGGING || USE_PPE_DEBUG
    constexpr int debugHeapEnabled = _CRTDBG_ALLOC_MEM_DF;
    constexpr int debugCheckMemory = _CRTDBG_CHECK_EVERY_1024_DF;
    constexpr int debugNecrophilia = _CRTDBG_DELAY_FREE_MEM_DF;
    constexpr int debugLeaks = _CRTDBG_LEAK_CHECK_DF;

    UNUSED(debugHeapEnabled);
    UNUSED(debugCheckMemory);
    UNUSED(debugNecrophilia);
    UNUSED(debugLeaks);

    int debugHeapFlag = 0
        | debugHeapEnabled
        //| debugCheckMemory //%_NOCOMMIT%
        //| debugNecrophilia //%_NOCOMMIT%
        | debugLeaks;

    UNUSED(debugHeapFlag);

    _CrtSetDbgFlag(debugHeapFlag);

    // Report errors with a dialog box :
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG | _CRTDBG_MODE_WNDW);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG | _CRTDBG_MODE_WNDW);

    //_CrtSetBreakAlloc(447); // for leak debugging purpose // %_NOCOMMIT%
    //_CrtSetBreakAlloc(1246); // for leak debugging purpose // %_NOCOMMIT%

#   endif

    GIsFirstInstance = MakeNamedMutex_(nullptr);
    FDbghelpWrapper::Create();
}
//----------------------------------------------------------------------------
void FWindowsPlatformProcess::OnProcessShutdown() {
    FDbghelpWrapper::Destroy();
    ReleaseNamedMutex_();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FWindowsPlatformProcess::Sleep(float seconds) {
    const ::DWORD milliseconds = (::DWORD)(seconds * 1000.0f);
    if (milliseconds == 0)
        ::SwitchToThread();
    else
        ::Sleep(milliseconds);
}
//----------------------------------------------------------------------------
void FWindowsPlatformProcess::SleepInfinite() {
    ::Sleep(INFINITE);
}
//----------------------------------------------------------------------------
// Hybrid spinning
// http://www.1024cores.net/home/lock-free-algorithms/tricks/spinning
void FWindowsPlatformProcess::SleepForSpinning(size_t& backoff) {
    if (backoff < 10)
        ::_mm_pause();
        // (1) improve performance (help to fight memory ordering issues inside of a processor),
        // (2) decrease power consumption,
        // (3) further improve performance in the context of HyperThreading/Simultaneous Multithreading/Chip-level Multithreading
    else if (backoff < 20)
        for (int i = 0; i != 50; i += 1) ::_mm_pause();
    else if (backoff < 22)
        ::SwitchToThread(); // limited to the current processor
    else if (backoff < 24)
        ::Sleep(0); //  limited to threads of no-less priority
    else if (backoff < 26)
        ::Sleep(1); // all threads
    else
        ::Sleep(10); // all threads

    ++backoff;
}
//----------------------------------------------------------------------------
auto FWindowsPlatformProcess::CurrentPID() -> FProcessId {
    return ::GetCurrentProcessId();
}
//----------------------------------------------------------------------------
void FWindowsPlatformProcess::Daemonize() {
    // #TODO
}
//----------------------------------------------------------------------------
bool FWindowsPlatformProcess::IsFirstInstance() {
    return GIsFirstInstance;
}
//----------------------------------------------------------------------------
bool FWindowsPlatformProcess::IsProcessAlive(FProcessId pid) {
    return IsProcessAlive(OpenProcess(pid));
}
//----------------------------------------------------------------------------
bool FWindowsPlatformProcess::IsProcessAlive(FProcessHandle process) {
    Assert(process);

    const ::DWORD waitResult = ::WaitForSingleObject(process, 0);
    return (waitResult == WAIT_TIMEOUT);
}
//----------------------------------------------------------------------------
auto FWindowsPlatformProcess::OpenProcess(FProcessId pid) -> FProcessHandle {
    return ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
}
//----------------------------------------------------------------------------
void FWindowsPlatformProcess::WaitForProcess(FProcessHandle process) {
    Assert(process);

    ::WaitForSingleObject(process, INFINITE);
}
//----------------------------------------------------------------------------
void FWindowsPlatformProcess::CloseProcess(FProcessHandle process) {
    Assert(process);

    ::CloseHandle(process);
}
//----------------------------------------------------------------------------
void FWindowsPlatformProcess::TerminateProcess(FProcessHandle process, bool killTree) {
    Assert(process);

    if (killTree) {
        ::HANDLE snapShot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

        if (snapShot != INVALID_HANDLE_VALUE) {
            ::DWORD processId = ::GetProcessId(process);

            ::PROCESSENTRY32 entry;
            entry.dwSize = sizeof(::PROCESSENTRY32);

            if (::Process32First(snapShot, &entry)) {
                do {
                    if (entry.th32ParentProcessID == processId) {
                        ::HANDLE childHandle = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, entry.th32ProcessID);

                        if (childHandle)
                            FWindowsPlatformProcess::TerminateProcess(childHandle, killTree);
                    }
                } while (::Process32Next(snapShot, &entry));
            }
        }
    }

    ::TerminateProcess(process, 0);
}
//----------------------------------------------------------------------------
FString FWindowsPlatformProcess::ProcessName(FProcessId pid) {
    const ::HANDLE process = ::OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);

    if (process != NULL) {
        char buffer[MAX_PATH + 1];
        ::DWORD len = lengthof(buffer);

#if WINVER == 0x0502
        ::GetProcessImageFileNameA(process, buffer, len);
#else
        ::QueryFullProcessImageNameA(process, 0, buffer, &len);
#endif

        Verify(::CloseHandle(process));

        return ToString(MakeCStringView(buffer));
    }

    return FString();
}
//----------------------------------------------------------------------------
u64 FWindowsPlatformProcess::ProcessMemoryUsage(FProcessId pid) {
    u64 memUsage = 0;
    const ::HANDLE process = ::OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);

    if (process != NULL) {
        ::PROCESS_MEMORY_COUNTERS_EX memInfo;

        if (::GetProcessMemoryInfo(process, (::PROCESS_MEMORY_COUNTERS*)&memInfo, sizeof(memInfo)))
            memUsage = checked_cast<u64>(memInfo.PrivateUsage);

        Verify(::CloseHandle(process));
    }

    return memUsage;
}
//----------------------------------------------------------------------------
bool FWindowsPlatformProcess::CreatePipe(FPipeHandle* pRead, FPipeHandle* pWrite) {
    Assert(pRead);
    Assert(pWrite);

    ::SECURITY_ATTRIBUTES security = {
        sizeof(::SECURITY_ATTRIBUTES),
        NULL, TRUE
    };

    if (not ::CreatePipe(pRead, pWrite, &security, 0))
        return false;

    if (not ::SetHandleInformation(*pRead, HANDLE_FLAG_INHERIT, 0))
        return false;

    return true;
}
//----------------------------------------------------------------------------
bool FWindowsPlatformProcess::ReadPipe(FPipeHandle read, FStringBuilder& buffer) {
    Assert(read);

    ::DWORD bytesAvailable = 0;
    if (::PeekNamedPipe(read, NULL, 0, NULL, &bytesAvailable, NULL) && (bytesAvailable > 0)) {

        const TMemoryView<char> data = buffer.AppendUninitialized(bytesAvailable);

        ::DWORD bytesRead = 0;
        if (::ReadFile(read, data.Pointer(), bytesAvailable, &bytesRead, NULL) && bytesRead > 0) {
            Assert(bytesRead == bytesAvailable);
            return true;
        }
    }

    return false;
}
//----------------------------------------------------------------------------
bool FWindowsPlatformProcess::WritePipe(FPipeHandle write, const FStringView& buffer) {
    // If there is not a message or WritePipe is null
    if (buffer.empty() || write == nullptr)
        return false;

    // Write to pipe
    ::DWORD bytesWritten = 0;
    const ::DWORD bytesAvailable = checked_cast<::DWORD>(buffer.SizeInBytes());

    if (::WriteFile(write, buffer.data(), bytesAvailable, &bytesWritten, NULL) == TRUE) {
        Assert(bytesWritten == bytesAvailable);
        return true;
    }
    else {
        Assert(bytesWritten == 0);
        return false;
    }
}
//----------------------------------------------------------------------------
void FWindowsPlatformProcess::ClosePipe(FPipeHandle read, FPipeHandle write) {
    if (read != NULL && read != INVALID_HANDLE_VALUE)
        ::CloseHandle(read);

    if (write != NULL && write != INVALID_HANDLE_VALUE)
        ::CloseHandle(write);
}
//----------------------------------------------------------------------------
auto FWindowsPlatformProcess::CreateProcess(
    FProcessId* pPID,
    const FWStringView& url,
    const TMemoryView<const FWStringView>& args,
    bool detached, bool hidden, bool noWindow, int priority,
    const FWStringView& optionalWorkingDir,
    FPipeHandle readPipe/* = nullptr */,
    FPipeHandle writePipe/* = nullptr */) -> FProcessHandle {
    Assert(pPID);
    Assert(not url.empty());

    // initialize process attributes
    ::SECURITY_ATTRIBUTES security;
    security.nLength = sizeof(::SECURITY_ATTRIBUTES);
    security.lpSecurityDescriptor = NULL;
    security.bInheritHandle = true;

    // initialize process creation flags
    ::DWORD createFlags = NORMAL_PRIORITY_CLASS;

    if (priority < 0)
        createFlags = (priority == -1) ? BELOW_NORMAL_PRIORITY_CLASS : IDLE_PRIORITY_CLASS;
    else if (priority > 0)
        createFlags = (priority ==  1) ? ABOVE_NORMAL_PRIORITY_CLASS : HIGH_PRIORITY_CLASS;

    if (detached)
        createFlags |= DETACHED_PROCESS;

    // initialize window flags
    ::DWORD dwFlags = 0;
    ::WORD showWindowFlags = SW_HIDE;
    if (noWindow) {
        dwFlags = STARTF_USESHOWWINDOW;
    }
    else if (hidden) {
        dwFlags = STARTF_USESHOWWINDOW;
        showWindowFlags = SW_SHOWMINNOACTIVE;
    }

    if (readPipe || writePipe)
        dwFlags |= STARTF_USESTDHANDLES;

    // initialize startup info
    ::STARTUPINFO startupInfo = {
        sizeof(::STARTUPINFO),
        NULL, NULL, NULL,
        (::DWORD)CW_USEDEFAULT,
        (::DWORD)CW_USEDEFAULT,
        (::DWORD)CW_USEDEFAULT,
        (::DWORD)CW_USEDEFAULT,
        (::DWORD)0, (::DWORD)0, (::DWORD)0,
        (::DWORD)dwFlags,
        showWindowFlags,
        0, NULL,
        readPipe,
        writePipe,
        writePipe
    };

    // create the child process
    const FWString commandLine = StringFormat(
        L"\"{0}\" {1}",
        url,
        Fmt::Join(
            args.Map([](const FWStringView& arg) { return Fmt::Quoted(arg, L'"'); }),
            L' ') );

    FWString workingDir;
    if (optionalWorkingDir.size())
        workingDir.assign(optionalWorkingDir);

    ::PROCESS_INFORMATION procInfo;
    if (not ::CreateProcessW(
        NULL,
        (::LPWSTR)commandLine.data(),
        &security, &security, TRUE,
        (::DWORD)createFlags, NULL,
        workingDir.data(),
        &startupInfo,
        &procInfo )) {
        const ::DWORD errorCode = ::GetLastError();

        LOG(HAL, Warning, L"CreateProcess failed: {0}", FLastError(errorCode));
        if (errorCode == ERROR_NOT_ENOUGH_MEMORY || errorCode == ERROR_OUTOFMEMORY) {
            // These errors are common enough that we want some available memory information
            const FPlatformMemory::FStats stats = FPlatformMemory::Stats();
            LOG(HAL, Warning, TEXT("Mem used: {0}, OS Free {1}"),
                Fmt::SizeInBytes(stats.UsedPhysical),
                Fmt::SizeInBytes(stats.AvailablePhysical) );
        }

        LOG(HAL, Warning, L"URL: {0}", commandLine);
        if (pPID != nullptr)
            *pPID = 0;

        return NULL;
    }

    if (pPID != nullptr)
        *pPID = procInfo.dwProcessId;

    ::CloseHandle(procInfo.hThread);

    Assert(procInfo.hProcess);
    return procInfo.hProcess;
}
//----------------------------------------------------------------------------
PRAGMA_MSVC_WARNING_PUSH()
PRAGMA_MSVC_WARNING_DISABLE(6387) // 'startupInfoEx.lpAttributeList' could be '0':  this does not adhere to the specification for the function 'UpdateProcThreadAttribute'.
bool FWindowsPlatformProcess::ExecProcess(
    int* pReturnCode,
    const FWStringView& url,
    const TMemoryView<const FWStringView>& args,
    FStringBuilder* pStdout/* = nullptr */,
    FStringBuilder* pStderr/* = nullptr */ ) {

    ::STARTUPINFOEXW startupInfoEx;
    ::ZeroMemory(&startupInfoEx, sizeof(startupInfoEx));
    startupInfoEx.StartupInfo.cb = (::DWORD)sizeof(startupInfoEx);
    startupInfoEx.StartupInfo.dwX = (::DWORD)CW_USEDEFAULT;
    startupInfoEx.StartupInfo.dwY = (::DWORD)CW_USEDEFAULT;
    startupInfoEx.StartupInfo.dwXSize = (::DWORD)CW_USEDEFAULT;
    startupInfoEx.StartupInfo.dwYSize = (::DWORD)CW_USEDEFAULT;
    startupInfoEx.StartupInfo.dwFlags = (::DWORD)STARTF_USESHOWWINDOW;
    startupInfoEx.StartupInfo.wShowWindow = (::WORD)SW_SHOWMINNOACTIVE;
    startupInfoEx.StartupInfo.hStdInput = ::GetStdHandle(STD_INPUT_HANDLE);

    ::HANDLE hStdOutRead = NULL;
    ::HANDLE hStdErrRead = NULL;

    TVector<u8> attributeList;

    if (pStdout || pStderr) {
        startupInfoEx.StartupInfo.dwFlags |= STARTF_USESTDHANDLES;

        ::SECURITY_ATTRIBUTES security;
        ::ZeroMemory(&security, sizeof(security));
        security.nLength = sizeof(::SECURITY_ATTRIBUTES);
        security.bInheritHandle = TRUE;

        if (pStdout)
            Verify(::CreatePipe(&hStdOutRead, &startupInfoEx.StartupInfo.hStdOutput, &security, 0));
        if (pStderr)
            Verify(::CreatePipe(&hStdErrRead, &startupInfoEx.StartupInfo.hStdError, &security, 0));

        ::SIZE_T bufferSize = 0;
        if (!::InitializeProcThreadAttributeList(NULL, 1, 0, &bufferSize) && ::GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
            attributeList.resize_AssumeEmpty(bufferSize);
            startupInfoEx.lpAttributeList = (::LPPROC_THREAD_ATTRIBUTE_LIST)attributeList.data();
            Verify(::InitializeProcThreadAttributeList(startupInfoEx.lpAttributeList, 1, 0, &bufferSize));
        }

        ::HANDLE inheritHandles[2] = {
            startupInfoEx.StartupInfo.hStdOutput,
            startupInfoEx.StartupInfo.hStdError
        };

        Verify(::UpdateProcThreadAttribute(startupInfoEx.lpAttributeList, 0, PROC_THREAD_ATTRIBUTE_HANDLE_LIST, inheritHandles, sizeof(inheritHandles), NULL, NULL));
    }

    bool succeed = false;

    const FWString commandLine = StringFormat(
        L"\"{0}\" {1}",
        url,
        Fmt::Join(
            args.Map([](const FWStringView& arg) { return Fmt::Quoted(arg, L'"'); }),
            L' ') );

    ::PROCESS_INFORMATION procInfo;
    if (::CreateProcessW(NULL, (::LPWSTR)commandLine.data(), NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS | DETACHED_PROCESS | EXTENDED_STARTUPINFO_PRESENT, NULL, NULL, &startupInfoEx.StartupInfo, &procInfo)) {
        if (pStdout || pStderr) {
            do {
                if (pStdout) ReadPipe(hStdOutRead, *pStdout);
                if (pStderr) ReadPipe(hStdErrRead, *pStderr);

                Sleep(0);

            } while (IsProcessAlive(procInfo.hProcess));

            if (pStdout) ReadPipe(hStdOutRead, *pStdout);
            if (pStderr) ReadPipe(hStdErrRead, *pStderr);
        }
        else {
            ::WaitForSingleObject(procInfo.hProcess, INFINITE);
        }

        if (pReturnCode) {
            ::DWORD exitCode = 0;
            Verify(::GetExitCodeProcess(procInfo.hProcess, (::LPDWORD)&exitCode));
            *pReturnCode = int(exitCode);
        }

        ::CloseHandle(procInfo.hProcess);
        ::CloseHandle(procInfo.hThread);

        succeed = true;
    }
    else
    {
        const ::DWORD errorCode = ::GetLastError();

        // if CreateProcess failed, we should return a useful error code, which GetLastError will have
        if (pReturnCode)
            *pReturnCode = int(errorCode);

        LOG(HAL, Warning, L"CreateProc failed : {0}", FLastError(errorCode));
        if (errorCode == ERROR_NOT_ENOUGH_MEMORY || errorCode == ERROR_OUTOFMEMORY) {
            // These errors are common enough that we want some available memory information
            const FPlatformMemory::FStats stats = FPlatformMemory::Stats();
            LOG(HAL, Warning, TEXT("Mem used: {0}, OS Free {1}"),
                Fmt::SizeInBytes(stats.UsedPhysical),
                Fmt::SizeInBytes(stats.AvailablePhysical) );
        }

        LOG(HAL, Warning, L"URL: {0}", commandLine);
    }

    if (startupInfoEx.StartupInfo.hStdOutput != NULL)
        ::CloseHandle(startupInfoEx.StartupInfo.hStdOutput);

    if (startupInfoEx.StartupInfo.hStdError != NULL)
        ::CloseHandle(startupInfoEx.StartupInfo.hStdError);

    if (hStdOutRead != NULL)
        ::CloseHandle(hStdOutRead);
    if (hStdErrRead != NULL)
        ::CloseHandle(hStdErrRead);

    if (startupInfoEx.lpAttributeList != NULL)
        ::DeleteProcThreadAttributeList(startupInfoEx.lpAttributeList);

    return succeed;
}
PRAGMA_MSVC_WARNING_POP()
//----------------------------------------------------------------------------
bool FWindowsPlatformProcess::ExecElevatedProcess(
    int* pReturnCode,
    const FWStringView& url,
    const TMemoryView<const FWStringView>& args ) {

    const FWString urlNullTerminated(url);
    const FWString argsNullTerminated = ToWString(
        Fmt::Join(
            args.Map([](const FWStringView& arg) { return Fmt::Quoted(arg, L'"'); }),
            L' ') );

    ::SHELLEXECUTEINFOW shellExecuteInfo;
    ::ZeroMemory(&shellExecuteInfo, sizeof(shellExecuteInfo));
    shellExecuteInfo.cbSize = sizeof(shellExecuteInfo);
    shellExecuteInfo.fMask = SEE_MASK_UNICODE | SEE_MASK_NOCLOSEPROCESS;
    shellExecuteInfo.lpFile = urlNullTerminated.data();
    shellExecuteInfo.lpVerb = L"runas";
    shellExecuteInfo.nShow = SW_SHOW;
    shellExecuteInfo.lpParameters = argsNullTerminated.data();

    bool succeed = false;
    if (::ShellExecuteEx(&shellExecuteInfo))
    {
        ::WaitForSingleObject(shellExecuteInfo.hProcess, INFINITE);
        if (pReturnCode) {
            ::DWORD exitCode = 0;
            Verify(::GetExitCodeProcess(shellExecuteInfo.hProcess, &exitCode));
            *pReturnCode = int(exitCode);
        }

        Verify(::CloseHandle(shellExecuteInfo.hProcess));

        succeed = true;
    }

    return succeed;
}

//----------------------------------------------------------------------------
bool FWindowsPlatformProcess::ExecDetachedProcess(const FWStringView& commandLine) {
    Assert(not commandLine.empty());

    // initialize process attributes
    ::SECURITY_ATTRIBUTES security;
    security.nLength = sizeof(::SECURITY_ATTRIBUTES);
    security.lpSecurityDescriptor = NULL;
    security.bInheritHandle = true;

    // initialize process creation flags
    ::DWORD createFlags = NORMAL_PRIORITY_CLASS | DETACHED_PROCESS;

    // initialize window flags
    ::DWORD dwFlags = 0;
    ::WORD showWindowFlags = SW_HIDE;

    // initialize startup info
    ::STARTUPINFO startupInfo = {
        sizeof(::STARTUPINFO),
        NULL, NULL, NULL,
        (::DWORD)CW_USEDEFAULT,
        (::DWORD)CW_USEDEFAULT,
        (::DWORD)CW_USEDEFAULT,
        (::DWORD)CW_USEDEFAULT,
        (::DWORD)0, (::DWORD)0, (::DWORD)0,
        (::DWORD)dwFlags,
        showWindowFlags,
        0, NULL, NULL, NULL, NULL
    };

    // create the child process
    ::PROCESS_INFORMATION procInfo;
    if (not ::CreateProcessW(
        NULL,
        (::LPWSTR)commandLine.data(),
        &security, &security, TRUE,
        (::DWORD)createFlags, NULL,
        NULL,
        &startupInfo,
        &procInfo) ) {
        const ::DWORD errorCode = ::GetLastError();

        LOG(HAL, Warning, L"CreateProcess failed: {0}", FLastError(errorCode));
        if (errorCode == ERROR_NOT_ENOUGH_MEMORY || errorCode == ERROR_OUTOFMEMORY) {
            // These errors are common enough that we want some available memory information
            const FPlatformMemory::FStats stats = FPlatformMemory::Stats();
            LOG(HAL, Warning, TEXT("Mem used: {0}, OS Free {1}"),
                Fmt::SizeInBytes(stats.UsedPhysical),
                Fmt::SizeInBytes(stats.AvailablePhysical));
        }

        LOG(HAL, Warning, L"URL: {0}", commandLine);

        return false;
    }

    ::CloseHandle(procInfo.hThread);
    ::CloseHandle(procInfo.hProcess);

    return true;
}
//----------------------------------------------------------------------------
bool FWindowsPlatformProcess::OpenURL(const FWStringView& url) {
    Assert(not url.empty());

    return ((StartsWithI(url, L"http://") || StartsWithI(url, L"https://"))
        ? OpenWebURL_(url)
        : OpenWithDefaultApp(url) );
}
//----------------------------------------------------------------------------
bool FWindowsPlatformProcess::OpenWithDefaultApp(const FWStringView& filename) {
    Assert(not filename.empty());

    // ShellExecute will open the default handler for a URL
    FWString urlNullTerminated(filename);
    const ::HINSTANCE code = ::ShellExecuteW(NULL, L"open", urlNullTerminated.data(), NULL, NULL, SW_SHOWNORMAL);
    if (intptr_t(code) <= 32) { // https://docs.microsoft.com/en-us/windows/desktop/api/shellapi/nf-shellapi-shellexecutea
        LOG(HAL, Error, L"failed open with default app : {0}", filename);
        return false;
    }
    else {
        return true;
    }
}
//----------------------------------------------------------------------------
bool FWindowsPlatformProcess::EditWithDefaultApp(const FWStringView& filename) {
    Assert(not filename.empty());

    // ShellExecute will open the default handler for a URL
    FWString urlNullTerminated(filename);
    const ::HINSTANCE code = ::ShellExecuteW(NULL, L"edit", urlNullTerminated.data(), NULL, NULL, SW_SHOWNORMAL);
    if (intptr_t(code) <= 32) { // https://docs.microsoft.com/en-us/windows/desktop/api/shellapi/nf-shellapi-shellexecutea
        LOG(HAL, Error, L"failed edit with default app : {0}", filename);
        return false;
    }
    else {
        return true;
    }
}
//----------------------------------------------------------------------------
auto FWindowsPlatformProcess::CreateSemaphore(const char* name, bool create, size_t maxLocks) -> FSemaphore {
    Assert(name);

    ::HANDLE semaphore = NULL;

    if (create) {
        semaphore = ::CreateSemaphoreA(NULL, checked_cast<long>(maxLocks), checked_cast<long>(maxLocks), name);
        if (NULL == semaphore) {
            LOG(HAL, Error, L"CreateSemaphore(Attrs=NULL, InitialValue={0}, MaxValue={0}, Name='{1}') failed with = {2}",
                maxLocks, MakeCStringView(name), FLastError() );

            return NULL;
        }
    }
    else
    {
        ::DWORD accessRights = SYNCHRONIZE | SEMAPHORE_MODIFY_STATE;
        semaphore = ::OpenSemaphoreA(accessRights, FALSE, name);
        if (NULL == semaphore) {
            LOG(HAL, Error, L"OpenSemaphore(AccessRights={0:#8x}, bInherit=false, Name='{1}') failed with = {2}",
                accessRights, MakeCStringView(name), FLastError() );

            return NULL;
        }
    }

    Assert(semaphore);
    return semaphore;
}
//----------------------------------------------------------------------------
void FWindowsPlatformProcess::LockSemaphore(FSemaphore semaphore) {
    Assert(semaphore);

    ::DWORD waitResult = ::WaitForSingleObject(semaphore, INFINITE);

    if (waitResult != WAIT_OBJECT_0) {
        LOG(HAL, Error, L"WaitForSingleObject(,INFINITE) for semaphore failed with return code {0:#8x} and = {1}",
            waitResult, FLastError() );
    }
}
//----------------------------------------------------------------------------
bool FWindowsPlatformProcess::TryLockSemaphore(FSemaphore semaphore, u64 nanoSeconds) {
    Assert(semaphore);

    ::DWORD millisecondsToWait = checked_cast<::DWORD>(nanoSeconds / 1000000ULL);
    ::DWORD waitResult = ::WaitForSingleObject(semaphore, millisecondsToWait);

    if (waitResult != WAIT_OBJECT_0 && waitResult != WAIT_TIMEOUT) { // timeout is not a warning
        LOG(HAL, Error, L"WaitForSingleObject(,INFINITE) for semaphore failed with return code {0:#8x} and = {1}",
            waitResult, FLastError() );
    }

    return (waitResult == WAIT_OBJECT_0);
}
//----------------------------------------------------------------------------
void FWindowsPlatformProcess::UnlockSemaphore(FSemaphore semaphore) {
    Assert(semaphore);

    if (not ::ReleaseSemaphore(semaphore, 1, NULL)) {
        LOG(HAL, Error, L"ReleaseSemaphore(,ReleaseCount=1,) for semaphore failed with = {0}",
            FLastError() );
    }
}
//----------------------------------------------------------------------------
bool FWindowsPlatformProcess::DestroySemaphore(FSemaphore semaphore) {
    if (NULL == semaphore)
        return false;

    if (::CloseHandle(semaphore) == TRUE) {
        return true;
    }
    else {
        LOG(HAL, Error, L"CloseHandle() for semaphore failed with = {0}", FLastError());
        return false;
    }
}
//----------------------------------------------------------------------------
auto FWindowsPlatformProcess::AttachToDynamicLibrary(const wchar_t* name) -> FDynamicLibraryHandle {
    Assert(name);

    return ::GetModuleHandleW(name);
}
//----------------------------------------------------------------------------
void FWindowsPlatformProcess::DetachFromDynamicLibrary(FDynamicLibraryHandle lib) {
    Assert(lib);

    //Verify(::FreeLibrary(lib)); // DO NOT CALL ::FreeLibrary() or the module could be prematurely unloaded !
}
//----------------------------------------------------------------------------
auto FWindowsPlatformProcess::OpenDynamicLibrary(const wchar_t* name) -> FDynamicLibraryHandle {
    Assert(name);

    return ::LoadLibraryW(name);
}
//----------------------------------------------------------------------------
void FWindowsPlatformProcess::CloseDynamicLibrary(FDynamicLibraryHandle lib) {
    Assert(lib);

    Verify(::FreeLibrary(lib));
}
//----------------------------------------------------------------------------
FWString FWindowsPlatformProcess::DynamicLibraryFilename(FDynamicLibraryHandle lib) {
    Assert(lib);

    wchar_t buffer[MAX_PATH + 1];
    const ::DWORD len = ::GetModuleFileNameW(lib, buffer, MAX_PATH);
    CLOG(len == 0, HAL, Fatal, L"GetModuleFileName({0}) failed with : {1}", (void*)lib, FLastError());

    return FWString(buffer, checked_cast<size_t>(len));
}
//----------------------------------------------------------------------------
void* FWindowsPlatformProcess::DynamicLibraryFunction(FDynamicLibraryHandle lib, const char* name) {
    Assert(lib);
    Assert(name);

    return (void*)::GetProcAddress(lib, name);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_WINDOWS
