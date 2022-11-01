#pragma once

#include "HAL/Generic/GenericPlatformMisc.h"

#ifdef PLATFORM_WINDOWS

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_CORE_API FWindowsPlatformMisc : FGenericPlatformMisc {
public:
    STATIC_CONST_INTEGRAL(bool, HasFixedHardware, false);

    using FGenericPlatformMisc::FCPUInfo;

    static FCPUInfo CPUInfo();
    static FString CPUBrand();
    static FString CPUVendor();

    static size_t NumCores() NOEXCEPT;
    static size_t NumCoresWithSMT() NOEXCEPT;

    static FString MachineName();
    static FString OSName();
    static FString UserName();

    static void SetGracefulTerminationHandler(); // for Ctrl-C/Control-Break/Close
    static void SetUTF8Output();

    static bool Is64bitOperatingSystem();
    static bool IsRunningOnBatery();
    static void PreventScreenSaver();

    static void ClipboardCopy(const char* src, size_t len);
    static void ClipboardCopy(const wchar_t* src, size_t len);
    static bool ClipboardPaste(FString& dst);

    static void CreateGuid(struct FGuid& dst);

    static bool EnvironmentVariable(const char* key, char* value, size_t capacity);
    static void SetEnvironmentVariable(const char* key, const char* value);

    static bool PersistentVariable(const char* storeId, const char* section, const char* key, FString* value);
    static bool SetPersistentVariable(const char* storeId, const char* section, const char* key, const char* value);
    static bool ErasePersistentVariable(const char* storeId, const char* section, const char* key);

    static bool ExternalTextEditor(const wchar_t* filename, size_t line = 0, size_t column = 0);

public: // specific to this platform

    static ::HWND FindProcessWindow(::DWORD pid);

    static bool QueryRegKey(const ::HKEY key, const char* subKey, const char* name, FString* pValue);
    static bool QueryRegKey(const ::HKEY key, const wchar_t* subKey, const wchar_t* name, FWString* pValue);

    // Basic Windows API Hooking
    // https://medium.com/geekculture/basic-windows-api-hooking-acb8d275e9b8

    static void* AllocateExecutablePageNearAddress(void* targetAddr);
    static void* AllocateExecutablePageNearAddressRemote(::HANDLE hProcess, void* targetAddr);

    struct FDetour {
        STATIC_CONST_INTEGRAL(u32, Size, 5);
        ::LPCSTR FunctionName{ nullptr };
        ::FARPROC OriginalFunc{ nullptr };
        ::LPVOID TrampolineAddress{ nullptr };
        ::BYTE PrologueBackup[Size];
    };

    static bool CreateDetour(FDetour* hook, ::LPVOID proxyFunc);
    static bool CreateDetour(FDetour* hook, ::LPCWSTR libraryName, ::LPCSTR functionName, ::LPVOID proxyFunc);
    static void DestroyDetour(FDetour* hook);

    template <auto _Func>
    struct TAutoDetour : FDetour {
        TAutoDetour(::LPCWSTR libraryName, ::LPCSTR functionName) {
            FWindowsPlatformMisc::CreateDetour(this, libraryName, functionName, _Func);
        }

        ~TAutoDetour() {
            FWindowsPlatformMisc::DestroyDetour(this);
        }

        TAutoDetour(const TAutoDetour&) = delete;
        TAutoDetour& operator =(const TAutoDetour&) = delete;

        // call inside the hook to give back execution to the original function
        template <typename... _Args>
        auto operator ()(_Args&&... args) const {
            return reinterpret_cast<decltype(_Func)>(FDetour::TrampolineAddress)(
                std::forward<_Args>(args)...);
        }

    };
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_WINDOWS
