#include "stdafx.h"

#include "Diagnostic/Diagnostics.h"

#include "Diagnostic/CurrentProcess.h"
#include "Diagnostic/DebugFunction.h"

#include "Memory/MemoryDomain.h"
#include "HAL/PlatformProfiler.h"

#if USE_PPE_PLATFORM_PROFILER
#   include "IO/String.h"
#endif

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FDiagnosticsStartup::Start(void *applicationHandle, int nShowCmd, const wchar_t* filename, size_t argc, const wchar_t** argv) {
    DEBUG_FUNCTION_START();

    FCurrentProcess::Create(applicationHandle, nShowCmd, filename, argc, argv);

#if USE_PPE_PLATFORM_PROFILER
    FPlatformProfiler::Name(FPlatformProfiler::GlobalLevel, ToString(filename).data());
#endif
}
//----------------------------------------------------------------------------
void FDiagnosticsStartup::Shutdown() {

    FCurrentProcess::Destroy();

    DEBUG_FUNCTION_SHUTDOWN();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
