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

public: // specific to this platform

    static bool QueryRegKey(const ::HKEY key, const char* subKey, const char* name, FString* pValue);
    static bool QueryRegKey(const ::HKEY key, const wchar_t* subKey, const wchar_t* name, FWString* pValue);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_WINDOWS
