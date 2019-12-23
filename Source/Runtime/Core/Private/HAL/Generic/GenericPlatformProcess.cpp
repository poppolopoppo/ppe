#include "stdafx.h"

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

    FCurrentProcess::Create(appHandle, nShowCmd, filename, argc, argv);
}
//----------------------------------------------------------------------------
void FGenericPlatformProcess::OnProcessShutdown() {
    FCurrentProcess::Destroy();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
