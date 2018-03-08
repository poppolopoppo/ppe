#include "stdafx.h"

#include "Diagnostics.h"

#include "Callstack.h"
#include "CrtDebug.h"
#include "CurrentProcess.h"
#include "DebugFunction.h"
#include "Profiling.h"

#include "Memory/MemoryDomain.h"

#ifdef PLATFORM_WINDOWS
#   include "DbghelpWrapper.h"
#endif

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FDiagnosticsStartup::Start(void *applicationHandle, int nShowCmd, const wchar_t* filename, size_t argc, const wchar_t** argv) {
    DEBUG_FUNCTION_START();

    FCurrentProcess::Create(applicationHandle, nShowCmd, filename, argc, argv);
#ifdef PLATFORM_WINDOWS
    FDbghelpWrapper::Create();
#endif
    FCallstack::Start();
#ifdef WITH_CORE_PROFILING
    FProfiler::FStartup();
#endif
    GLOBAL_CHECK_MEMORY_LEAKS(true);
}
//----------------------------------------------------------------------------
void FDiagnosticsStartup::Shutdown() {
    GLOBAL_CHECK_MEMORY_LEAKS(false);
#ifdef WITH_CORE_PROFILING
    FProfiler::Shutdown();
#endif
    FCallstack::Shutdown();
#ifdef PLATFORM_WINDOWS
    FDbghelpWrapper::Destroy();
#endif
    FCurrentProcess::Destroy();

    DEBUG_FUNCTION_SHUTDOWN();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
