#include "stdafx.h"

#include "HAL/Windows/WindowsPlatformProcess.h"

#ifdef PLATFORM_WINDOWS

#include "Diagnostic/Logger.h"
#include "Container/Vector.h"
#include "IO/Format.h"
#include "IO/FormatHelpers.h"
#include "IO/String.h"
#include "IO/StringBuilder.h"
#include "IO/StringView.h"
#include "IO/TextWriter.h"
#include "Memory/MemoryProvider.h"
#include "Meta/Utility.h"

#include "HAL/PlatformMemory.h"
#include "HAL/PlatformMisc.h"

#include "HAL/Windows/DbgHelpWrapper.h"
#include "HAL/Windows/VSPerfWrapper.h"
#include "HAL/Windows/VSToolsWrapper.h"
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
        if (FWindowsPlatformProcess::ExecDetachedProcess(browserOpenCommand.data(), nullptr, nullptr))
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
// CreateProcessW() can reuse given command line buffer internally
// https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-createprocessw
struct FWindowsCommandLine_ {
    wchar_t Data[32768];

    FWindowsCommandLine_(const wchar_t* executable, const wchar_t* parameters) {
        FMemoryViewWriter writer(Data);
        FWTextWriter oss(&writer);

        FWStringView exe{ MakeCStringView(executable) };
        if (HasSpace(exe) && (exe.front() != L'"') && (exe.front() != L'\''))
            oss << Fmt::DoubleQuote << exe << Fmt::DoubleQuote;
        else
            oss << exe;

        if (parameters)
            oss << Fmt::Space << MakeCStringView(parameters);

        oss << Eos;
    }

    operator wchar_t* () NOEXCEPT {
        return Data;
    }
};
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

#if USE_PPE_VSTOOLS_WRAPPER
    FVSToolsWrapper::Create();
#endif
#if USE_PPE_PLATFORM_PROFILER
    FVSPerfWrapper::Create();
#endif

    FDbghelpWrapper::Create();
}
//----------------------------------------------------------------------------
void FWindowsPlatformProcess::OnProcessShutdown() {
    FDbghelpWrapper::Destroy();

#if USE_PPE_PLATFORM_PROFILER
    FVSPerfWrapper::Destroy();
#endif
#if USE_PPE_VSTOOLS_WRAPPER
    FVSToolsWrapper::Destroy();
#endif

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
auto FWindowsPlatformProcess::CurrentProcess() -> FProcessHandle {
    return ::GetCurrentProcess();
}
//----------------------------------------------------------------------------
bool FWindowsPlatformProcess::EnableDebugPrivileges() {
    ::HANDLE hToken;
    ::LUID luid;
    ::TOKEN_PRIVILEGES tkp;

    if (not ::OpenProcessToken(::GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
        LOG_LASTERROR(HAL, L"OpenProcessToken");
        return false;
    }

    ON_SCOPE_EXIT([&]() {
        if (not ::CloseHandle(hToken)) {
            LOG_LASTERROR(HAL, L"CloseHandle(hToken)");
            AssertNotReached();
        }
    });

    if (not ::LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid)) {
        LOG_LASTERROR(HAL, L"LookupPrivilegeValue");
        return false;
    }

    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Luid = luid;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (not ::AdjustTokenPrivileges(hToken, false, &tkp, sizeof(tkp), NULL, NULL)) {
        LOG_LASTERROR(HAL, L"AdjustTokenPrivileges");
        return false;
    }

    return true;
}
//----------------------------------------------------------------------------
void FWindowsPlatformProcess::Daemonize() {
    // #TODO
    AssertNotImplemented();
}
//----------------------------------------------------------------------------
bool FWindowsPlatformProcess::IsForeground() {
    const ::HWND hWnd = GetForegroundWindow();
    if (NULL == hWnd)
        return false;

    ::DWORD dwPid;
    if (::GetWindowThreadProcessId(hWnd, &dwPid)) {
        return (::GetCurrentProcessId() == dwPid);
    }
    else {
        LOG_LASTERROR(HAL, L"GetWindowThreadProcessId");
        return false;
    }
}
//----------------------------------------------------------------------------
bool FWindowsPlatformProcess::IsFirstInstance() {
    return GIsFirstInstance;
}
//----------------------------------------------------------------------------
bool FWindowsPlatformProcess::IsProcessAlive(FProcessHandle process) {
    Assert(process);

    const ::DWORD waitResult = ::WaitForSingleObject(process, 0);
    return (waitResult == WAIT_TIMEOUT);
}
//----------------------------------------------------------------------------
auto FWindowsPlatformProcess::OpenProcess(FProcessId pid, bool fullAccess/* = true */) -> FProcessHandle {
    const ::HANDLE hProc = ::OpenProcess(
        (fullAccess
            ? PROCESS_ALL_ACCESS
            : PROCESS_QUERY_INFORMATION ),
        FALSE, pid );

    if (NULL == hProc)
        LOG_LASTERROR(HAL, L"OpenProcess");

    return hProc;
}
//----------------------------------------------------------------------------
void FWindowsPlatformProcess::WaitForProcess(FProcessHandle process) {
    Assert(process);

    VerifyRelease(WAIT_OBJECT_0 == ::WaitForSingleObject(process, INFINITE));
}
//----------------------------------------------------------------------------
NODISCARD bool FWindowsPlatformProcess::WaitForProcess(FProcessHandle process, size_t timeoutMs) {
    Assert(process);

    const ::DWORD waitResult = ::WaitForSingleObject(process, checked_cast<::DWORD>(timeoutMs));

    switch (waitResult) {
    case WAIT_OBJECT_0:
        return true;

    case WAIT_FAILED:
        LOG_LASTERROR(HAL, L"WaitForSingleObject");
    case WAIT_ABANDONED:
    case WAIT_TIMEOUT:
        return false;

    default:
        AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
void FWindowsPlatformProcess::CloseProcess(FProcessHandle process) {
    Assert(process);

    if (not ::CloseHandle(process))
        LOG_LASTERROR(HAL, L"CloseHandle");
}
//----------------------------------------------------------------------------
void FWindowsPlatformProcess::TerminateProcess(FProcessHandle process, bool killTree) {
    Assert(process);

    if (killTree) {
        const ::HANDLE snapShot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

        if (snapShot != INVALID_HANDLE_VALUE) {
            ON_SCOPE_EXIT([&]() {
                if (not ::CloseHandle(snapShot)) {
                    LOG_LASTERROR(HAL, L"CloseHandle(snapShot)");
                    AssertNotReached();
                }
            });

            const ::DWORD processId = ::GetProcessId(process);

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
        else {
            LOG_LASTERROR(HAL, L"CreateToolhelp32Snapshot");
        }
    }

    if (not ::TerminateProcess(process, 0))
        LOG_LASTERROR(HAL, L"TerminateProcess");
}
//------------------------------------------------------------------------
bool FWindowsPlatformProcess::FindByName(FProcessId* pPid, const FWStringView& name) {
    Assert(pPid);
    Assert(not name.empty());

    const ::HANDLE snapShot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (snapShot != INVALID_HANDLE_VALUE) {
        ON_SCOPE_EXIT([&]() {
            if (not ::CloseHandle(snapShot)) {
                LOG_LASTERROR(HAL, L"CloseHandle(snapShot)");
                AssertNotReached();
            }
        });

        ::PROCESSENTRY32 entry;
        entry.dwSize = sizeof(::PROCESSENTRY32);

        if (::Process32FirstW(snapShot, &entry)) {
            do {
                const FWStringView processName{ MakeCStringView(entry.szExeFile) };
                if (EqualsI(processName, name)) {
                    *pPid = checked_cast<FProcessId>(entry.th32ProcessID);
                    return true;
                }
            } while (::Process32NextW(snapShot, &entry));
        }
    }
    else {
        LOG_LASTERROR(HAL, L"CreateToolhelp32Snapshot");
    }

    *pPid = 0;
    return false;
}
//------------------------------------------------------------------------
// process infos
//----------------------------------------------------------------------------
bool FWindowsPlatformProcess::ExitCode(int* pExitCode, FProcessHandle process) {
    Assert(pExitCode);

    ::DWORD dwExitCode;
    if (::GetExitCodeProcess(process, &dwExitCode)) {
        *pExitCode = checked_cast<int>(dwExitCode);
        return true;
    }
    else {
        LOG_LASTERROR(HAL, L"GetExitCodeProcess");
        return false;
    }
}
//----------------------------------------------------------------------------
bool FWindowsPlatformProcess::MemoryStats(FMemoryStats* pStats, FProcessHandle process) {
    Assert(pStats);

    ::PROCESS_MEMORY_COUNTERS counters;
    ::ZeroMemory(&counters, sizeof(counters));
    if (::GetProcessMemoryInfo(process, &counters, sizeof(counters))) {
        ::ZeroMemory(pStats, sizeof(*pStats));

        pStats->UsedPhysical = checked_cast<u64>(counters.WorkingSetSize);
        pStats->UsedVirtual = checked_cast<u64>(counters.PagefileUsage);
        pStats->PeakUsedPhysical = checked_cast<u64>(counters.PeakWorkingSetSize);
        pStats->PeakUsedVirtual = checked_cast<u64>(counters.PeakPagefileUsage);

        return true;
    }
    else {
        LOG_LASTERROR(HAL, L"GetProcessMemoryInfo");
        return false;
    }
}
//----------------------------------------------------------------------------
bool FWindowsPlatformProcess::Name(FString* pName, FProcessHandle process) {
    Assert(pName);

    char buffer[MAX_PATH + 1];
    ::DWORD len = lengthof(buffer);

#if WINVER == 0x0502
    if ((len = ::GetProcessImageFileNameA(process, buffer, len)) != 0) {
        pName->assign(buffer, buffer + len);
        return true;
    }
    else {
        LOG_LASTERROR(HAL, L"GetProcessImageFileNameA");
        return false;
    }
#else
    if (::QueryFullProcessImageNameA(process, 0, buffer, &len)) {
        pName->assign(buffer, buffer + len);
        return true;
    }
    else {
        LOG_LASTERROR(HAL, L"QueryFullProcessImageNameA");
        return false;
    }
#endif
}
//----------------------------------------------------------------------------
bool FWindowsPlatformProcess::Pid(FProcessId* pPid, FProcessHandle process) {
    Assert(pPid);

    return checked_cast<FProcessId>(::GetProcessId(process));
}
//----------------------------------------------------------------------------
bool FWindowsPlatformProcess::Priority(EProcessPriority* pPriority, FProcessHandle process) {
    Assert(pPriority);

    switch (::GetPriorityClass(process)) {
    case REALTIME_PRIORITY_CLASS:
        *pPriority = EProcessPriority::Realtime;
        return true;
    case HIGH_PRIORITY_CLASS:
        *pPriority = EProcessPriority::High;
        return true;
    case ABOVE_NORMAL_PRIORITY_CLASS:
        *pPriority = EProcessPriority::AboveNormal;
        return true;
    case NORMAL_PRIORITY_CLASS:
        *pPriority = EProcessPriority::Normal;
        return true;
    case BELOW_NORMAL_PRIORITY_CLASS:
        *pPriority = EProcessPriority::BelowNormal;
        return true;
    case IDLE_PRIORITY_CLASS:
        *pPriority = EProcessPriority::Idle;
        return true;

    default:
        LOG_LASTERROR(HAL, L"GetPriorityClass");
        return false;
    }
}
//----------------------------------------------------------------------------
bool FWindowsPlatformProcess::SetPriority(FProcessHandle process, EProcessPriority priority) {
    ::DWORD dPriorityClass = 0;

    switch (priority) {
    case PPE::EProcessPriority::Realtime:
        dPriorityClass = REALTIME_PRIORITY_CLASS; break;
    case PPE::EProcessPriority::High:
        dPriorityClass = HIGH_PRIORITY_CLASS; break;
    case PPE::EProcessPriority::AboveNormal:
        dPriorityClass = ABOVE_NORMAL_PRIORITY_CLASS; break;
    case PPE::EProcessPriority::Normal:
        dPriorityClass = NORMAL_PRIORITY_CLASS; break;
    case PPE::EProcessPriority::BelowNormal:
        dPriorityClass = BELOW_NORMAL_PRIORITY_CLASS; break;
    case PPE::EProcessPriority::Idle:
        dPriorityClass = IDLE_PRIORITY_CLASS; break;

    default:
        AssertNotImplemented();
    }

    if (::SetPriorityClass(process, dPriorityClass)) {
        return true;
    }
    else {
        LOG_LASTERROR(HAL, L"SetPriorityClass");
        return false;
    }
}
//----------------------------------------------------------------------------
// Use a constref to avoid issues with static variables initialization order !
const FWindowsPlatformProcess::FAffinityMask& FWindowsPlatformProcess::AllCoresAffinity =
    FWindowsPlatformThread::AllCoresAffinity;
//----------------------------------------------------------------------------
auto FWindowsPlatformProcess::AffinityMask(FProcessHandle process) -> FAffinityMask {
    ::DWORD_PTR dwProcessAffinityMask, dwSystemAffinityMask;
    if (::GetProcessAffinityMask(process, &dwProcessAffinityMask, &dwSystemAffinityMask)) {
        return checked_cast<FAffinityMask>(dwProcessAffinityMask);
    }
    else {
        LOG_LASTERROR(HAL, L"GetProcessAffinityMask");
        return FAffinityMask{ 0 };
    }
}
//----------------------------------------------------------------------------
bool FWindowsPlatformProcess::SetAffinityMask(FProcessHandle process, FAffinityMask mask) {
    ::DWORD_PTR dwProcessAffinityMask = checked_cast<::DWORD_PTR>(mask);
    if (::SetProcessAffinityMask(process, dwProcessAffinityMask)) {
        return true;
    }
    else {
        LOG_LASTERROR(HAL, L"SetProcessAffinityMask");
        return false;
    }
}
//----------------------------------------------------------------------------
// Pipes
//----------------------------------------------------------------------------
bool FWindowsPlatformProcess::IsPipeBlocked(FPipeHandle pipe) {
    ::DWORD dwFlags;
    return (not ::GetHandleInformation(pipe, &dwFlags));
}
//----------------------------------------------------------------------------
bool FWindowsPlatformProcess::CreatePipe(FPipeHandle* pRead, FPipeHandle* pWrite, bool shareRead) {
    Assert(pRead);
    Assert(pWrite);

    ::SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(saAttr);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    if (not ::CreatePipe(pRead, pWrite, &saAttr, 0)) {
        LOG_LASTERROR(HAL, L"CreatePipe");
        return false;
    }

    // Need to duplicate the non shared handle for 2 reasons:
    //  1) disable inheritance of the non-shared handle
    //  2) can close the shared handle without closing the non-shared side
    const ::HANDLE hProc = ::GetCurrentProcess();
    if (shareRead) {
        if (not ::DuplicateHandle(hProc, *pWrite, hProc, pWrite, 0, FALSE, DUPLICATE_SAME_ACCESS)) {
            LOG_LASTERROR(HAL, L"DuplicateHandle");
            return false;
        }
    }
    else {
        if (not ::DuplicateHandle(hProc, *pRead, hProc, pRead, 0, FALSE, DUPLICATE_SAME_ACCESS)) {
            LOG_LASTERROR(HAL, L"DuplicateHandle");
            return false;
        }
    }

    return true;
}
//----------------------------------------------------------------------------
size_t FWindowsPlatformProcess::PeekPipe(FPipeHandle read) {
    Assert(read);

    ::DWORD bytesAvailable = 0;
    if (::PeekNamedPipe(read, NULL, 0, NULL, &bytesAvailable, NULL)) {
        return checked_cast<size_t>(bytesAvailable);
    }
    else {
        //LOG_LASTERROR(HAL, L"PeekNamedPipe"); too much log generated ^^;
        return 0;
    }
}
//----------------------------------------------------------------------------
size_t FWindowsPlatformProcess::ReadPipe(FPipeHandle read, const FRawMemory& buffer) {
    Assert(read);

    ::DWORD bytesRead = 0;
    const ::DWORD bytesToRead = checked_cast<::DWORD>(buffer.SizeInBytes());
    if (::ReadFile(read, buffer.Pointer(), bytesToRead, &bytesRead, NULL)) {
        return checked_cast<size_t>(bytesRead);
    }
    else {
        Assert(bytesRead == 0);
        //LOG_LASTERROR(HAL, L"ReadFile"); // too much verbose
        return 0;
    }
}
//----------------------------------------------------------------------------
size_t FWindowsPlatformProcess::WritePipe(FPipeHandle write, const FRawMemoryConst& buffer) {
    Assert(write);

    // If there is not a message or WritePipe is null
    if (buffer.empty())
        return 0;

    // Write to pipe
    ::DWORD bytesWritten = 0;
    const ::DWORD bytesToWrite = checked_cast<::DWORD>(buffer.SizeInBytes());

    if (::WriteFile(write, buffer.data(), bytesToWrite, &bytesWritten, NULL)) {
        return checked_cast<size_t>(bytesWritten);
    }
    else {
        Assert(bytesWritten == 0);
        LOG_LASTERROR(HAL, L"WriteFile");
        return 0;
    }
}
//----------------------------------------------------------------------------
void FWindowsPlatformProcess::ClosePipe(FPipeHandle read, FPipeHandle write) {
    if (((read != NULL) & (read != INVALID_HANDLE_VALUE)) && not ::CloseHandle(read))
        LOG_LASTERROR(HAL, L"CloseHandle(read)");

    if (((write != NULL) & (write != INVALID_HANDLE_VALUE)) && not ::CloseHandle(write))
        LOG_LASTERROR(HAL, L"CloseHandle(write)");
}
//----------------------------------------------------------------------------
// Spawn process
//----------------------------------------------------------------------------
auto FWindowsPlatformProcess::CreateProcess(
    FProcessId* pPID,
    const wchar_t* executable,
    const wchar_t* parameters,
    const wchar_t* workingDir,
    bool detached, bool hidden, bool inheritHandles, bool noWindow,
    EProcessPriority priority,
    FPipeHandle hStdin/* = nullptr */,
    FPipeHandle hStderr/* = nullptr */,
    FPipeHandle hStdout/* = nullptr */) -> FProcessHandle {
    Assert(pPID);
    Assert(executable);

#if USE_PPE_DEBUG
    for (FPipeHandle hPipe : { hStdin, hStderr, hStdout }) {
        if (hPipe) {
            ::DWORD dwFlags;
            Verify(::GetHandleInformation(hPipe, &dwFlags));
            Assert(HANDLE_FLAG_INHERIT & dwFlags);
        }
    }
#endif

    // initialize window flags
    ::DWORD dwFlags = 0;
    ::WORD wShowWindow = SW_HIDE;

    if (!!hStdin | !!hStdout | !!hStderr)
        dwFlags |= STARTF_USESTDHANDLES;
    if (noWindow || hidden)
        dwFlags |= STARTF_USESHOWWINDOW;
    if (hidden)
        wShowWindow = SW_SHOWMINNOACTIVE;

    // initialize startup info
    ::STARTUPINFOW startupInfo;
    ::ZeroMemory(&startupInfo, sizeof(startupInfo));
    startupInfo.cb = (::DWORD)sizeof(startupInfo);
    startupInfo.dwX = (::DWORD)CW_USEDEFAULT;
    startupInfo.dwY = (::DWORD)CW_USEDEFAULT;
    startupInfo.dwXSize = (::DWORD)CW_USEDEFAULT;
    startupInfo.dwYSize = (::DWORD)CW_USEDEFAULT;
    startupInfo.dwFlags = dwFlags;
    startupInfo.wShowWindow = wShowWindow;
    startupInfo.hStdInput = (hStdin ? hStdin : (detached ? nullptr : ::GetStdHandle(STD_INPUT_HANDLE)) );
    startupInfo.hStdError = hStderr;
    startupInfo.hStdOutput = hStdout;

    // initialize process creation flags
    ::DWORD dwCreationFlags;
    switch (priority) {
    case PPE::EProcessPriority::Realtime:
        dwCreationFlags = REALTIME_PRIORITY_CLASS; break;
    case PPE::EProcessPriority::High:
        dwCreationFlags = HIGH_PRIORITY_CLASS; break;
    case PPE::EProcessPriority::AboveNormal:
        dwCreationFlags = ABOVE_NORMAL_PRIORITY_CLASS; break;
    case PPE::EProcessPriority::Normal:
        dwCreationFlags = NORMAL_PRIORITY_CLASS; break;
    case PPE::EProcessPriority::BelowNormal:
        dwCreationFlags = BELOW_NORMAL_PRIORITY_CLASS; break;
    case PPE::EProcessPriority::Idle:
        dwCreationFlags = IDLE_PRIORITY_CLASS; break;

    default:
        AssertNotImplemented();
    }

    if (detached)
        dwCreationFlags |= DETACHED_PROCESS;

    // create the child process
    ::PROCESS_INFORMATION procInfo;
    ::ZeroMemory(&procInfo, sizeof(procInfo));

    // Prepare executable path and parameters
    FWindowsCommandLine_ commandLine{ executable, parameters };

    if (not ::CreateProcessW(
        NULL, // don't specify module name, use command-line instead
        commandLine,
        NULL, // process security attributes
        NULL, // primary thread security attributes
        inheritHandles,
        dwCreationFlags,
        NULL, // use parent's environment
        workingDir,
        &startupInfo, &procInfo )) {
        const ::DWORD errorCode = ::GetLastError();

        LOG(HAL, Error, L"CreateProcess failed: {0}\n\turl: {1}\n\t{2}",
            FLastError(errorCode), MakeCStringView(commandLine.Data), MakeCStringView(workingDir) );

        if (errorCode == ERROR_NOT_ENOUGH_MEMORY || errorCode == ERROR_OUTOFMEMORY) {
            // These errors are common enough that we want some available memory information
            const FPlatformMemory::FStats stats = FPlatformMemory::Stats();
            LOG(HAL, Warning, TEXT("Mem used: {0}, OS Free {1}"),
                Fmt::SizeInBytes(stats.UsedPhysical),
                Fmt::SizeInBytes(stats.AvailablePhysical) );
        }
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
bool FWindowsPlatformProcess::ExecDetachedProcess(
    const wchar_t* executable,
    const wchar_t* parameters,
    const wchar_t* workingDir) {
    Assert(executable);

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

    // Prepare executable path and parameters
    FWindowsCommandLine_ commandLine{ executable, parameters };

    // create the child process
    ::PROCESS_INFORMATION procInfo;
    if (not ::CreateProcessW(
        NULL,
        commandLine,
        &security, &security,
        TRUE, // inherit parent's handles
        (::DWORD)createFlags,
        NULL, // use parent's environment
        workingDir,
        &startupInfo, &procInfo)) {
        const ::DWORD errorCode = ::GetLastError();

        LOG(HAL, Error, L"CreateProcess failed: {0}\n\turl: {1}\n\t{2}",
            FLastError(errorCode), MakeCStringView(commandLine.Data), MakeCStringView(workingDir) );

        if (errorCode == ERROR_NOT_ENOUGH_MEMORY || errorCode == ERROR_OUTOFMEMORY) {
            // These errors are common enough that we want some available memory information
            const FPlatformMemory::FStats stats = FPlatformMemory::Stats();
            LOG(HAL, Warning, TEXT("Mem used: {0}, OS Free {1}"),
                Fmt::SizeInBytes(stats.UsedPhysical),
                Fmt::SizeInBytes(stats.AvailablePhysical));
        }

        return false;
    }

    ::CloseHandle(procInfo.hThread);
    ::CloseHandle(procInfo.hProcess);

    return true;
}
//----------------------------------------------------------------------------
bool FWindowsPlatformProcess::ExecElevatedProcess(
    int* pReturnCode,
    const wchar_t* executable,
    const wchar_t* parameters,
    const wchar_t* workingDir ) {
    Assert(executable);

    ::SHELLEXECUTEINFOW shellExecuteInfo;
    ::ZeroMemory(&shellExecuteInfo, sizeof(shellExecuteInfo));
    shellExecuteInfo.cbSize = sizeof(shellExecuteInfo);
    shellExecuteInfo.fMask = SEE_MASK_UNICODE | SEE_MASK_NOCLOSEPROCESS;
    shellExecuteInfo.lpFile = executable;
    shellExecuteInfo.lpVerb = L"runas";
    shellExecuteInfo.nShow = SW_SHOW;
    shellExecuteInfo.lpParameters = parameters;
    shellExecuteInfo.lpDirectory = workingDir;

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
bool FWindowsPlatformProcess::OpenURL(const wchar_t* url) {
    Assert(url);

    const FWStringView s{ MakeCStringView(url) };
    return ((StartsWithI(s, L"http://") || StartsWithI(s, L"https://"))
        ? OpenWebURL_(s)
        : OpenWithDefaultApp(url) );
}
//----------------------------------------------------------------------------
bool FWindowsPlatformProcess::OpenWithDefaultApp(const wchar_t* filename) {
    Assert(filename);

    // ShellExecute will open the default handler for a URL
    const ::HINSTANCE code = ::ShellExecuteW(NULL, L"open", filename, NULL, NULL, SW_SHOWNORMAL);
    if (intptr_t(code) <= 32) { // https://docs.microsoft.com/en-us/windows/desktop/api/shellapi/nf-shellapi-shellexecutea
        LOG(HAL, Error, L"failed open with default app : {0}", MakeCStringView(filename));
        return false;
    }
    else {
        return true;
    }
}
//----------------------------------------------------------------------------
bool FWindowsPlatformProcess::EditWithDefaultApp(const wchar_t* filename) {
    Assert(filename);

    // ShellExecute will open the default handler for a URL
    const ::HINSTANCE code = ::ShellExecuteW(NULL, L"edit", filename, NULL, NULL, SW_SHOWNORMAL);
    if (intptr_t(code) <= 32) { // https://docs.microsoft.com/en-us/windows/desktop/api/shellapi/nf-shellapi-shellexecutea
        LOG(HAL, Error, L"failed edit with default app : {0}", MakeCStringView(filename));
        return false;
    }
    else {
        return true;
    }
}
//----------------------------------------------------------------------------
// Semaphores
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
// Dynamic libraries (DLL)
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

    const FDynamicLibraryHandle hDll = ::LoadLibraryW(name);
    CLOG(NULL == hDll, HAL, Error, L"LoadLibraryW({0}) failed with : {1}", MakeCStringView(name), FLastError());

    return hDll;
}
//----------------------------------------------------------------------------
void FWindowsPlatformProcess::CloseDynamicLibrary(FDynamicLibraryHandle lib) {
    Assert(lib);

    if (not ::FreeLibrary(lib))
        LOG_LASTERROR(HAL, L"FreeLibrary()");
}
//----------------------------------------------------------------------------
FWString FWindowsPlatformProcess::DynamicLibraryFilename(FDynamicLibraryHandle lib) {
    Assert(lib);

    wchar_t buffer[MAX_PATH + 1];
    const ::DWORD len = ::GetModuleFileNameW(lib, buffer, MAX_PATH);
    CLOG(len == 0, HAL, Error, L"GetModuleFileName({0}) failed with : {1}", (void*)lib, FLastError());

    return FWString(buffer, checked_cast<size_t>(len));
}
//----------------------------------------------------------------------------
void* FWindowsPlatformProcess::DynamicLibraryFunction(FDynamicLibraryHandle lib, const char* name) {
    Assert(lib);
    Assert(name);

    const ::FARPROC fnAddr = ::GetProcAddress(lib, name);
    CLOG(fnAddr == nullptr, HAL, Error, L"GetProcAddress({0}) failed with : {1}", MakeCStringView(name), FLastError());

    return (void*)fnAddr;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_WINDOWS
