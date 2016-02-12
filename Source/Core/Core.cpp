#include "stdafx.h"

#include "Core.h"

#include "Allocator/Allocation.h"
#include "Allocator/PoolAllocatorTag.h"
#include "Allocator/PoolAllocatorTag-impl.h"
#include "Allocator/NodeBasedContainerAllocator.h"
#include "Diagnostic/Callstack.h"
#include "Diagnostic/CrtDebug.h"
#include "Diagnostic/Logger.h"
#include "Diagnostic/MiniDump.h"
#include "Diagnostic/Profiling.h"
#include "IO/FileSystem.h"
#include "IO/VirtualFileSystem.h"
#include "Meta/AutoSingleton.h"
#include "Misc/CurrentProcess.h"
#include "Thread/Task/TaskPool.h"
#include "Thread/ThreadContext.h"
#include "Thread/ThreadPool.h"

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
    // 1 - logger
#ifdef USE_DEBUG_LOGGER
    LoggerStartup::Start();
#endif
    // 2 - current process
    CurrentProcess::Create(applicationHandle, nShowCmd, argc, argv);
    // 3 - call stack symbols
    Callstack::Start();
    // 4 - memory domains
    MemoryDomainStartup::Start();
    // 5 - memory guards
    GLOBAL_CHECK_MEMORY_LEAKS(true);
    // 6 - main thread context
    ThreadContextStartup::Start_MainThread();
    // 7 - heap allocators
    Heaps::Process::Create(Heap::current_process_t());
    // 8 - pool allocators
    POOLTAG(Default)::Start();
    POOLTAG(NodeBasedContainer)::Start();
    POOLTAG(TaskPool)::Start();
    // 9 - auto singleton manager
    Meta::AutoSingletonManager::Start();
    //10 - thread pool
    ThreadPoolStartup::Start();
    //11 - file system
    FileSystemStartup::Start();
    //12 - virtual file system
    VirtualFileSystemStartup::Start();
    //13 - minidump writer
    MiniDump::Start();
#ifdef WITH_CORE_PROFILING
    //14 - profiling
    Profiler::Startup();
#endif
}
//----------------------------------------------------------------------------
void CoreStartup::Shutdown() {
#ifdef WITH_CORE_PROFILING
    //14 - profiling
    Profiler::Shutdown();
#endif
    //13 - minidump writer
    MiniDump::Shutdown();
    //12 - virtual file system
    VirtualFileSystemStartup::Shutdown();
    //11 - file system
    FileSystemStartup::Shutdown();
    //10 - thread pool
    ThreadPoolStartup::Shutdown();
    // 9 - auto singleton manager
    Meta::AutoSingletonManager::Shutdown();
    // 8 - pool allocators
    POOLTAG(TaskPool)::Shutdown();
    POOLTAG(NodeBasedContainer)::Shutdown();
    POOLTAG(Default)::Shutdown();
    // 7 - heap allocators
    Heaps::Process::Destroy();
    // 6 - main thread context
    ThreadContextStartup::Shutdown();
    // 5 - memory guards
    GLOBAL_CHECK_MEMORY_LEAKS(false);
    // 4 - memory domains
    MemoryDomainStartup::Shutdown();
    // 3 - call stack symbols
    Callstack::Shutdown();
    // 2 - current process
    CurrentProcess::Destroy();
    // 1 - logger
#ifdef USE_DEBUG_LOGGER
    LoggerStartup::Shutdown();
#endif
}
//----------------------------------------------------------------------------
void CoreStartup::ClearAll_UnusedMemory() {
    POOLTAG(VirtualFileSystem)::ClearAll_UnusedMemory();
    POOLTAG(FileSystem)::ClearAll_UnusedMemory();
    POOLTAG(TaskPool)::ClearAll_UnusedMemory();
    POOLTAG(NodeBasedContainer)::ClearAll_UnusedMemory();
    POOLTAG(Default)::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
