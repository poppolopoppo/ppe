// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "HAL/Windows/WindowsPlatformMisc.h"

#ifdef PLATFORM_WINDOWS

#include "Allocator/Alloca.h"
#include "Diagnostic/Logger.h"
#include "HAL/PlatformFile.h"
#include "HAL/PlatformMemory.h"
#include "IO/Format.h"
#include "IO/String.h"
#include "IO/StringBuilder.h"
#include "IO/StringView.h"
#include "Meta/Utility.h"

#include "HAL/Windows/LastError.h"
#include "HAL/Windows/WindowsPlatformIncludes.h"

#include <clocale>
#include <io.h>
#include <fcntl.h>
#include <Lmcons.h>
#include <locale.h>
#include <mbctype.h>
#include <VersionHelpers.h>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
PRAGMA_MSVC_WARNING_PUSH()
PRAGMA_MSVC_WARNING_DISABLE(4191) // unsafe conversion from 'type of expression' to 'type required'
bool IsProcess64Bit_(::HANDLE hProcess) {
    typedef ::BOOL(WINAPI* LPFN_ISWOW64PROCESS)(::HANDLE, ::PBOOL);
    ::HMODULE const hKernel32 = ::GetModuleHandleA("kernel32");
    Assert(hKernel32);

    LPFN_ISWOW64PROCESS const fnIsWow64Process = (LPFN_ISWOW64PROCESS)::GetProcAddress(hKernel32, "IsWow64Process");
    ::BOOL bIsWoW64Process = FALSE;
    if (fnIsWow64Process != NULL) {
        if (fnIsWow64Process(hProcess, &bIsWoW64Process) == 0)
            bIsWoW64Process = FALSE;
    }

    if (bIsWoW64Process) {
        //process is 32 bit, running on 64 bit machine
        return false;
    }
    else {
        ::SYSTEM_INFO sysInfo;
        ::GetSystemInfo(&sysInfo);
        return sysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64;
    }
}
PRAGMA_MSVC_WARNING_POP()
//----------------------------------------------------------------------------
static FWindowsPlatformMisc::FCPUInfo FetchCPUInfo_() {
    FWindowsPlatformMisc::FCPUInfo result;

    i32 args[4];
    ::__cpuid(args, 1);

    *((u32*)&result) = checked_cast<u32>(args[0]);

    return result;
}
//----------------------------------------------------------------------------
static size_t FetchNumCores_() {

    // Get only physical cores
    ::PSYSTEM_LOGICAL_PROCESSOR_INFORMATION cpu = NULL;
    ::DWORD sizeInBytes = 0;

    // Get the size of the buffer to hold processor information.
    ::BOOL result = ::GetLogicalProcessorInformation(cpu, &sizeInBytes);
    Assert(!result&& ::GetLastError() == ERROR_INSUFFICIENT_BUFFER);
    Assert(sizeInBytes > 0);

    // Allocate the buffer to hold the processor info.
    STACKLOCAL_POD_ARRAY(u8, storage, sizeInBytes);
    cpu = ::PSYSTEM_LOGICAL_PROCESSOR_INFORMATION(storage.data());

    // Get the actual information.
    result = ::GetLogicalProcessorInformation(cpu, &sizeInBytes);
    AssertRelease(result);

    // Count physical cores
    size_t numCores = 0;
    const size_t n = size_t(sizeInBytes / sizeof(::SYSTEM_LOGICAL_PROCESSOR_INFORMATION));
    forrange(i, 0, n) {
        const ::SYSTEM_LOGICAL_PROCESSOR_INFORMATION& core = cpu[i];
        if (core.Relationship == RelationProcessorCore)
            numCores++;
    }

    return numCores;
}
//----------------------------------------------------------------------------
static size_t FetchNumCoresWithSMT_() {

    // Get the number of logical processors, including hyper-threaded ones.
    ::SYSTEM_INFO sys;
    ::GetSystemInfo(&sys);

    return checked_cast<size_t>(sys.dwNumberOfProcessors);
}
//----------------------------------------------------------------------------
static BOOL WINAPI ConsoleCtrlHandler_(::DWORD /*Type*/) {

    // make sure as much data is written to disk as possible
    PPE_LOG_FLUSH();

    static bool GIsRequestingExit = false;
    if (not GIsRequestingExit) {
        ::PostQuitMessage(0);
        GIsRequestingExit = true;
    }
    else {
        // User has pressed Ctrl-C twice and we should forcibly terminate the application.
        // ExitProcess would run global destructors, possibly causing assertions.
        ::TerminateProcess(::GetCurrentProcess(), 0);
    }

    return TRUE;
}
//----------------------------------------------------------------------------
template <typename _Char>
static void ClipboardCopy_(::UINT fmt, const TMemoryView<const _Char>& src) {
    const ::HWND hWindow = ::GetActiveWindow();

    if (::OpenClipboard(hWindow)) {
        if (not ::EmptyClipboard())
            PPE_LOG_LASTERROR(HAL, "EmptyClipboard()");

        ::HGLOBAL const hGlobalMem = ::GlobalAlloc(GMEM_MOVEABLE, src.SizeInBytes() + sizeof(_Char));
        if (not hGlobalMem)
            PPE_LOG_LASTERROR(HAL, "GlobalAlloc()");

        void* const pData = ::GlobalLock(hGlobalMem);
        if (pData) {
            FPlatformMemory::Memcpy(pData, src.data(), src.SizeInBytes());
            static_cast<_Char*>(pData)[src.size()] = _Char(0);
        }
        else {
            PPE_LOG_LASTERROR(HAL, "GlobalLock()");
        }

        ::GlobalUnlock(hGlobalMem);

        if (::SetClipboardData(fmt, hGlobalMem) == NULL)
            PPE_LOG(HAL, Fatal, "SetClipboardData failed with error : {0}", FLastError());

        if (not ::CloseClipboard())
            PPE_LOG_LASTERROR(HAL, "CloseClipboard()");
    }
    else {
        PPE_LOG_LASTERROR(HAL, "OpenClipboard()");
    }
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
auto FWindowsPlatformMisc::CPUInfo() -> FCPUInfo {
    static const FCPUInfo GCPUInfo = FetchCPUInfo_();
    return GCPUInfo;
}
//----------------------------------------------------------------------------
FString FWindowsPlatformMisc::CPUBrand() {
    // http://msdn.microsoft.com/en-us/library/vstudio/hskdteyh(v=vs.100).aspx
    i32 CPUInfo[4];
    ::__cpuid(CPUInfo, 0x80000000);

    char brandString[0x40];
    const u32 maxBrandId = CPUInfo[0];
    forrange(reg, 0x80000000, maxBrandId) {
        const SIZE_T CPUInfoSize = sizeof(CPUInfo);
        ::__cpuid(CPUInfo, reg);
        if (0x80000002 == reg)
            FPlatformMemory::Memcpy(brandString, CPUInfo, CPUInfoSize);
        else if (0x80000003 == reg)
            FPlatformMemory::Memcpy(brandString + CPUInfoSize, CPUInfo, CPUInfoSize);
        else if (0x80000004 == reg)
            FPlatformMemory::Memcpy(brandString + CPUInfoSize * 2, CPUInfo, CPUInfoSize);
    }

    return FString(Strip(MakeCStringView(brandString)));
}
//----------------------------------------------------------------------------
FString FWindowsPlatformMisc::CPUVendor() {
    union
    {
        char Buffer[12 + 1];
        struct
        {
            int dw0;
            int dw1;
            int dw2;
        } Dw;
    } vendorResult;

    int args[4];
    ::__cpuid(args, 0);

    vendorResult.Dw.dw0 = args[1];
    vendorResult.Dw.dw1 = args[3];
    vendorResult.Dw.dw2 = args[2];
    vendorResult.Buffer[12] = 0;

    return FString(Strip(MakeCStringView(vendorResult.Buffer)));
}
//----------------------------------------------------------------------------
size_t FWindowsPlatformMisc::NumCores() NOEXCEPT {
    static const size_t GNumCores = FetchNumCores_();
    return GNumCores;
}
//----------------------------------------------------------------------------
size_t FWindowsPlatformMisc::NumCoresWithSMT() NOEXCEPT {
    static const size_t GNumCoresWHyperThreading = FetchNumCoresWithSMT_();
    return GNumCoresWHyperThreading;
}
//----------------------------------------------------------------------------
FString FWindowsPlatformMisc::OSName() {
    FStringLiteral osname;
    if (::IsWindowsXPOrGreater())
        osname = "Windows XP";
    if (::IsWindowsXPSP1OrGreater())
        osname = "Windows XP SP1";
    if (::IsWindowsXPSP2OrGreater())
        osname = "Windows XP SP2";
    if (::IsWindowsXPSP3OrGreater())
        osname = "Windows XP SP3";
    if (::IsWindowsVistaOrGreater())
        osname = "Windows Vista";
    if (::IsWindowsVistaSP1OrGreater())
        osname = "Windows Vista SP1";
    if (::IsWindowsVistaSP2OrGreater())
        osname = "Windows Vista SP2";
    if (::IsWindows7OrGreater())
        osname = "Windows 7";
    if (::IsWindows7SP1OrGreater())
        osname = "Windows 7 SP1";
    if (::IsWindows8OrGreater())
        osname = "Windows 8";
    if (::IsWindows8Point1OrGreater())
        osname = "Windows 8.1";
    if (::IsWindows10OrGreater())
        osname = "Windows 10 or greater";

    const FStringLiteral arch = (Is64bitOperatingSystem()
        ? "64 bit"_literal
        : "32 bit"_literal );

    const FStringLiteral clientOrServer = (::IsWindowsServer()
        ? "Server"_literal
        : "Client"_literal );

    // build version number
    // https://docs.microsoft.com/fr-fr/windows/desktop/SysInfo/getting-the-system-version
    const FWStringLiteral systemDLL = L"kernel32.dll";

    ::DWORD dummy = 0;
    const ::DWORD cbInfo = ::GetFileVersionInfoSizeExW(FILE_VER_GET_NEUTRAL, *systemDLL, &dummy);

    STACKLOCAL_POD_ARRAY(u8, buffer, checked_cast<size_t>(cbInfo));
    ::GetFileVersionInfoExW(FILE_VER_GET_NEUTRAL, *systemDLL, 0, checked_cast<::DWORD>(buffer.size()), buffer.data());

    void *p = nullptr;
    ::UINT size = 0;
    ::VerQueryValueW(buffer.data(), L"\\", &p, &size);

    Assert(size >= sizeof(::VS_FIXEDFILEINFO));
    Assert(p != nullptr);
    auto pFixed = static_cast<const ::VS_FIXEDFILEINFO *>(p);

    return StringFormat("{0} {1} {2} - {3}.{4}.{5}.{6}",
        osname, arch, clientOrServer,
        HIWORD(pFixed->dwFileVersionMS),
        LOWORD(pFixed->dwFileVersionMS),
        HIWORD(pFixed->dwFileVersionLS),
        LOWORD(pFixed->dwFileVersionLS) );
}
//----------------------------------------------------------------------------
FString FWindowsPlatformMisc::MachineName() {
    char buffer[MAX_COMPUTERNAME_LENGTH + 1];
    ::DWORD size = checked_cast<::DWORD>(lengthof(buffer));
    Verify(::GetComputerNameA(buffer, &size));
    return FString(buffer, checked_cast<size_t>(size));
}
//----------------------------------------------------------------------------
FString FWindowsPlatformMisc::UserName() {
    char buffer[UNLEN + 1];
    ::DWORD size = checked_cast<::DWORD>(lengthof(buffer));
    Verify(::GetUserNameA(buffer, &size));
    return FString(buffer, checked_cast<size_t>(size - 1/* includes null char */));
}
//----------------------------------------------------------------------------
void FWindowsPlatformMisc::SetGracefulTerminationHandler() {
    // Set console control handler so we can exit if requested.
    ::SetConsoleCtrlHandler(ConsoleCtrlHandler_, TRUE);
}
//----------------------------------------------------------------------------
PRAGMA_MSVC_WARNING_PUSH()
PRAGMA_MSVC_WARNING_DISABLE(6031) // Return value ignored: '_setmode'.
void FWindowsPlatformMisc::SetUTF8Output() {

    // Force locale to EN with UTF-8 encoding
    std::setlocale(LC_ALL, "en_US.UTF-8");

    ::_setmode(_fileno(stdout), _O_U8TEXT);
    ::_setmode(_fileno(stderr), _O_U8TEXT);

    // _setmbcp, with an argument of _MB_CP_LOCALE makes the multibyte code page the same as the setlocale code page.
    // https://docs.microsoft.com/en-us/cpp/c-runtime-library/locale
    ::_setmbcp(_MB_CP_LOCALE);

    ::SetConsoleOutputCP(CP_UTF8);
}
PRAGMA_MSVC_WARNING_POP()
//----------------------------------------------------------------------------
bool FWindowsPlatformMisc::Is64bitOperatingSystem() {
    return IsProcess64Bit_(::GetCurrentProcess());
}
//----------------------------------------------------------------------------
bool FWindowsPlatformMisc::IsRunningOnBatery() {

    ::SYSTEM_POWER_STATUS status;
    ::GetSystemPowerStatus(&status);

    switch (status.BatteryFlag) {
    case 4://	"Critical-the battery capacity is at less than five percent"
    case 2://	"Low-the battery capacity is at less than 33 percent"
    case 1://	"High-the battery capacity is at more than 66 percent"
    case 8://	"Charging"
        return true;
    case 128://	"No system battery" - desktop, NB: UPS don't count as batteries under Windows
    case 255://	"Unknown status-unable to read the battery flag information"
    default:
        return false;
    }
}
//----------------------------------------------------------------------------
void FWindowsPlatformMisc::PreventScreenSaver() {

    ::INPUT input;
    ::ZeroMemory(&input, sizeof(input));
    input.type = INPUT_MOUSE;
    input.mi.dx = 0;
    input.mi.dy = 0;
    input.mi.mouseData = 0;
    input.mi.dwFlags = MOUSEEVENTF_MOVE;
    input.mi.time = 0;
    input.mi.dwExtraInfo = 0;
    ::SendInput(1, &input, sizeof(::INPUT));
}
//----------------------------------------------------------------------------
void FWindowsPlatformMisc::ClipboardCopy(const char* src, size_t len) {
    Assert(src);
    ClipboardCopy_(CF_TEXT, FStringView(src, len));
}
//----------------------------------------------------------------------------
void FWindowsPlatformMisc::ClipboardCopy(const wchar_t* src, size_t len) {
    Assert(src);
    ClipboardCopy_(CF_UNICODETEXT, FWStringView(src, len));
}
//----------------------------------------------------------------------------
bool FWindowsPlatformMisc::ClipboardPaste(FString& dst) {
    if (::OpenClipboard(::GetActiveWindow()))
    {
        ::HGLOBAL globalMem = ::GetClipboardData(CF_UNICODETEXT);

        bool unicode = true;
        if (!globalMem)
        {
            globalMem = ::GetClipboardData(CF_TEXT);
            unicode = false;
        }

        if (!globalMem) {
            dst.clear();
            return false;
        }

        Assert(globalMem);
        void* data = ::GlobalLock(globalMem);
        Assert(data);

        if (unicode) {
            const FWStringView wide = MakeCStringView((const wchar_t*)data);
            STACKLOCAL_POD_ARRAY(char, ansi, wide.size());
            dst.assign(WCHAR_to_CHAR(ECodePage::ACP, ansi, wide));
        }
        else {
            dst.assign(MakeCStringView((const char*)data));
        }

        ::GlobalUnlock(globalMem);
        Verify(::CloseClipboard());

        return true;
    }
    else {
        dst.clear();
        return false;
    }
}
//----------------------------------------------------------------------------
void FWindowsPlatformMisc::CreateGuid(struct FGuid& dst) {
    VerifyRelease(::CoCreateGuid(&reinterpret_cast<::GUID&>(dst)) == S_OK);
}
//----------------------------------------------------------------------------
bool FWindowsPlatformMisc::EnvironmentVariable(const char* key, char* value, size_t capacity) {
    Assert(key);
    Assert(value);
    Assert(capacity > 0);

    return (::GetEnvironmentVariableA(key, value, checked_cast<::DWORD>(capacity)) != 0);
}
//----------------------------------------------------------------------------
void FWindowsPlatformMisc::SetEnvironmentVariable(const char* key, const char* value) {
    Assert(key);
    Assert(value);

    VerifyRelease(::SetEnvironmentVariableA(key, value));
}
//----------------------------------------------------------------------------
bool FWindowsPlatformMisc::PersistentVariable(const char* storeId, const char* section, const char* key, FString* value) {
    Assert(storeId);
    Assert(section);
    Assert(key);

    FString regKey = StringFormat("Software/{0}/{1}", MakeCStringView(storeId), MakeCStringView(section));
    regKey.gsub('/', '\\'); // we use forward slashes, but the registry needs back slashes

    return FWindowsPlatformMisc::QueryRegKey(HKEY_CURRENT_USER, regKey.c_str(), key, value);
}
//----------------------------------------------------------------------------
bool FWindowsPlatformMisc::SetPersistentVariable(const char* storeId, const char* section, const char* key, const char* value) {
    Assert(storeId);
    Assert(key);
    Assert(section);
    Assert(value);

    FString regKey = StringFormat("Software/{0}/{1}", MakeCStringView(storeId), MakeCStringView(section));
    regKey.gsub('/', '\\'); // we use forward slashes, but the registry needs back slashes

    ::HKEY hKey;
    ::HRESULT result = ::RegCreateKeyExA(HKEY_CURRENT_USER, regKey.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);
    if (result == ERROR_SUCCESS) {
        const FStringView buffer = MakeCStringView(value);
        result = ::RegSetValueExA(hKey, key, 0, REG_SZ, (const ::BYTE*)buffer.data(), checked_cast<::DWORD>((buffer.size() + 1) * sizeof(*value)) );
        ::RegCloseKey(hKey);
    }

    if (result != ERROR_SUCCESS) {
        PPE_LOG(HAL, Error, "SetPersistentVariable: ERROR: could not store value for '{0}'. error Code : {1}",
            MakeCStringView(key),
            FLastError(result) );

        return false;
    }

    return true;
}
//----------------------------------------------------------------------------
bool FWindowsPlatformMisc::ErasePersistentVariable(const char* storeId, const char* section, const char* key) {
    Assert(storeId);
    Assert(section);
    Assert(key);

    FString regKey = StringFormat("Software/{0}/{1}", MakeCStringView(storeId), MakeCStringView(section));
    regKey.gsub('/', '\\'); // we use forward slashes, but the registry needs back slashes

    ::HKEY hKey;
    ::HRESULT result = ::RegOpenKeyExA(HKEY_CURRENT_USER, regKey.c_str(), 0, KEY_WRITE | KEY_READ, &hKey);
    if (result == ERROR_SUCCESS) {
        result = ::RegDeleteValueA(hKey, key);

        ::HRESULT enumResult;

        // Query for sub-keys in the open key
        char checkKeyName[256];
        ::DWORD checkKeyNameLength = static_cast<::DWORD>(lengthof(checkKeyName));
        enumResult = ::RegEnumKeyExA(hKey, 0, checkKeyName, &checkKeyNameLength, NULL, NULL, NULL, NULL);
        const bool zeroSubKeys = (enumResult != ERROR_SUCCESS);

        // Query for a remaining value in the open key
        char checkValueName[256];
        ::DWORD checkValueNameLength = static_cast<::DWORD>(lengthof(checkValueName));
        enumResult = ::RegEnumValueA(hKey, 0, checkValueName, &checkValueNameLength, NULL, NULL, NULL, NULL);
        const bool zeroValues = (enumResult != ERROR_SUCCESS);

        ::RegCloseKey(hKey);

        if (zeroSubKeys && zeroValues) {
            // No more values - delete the section
            ::RegDeleteKeyA(HKEY_CURRENT_USER, regKey.c_str());
        }
    }

    return (result == ERROR_SUCCESS);
}
//----------------------------------------------------------------------------
bool FWindowsPlatformMisc::ExternalTextEditor(const wchar_t* filename, size_t line/* = 0 */, size_t column/* = 0 */) {
    Assert(filename);

    CONSTEXPR const FWStringLiteral editors[] = {
        // visual studio code (appdata)
        L"\"%LOCALAPPDATA%\\Programs\\Microsoft VS Code\\bin\\code\" -g \"{0}:{1}:{2}\"",
        // visual studio code (classic program files)
        L"\"%ProgramFiles%\\Microsoft VS Code\\bin\\code\" -g \"{0}:{1}:{2}\"",
        // visual studio code (batch file in appdata)
        L"\"%LOCALAPPDATA%\\Programs\\Microsoft VS Code\\bin\\code.cmd\" -g \"{0}:{1}:{2}\"",
        // visual studio code (batch file in classic program files)
        L"\"%ProgramFiles%\\Microsoft VS Code\\bin\\code.cmd\" -g \"{0}:{1}:{2}\"",
        // sublime text 3
        L"\"%ProgramFiles%\\Sublime Text 3\\sublime_text.exe\" \"{0}:{1}:{2}\"",
        // notepad++ (not tested :p)
        L"\"%ProgramFiles%\\Notepad++\\Notepad++.exe\" \"{0}\" -n{1} -c{2}",
        // notepad(2-mod)
        L"\"C:\\Windows\\System32\\notepad.exe\" \"{0}\" /g {1}",
    };

    wchar_t format[2048];
    wchar_t buffer[2048];
    ::STARTUPINFO startupInfo;
    ::PROCESS_INFORMATION processInfo;

    for (const FWStringLiteral& ed : editors) {
        {
            FWFixedSizeTextWriter oss(format);
            Format(oss, ed, filename, line, column);
            oss << Eos;
        }

        ::ExpandEnvironmentStringsW(format, buffer, static_cast<::DWORD>(lengthof(buffer)));

        ZeroMemory(&startupInfo, sizeof(startupInfo));
        ZeroMemory(&processInfo, sizeof(processInfo));
        startupInfo.cb = sizeof(::STARTUPINFO);

        // create a new process for external editor :
        if (::CreateProcessW(NULL, buffer,
            0, 0, FALSE, CREATE_NO_WINDOW|DETACHED_PROCESS, 0, 0,
            &startupInfo, &processInfo) == 0) {

            PPE_LOG(HAL, Error, "failed to open external editor : {0}\n\t{1}",
                FLastError(), MakeCStringView(buffer));
        }
        else {
            // Immediately close handles since we run detached :
            ::CloseHandle(processInfo.hThread);
            ::CloseHandle(processInfo.hProcess);

            PPE_LOG(HAL, Emphasis, "opened external editor : {0}", MakeCStringView(buffer));

            return true;
        }
    }

    return false;
}
//----------------------------------------------------------------------------
::HWND FWindowsPlatformMisc::FindProcessWindow(::DWORD pid) {
    using lparams_type = TPair<::HWND, ::DWORD>;
    lparams_type params{ NULL, pid };

    const ::BOOL bFound = ::EnumWindows([](::HWND hWnd, ::LPARAM lParam) -> BOOL {
        lparams_type* const pParams = (lparams_type*)lParam;

        ::DWORD windowPid;
        if (::GetWindowThreadProcessId(hWnd, &windowPid) && windowPid == pParams->second) {
            // stop enumeration
            ::SetLastError(INDEX_NONE);
            pParams->first = hWnd;
            return FALSE;
        }

        // continue enumeration
        return TRUE;
    }, (::LPARAM)&params );

    if (not bFound && ::GetLastError() == INDEX_NONE && params.first)
        return params.first;

    return NULL;
}
//----------------------------------------------------------------------------
bool FWindowsPlatformMisc::QueryRegKey(const ::HKEY key, const char* subKey, const char* name, FString* pValue) {
    Assert(subKey);
    Assert(pValue);

    bool succeed = false;

    // Redirect key depending on system
    for (::DWORD regIndex = 0; regIndex < 2 && !succeed; ++regIndex) {
        ::HKEY k = 0;
        const ::DWORD regFlags = (regIndex == 0 ? KEY_WOW64_32KEY : KEY_WOW64_64KEY);
        if (::RegOpenKeyExA(key, subKey, 0, KEY_READ | regFlags, &k) == ERROR_SUCCESS) {
            ::DWORD sz = 0;
            // First, we'll call RegQueryValueEx to find out how large of a buffer we need
            if ((::RegQueryValueExA(k, name, NULL, NULL, NULL, &sz) == ERROR_SUCCESS) && sz) {
                // Allocate a buffer to hold the value and call the function again to get the data
                pValue->resize(checked_cast<size_t>(sz));
                if (::RegQueryValueExA(k, name, NULL, NULL, (::LPBYTE)pValue->data(), &sz) == ERROR_SUCCESS)
                    succeed = true;
            }
            ::RegCloseKey(k);
        }
    }

    return succeed;
}

//----------------------------------------------------------------------------
bool FWindowsPlatformMisc::QueryRegKey(const ::HKEY key, const wchar_t* subKey, const wchar_t* name, FWString* pValue) {
    Assert(subKey);
    Assert(pValue);

    bool succeed = false;

    STACKLOCAL_POD_ARRAY(wchar_t, buffer, 1024);

    // Redirect key depending on system
    for (::DWORD regIndex = 0; regIndex < 2 && !succeed; ++regIndex) {
        ::HKEY k = 0;
        const ::DWORD regFlags = (regIndex == 0 ? KEY_WOW64_32KEY : KEY_WOW64_64KEY);
        if (::RegOpenKeyExW(key, subKey, 0, KEY_READ | regFlags, &k) == ERROR_SUCCESS) {
            ::DWORD sz = 0;
            // First, we'll call RegQueryValueEx to find out how large of a buffer we need
            if ((::RegQueryValueExW(k, name, NULL, NULL, NULL, &sz) == ERROR_SUCCESS) && sz) {
                // Allocate a buffer to hold the value and call the function again to get the data
                ::DWORD writtenInBytes = checked_cast<::DWORD>(buffer.SizeInBytes());
                if (::RegQueryValueExW(k, name, NULL, NULL, (::LPBYTE)buffer.data(), &writtenInBytes) == ERROR_SUCCESS) {
                    succeed = true;
                    FWStringView value = MakeCStringView(buffer.data());
                    pValue->assign(Strip(value));
                }
                else {
                    PPE_LOG_LASTERROR(HAL, "RegQueryValueExW()");
                }
            }
            Verify(ERROR_SUCCESS == ::RegCloseKey(k));
        }
    }

    return succeed;
}
//----------------------------------------------------------------------------
// https://github.com/khalladay/hooking-by-example/blob/master/hooking-by-example/hooking_common.h#L88
//----------------------------------------------------------------------------
void* FWindowsPlatformMisc::AllocateExecutablePageNearAddress(void* targetAddr) {
    return AllocateExecutablePageNearAddressRemote(::GetCurrentProcess(), targetAddr);
}
//----------------------------------------------------------------------------
void* FWindowsPlatformMisc::AllocateExecutablePageNearAddressRemote(::HANDLE hProcess, void* targetAddr) {
    Assert_NoAssume(IsProcess64Bit_(hProcess));

    ::SYSTEM_INFO sysInfo;
    ::GetSystemInfo(&sysInfo);

    const uintptr_t startAddr = (uintptr_t(targetAddr) & ~uintptr_t(sysInfo.dwPageSize - 1)); //round down to nearest page boundary
    const uintptr_t minAddr = Min(startAddr - 0x7FFFFF00, (uintptr_t)sysInfo.lpMinimumApplicationAddress);
    const uintptr_t maxAddr = Max(startAddr + 0x7FFFFF00, (uintptr_t)sysInfo.lpMaximumApplicationAddress);

    const u64 startPage = (startAddr - (startAddr % sysInfo.dwPageSize));

    u64 pageOffset = 1;
    for (;;) {
        const u64 byteOffset = pageOffset * PAGE_SIZE;
        const u64 highAddr = startPage + byteOffset;
        const u64 lowAddr = (startPage > byteOffset) ? startPage - byteOffset : 0;

        const bool needsExit = highAddr > maxAddr && lowAddr < minAddr;

        if (highAddr < maxAddr) {
            void* const outAddr = ::VirtualAllocEx(hProcess, (void*)highAddr, (size_t)PAGE_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
            if (outAddr)
                return outAddr;
        }

        if (lowAddr > minAddr) {
            void* const outAddr = ::VirtualAllocEx(hProcess, (void*)lowAddr, (size_t)PAGE_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
            if (outAddr != nullptr)
                return outAddr;
        }

        pageOffset++;

        if (needsExit) {
            break;
        }
    }

    return nullptr;
}
//----------------------------------------------------------------------------
// Basic Windows API Hooking
// https://github.com/khalladay/hooking-by-example
// https://medium.com/geekculture/basic-windows-api-hooking-acb8d275e9b8
//----------------------------------------------------------------------------
bool FWindowsPlatformMisc::CreateDetour(FDetour* hook, ::LPCWSTR libraryName, ::LPCSTR functionName, ::LPVOID proxyFunc) {
    Assert(libraryName);
    Assert(functionName);

    const ::HMODULE hLibrary = ::LoadLibraryW(libraryName);
    if (NULL == hLibrary) {
        PPE_LOG_LASTERROR(HAL, "LoadLibraryW");
        return false;
    }

    const ::FARPROC lpFunction = ::GetProcAddress(hLibrary, functionName);
    if (NULL == lpFunction) {
        PPE_LOG_LASTERROR(HAL, "GetProcAddress");
        return false;
    }

    hook->FunctionName = functionName;
    hook->OriginalFunc = lpFunction;
    return CreateDetour(hook, proxyFunc);
}
//----------------------------------------------------------------------------
PRAGMA_MSVC_WARNING_PUSH()
PRAGMA_MSVC_WARNING_DISABLE(4302) // 'type cast': truncation from 'FARPROC' to 'DWORD'
PRAGMA_MSVC_WARNING_DISABLE(4311) // 'type cast': pointer truncation from 'FARPROC' to 'DWORD'
PRAGMA_MSVC_WARNING_DISABLE(4312) // 'type cast': conversion from 'DWORD' to 'DWORD *' of greater size
#ifdef __clang__
#   pragma clang diagnostic push,
#   pragma clang diagnostic ignored "-Wmicrosoft-cast"
#endif
static u32 WriteRelativeJump_(void* func2hook, void* jumpTarget) {
    using FDetour = FWindowsPlatformMisc::FDetour;

    u8 jmpInstruction[FDetour::Size] = { 0xE9, 0x0, 0x0, 0x0, 0x0 };

    const intptr_t relativeToJumpTarget64 = (intptr_t)jumpTarget - ((intptr_t)func2hook + FDetour::Size);
    AssertRelease(relativeToJumpTarget64 < INT32_MAX);

    const i32 relativeToJumpTarget = checked_cast<i32>(relativeToJumpTarget64);
    ::memcpy(jmpInstruction + 1, &relativeToJumpTarget, 4);

    PPE_LOG_CHECK(HAL, ::WriteProcessMemory(
        ::GetCurrentProcess(),
        func2hook,
        jmpInstruction,
        sizeof(jmpInstruction),
        nullptr));

    return sizeof(jmpInstruction);
}
#if defined(ARCH_X86)
static bool CreateDetour_X86_(FWindowsPlatformMisc::FDetour* hook, LPVOID proxyFunc) {
    using FDetour = FWindowsPlatformMisc::FDetour;

    // save the first 5 bytes of OriginalFunc into PrologueBackup
    PPE_LOG_CHECK(HAL, ::ReadProcessMemory(
        ::GetCurrentProcess(),
        hook->OriginalFunc,
        hook->PrologueBackup,
        FDetour::Size,
        nullptr));

    // build the trampoline
    ::DWORD* const hookAddress = (::DWORD*)(
        (::DWORD)(hook->OriginalFunc) + FDetour::Size);

    hook->TrampolineAddress = ::VirtualAlloc(NULL, 11, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    PPE_LOG_CHECK(HAL, !!hook->TrampolineAddress);

    ::memcpy((BYTE*)hook->TrampolineAddress +  0, hook->PrologueBackup, FDetour::Size);
    ::memcpy((BYTE*)hook->TrampolineAddress +  5, "\x68", 1); // 68	PUSH imm16 / 32     Push Word, Doubleword or Quadword Onto the Stack
    ::memcpy((BYTE*)hook->TrampolineAddress +  6, &hookAddress, 4);
    ::memcpy((BYTE*)hook->TrampolineAddress + 10, "\xC3", 1); // C3	RETN                Return from procedure

    return (FDetour::Size == WriteRelativeJump_(hook->OriginalFunc, proxyFunc));
}
#elif defined(ARCH_X64)
// https://github.com/khalladay/hooking-by-example/blob/master/hooking-by-example/09%20-%20Trampoline%20Free%20Function%20In%20Same%20Process/trampoline-free-function.cpp
static u32 WriteAbsoluteJump64_(void* absJumpMemory, void* addrToJumpTo) {
    Assert_NoAssume(IsProcess64Bit_(::GetCurrentProcess()));

    //this writes the absolute jump instructions into the memory allocated near the target
    //the E9 jump installed in the target function (GetNum) will jump to here
    u8 absJumpInstructions[] = {
        0x49, 0xBA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, //mov 64 bit value into r10
        0x41, 0xFF, 0xE2 }; //jmp r10

    u64 addrToJumpTo64 = (u64)addrToJumpTo;
    ::memcpy(&absJumpInstructions[2], &addrToJumpTo64, sizeof(addrToJumpTo64));
    ::memcpy(absJumpMemory, absJumpInstructions, sizeof(absJumpInstructions));
    return sizeof(absJumpInstructions);
}
static bool CreateDetour_X64_(FWindowsPlatformMisc::FDetour* hook, LPVOID proxyFunc) {
    using FDetour = FWindowsPlatformMisc::FDetour;

    ::DWORD oldProtect;
    PPE_LOG_CHECK(HAL, ::VirtualProtect(hook->OriginalFunc, FDetour::Size, PAGE_EXECUTE_READWRITE, &oldProtect));
    ::memcpy(hook->PrologueBackup, hook->OriginalFunc, FDetour::Size);

    //we need to use JMP rel32 even on x64 to fit in 5 bytes, so the offset between the trampoline
    //and the original function must fit inside a 32 bits integer: so we try to allocate the page
    //for the trampoline near the original function
    hook->TrampolineAddress = FWindowsPlatformMisc::AllocateExecutablePageNearAddress(hook->OriginalFunc);
    PPE_LOG_CHECK(HAL, !!hook->TrampolineAddress);

    //the trampoline consists of the stolen bytes from the target function, following by a jump back
    //to the target function + 5 bytes, in order to continue the execution of that function. This continues like
    //a normal function call
    void* const trampolineJumpTarget = ((u8*)hook->OriginalFunc + FDetour::Size);

    u8* const dst = (u8*)hook->TrampolineAddress;
    ::memcpy(dst, hook->PrologueBackup, FDetour::Size);
    PPE_LOG_CHECK(HAL, !!WriteAbsoluteJump64_(dst + FDetour::Size, trampolineJumpTarget));

    //the last operation is to finally overwrite the first 5 bytes of the original function with a jump
    //to our proxy function
    return (FDetour::Size == WriteRelativeJump_(hook->OriginalFunc, proxyFunc));
}
#else
#   error "Detour support is missing for current architecture"
#endif
#ifdef __clang__
#   pragma clang diagnostic pop
#endif
PRAGMA_MSVC_WARNING_POP()
//----------------------------------------------------------------------------
bool FWindowsPlatformMisc::CreateDetour(FDetour* hook, LPVOID proxyFunc) {
    Assert(hook);
    Assert(proxyFunc);
    Assert(hook->OriginalFunc);
    Assert_NoAssume(not hook->TrampolineAddress);

    // note: "5 bytes classic hook" is only guaranteed on specific WinAPI,
    // more general support involves disasm and a dedicated library!
    if (CONCAT(CreateDetour_, CODE3264(X86_, X64_))(hook, proxyFunc)) {
        PPE_LOG(HAL, Info, "created detour hook on WinAPI function {0}()",
            MakeCStringView(hook->FunctionName));

        return true;
    }

    return false;
}
//----------------------------------------------------------------------------
void FWindowsPlatformMisc::DestroyDetour(FDetour* hook) {
    Assert(hook);

    if (not hook->TrampolineAddress)
        return;

    // same code for both x86 and x64
    PPE_LOG(HAL, Info, "destroy detour hook on WinAPI function {0}()",
        MakeCStringView(hook->FunctionName));

    // release virtual memory used by trampoline
    ::VirtualFree(hook->TrampolineAddress, 0, MEM_RELEASE);
    hook->TrampolineAddress = nullptr;

    // restore function prologue backup
    PPE_LOG_CHECKVOID(HAL, ::WriteProcessMemory(
        ::GetCurrentProcess(),
        (::LPVOID)hook->OriginalFunc,
        hook->PrologueBackup,
        FDetour::Size,
        nullptr));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_WINDOWS
