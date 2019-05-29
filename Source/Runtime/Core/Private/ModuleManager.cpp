#include "stdafx.h"

#include "ModuleManager.h"

#include "Module.h"

#include "Diagnostic/Logger.h"
#include "Memory/MemoryTracking.h"

#if USE_PPE_LOGGER
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
:   _appHandle(appHandle)
,   _showCmd(showCmd)
,   _filename(filename)
,   _argc(argc), _argv(argv)
,   _startup(nullptr) {
    Assert(nullptr == GModuleManager_);
    GModuleManager_ = this;

    STATIC_ASSERT(PP_NUM_ARGS() == 0);
    STATIC_ASSERT(PP_NUM_ARGS(1) == 1);
    STATIC_ASSERT(PP_NUM_ARGS(a, b) == 2);
    STATIC_ASSERT(PP_NUM_ARGS(a, b, c) == 3);

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
    LOG(Module, Emphasis, L"releasing memory in <{0}> ...", MakeCStringView(module.Name()));

#if USE_PPE_LOGGER
    const FTimedScope t;
#endif
    module.ReleaseMemory();

    LOG(Module, Debug, L" -> released memory in <{0}> during {1:f3}",
        MakeCStringView(module.Name()),
        Fmt::DurationInMs(t.Elapsed()) );
}
//----------------------------------------------------------------------------
void FModuleManager::PreInit(IModuleStartup& startup) {
    Assert(nullptr == _startup);

    LOG(Module, Emphasis, L"pre init module...");

    _startup = &startup;
}
//----------------------------------------------------------------------------
void FModuleManager::PostDestroy(IModuleStartup& startup) {
    Assert(&startup == _startup);

    LOG(Module, Emphasis, L"post destroy module...");

    _startup = nullptr;
}
//----------------------------------------------------------------------------
void FModuleManager::Start(FModule& module) {
    Assert(_startup);

    LOG(Module, Emphasis, L"starting module <{0}> ...", MakeCStringView(module.Name()));

#if USE_PPE_LOGGER
    const FTimedScope t;
#endif
    module.Start(*this);

    LOG(Module, Debug, L" -> started module <{0}> during {1:f3}",
        MakeCStringView(module.Name()),
        Fmt::DurationInMs(t.Elapsed()) );
}
//----------------------------------------------------------------------------
void FModuleManager::Shutdown(FModule& module) {
    Assert(_startup);

    LOG(Module, Emphasis, L"shutting down module <{0}> ...", MakeCStringView(module.Name()));

#if USE_PPE_LOGGER
    const FTimedScope t;
#endif
    module.Shutdown();

    LOG(Module, Debug, L" -> shutted down module <{0}> during {1:f3}",
        MakeCStringView(module.Name()),
        Fmt::DurationInMs(t.Elapsed()) );
}
//----------------------------------------------------------------------------
void FModuleManager::ReleaseMemoryInModules() const {
    Assert(_startup);

    LOG(Module, Emphasis, L"releasing memory in modules ...");
    LOG(Module, Info, L"used memory before release : {0} blocks / usr: {1} / sys: {2}",
        Fmt::CountOfElements(FMemoryTracking::UsedMemory().System().NumAllocs),
        Fmt::SizeInBytes(FMemoryTracking::UsedMemory().User().TotalSize),
        Fmt::SizeInBytes(FMemoryTracking::UsedMemory().System().TotalSize));
    LOG(Module, Info, L"pooled memory before release : {0} blocks / usr: {1} / sys: {2}",
        Fmt::CountOfElements(FMemoryTracking::PooledMemory().System().NumAllocs),
        Fmt::SizeInBytes(FMemoryTracking::PooledMemory().User().TotalSize),
        Fmt::SizeInBytes(FMemoryTracking::PooledMemory().System().TotalSize));
    LOG(Module, Info, L"reserved memory before release : {0} blocks / usr: {1} / sys: {2}",
        Fmt::CountOfElements(FMemoryTracking::ReservedMemory().System().NumAllocs),
        Fmt::SizeInBytes(FMemoryTracking::ReservedMemory().User().TotalSize),
        Fmt::SizeInBytes(FMemoryTracking::ReservedMemory().System().TotalSize));

    FLUSH_LOG();

#if USE_PPE_LOGGER
    const FTimedScope t;
#endif
    _startup->ReleaseMemory();

    LOG(Module, Debug, L" -> released memory in modules during {0:f3}",
        Fmt::DurationInMs(t.Elapsed()) );
    LOG(Module, Info, L"used memory after release : {0} blocks / usr: {1} / sys: {2}",
        Fmt::CountOfElements(FMemoryTracking::UsedMemory().System().NumAllocs),
        Fmt::SizeInBytes(FMemoryTracking::UsedMemory().User().TotalSize),
        Fmt::SizeInBytes(FMemoryTracking::UsedMemory().System().TotalSize));
    LOG(Module, Info, L"pooled memory after release : {0} blocks / usr: {1} / sys: {2}",
        Fmt::CountOfElements(FMemoryTracking::PooledMemory().System().NumAllocs),
        Fmt::SizeInBytes(FMemoryTracking::PooledMemory().User().TotalSize),
        Fmt::SizeInBytes(FMemoryTracking::PooledMemory().System().TotalSize));
    LOG(Module, Info, L"reserved memory after release : {0} blocks / usr: {1} / sys: {2}",
        Fmt::CountOfElements(FMemoryTracking::ReservedMemory().System().NumAllocs),
        Fmt::SizeInBytes(FMemoryTracking::ReservedMemory().User().TotalSize),
        Fmt::SizeInBytes(FMemoryTracking::ReservedMemory().System().TotalSize));
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
