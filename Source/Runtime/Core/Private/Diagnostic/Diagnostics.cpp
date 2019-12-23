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
void FDiagnosticsStartup::Start() {
    DEBUG_FUNCTION_START();

#if USE_PPE_PLATFORM_PROFILER
    FPlatformProfiler::Name(FPlatformProfiler::GlobalLevel, ToString(FCurrentProcess::Get().FileName()).data());
#endif
}
//----------------------------------------------------------------------------
void FDiagnosticsStartup::Shutdown() {

    DEBUG_FUNCTION_SHUTDOWN();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
