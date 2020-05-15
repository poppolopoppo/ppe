#include "stdafx.h"

#include "CoreModule.h"

#include "Modular/ModularDomain.h"
#include "Modular/ModuleRegistration.h"

#include "Allocator/Allocation.h"
#include "Allocator/StlAllocator.h"
#include "Diagnostic/Diagnostics.h"
#include "Diagnostic/Logger.h"
#include "IO/FileStream.h"
#include "IO/FileSystem.h"
#include "Meta/AutoSingleton.h"
#include "Thread/ThreadContext.h"
#include "Thread/ThreadPool.h"

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
const FModuleInfo FCoreModule::StaticInfo{
    FModuleStaticRegistration::MakeInfo<FCoreModule>(
        STRINGIZE(BUILD_TARGET_NAME),
        EModulePhase::Bare,
        EModuleUsage::Runtime,
        EModuleSource::Core,
        BUILD_TARGET_ORDINAL,
        FStringView{}/* always empty since this is the root module */)
};
//----------------------------------------------------------------------------
FCoreModule::FCoreModule() NOEXCEPT
:   IModuleInterface(StaticInfo)
{}
//----------------------------------------------------------------------------
void FCoreModule::Start(FModularDomain& domain) {
    IModuleInterface::Start(domain);

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
void FCoreModule::Shutdown(FModularDomain& domain) {
    IModuleInterface::Shutdown(domain);

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
void FCoreModule::DutyCycle(FModularDomain& domain) {
    IModuleInterface::DutyCycle(domain);

    // will release dangling blocks in every worker thread (keep cache)
    FThreadPoolStartup::ReleaseMemory();

    // release dangling block in current thread
    malloc_release_pending_blocks();
}
//----------------------------------------------------------------------------
void FCoreModule::ReleaseMemory(FModularDomain& domain) NOEXCEPT {
    IModuleInterface::ReleaseMemory(domain);

    // will release cached memory in every worker thread
    FThreadPoolStartup::ReleaseMemory();

    // release cached memory in current thread
    malloc_release_cache_memory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
