#include "stdafx.h"

#include "ModuleExport.h"

#include "Allocator/Allocation.h"
#include "Allocator/StlAllocator.h"
#include "Diagnostic/Diagnostics.h"
#include "Diagnostic/Logger.h"
#include "IO/FileStream.h"
#include "IO/FileSystem.h"
#include "IO/TextWriter.h"
#include "Meta/AutoSingleton.h"
#include "ModuleManager.h"
#include "Thread/ThreadContext.h"
#include "Thread/ThreadPool.h"

#include <memory_resource>

#include "Module-impl.h"

#if PPE_OVERRIDE_NEW_ONCE
//  when compiling statically without inlined new operators it must be defined once in a separate TU
#   include "Allocator/New.Definitions-inl.h"
#endif

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
STATIC_ASSERT(std::allocator_traits<TStlAllocator<char, ALLOCATOR(Container)>>::propagate_on_container_copy_assignment::value);
STATIC_ASSERT(std::allocator_traits<TStlAllocator<char, ALLOCATOR(Container)>>::propagate_on_container_move_assignment::value);
STATIC_ASSERT(std::allocator_traits<TStlAllocator<char, ALLOCATOR(Container)>>::propagate_on_container_swap::value);
STATIC_ASSERT(std::allocator_traits<TStlAllocator<char, ALLOCATOR(Container)>>::is_always_equal::value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FCoreModule::FCoreModule()
:   FModule("Runtime/Core")
{}
//----------------------------------------------------------------------------
FCoreModule::~FCoreModule() = default;
//----------------------------------------------------------------------------
void FCoreModule::Start() {
    FModule::Start();

    // 0 - main thread context
    FThreadContextStartup::Start_MainThread();
    // 1 - low-level IO
    FFileStream::Start();
    // 2 - diagnostics
    FDiagnosticsStartup::Start();
    // 3 - auto singleton manager
    Meta::FAutoSingletonManager::Start();
    // 4 - thread pool
    FThreadPoolStartup::Start();
    // 5 - file system
    FFileSystemStartup::Start();
    // 6 - logger
#if USE_PPE_LOGGER
    FLogger::Start();
#endif
}
//----------------------------------------------------------------------------
void FCoreModule::Shutdown() {
    FModule::Shutdown();

    // 6 - logger
#if USE_PPE_LOGGER
    FLogger::Shutdown();
#endif
    // 5 - file system
    FFileSystemStartup::Shutdown();
    // 4 - thread pool
    FThreadPoolStartup::Shutdown();
    // 3 - auto singleton manager
    Meta::FAutoSingletonManager::Shutdown();
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

    // will release cached memory in every worker thread
    FThreadPoolStartup::ReleaseMemory();

    // release cached memory in current thread
    malloc_release_cache_memory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
