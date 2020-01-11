#pragma once

#include "HAL/Generic/GenericPlatformMisc.h"

#ifdef PLATFORM_LINUX

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_CORE_API FLinuxPlatformMisc : FGenericPlatformMisc {
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

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_LINUX
