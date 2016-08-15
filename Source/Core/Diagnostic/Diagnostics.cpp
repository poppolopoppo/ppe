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
void DiagnosticsStartup::Start(void *applicationHandle, int nShowCmd, size_t argc, const wchar_t** argv) {
    CurrentProcess::Create(applicationHandle, nShowCmd, argc, argv);
#ifdef OS_WINDOWS
    DbghelpWrapper::Create();
#endif
    Callstack::Start();
    MemoryDomainStartup::Start();
    MiniDump::Start();
#ifdef WITH_CORE_PROFILING
    Profiler::Startup();
#endif
    GLOBAL_CHECK_MEMORY_LEAKS(true);
}
//----------------------------------------------------------------------------
void DiagnosticsStartup::Shutdown() {
    GLOBAL_CHECK_MEMORY_LEAKS(false);
#ifdef WITH_CORE_PROFILING
    Profiler::Shutdown();
#endif
    MiniDump::Shutdown();
    MemoryDomainStartup::Shutdown();
    Callstack::Shutdown();
#ifdef OS_WINDOWS
    DbghelpWrapper::Destroy();
#endif
    CurrentProcess::Destroy();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core