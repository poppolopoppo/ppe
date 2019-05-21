#pragma once

#include "HAL/TargetPlatform.h"
#include "IO/String_fwd.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// On x86(-64) platforms, uses cpuid instruction to get the CPU signature
struct FGenericPlatformCPUInfo {
    /*  0 -  3 */u32 SteppingID     : 4;
    /*  4 -  7 */u32 Model          : 4;
    /*  8 - 11 */u32 Family         : 4;
    /* 12 - 13 */u32 Type           : 2; // Intel : type / Amd : reserved
    /* 14 - 15 */u32 Reserved0      : 2;
    /* 16 - 19 */u32 ExtendedModel  : 4;
    /* 20 - 27 */u32 ExtendedFamily : 8;
    /* 28 - 31 */u32 Reserved1      : 4;
    //                              = 32 bits
    u32 Ordinal() const {
        STATIC_ASSERT(sizeof(*this) == sizeof(u32));
        return (*reinterpret_cast<const u32*>(this));
    }
};
//----------------------------------------------------------------------------
struct PPE_CORE_API FGenericPlatformMisc {
public: // must be defined for every platform

    STATIC_CONST_INTEGRAL(bool, HasFixedHardware, false);

    using FCPUInfo = FGenericPlatformCPUInfo;

    static FCPUInfo CPUInfo() = delete;
    static FString CPUBrand() = delete;
    static FString CPUVendor() = delete;

    static size_t NumCores() = delete;
    static size_t NumCoresWithSMT() = delete;

    static FString MachineName() = delete;
    static FString OSName() = delete;
    static FString UserName() = delete;

    static void SetGracefulTerminationHandler() = delete; // for Ctrl-C/Control-Break/Close
    static void SetUTF8Output() = delete;

    static bool Is64bitOperatingSystem() = delete;
    static bool IsRunningOnBatery() = delete;
    static void PreventScreenSaver() = delete;

    static void ClipboardCopy(const char* src, size_t len) = delete;
    static void ClipboardCopy(const wchar_t* src, size_t len) = delete;
    static bool ClipboardPaste(FString& dst) = delete;

    static void CreateGuid(struct FGuid& dst) = delete;

    static bool EnvironmentVariable(const char* key, char* value, size_t capacity) = delete;
    static void SetEnvironmentVariable(const char* key, const char* value) = delete;

    static bool PersistentVariable(const char* storeId, const char* key, FString* value) = delete;
    static bool SetPersistentVariable(const char* storeId, const char* key, const char* value) = delete;
    static bool ErasePersistentVariable(const char* storeId, const char* key) = delete;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
