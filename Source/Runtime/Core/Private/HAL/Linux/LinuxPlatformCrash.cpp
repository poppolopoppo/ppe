﻿// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "HAL/Linux/LinuxPlatformCrash.h"

#ifdef PLATFORM_LINUX

#include "Diagnostic/Logger.h"

#include "HAL/TargetPlatform.h"
#include "HAL/Linux/LinuxPlatformIncludes.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
auto FLinuxPlatformCrash::WriteMiniDump() -> EResult {
    AssertNotImplemented();
    return EResult::NotAvailable;
}
//----------------------------------------------------------------------------
auto FLinuxPlatformCrash::WriteMiniDump(
    const FWStringView& filename,
    EInfoLevel level/* = Medium */,
    const void* exception_ptrs/* = nullptr */,
    bool suspendThreads/* = true */) -> EResult {

    Unused(filename);
    Unused(level);
    Unused(exception_ptrs);
    Unused(suspendThreads);

    // #TODO
    // http://www.scribd.com/doc/3726406/Crash-N-Burn-Writing-Linux-application-fault-handlers
    AssertNotImplemented();
    return EResult::NotAvailable;
}
//----------------------------------------------------------------------------
void FLinuxPlatformCrash::SetExceptionHandlers() {
    // http://www.scribd.com/doc/3726406/Crash-N-Burn-Writing-Linux-application-fault-handlers
    PPE_LOG(HAL, Warning, "SetExceptionHandlers() is not implemented on linux");
}
//----------------------------------------------------------------------------
void FLinuxPlatformCrash::AbortProgramWithDump() {
    throw std::runtime_error("abort program with dump");
}
//----------------------------------------------------------------------------
void FLinuxPlatformCrash::SubmitErrorReport(const FStringView& context, EReportMode mode/* = Unattended */) {
    // #TODO no support on linux ?
    Unused(context);
    Unused(mode);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_LINUX
