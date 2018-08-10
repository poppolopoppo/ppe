#pragma once

#include "HAL/Generic/GenericPlatformCrash.h"

#ifdef PLATFORM_WINDOWS

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_CORE_API FWindowsPlatformCrash : FGenericPlatformCrash {
public: // must be defined for every platform
    STATIC_CONST_INTEGRAL(bool, HasCrashDump, true);
    STATIC_CONST_INTEGRAL(bool, HasErrorReport, false); // TODO

    using FGenericPlatformCrash::EResult;
    using FGenericPlatformCrash::EInfoLevel;
    using FGenericPlatformCrash::EReportMode;

    static EResult WriteMiniDump(
        const FWStringView& filename,
        EInfoLevel level = Medium,
        const void* exception_ptrs = nullptr,
        bool suspendThreads = true );

    static void SetExceptionHandlers();

    static void AbortProgramWithDump();

    static void SubmitErrorReport(const FStringView& context, EReportMode mode = Unattended);

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_WINDOWS
