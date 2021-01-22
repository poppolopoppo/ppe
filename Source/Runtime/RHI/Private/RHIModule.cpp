#include "stdafx.h"

#include "RHIModule.h"

#include "RHI/FrameGraph.h"
#include "RHI/PipelineCompiler.h"

#include "Diagnostic/Logger.h"
#include "IO/FormatHelpers.h"
#include "Modular/ModularDomain.h"
#include "Modular/ModuleRegistration.h"

#include "BuildModules.generated.h"
#include "Diagnostic/CurrentProcess.h"
#include "Memory/RefPtr.h"

namespace PPE {
namespace RHI {
LOG_CATEGORY(PPE_RHI_API, RHI)
} //!namespace RHI
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const FModuleInfo FRHIModule::StaticInfo{
    FModuleStaticRegistration::MakeInfo<FRHIModule>(
        STRINGIZE(BUILD_TARGET_NAME),
        EModulePhase::Bare,
        EModuleUsage::Runtime,
        EModuleSource::Core,
        BUILD_TARGET_ORDINAL,
        Generated::DependencyList )
};
//----------------------------------------------------------------------------
FRHIModule& FRHIModule::Get(const FModularDomain& domain) {
    return domain.ModuleChecked<FRHIModule>(STRINGIZE(BUILD_TARGET_NAME));
}
//----------------------------------------------------------------------------
FRHIModule::FRHIModule() NOEXCEPT
:   IModuleInterface(StaticInfo)
{}
//----------------------------------------------------------------------------
FRHIModule::~FRHIModule() NOEXCEPT {
    Assert(_targets.empty());
    Assert(_compilerFactories.empty());
    Assert(_frameGraphFactories.empty());
    Assert(_frameGraphs.empty());
}
//----------------------------------------------------------------------------
void FRHIModule::Start(FModularDomain& domain) {
    IModuleInterface::Start(domain);

    const auto& process = FCurrentProcess::Get();
    ERHIFeature features = ERHIFeature::Default;

    if (process.HasArgument(L"-RHIHeadless"))
        features += ERHIFeature::Headless;
    if (process.HasArgument(L"-RHINoHeadless"))
        features -= ERHIFeature::Headless;

    if (process.HasArgument(L"-RHIHdr"))
        features += ERHIFeature::HighDynamicRange;
    if (process.HasArgument(L"-RHINoHdr"))
        features -= ERHIFeature::HighDynamicRange;

#if USE_PPE_RHIDEBUG
    if (process.HasArgument(L"-RHIDebug"))
        features += ERHIFeature::Debugging;
    if (process.HasArgument(L"-RHINoDebug"))
        features -= ERHIFeature::Debugging;
#endif

#if USE_PPE_RHIPROFILING
    if (process.HasArgument(L"-RHIProfiling"))
        features += ERHIFeature::Debugging;
    if (process.HasArgument(L"-RHINoProfiling"))
        features -= ERHIFeature::Debugging;
#endif

    _systemFlags = features;
}
//----------------------------------------------------------------------------
void FRHIModule::Shutdown(FModularDomain& domain) {
    IModuleInterface::Shutdown(domain);

    _systemFlags = Default;
}
//----------------------------------------------------------------------------
void FRHIModule::DutyCycle(FModularDomain& domain) {
    IModuleInterface::DutyCycle(domain);

}
//----------------------------------------------------------------------------
void FRHIModule::ReleaseMemory(FModularDomain& domain) NOEXCEPT {
    IModuleInterface::ReleaseMemory(domain);

}
//----------------------------------------------------------------------------
void FRHIModule::RegisterTarget(ETargetRHI rhi, const ITargetRHI* target) {
    Assert(target);
    const FCriticalScope scope(&_barrier);

    _targets.Add(rhi) = target;
}
//----------------------------------------------------------------------------
void FRHIModule::UnregisterTarget(ETargetRHI rhi) {
    const FCriticalScope scope(&_barrier);

    _targets.Remove_AssertExists(rhi);
}
//----------------------------------------------------------------------------
const ITargetRHI* FRHIModule::Target(ETargetRHI rhi) const {
    const FCriticalScope scope(&_barrier);

    const auto it = _targets.find(rhi);
    return (it == _targets.end() ? nullptr : it->second);
}
//----------------------------------------------------------------------------
void FRHIModule::RegisterCompiler(ETargetRHI rhi, FPipelineCompilerFactory&& rfactory) {
    Assert(rfactory);
    const FCriticalScope scope(&_barrier);

    _compilerFactories.Add(rhi) = std::move(rfactory);
}
//----------------------------------------------------------------------------
void FRHIModule::UnregisterCompiler(ETargetRHI rhi) {
    const FCriticalScope scope(&_barrier);

    _compilerFactories.Remove_AssertExists(rhi);
}
//----------------------------------------------------------------------------
RHI::PPipelineCompiler FRHIModule::CreateCompiler() {
    return CreateCompiler(ETargetRHI::Current);
}
//----------------------------------------------------------------------------
RHI::PPipelineCompiler FRHIModule::CreateCompiler(ETargetRHI rhi) {
    const FCriticalScope scope(&_barrier);

    OnPipelineCompilerPreCreate_(rhi);

    if (const FPipelineCompilerFactory* const pfactory = _compilerFactories.GetIFP(rhi)) {
        RHI::PPipelineCompiler compiler;
        if ((*pfactory)(&compiler))
            Add_AssertUnique(_compilers, MakeSafePtr(compiler));
            OnPipelineCompilerPostCreate_(*compiler);
            return compiler;
    }

    AssertNotImplemented();
}
//----------------------------------------------------------------------------
void FRHIModule::DestroyCompiler(RHI::PPipelineCompiler& compiler) {
    Assert(compiler);
    const FCriticalScope scope(&_barrier);

    const ETargetRHI rhi = compiler->TargetRHI();
    OnPipelineCompilerPreDestroy_(*compiler);

    Remove_AssertExists(_compilers, MakeSafePtr(compiler));

    RemoveRef_AssertReachZero(compiler);

    OnPipelineCompilerPostDestroy_(rhi);
}
//----------------------------------------------------------------------------
void FRHIModule::RegisterFrameGraph(ETargetRHI rhi, FFrameGraphFactory&& rfactory) {
    Assert(rfactory);
    const FCriticalScope scope(&_barrier);

    _frameGraphFactories.Add(rhi) = std::move(rfactory);
}
//----------------------------------------------------------------------------
void FRHIModule::UnregisterFrameGraph(ETargetRHI rhi) {
    const FCriticalScope scope(&_barrier);

    _frameGraphFactories.Remove_AssertExists(rhi);
}
//----------------------------------------------------------------------------
RHI::PFrameGraph FRHIModule::CreateFrameGraph(ERHIFeature features, RHI::FWindowHandle mainWindow) {
    return CreateFrameGraph(ETargetRHI::Current, features, mainWindow);
}
//----------------------------------------------------------------------------
RHI::PFrameGraph FRHIModule::CreateFrameGraph(ETargetRHI rhi, ERHIFeature features, RHI::FWindowHandle mainWindow) {
    Assert((!!mainWindow) || (features & ERHIFeature::Headless)); // sanity checks
    Assert((!mainWindow) || !(features & ERHIFeature::Headless));

    const FCriticalScope scope(&_barrier);

    OnFrameGraphPreCreate_(rhi);

    if (const FFrameGraphFactory* const pfactory = _frameGraphFactories.GetIFP(rhi)) {
        RHI::PFrameGraph fg;
        if ((*pfactory)(&fg, features, mainWindow)) {
            Add_AssertUnique(_frameGraphs, MakeSafePtr(fg));
            OnFrameGraphPostCreate_(*fg);
            return fg;
        }
    }

    AssertNotReached();
}
//----------------------------------------------------------------------------
void FRHIModule::DestroyFrameGraph(RHI::PFrameGraph& fg) {
    Assert(fg);
    const FCriticalScope scope(&_barrier);

    const ETargetRHI rhi = fg->TargetRHI();
    OnFrameGraphPreDestroy_(*fg);

    Remove_AssertExists(_frameGraphs, MakeSafePtr(fg));

    fg->TearDown();

    RemoveRef_AssertReachZero(fg);

    OnFrameGraphPostDestroy_(rhi);
}
//----------------------------------------------------------------------------
void FRHIModule::OnPipelineCompilerPreCreate_(ETargetRHI rhi) {
    using namespace RHI;
    LOG(RHI, Info, L"broadcast pre-create pipeline compiler event for {0}RHI", rhi);
    _OnPipelineCompilerPreCreate(rhi);
}
//----------------------------------------------------------------------------
void FRHIModule::OnPipelineCompilerPostCreate_(RHI::IPipelineCompiler& compiler) {
    using namespace RHI;
    LOG(RHI, Info, L"broadcast post-create pipeline compiler event for {0}RHI <{1}>", compiler.TargetRHI(), Fmt::Pointer(&compiler));
    _OnPipelineCompilerPostCreate(compiler);
}
//----------------------------------------------------------------------------
void FRHIModule::OnPipelineCompilerPreDestroy_(RHI::IPipelineCompiler& compiler) {
    using namespace RHI;
    LOG(RHI, Info, L"broadcast pre-destroy pipeline compiler event for {0}RHI <{1}>", compiler.TargetRHI(), Fmt::Pointer(&compiler));
    _OnPipelineCompilerPreDestroy(compiler);
}
//----------------------------------------------------------------------------
void FRHIModule::OnPipelineCompilerPostDestroy_(ETargetRHI rhi) {
    using namespace RHI;
    LOG(RHI, Info, L"broadcast post-destroy pipeline compiler event for {0}RHI", rhi);
    _OnPipelineCompilerPostDestroy(rhi);
}
//----------------------------------------------------------------------------
void FRHIModule::OnFrameGraphPreCreate_(ETargetRHI rhi) {
    using namespace RHI;
    LOG(RHI, Info, L"broadcast pre-create framegraph event for {0}RHI", rhi);
    _OnFrameGraphPreCreate(rhi);
}
//----------------------------------------------------------------------------
void FRHIModule::OnFrameGraphPostCreate_(RHI::IFrameGraph& fg) {
    using namespace RHI;
    LOG(RHI, Info, L"broadcast post-create framegraph event for {0}RHI <{1}>", fg.TargetRHI(), Fmt::Pointer(&fg));
    _OnFrameGraphPostCreate(fg);
}
//----------------------------------------------------------------------------
void FRHIModule::OnFrameGraphPreDestroy_(RHI::IFrameGraph& fg) {
    using namespace RHI;
    LOG(RHI, Info, L"broadcast pre-destroy framegraph event for {0}RHI <{1}>", fg.TargetRHI(), Fmt::Pointer(&fg));
    _OnFrameGraphPreDestroy(fg);
}
//----------------------------------------------------------------------------
void FRHIModule::OnFrameGraphPostDestroy_(ETargetRHI rhi) {
    using namespace RHI;
    LOG(RHI, Info, L"broadcast post-destroy framegraph event for {0}RHI", rhi);
    _OnFrameGraphPostDestroy(rhi);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
