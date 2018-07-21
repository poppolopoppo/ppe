#include "stdafx.h"

#include "Diagnostics.h"

#include "CurrentProcess.h"
#include "DebugFunction.h"
#include "Profiling.h"

#include "Memory/MemoryDomain.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FDiagnosticsStartup::Start(void *applicationHandle, int nShowCmd, const wchar_t* filename, size_t argc, const wchar_t** argv) {
    DEBUG_FUNCTION_START();

    FCurrentProcess::Create(applicationHandle, nShowCmd, filename, argc, argv);

#ifdef WITH_CORE_PROFILING
    FProfiler::FStartup();
#endif
}
//----------------------------------------------------------------------------
void FDiagnosticsStartup::Shutdown() {
#ifdef WITH_CORE_PROFILING
    FProfiler::Shutdown();
#endif

    FCurrentProcess::Destroy();

    DEBUG_FUNCTION_SHUTDOWN();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
