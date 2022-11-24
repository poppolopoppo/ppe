// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "HAL/Generic/GenericPlatformProcess.h"

#include "HAL/PlatformCrash.h"
#include "HAL/PlatformMaths.h"
#include "HAL/PlatformMisc.h"

#include "Diagnostic/CurrentProcess.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FGenericPlatformProcess::OnProcessStart(
    void* appHandle, int nShowCmd,
    const wchar_t* filename, size_t argc, const wchar_t* const* argv) {

    // Set FTZ + DAZ for FP_ASSIST
    FPlatformMaths::Disable_FP_Assist();
    // Force locale to EN with UTF-8 encoding
    FPlatformMisc::SetUTF8Output();
    // Install crash exception handlers
    FPlatformCrash::SetExceptionHandlers();

#if !USE_PPE_FINAL_RELEASE
    FMallocDebug::StartLeakDetector();
#endif

    FCurrentProcess::Create(appHandle, nShowCmd, filename, argc, argv);
}
//----------------------------------------------------------------------------
void FGenericPlatformProcess::OnProcessShutdown() {
    FCurrentProcess::Destroy();

#if !USE_PPE_FINAL_RELEASE
    FMallocDebug::ShutdownLeakDetector();
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
