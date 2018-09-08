#include "stdafx.h"

#include "ModuleManager.h"

#include "Diagnostic/Logger.h"
#include "Memory/MemoryTracking.h"
#include "Module.h"
#include "Thread/ThreadContext.h"

#ifdef USE_DEBUG_LOGGER
#   include "IO/Format.h"
#   include "IO/FormatHelpers.h"
#   include "Time/TimedScope.h"
#endif

namespace PPE {
LOG_CATEGORY(, Module);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static FModuleManager* GModuleManager_ = nullptr;
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FModuleManager::FModuleManager(
    void* appHandle, int showCmd,
    const wchar_t* filename,
    size_t argc, const wchar_t** argv)
:   _appHandle(appHandle), _showCmd(showCmd)
,   _filename(filename)
,   _argc(argc), _argv(argv) {
    Assert(nullptr == GModuleManager_);
    GModuleManager_ = this;

    LOG(Module, Info, L"created module manager");
}
//----------------------------------------------------------------------------
FModuleManager::~FModuleManager() {
    Assert(this == GModuleManager_);
    GModuleManager_ = nullptr;

    LOG(Module, Info, L"destroyed module manager");
}
//----------------------------------------------------------------------------
void FModuleManager::ReleaseMemory(FModule& module) {
    //THIS_THREADRESOURCE_CHECKACCESS(); // not guaranteed/needed

    LOG(Module, Emphasis, L"releasing memory in <{0}> ...", MakeCStringView(module.Name()));

#ifdef USE_DEBUG_LOGGER
    const FTimedScope t;
#endif
    module.ReleaseMemory();

    LOG(Module, Debug, L" -> released memory in <{0}> during {1:f3}",
        MakeCStringView(module.Name()),
        Fmt::DurationInMs(t.Elapsed()) );
}
//----------------------------------------------------------------------------
void FModuleManager::PreInit(IModuleStartup& startup) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(nullptr == _startup);

    LOG(Module, Emphasis, L"pre init module startup ...");

    _startup = &startup;
}
//----------------------------------------------------------------------------
void FModuleManager::PostDestroy(IModuleStartup& startup) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(&startup == _startup);

    LOG(Module, Emphasis, L"post destroy module startup ...");

    _startup = nullptr;
}
//----------------------------------------------------------------------------
void FModuleManager::Start(FModule& module) {
    THIS_THREADRESOURCE_CHECKACCESS();

    LOG(Module, Emphasis, L"starting module <{0}> ...", MakeCStringView(module.Name()));

#ifdef USE_DEBUG_LOGGER
    const FTimedScope t;
#endif
    module.Start(*this);

    LOG(Module, Debug, L" -> started module <{0}> during {1:f3}",
        MakeCStringView(module.Name()),
        Fmt::DurationInMs(t.Elapsed()) );
}
//----------------------------------------------------------------------------
void FModuleManager::Shutdown(FModule& module) {
    THIS_THREADRESOURCE_CHECKACCESS();

    LOG(Module, Emphasis, L"shutting down module <{0}> ...", MakeCStringView(module.Name()));

#ifdef USE_DEBUG_LOGGER
    const FTimedScope t;
#endif
    module.Shutdown();

    LOG(Module, Debug, L" -> shutted down module <{0}> during {1:f3}",
        MakeCStringView(module.Name()),
        Fmt::DurationInMs(t.Elapsed()) );
}
//----------------------------------------------------------------------------
void FModuleManager::ReleaseMemoryInModules() const {
    //THIS_THREADRESOURCE_CHECKACCESS(); // not guaranteed/needed

    LOG(Module, Emphasis, L"releasing memory in modules ...");
    LOG(Module, Info, L"used memory before release : {0} blocks / {1}",
        Fmt::CountOfElements(FMemoryTracking::UsedMemory().AllocationCount()),
        Fmt::SizeInBytes(FMemoryTracking::UsedMemory().TotalSizeInBytes()) );
    LOG(Module, Info, L"pooled memory before release : {0} blocks / {1}",
        Fmt::CountOfElements(FMemoryTracking::PooledMemory().AllocationCount()),
        Fmt::SizeInBytes(FMemoryTracking::PooledMemory().TotalSizeInBytes()) );
    LOG(Module, Info, L"reserved memory before release : {0} blocks / {1}",
        Fmt::CountOfElements(FMemoryTracking::ReservedMemory().AllocationCount()),
        Fmt::SizeInBytes(FMemoryTracking::ReservedMemory().TotalSizeInBytes()) );

#ifdef USE_DEBUG_LOGGER
    const FTimedScope t;
#endif
    _startup->ReleaseMemory();

    LOG(Module, Debug, L" -> released memory in modules during {0:f3}",
        Fmt::DurationInMs(t.Elapsed()) );
    LOG(Module, Info, L"used memory after release : {0} blocks / {1}",
        Fmt::CountOfElements(FMemoryTracking::UsedMemory().AllocationCount()),
        Fmt::SizeInBytes(FMemoryTracking::UsedMemory().TotalSizeInBytes()) );
    LOG(Module, Info, L"pooled memory after release : {0} blocks / {1}",
        Fmt::CountOfElements(FMemoryTracking::PooledMemory().AllocationCount()),
        Fmt::SizeInBytes(FMemoryTracking::PooledMemory().TotalSizeInBytes()) );
    LOG(Module, Info, L"reserved memory after release : {0} blocks / {1}",
        Fmt::CountOfElements(FMemoryTracking::ReservedMemory().AllocationCount()),
        Fmt::SizeInBytes(FMemoryTracking::ReservedMemory().TotalSizeInBytes()) );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void ReleaseMemoryInModules() {
    Assert(GModuleManager_);

    GModuleManager_->ReleaseMemoryInModules();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
