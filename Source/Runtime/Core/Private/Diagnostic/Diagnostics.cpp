// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Diagnostic/Diagnostics.h"

#include "Diagnostic/DebugFunction.h"
#include "Diagnostic/IgnoreList.h"

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

#if USE_PPE_IGNORELIST
    FIgnoreList::Create();
#endif
}
//----------------------------------------------------------------------------
void FDiagnosticsStartup::Shutdown() {
#if USE_PPE_IGNORELIST
    FIgnoreList::Destroy();
#endif

    DEBUG_FUNCTION_SHUTDOWN();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
