#include "stdafx.h"

#include "Core.h"

#include "Allocator/Allocation.h"
#include "Allocator/PoolAllocatorTag.h"
#include "Allocator/PoolAllocatorTag-impl.h"
#include "Allocator/NodeBasedContainerAllocator.h"
#include "Diagnostic/Diagnostics.h"
#include "IO/FileSystem.h"
#include "IO/VirtualFileSystem.h"
#include "Meta/AutoSingleton.h"
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
STATIC_ASSERT(std::allocator_traits<DECORATE_ALLOCATOR(Container, Mallocator<int>)>::propagate_on_container_copy_assignment::value);
STATIC_ASSERT(std::allocator_traits<DECORATE_ALLOCATOR(Container, Mallocator<int>)>::propagate_on_container_move_assignment::value);
STATIC_ASSERT(std::allocator_traits<DECORATE_ALLOCATOR(Container, Mallocator<int>)>::propagate_on_container_swap::value);
STATIC_ASSERT(std::allocator_traits<DECORATE_ALLOCATOR(Container, Mallocator<int>)>::is_always_equal::value);
//----------------------------------------------------------------------------
STATIC_ASSERT(std::allocator_traits<DECORATE_ALLOCATOR(Container, HeapAllocator<int>)>::propagate_on_container_copy_assignment::value);
STATIC_ASSERT(std::allocator_traits<DECORATE_ALLOCATOR(Container, HeapAllocator<int>)>::propagate_on_container_move_assignment::value);
STATIC_ASSERT(std::allocator_traits<DECORATE_ALLOCATOR(Container, HeapAllocator<int>)>::propagate_on_container_swap::value);
STATIC_ASSERT(std::allocator_traits<DECORATE_ALLOCATOR(Container, HeapAllocator<int>)>::is_always_equal::value);
//----------------------------------------------------------------------------
STATIC_ASSERT(std::allocator_traits<DECORATE_ALLOCATOR(Container, SingletonPoolAllocator<int>)>::propagate_on_container_copy_assignment::value);
STATIC_ASSERT(std::allocator_traits<DECORATE_ALLOCATOR(Container, SingletonPoolAllocator<int>)>::propagate_on_container_move_assignment::value);
STATIC_ASSERT(std::allocator_traits<DECORATE_ALLOCATOR(Container, SingletonPoolAllocator<int>)>::propagate_on_container_swap::value);
STATIC_ASSERT(std::allocator_traits<DECORATE_ALLOCATOR(Container, SingletonPoolAllocator<int>)>::is_always_equal::value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void CoreStartup::Start(void *applicationHandle, int nShowCmd, size_t argc, const wchar_t** argv) {
    CORE_MODULE_START(Core);

    // 1 - diagnostics
    DiagnosticsStartup::Start(applicationHandle, nShowCmd, argc, argv);
    // 2 - main thread context
    ThreadContextStartup::Start_MainThread();
    // 3 - heap allocators
    Heaps::Process::Create(Heap::current_process_t());
    // 4 - pool allocators
    POOL_TAG(Default)::Start();
    POOL_TAG(NodeBasedContainer)::Start();
    // 5 - auto singleton manager
    Meta::AutoSingletonManager::Start();
    // 6 - thread pool
    ThreadPoolStartup::Start();
    // 7 - file system
    FileSystemStartup::Start();
    // 8 - virtual file system
    VirtualFileSystemStartup::Start();
}
//----------------------------------------------------------------------------
void CoreStartup::Shutdown() {
    CORE_MODULE_SHUTDOWN(Core);

    // 8 - virtual file system
    VirtualFileSystemStartup::Shutdown();
    // 7 - file system
    FileSystemStartup::Shutdown();
    // 6 - thread pool
    ThreadPoolStartup::Shutdown();
    // 5 - auto singleton manager
    Meta::AutoSingletonManager::Shutdown();
    // 4 - pool allocators
    POOL_TAG(NodeBasedContainer)::Shutdown();
    POOL_TAG(Default)::Shutdown();
    // 3 - heap allocators
    Heaps::Process::Destroy();
    // 2 - main thread context
    ThreadContextStartup::Shutdown();
    // 1 - logger
    DiagnosticsStartup::Shutdown();
}
//----------------------------------------------------------------------------
void CoreStartup::ClearAll_UnusedMemory() {
    CORE_MODULE_CLEARALL(Core);

    POOL_TAG(VirtualFileSystem)::ClearAll_UnusedMemory();
    POOL_TAG(FileSystem)::ClearAll_UnusedMemory();
    POOL_TAG(NodeBasedContainer)::ClearAll_UnusedMemory();
    POOL_TAG(Default)::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifndef FINAL_RELEASE
//----------------------------------------------------------------------------
void CheckMemory() {
    static THREAD_LOCAL bool bInScope = false;
    if (bInScope)
        return;

    bInScope = true;
    AssertRelease(!std::cout.bad());
    AssertRelease(!std::wcout.bad());
    AssertRelease(!std::cerr.bad());
    AssertRelease(!std::wcerr.bad());
    AssertRelease(!std::cin.bad());
    AssertRelease(!std::wcin.bad());
#ifdef OS_WINDOWS
    _CrtCheckMemory();
#endif
    bInScope = false;
}
//----------------------------------------------------------------------------
OnModuleStart::OnModuleStart(const wchar_t* moduleName) : ModuleName(moduleName) {
    LOG(Info, L"[{0}] Begin start module", ModuleName);
    CheckMemory();
}
//----------------------------------------------------------------------------
OnModuleStart::~OnModuleStart() {
    LOG(Info, L"[{0}] End start module", ModuleName);
    CheckMemory();
}
//----------------------------------------------------------------------------
OnModuleShutdown::OnModuleShutdown(const wchar_t* moduleName) : ModuleName(moduleName) {
    LOG(Info, L"[{0}] Begin shutdown module", ModuleName);
    CheckMemory();
}
//----------------------------------------------------------------------------
OnModuleShutdown::~OnModuleShutdown() {
    LOG(Info, L"[{0}] End shutdown module", ModuleName);
    CheckMemory();
}
//----------------------------------------------------------------------------
OnModuleClearAll::OnModuleClearAll(const wchar_t* moduleName) : ModuleName(moduleName) {
    LOG(Info, L"[{0}] Begin clear all module", ModuleName);
    CheckMemory();
}
//----------------------------------------------------------------------------
OnModuleClearAll::~OnModuleClearAll() {
    LOG(Info, L"[{0}] End clear all module", ModuleName);
    CheckMemory();
}
//----------------------------------------------------------------------------
#endif //!FINAL_RELEASE
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
