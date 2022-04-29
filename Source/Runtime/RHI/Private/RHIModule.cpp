#include "stdafx.h"

#include "RHIModule.h"

#include "RHI/EnumToString.h"
#include "RHI/FrameGraph.h"
#include "RHI/PipelineCompiler.h"

#include "Diagnostic/CurrentProcess.h"
#include "Diagnostic/Logger.h"
#include "IO/FormatHelpers.h"
#include "Memory/RefPtr.h"
#include "Modular/ModularDomain.h"
#include "Modular/ModuleRegistration.h"

#include "BuildModules.generated.h"
#include "Diagnostic/BuildVersion.h"

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
        Generated::DependencyList,
        CurrentBuildVersion() )
};
//----------------------------------------------------------------------------
FRHIModule& FRHIModule::Get(const FModularDomain& domain) {
    return domain.ModuleChecked<FRHIModule>(STRINGIZE(BUILD_TARGET_NAME));
}
//----------------------------------------------------------------------------
FRHIModule::FRHIModule() NOEXCEPT
:   IModuleInterface(StaticInfo)
,   _maxStagingBufferMemory(~0_b)
,   _stagingBufferSize(8_MiB) // #TODO: config layer?
{}
//----------------------------------------------------------------------------
FRHIModule::~FRHIModule() NOEXCEPT {
    Assert(_targets.empty());
    Assert(_compilers.empty());
}
//----------------------------------------------------------------------------
void FRHIModule::Start(FModularDomain& domain) {
    IModuleInterface::Start(domain);
}
//----------------------------------------------------------------------------
void FRHIModule::Shutdown(FModularDomain& domain) {
    IModuleInterface::Shutdown(domain);
}
//----------------------------------------------------------------------------
void FRHIModule::DutyCycle(FModularDomain& domain) {
    IModuleInterface::DutyCycle(domain);
}
//----------------------------------------------------------------------------
void FRHIModule::ReleaseMemory(FModularDomain& domain) NOEXCEPT {
    IModuleInterface::ReleaseMemory(domain);

    const FReadWriteLock::FScopeLockWrite slopeLock(_barrierRW);

    for (const auto& it : _compilers) {
        it.second->ReleaseUnusedMemory();
    }
}
//----------------------------------------------------------------------------
ERHIFeature FRHIModule::RecommendedFeatures(ERHIFeature features) const NOEXCEPT {
    const auto& process = FCurrentProcess::Get();

    if (process.HasArgument(L"-RHIHeadless"))
        features += ERHIFeature::Headless;
    if (process.HasArgument(L"-RHINoHeadless"))
        features -= ERHIFeature::Headless;

    if (process.HasArgument(L"-RHIHdr"))
        features += ERHIFeature::HighDynamicRange;
    if (process.HasArgument(L"-RHINoHdr"))
        features -= ERHIFeature::HighDynamicRange;

    if (process.HasArgument(L"-RHIVSync"))
        features += ERHIFeature::VSync;
    if (process.HasArgument(L"-RHINoVSync"))
        features -= ERHIFeature::VSync;

#if USE_PPE_RHIDEBUG
    features += ERHIFeature::Debugging;
#endif
#if USE_PPE_PROFILING
    features += ERHIFeature::Profiling;
#endif

#if !USE_PPE_FINAL_RELEASE
    if (process.HasArgument(L"-RHIDebug"))
        features += ERHIFeature::Debugging;
    if (process.HasArgument(L"-RHINoDebug"))
        features -= ERHIFeature::Debugging;
#endif

#if !USE_PPE_FINAL_RELEASE
    if (process.HasArgument(L"-RHIProfiling"))
        features += ERHIFeature::Debugging;
    if (process.HasArgument(L"-RHINoProfiling"))
        features -= ERHIFeature::Debugging;
#endif

    return features;
}
//----------------------------------------------------------------------------
void FRHIModule::RegisterTarget(ETargetRHI rhi, TPtrRef<const ITargetRHI>&& rtarget) {
    Assert(rtarget);

    const FReadWriteLock::FScopeLockWrite slopeLock(_barrierRW);

    using namespace RHI;
    LOG(RHI, Info, L"register target {0} for {1}", rtarget->DisplayName(), rhi);

    _OnRegisterTarget.Invoke(rhi, rtarget);

    _targets.Insert_AssertUnique(rhi, std::move(rtarget));
}
//----------------------------------------------------------------------------
void FRHIModule::UnregisterTarget(ETargetRHI rhi) {
    const FReadWriteLock::FScopeLockWrite slopeLock(_barrierRW);

    const auto it = _targets.find(rhi);
    AssertRelease(_targets.end() != it);

    using namespace RHI;
    LOG(RHI, Info, L"unregister target {0} for {1}", it->second->DisplayName(), rhi);

    _targets.Erase(it);

    _OnUnregisterTarget.Invoke(rhi, it->second);
}
//----------------------------------------------------------------------------
TPtrRef<const ITargetRHI> FRHIModule::Target() const NOEXCEPT {
    const FReadWriteLock::FScopeLockRead slopeLock(_barrierRW);
    return (_targets.empty() ? nullptr : _targets.Vector().back().second);
}
//----------------------------------------------------------------------------
TPtrRef<const ITargetRHI> FRHIModule::Target(ETargetRHI rhi) const NOEXCEPT {
    const FReadWriteLock::FScopeLockRead slopeLock(_barrierRW);
    const auto it = _targets.find(rhi);

    if (_targets.end() != it)
        return it->second;

    using namespace RHI;
    LOG(RHI, Error, L"could not find a target for {0}", rhi);
    return nullptr;
}
//----------------------------------------------------------------------------
void FRHIModule::RegisterCompiler(RHI::EShaderLangFormat lang, RHI::PPipelineCompiler&& rcompiler) {
    Assert(rcompiler);

    const FReadWriteLock::FScopeLockWrite slopeLock(_barrierRW);

    using namespace RHI;
    LOG(RHI, Info, L"register pipeline compiler {0} for {1}", rcompiler->DisplayName(), lang);

    _OnRegisterCompiler.Invoke(lang, rcompiler);

    _compilers.Insert_AssertUnique(lang, std::move(rcompiler));
}
//----------------------------------------------------------------------------
void FRHIModule::UnregisterCompiler(RHI::EShaderLangFormat lang) {
    const FReadWriteLock::FScopeLockWrite slopeLock(_barrierRW);

    const auto it = _compilers.find(lang);
    AssertRelease(_compilers.end() != it);

    using namespace RHI;
    LOG(RHI, Info, L"unregister pipeline compiler {0} for {1}", it->second->DisplayName(), lang);

    _compilers.Erase(it);

    _OnUnregisterCompiler.Invoke(lang, it->second);
}
//----------------------------------------------------------------------------
RHI::SPipelineCompiler FRHIModule::Compiler(RHI::EShaderLangFormat lang) const noexcept {
    const FReadWriteLock::FScopeLockRead slopeLock(_barrierRW);

    const auto view = _compilers.Vector().MakeConstView();
    const auto it = view.FindIf([lang](const auto& pair) {
        return (pair.first ^ lang);
    });

    if (view.end() != it)
        return it->second;

    using namespace RHI;
    LOG(RHI, Error, L"could not find a compiler for {0}", lang);
    return SPipelineCompiler{};
}
//----------------------------------------------------------------------------
void FRHIModule::ListCompilers(TAppendable<RHI::SPipelineCompiler>&& compilers) const NOEXCEPT {
    const FReadWriteLock::FScopeLockRead slopeLock(_barrierRW);

    for (const auto& it : _compilers)
        compilers.push_back(it.second);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
