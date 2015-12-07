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
#include "IO/FileSystem.h"
#include "IO/VirtualFileSystem.h"
#include "Meta/AutoSingleton.h"
#include "Misc/CurrentProcess.h"
#include "Thread/Task/TaskPool.h"
#include "Thread/ThreadContext.h"
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
    // 7 - main thread context
    ThreadContextStartup::Start_MainThread();
    // 8 - heap allocators
    Heaps::Process::Create(Heap::current_process_t());
    // 9 - pool allocators
    POOLTAG(Default)::Start();
    POOLTAG(NodeBasedContainer)::Start();
    POOLTAG(TaskPool)::Start();
    //10 - auto singleton manager
    Meta::AutoSingletonManager::Start();
    //11 - thread pool
    ThreadPoolStartup::Start();
    //12 - file system
    FileSystemStartup::Start();
    //13 - virtual file system
    VirtualFileSystemStartup::Start();
    //14 - minidump writer
    MiniDump::Start();
}
//----------------------------------------------------------------------------
void CoreStartup::Shutdown() {
    //14 - minidump writer
    MiniDump::Shutdown();
    //13 - virtual file system
    VirtualFileSystemStartup::Shutdown();
    //12 - file system
    FileSystemStartup::Shutdown();
    //11 - thread pool
    ThreadPoolStartup::Shutdown();
    //10 - auto singleton manager
    Meta::AutoSingletonManager::Shutdown();
    // 9 - pool allocators
    POOLTAG(TaskPool)::Shutdown();
    POOLTAG(NodeBasedContainer)::Shutdown();
    POOLTAG(Default)::Shutdown();
    // 8 - heap allocators
    Heaps::Process::Destroy();
    // 7 - main thread context
    ThreadContextStartup::Shutdown();
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
