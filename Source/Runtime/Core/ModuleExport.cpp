#include "stdafx.h"

#include "ModuleExport.h"

#include "Allocator/Allocation.h"
#include "Allocator/PoolAllocatorTag.h"
#include "Allocator/PoolAllocatorTag-impl.h"
#include "Allocator/NodeBasedContainerAllocator.h"
#include "Diagnostic/Diagnostics.h"
#include "Diagnostic/Logger.h"
#include "IO/FileStream.h"
#include "IO/FileSystem.h"
#include "IO/TextWriter.h"
#include "Meta/AutoSingleton.h"
#include "Thread/ThreadContext.h"
#include "Thread/ThreadPool.h"

#include "ModuleManager.h"

#include <memory_resource>

PRAGMA_INITSEG_LIB

namespace PPE {
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
STATIC_ASSERT(std::allocator_traits<DECORATE_ALLOCATOR(Container, TMallocator<int>)>::propagate_on_container_copy_assignment::value);
STATIC_ASSERT(std::allocator_traits<DECORATE_ALLOCATOR(Container, TMallocator<int>)>::propagate_on_container_move_assignment::value);
STATIC_ASSERT(std::allocator_traits<DECORATE_ALLOCATOR(Container, TMallocator<int>)>::propagate_on_container_swap::value);
STATIC_ASSERT(std::allocator_traits<DECORATE_ALLOCATOR(Container, TMallocator<int>)>::is_always_equal::value);
//----------------------------------------------------------------------------
STATIC_ASSERT(std::allocator_traits<DECORATE_ALLOCATOR(Container, TSingletonPoolAllocator<int>)>::propagate_on_container_copy_assignment::value);
STATIC_ASSERT(std::allocator_traits<DECORATE_ALLOCATOR(Container, TSingletonPoolAllocator<int>)>::propagate_on_container_move_assignment::value);
STATIC_ASSERT(std::allocator_traits<DECORATE_ALLOCATOR(Container, TSingletonPoolAllocator<int>)>::propagate_on_container_swap::value);
STATIC_ASSERT(std::allocator_traits<DECORATE_ALLOCATOR(Container, TSingletonPoolAllocator<int>)>::is_always_equal::value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FCoreModule::FCoreModule()
:   FModule("Runtime/Core")
{}
//----------------------------------------------------------------------------
FCoreModule::~FCoreModule()
{}
//----------------------------------------------------------------------------
void FCoreModule::Start(FModuleManager& manager) {
    FModule::Start(manager);

    // 0 - main thread context
    FThreadContextStartup::Start_MainThread();
    // 1 - low-level IO
    FFileStream::Start();
    // 2 - diagnostics
    FDiagnosticsStartup::Start(
        manager.AppHandle(), manager.ShowCmd(),
        manager.Filename(),
        manager.Argc(), manager.Argv() );
    // 3 - pool allocators
    POOL_TAG(Default)::Start();
    POOL_TAG(NodeBasedContainer)::Start();
    // 4 - auto singleton manager
    Meta::FAutoSingletonManager::Start();
    // 5 - thread pool
    FThreadPoolStartup::Start();
    // 6 - file system
    FFileSystemStartup::Start();
    // 7 - logger
#ifdef USE_DEBUG_LOGGER
    FLogger::Start();
#endif
}
//----------------------------------------------------------------------------
void FCoreModule::Shutdown() {
    FModule::Shutdown();

    // 7 - logger
#ifdef USE_DEBUG_LOGGER
    FLogger::Shutdown();
#endif
    // 6 - file system
    FFileSystemStartup::Shutdown();
    // 5 - thread pool
    FThreadPoolStartup::Shutdown();
    // 4 - auto singleton manager
    Meta::FAutoSingletonManager::Shutdown();
    // 3 - pool allocators
    POOL_TAG(NodeBasedContainer)::Shutdown();
    POOL_TAG(Default)::Shutdown();
    // 2 - diagnostics
    FDiagnosticsStartup::Shutdown();
    // 1 - low-level IO
    FFileStream::Shutdown();
    // 0 - main thread context
    FThreadContextStartup::Shutdown();
}
//----------------------------------------------------------------------------
void FCoreModule::ReleaseMemory() {
    FModule::ReleaseMemory();

    POOL_TAG(FileSystem)::ClearAll_UnusedMemory();
    POOL_TAG(NodeBasedContainer)::ClearAll_UnusedMemory();
    POOL_TAG(Default)::ClearAll_UnusedMemory();

    // will release cached memory in every worker thread
    FThreadPoolStartup::ReleaseMemory();

    // release cached memory in current thread
    malloc_release_cache_memory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
