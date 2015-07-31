#include "stdafx.h"

#include "Core.h"

#include "Allocator/Allocation.h"
#include "Diagnostic/Callstack.h"
#include "Diagnostic/CrtDebug.h"
#include "Diagnostic/CurrentProcess.h"
#include "Diagnostic/Logger.h"
#include "IO/FileSystem.h"
#include "IO/VirtualFileSystem.h"
#include "Meta/AutoSingleton.h"
#include "RTTI/RTTI.h"
#include "Thread/ThreadContext.h"
#include "Thread/ThreadLocalStorage.h"
#include "Thread/ThreadPool.h"
#include "Time/ProcessTime.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if     defined(ARCH_X64)
static_assert(sizeof(size_t) == sizeof(uint64_t), "incoherent define");
#elif   defined(ARCH_X86)
static_assert(sizeof(size_t) == sizeof(uint32_t), "incoherent define");
#else
#   error "unknown architecture !"
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void CoreStartup::Start(void *applicationHandle, int nShowCmd, size_t argc, const wchar_t** argv) {
    // 1 - process time
    ProcessTime::Start();
    // 2 - logger
#ifdef USE_LOGGER
    LoggerStartup::Start();
#endif
    // 3 - current process
    CurrentProcess::Create(applicationHandle, nShowCmd, argc, argv);
    // 4 - call stack symbols
    Callstack::Start();
    // 5 - memory domains
    MemoryDomainStartup::Start();
    // 6 - memory guards
    GLOBAL_CHECK_MEMORY_LEAKS(true);
    // 7 - thread local storage
    ThreadLocalManager::Create();
    // 8 - main thread context
    ThreadContextStartup::Start_MainThread();
    // 9 - heap allocators
    Heaps::Process::Create(Heap::current_process_t());
    //10 - auto singleton manager
    Meta::AutoSingletonManager::Start();
    //11 - thread pool
    ThreadPoolStartup::Start();
    //12 - file system
    FileSystemStartup::Start();
    //13 - virtual file system
    VirtualFileSystemStartup::Start();
    //14 - RTTI
    RTTI::RTTIStartup::Start();
}
//----------------------------------------------------------------------------
void CoreStartup::Shutdown() {
    //14 - RTTI
    RTTI::RTTIStartup::Shutdown();
    //13 - virtual file system
    VirtualFileSystemStartup::Shutdown();
    //12 - file system
    FileSystemStartup::Shutdown();
    //11 - thread pool
    ThreadPoolStartup::Shutdown();
    //10 - auto singleton manager
    Meta::AutoSingletonManager::Shutdown();
    // 9 - heap allocators
    Heaps::Process::Destroy();
    // 8 - main thread context
    ThreadContextStartup::Shutdown();
    // 7 - thread local storage
    ThreadLocalManager::Destroy();
    // 6 - memory guards
    GLOBAL_CHECK_MEMORY_LEAKS(false);
    // 5 - memory domains
    MemoryDomainStartup::Shutdown();
    // 4 - call stack symbols
    Callstack::Shutdown();
    // 3 - current process
    CurrentProcess::Destroy();
    // 2 - logger
#ifdef USE_LOGGER
    LoggerStartup::Shutdown();
#endif
    // 1 - process time
    ProcessTime::Shutdown();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
