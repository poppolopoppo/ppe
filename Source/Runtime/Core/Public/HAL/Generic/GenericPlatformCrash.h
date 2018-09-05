#pragma once

#include "HAL/TargetPlatform.h"
#include "IO/String_fwd.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_CORE_API FGenericPlatformCrash {
public: // must be defined for every platform
    STATIC_CONST_INTEGRAL(bool, HasCrashDump, false);
    STATIC_CONST_INTEGRAL(bool, HasErrorReport, false);

    enum EResult {
        Success = 0,
        NoDbgHelpDLL,
        InvalidFilename,
        CantCreateFile,
        DumpFailed,
        FailedToCloseHandle,
        NotAvailable,
        Reentrancy,
    };

    enum EInfoLevel {
        Small = 0,
        Medium,
        Large,
    };

    enum EReportMode {
        Interactive,
        Unattended,
        Balloon,
    };

    static EResult WriteMiniDump();
    static EResult WriteMiniDump(
        const FWStringView& filename,
        EInfoLevel level = Medium,
        const void* exception_ptrs = nullptr,
        bool suspendThreads = true ) = delete;

    static void SetExceptionHandlers() = delete;

    static void AbortProgramWithDump() = delete;

    static void SubmitErrorReport(const FStringView& context, EReportMode mode = Unattended) = delete;

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
