#include "stdafx.h"

#include "Diagnostics.h"

#include "Callstack.h"
#include "CrtDebug.h"
#include "CurrentProcess.h"
#include "MiniDump.h"
#include "Profiling.h"

#include "Memory/MemoryDomain.h"

#ifdef OS_WINDOWS
#   include "DbghelpWrapper.h"
#endif

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FDiagnosticsStartup::Start(void *applicationHandle, int nShowCmd, size_t argc, const wchar_t** argv) {
    FCurrentProcess::Create(applicationHandle, nShowCmd, argc, argv);
#ifdef OS_WINDOWS
    FDbghelpWrapper::Create();
#endif
    FCallstack::Start();
    FMemoryDomainStartup::Start();
    MiniDump::Start();
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
    MiniDump::Shutdown();
    FMemoryDomainStartup::Shutdown();
    FCallstack::Shutdown();
#ifdef OS_WINDOWS
    FDbghelpWrapper::Destroy();
#endif
    FCurrentProcess::Destroy();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
