#include "stdafx.h"

#include "Diagnostic/Diagnostics.h"

#include "Diagnostic/CurrentProcess.h"
#include "Diagnostic/DebugFunction.h"
#include "Diagnostic/Profiling.h"

#include "Memory/MemoryDomain.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FDiagnosticsStartup::Start(void *applicationHandle, int nShowCmd, const wchar_t* filename, size_t argc, const wchar_t** argv) {
    DEBUG_FUNCTION_START();

    FCurrentProcess::Create(applicationHandle, nShowCmd, filename, argc, argv);

#ifdef WITH_PPE_PROFILING
    FProfiler::FStartup();
#endif
}
//----------------------------------------------------------------------------
void FDiagnosticsStartup::Shutdown() {
#ifdef WITH_PPE_PROFILING
    FProfiler::Shutdown();
#endif

    FCurrentProcess::Destroy();

    DEBUG_FUNCTION_SHUTDOWN();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE