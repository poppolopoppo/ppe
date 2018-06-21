#include "stdafx.h"

#include "Core.h"

#include "Allocator/Allocation.h"
#include "Allocator/PoolAllocatorTag.h"
#include "Allocator/PoolAllocatorTag-impl.h"
#include "Allocator/NodeBasedContainerAllocator.h"
#include "Diagnostic/Diagnostics.h"
#include "Diagnostic/Logger.h"
#include "IO/FileStream.h"
#include "IO/FileSystem.h"
#include "IO/TextWriter.h"
#include "IO/VirtualFileSystem.h"
#include "Meta/AutoSingleton.h"
#include "Misc/TargetPlatform.h"
#include "Thread/ThreadContext.h"
#include "Thread/ThreadPool.h"

PRAGMA_INITSEG_LIB

namespace Core {
LOG_CATEGORY(CORE_API, Module)
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

    FPlatformMisc::CheckMemory();

    bInScope = false;
}
#endif //!FINAL_RELEASE
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FCoreModule::Start(void *applicationHandle, int nShowCmd, const wchar_t* filename, size_t argc, const wchar_t** argv) {
    CORE_MODULE_START(Core);

    // 0 - main thread context
    FThreadContextStartup::Start_MainThread();
    // 1 - low-level IO
    FFileStream::Start();
    // 2 - diagnostics
    FDiagnosticsStartup::Start(applicationHandle, nShowCmd, filename, argc, argv);
    // 3 - pool allocators
    POOL_TAG(Default)::Start();
    POOL_TAG(NodeBasedContainer)::Start();
    // 4 - auto singleton manager
    Meta::FAutoSingletonManager::Start();
    // 5 - thread pool
    FThreadPoolStartup::Start();
    // 6 - file system
    FFileSystemStartup::Start();
    // 7 - virtual file system
    FVirtualFileSystemStartup::Start();
    // 8 - logger
#ifdef USE_DEBUG_LOGGER
    FLogger::Start();
#endif
}
//----------------------------------------------------------------------------
void FCoreModule::Shutdown() {
    CORE_MODULE_SHUTDOWN(Core);

    // 8 - logger
#ifdef USE_DEBUG_LOGGER
    FLogger::Shutdown();
#endif
    // 7 - virtual file system
    FVirtualFileSystemStartup::Shutdown();
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
OnModuleStart::OnModuleStart(const wchar_t* moduleName) : ModuleName(moduleName) {
    LOG(Module, Emphasis, L"begin start module {0}", ModuleName);
    CheckMemory_();
}
//----------------------------------------------------------------------------
OnModuleStart::~OnModuleStart() {
    LOG(Module, Emphasis, L"end start module {0}", ModuleName);
    CheckMemory_();
}
//----------------------------------------------------------------------------
OnModuleShutdown::OnModuleShutdown(const wchar_t* moduleName) : ModuleName(moduleName) {
    LOG(Module, Emphasis, L"begin shutdown module {0}", ModuleName);
    CheckMemory_();
}
//----------------------------------------------------------------------------
OnModuleShutdown::~OnModuleShutdown() {
    LOG(Module, Emphasis, L"end shutdown module {0}", ModuleName);
    CheckMemory_();
}
//----------------------------------------------------------------------------
OnModuleClearAll::OnModuleClearAll(const wchar_t* moduleName) : ModuleName(moduleName) {
    LOG(Module, Emphasis, L"begin clear all module {0}", ModuleName);
    CheckMemory_();
}
//----------------------------------------------------------------------------
OnModuleClearAll::~OnModuleClearAll() {
    LOG(Module, Emphasis, L"end clear all module {0}", ModuleName);
    CheckMemory_();
}
//----------------------------------------------------------------------------
#endif //!FINAL_RELEASE
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
