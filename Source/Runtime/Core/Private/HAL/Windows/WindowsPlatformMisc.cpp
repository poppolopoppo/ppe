#include "stdafx.h"

#include "HAL/Windows/WindowsPlatformMisc.h"

#ifdef PLATFORM_WINDOWS

#include "Allocator/Alloca.h"
#include "Diagnostic/Logger.h"
#include "HAL/PlatformMemory.h"
#include "IO/Format.h"
#include "IO/String.h"
#include "IO/StringBuilder.h"
#include "IO/StringView.h"

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
static FWindowsPlatformMisc::FCPUInfo FetchCPUInfo_() {
    FWindowsPlatformMisc::FCPUInfo result;

    int args[4];
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
static size_t FetchNumCoresWHyperThreading_() {

    // Get the number of logical processors, including hyper-threaded ones.
    ::SYSTEM_INFO sys;
    ::GetSystemInfo(&sys);

    return checked_cast<size_t>(sys.dwNumberOfProcessors);
}
//----------------------------------------------------------------------------
static BOOL WINAPI ConsoleCtrlHandler_(::DWORD /*Type*/) {

    // make sure as much data is written to disk as possible
    FLUSH_LOG();

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
    char brandString[0x40] = { 0 };
    i32 CPUInfo[4] = { -1 };
    const SIZE_T CPUInfoSize = sizeof(CPUInfo);

    ::__cpuid(CPUInfo, 0x80000000);
    const u32 maxExtIDs = CPUInfo[0];

    if (maxExtIDs >= 0x80000004)
    {
        const u32 firstBrandString = 0x80000002;
        const u32 numBrandStrings = 3;
        for (u32 i = 0; i < numBrandStrings; i++) {
            ::__cpuid(CPUInfo, firstBrandString + i);
            FPlatformMemory::Memcpy(brandString + CPUInfoSize * i, CPUInfo, CPUInfoSize);
        }
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
size_t FWindowsPlatformMisc::NumCores() {
    static const size_t GNumCores = FetchNumCores_();
    return GNumCores;
}
//----------------------------------------------------------------------------
size_t FWindowsPlatformMisc::NumCoresWHyperThreading() {
    static const size_t GNumCoresWHyperThreading = FetchNumCoresWHyperThreading_();
    return GNumCoresWHyperThreading;
}
//----------------------------------------------------------------------------
FString FWindowsPlatformMisc::OSName() {
    const char* osname = nullptr;
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
        osname = "Windows 10";

    const char* const arch = (Is64bitOperatingSystem()
        ? "64 bit"
        : "32 bit" );

    const char* clientOrServer = (::IsWindowsServer()
        ? "Server"
        : "Client" );

    // build version number
    // https://docs.microsoft.com/fr-fr/windows/desktop/SysInfo/getting-the-system-version
    const wchar_t* systemDLL = L"kernel32.dll";

    ::DWORD dummy;
    const ::DWORD cbInfo = ::GetFileVersionInfoSizeExW(FILE_VER_GET_NEUTRAL, systemDLL, &dummy);

    STACKLOCAL_POD_ARRAY(u8, buffer, checked_cast<size_t>(cbInfo));
    ::GetFileVersionInfoExW(FILE_VER_GET_NEUTRAL, systemDLL, dummy, checked_cast<::DWORD>(buffer.size()), buffer.data());

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
void FWindowsPlatformMisc::SetUTF8Output() {

    // Force locale to EN with UTF-8 encoding
    std::setlocale(LC_ALL, "en_US.UTF-8");

    ::_setmode(::_fileno(stdout), _O_U8TEXT);
    ::_setmode(::_fileno(stderr), _O_U8TEXT);

    // _setmbcp, with an argument of _MB_CP_LOCALE makes the multibyte code page the same as the setlocale code page.
    // https://docs.microsoft.com/en-us/cpp/c-runtime-library/locale
    ::_setmbcp(_MB_CP_LOCALE);
}
//----------------------------------------------------------------------------
bool FWindowsPlatformMisc::Is64bitOperatingSystem() {
#ifdef ARCH_X64
    return true;
#else
    PRAGMA_MSVC_WARNING_PUSH()
    PRAGMA_MSVC_WARNING_DISABLE(4191) // unsafe conversion from 'type of expression' to 'type required'
    typedef ::BOOL(WINAPI *LPFN_ISWOW64PROCESS)(::HANDLE, ::PBOOL);
    LPFN_ISWOW64PROCESS const fnIsWow64Process = (LPFN_ISWOW64PROCESS)::GetProcAddress(::GetModuleHandleA("kernel32"), "IsWow64Process");
    ::BOOL bIsWoW64Process = FALSE;
    if (fnIsWow64Process != NULL) {
        if (fnIsWow64Process(::GetCurrentProcess(), &bIsWoW64Process) == 0)
            bIsWoW64Process = FALSE;
    }
    PRAGMA_MSVC_WARNING_POP()
    return (bIsWoW64Process == TRUE);
#endif
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

    ::INPUT Input = { 0 };
    Input.type = INPUT_MOUSE;
    Input.mi.dx = 0;
    Input.mi.dy = 0;
    Input.mi.mouseData = 0;
    Input.mi.dwFlags = MOUSEEVENTF_MOVE;
    Input.mi.time = 0;
    Input.mi.dwExtraInfo = 0;
    ::SendInput(1, &Input, sizeof(INPUT));
}
//----------------------------------------------------------------------------
void FWindowsPlatformMisc::ClipboardCopy(const char* src) {
    Assert(src);

    if (::OpenClipboard(::GetActiveWindow()))
    {
        Verify(::EmptyClipboard());

        const size_t len = Length(src);

        ::HGLOBAL globalMem = ::GlobalAlloc(GMEM_MOVEABLE, sizeof(char)*(len + 1));
        Assert(globalMem);
        {
            char* data = (char*)::GlobalLock(globalMem);
            FPlatformMemory::Memcpy(data, src, (len + 1) * sizeof(char));
            ::GlobalUnlock(globalMem);
        }

        if (::SetClipboardData(CF_TEXT, globalMem) == NULL)
            LOG(HAL, Fatal, L"SetClipboardData failed with error : {0}", FLastError());

        Verify(::CloseClipboard());
    }
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
        LOG(HAL, Error, L"SetPersistentVariable: ERROR: could not store value for '{0}'. error Code : {1}",
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
        ::DWORD checkKeyNameLength = lengthof(checkKeyName);
        enumResult = ::RegEnumKeyExA(hKey, 0, checkKeyName, &checkKeyNameLength, NULL, NULL, NULL, NULL);
        const bool zeroSubKeys = (enumResult != ERROR_SUCCESS);

        // Query for a remaining value in the open key
        char checkValueName[256];
        ::DWORD checkValueNameLength = lengthof(checkValueName);
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

    // Redirect key depending on system
    for (::DWORD regIndex = 0; regIndex < 2 && !succeed; ++regIndex) {
        ::HKEY k = 0;
        const ::DWORD regFlags = (regIndex == 0 ? KEY_WOW64_32KEY : KEY_WOW64_64KEY);
        if (::RegOpenKeyExW(key, subKey, 0, KEY_READ | regFlags, &k) == ERROR_SUCCESS) {
            ::DWORD sz = 0;
            // First, we'll call RegQueryValueEx to find out how large of a buffer we need
            if ((::RegQueryValueExW(k, name, NULL, NULL, NULL, &sz) == ERROR_SUCCESS) && sz) {
                // Allocate a buffer to hold the value and call the function again to get the data
                STACKLOCAL_POD_ARRAY(u8, buffer, checked_cast<size_t>(sz));
                if (::RegQueryValueExW(k, name, NULL, NULL, (::LPBYTE)buffer.data(), &sz) == ERROR_SUCCESS) {
                    succeed = true;
                    pValue->assign(MakeCStringView((const wchar_t*)buffer.data()));
                }
            }
            ::RegCloseKey(k);
        }
    }

    return succeed;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_WINDOWS