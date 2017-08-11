#include "stdafx.h"

#include "Core.h"

#include "Allocator/Allocation.h"
#include "Allocator/PoolAllocatorTag.h"
#include "Allocator/PoolAllocatorTag-impl.h"
#include "Allocator/NodeBasedContainerAllocator.h"
#include "Diagnostic/Diagnostics.h"
#include "Diagnostic/Logger.h"
#include "IO/FileSystem.h"
#include "IO/VirtualFileSystem.h"
#include "Meta/AutoSingleton.h"
#include "Misc/TargetPlatform.h"
#include "Thread/ThreadContext.h"
#include "Thread/ThreadPool.h"

PRAGMA_INITSEG_LIB

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// VS2013 and VS2015 are still not C++-11 compliant ...
//static_assert(__cplusplus > 199711L, "Program requires C++11 capable compiler");
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
STATIC_ASSERT(std::allocator_traits<DECORATE_ALLOCATOR(Container, THeapAllocator<int>)>::propagate_on_container_copy_assignment::value);
STATIC_ASSERT(std::allocator_traits<DECORATE_ALLOCATOR(Container, THeapAllocator<int>)>::propagate_on_container_move_assignment::value);
STATIC_ASSERT(std::allocator_traits<DECORATE_ALLOCATOR(Container, THeapAllocator<int>)>::propagate_on_container_swap::value);
STATIC_ASSERT(std::allocator_traits<DECORATE_ALLOCATOR(Container, THeapAllocator<int>)>::is_always_equal::value);
//----------------------------------------------------------------------------
STATIC_ASSERT(std::allocator_traits<DECORATE_ALLOCATOR(Container, TSingletonPoolAllocator<int>)>::propagate_on_container_copy_assignment::value);
STATIC_ASSERT(std::allocator_traits<DECORATE_ALLOCATOR(Container, TSingletonPoolAllocator<int>)>::propagate_on_container_move_assignment::value);
STATIC_ASSERT(std::allocator_traits<DECORATE_ALLOCATOR(Container, TSingletonPoolAllocator<int>)>::propagate_on_container_swap::value);
STATIC_ASSERT(std::allocator_traits<DECORATE_ALLOCATOR(Container, TSingletonPoolAllocator<int>)>::is_always_equal::value);
//----------------------------------------------------------------------------
#ifndef FINAL_RELEASE
static void CheckMemory_() {
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

    FPlatform::CheckMemory();

    bInScope = false;
}
#endif //!FINAL_RELEASE
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FCoreModule::Start(void *applicationHandle, int nShowCmd, const wchar_t* filename, size_t argc, const wchar_t** argv) {
    CORE_MODULE_START(Core);

    // 1 - diagnostics
    FDiagnosticsStartup::Start(applicationHandle, nShowCmd, filename, argc, argv);
    // 2 - main thread context
    FThreadContextStartup::Start_MainThread();
    // 3 - heap allocators
    Heaps::Process::Create(FHeap::current_process_t());
    // 4 - pool allocators
    POOL_TAG(Default)::Start();
    POOL_TAG(NodeBasedContainer)::Start();
    // 5 - auto singleton manager
    Meta::FAutoSingletonManager::Start();
    // 6 - thread pool
    FThreadPoolStartup::Start();
    // 7 - file system
    FFileSystemStartup::Start();
    // 8 - virtual file system
    FVirtualFileSystemStartup::Start();
    // 9 - logger
#ifdef USE_DEBUG_LOGGER
    FLoggerStartup::Start();
#endif
}
//----------------------------------------------------------------------------
void FCoreModule::Shutdown() {
    CORE_MODULE_SHUTDOWN(Core);

    // 9 - logger
#ifdef USE_DEBUG_LOGGER
    FLoggerStartup::Shutdown();
#endif
    // 8 - virtual file system
    FVirtualFileSystemStartup::Shutdown();
    // 7 - file system
    FFileSystemStartup::Shutdown();
    // 6 - thread pool
    FThreadPoolStartup::Shutdown();
    // 5 - auto singleton manager
    Meta::FAutoSingletonManager::Shutdown();
    // 4 - pool allocators
    POOL_TAG(NodeBasedContainer)::Shutdown();
    POOL_TAG(Default)::Shutdown();
    // 3 - heap allocators
    Heaps::Process::Destroy();
    // 2 - main thread context
    FThreadContextStartup::Shutdown();
    // 1 - logger
    FDiagnosticsStartup::Shutdown();
}
//----------------------------------------------------------------------------
void FCoreModule::ClearAll_UnusedMemory() {
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
static void CheckMemory_() {
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

    FPlatform::CheckMemory();

    bInScope = false;
}
//----------------------------------------------------------------------------
OnModuleStart::OnModuleStart(const wchar_t* moduleName) : ModuleName(moduleName) {
    LOG(Info, L"[{0}] Begin start module", ModuleName);
    CheckMemory_();
}
//----------------------------------------------------------------------------
OnModuleStart::~OnModuleStart() {
    LOG(Info, L"[{0}] End start module", ModuleName);
    CheckMemory_();
}
//----------------------------------------------------------------------------
OnModuleShutdown::OnModuleShutdown(const wchar_t* moduleName) : ModuleName(moduleName) {
    LOG(Info, L"[{0}] Begin shutdown module", ModuleName);
    CheckMemory_();
}
//----------------------------------------------------------------------------
OnModuleShutdown::~OnModuleShutdown() {
    LOG(Info, L"[{0}] End shutdown module", ModuleName);
    CheckMemory_();
}
//----------------------------------------------------------------------------
OnModuleClearAll::OnModuleClearAll(const wchar_t* moduleName) : ModuleName(moduleName) {
    LOG(Info, L"[{0}] Begin clear all module", ModuleName);
    CheckMemory_();
}
//----------------------------------------------------------------------------
OnModuleClearAll::~OnModuleClearAll() {
    LOG(Info, L"[{0}] End clear all module", ModuleName);
    CheckMemory_();
}
//----------------------------------------------------------------------------
#endif //!FINAL_RELEASE
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
