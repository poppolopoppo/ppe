#pragma once

#include "RHI_fwd.h"

#include "HAL/TargetRHI.h"

#include "Modular/ModuleInterface.h"

#include "Container/AssociativeVector.h"
#include "Container/Vector.h"
#include "Diagnostic/Logger_fwd.h"
#include "Misc/Event.h"
#include "Misc/Function.h"
#include "RHI/FrameGraph.h"
#include "Thread/CriticalSection.h"

namespace PPE {
namespace RHI {
EXTERN_LOG_CATEGORY(PPE_RHI_API, RHI)
FWD_INTERFACE_REFPTR(FrameGraph);
FWD_INTERFACE_REFPTR(PipelineCompiler);
} //!namespace RHI
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHI_API FRHIModule final : public IModuleInterface {
public:
    static const FModuleInfo StaticInfo;

    FRHIModule() NOEXCEPT;
    ~FRHIModule() NOEXCEPT;

    virtual void Start(FModularDomain& domain) override;
    virtual void Shutdown(FModularDomain& domain) override;

    virtual void DutyCycle(FModularDomain& domain) override;
    virtual void ReleaseMemory(FModularDomain& domain) NOEXCEPT override;

public:
    static FRHIModule& Get(const FModularDomain& domain);

    ERHIFeature SystemFlags() const { return _systemFlags; }

    void RegisterTarget(ETargetRHI rhi, const ITargetRHI* target);
    void UnregisterTarget(ETargetRHI rhi);

    const ITargetRHI* Target(ETargetRHI rhi) const;

    using FPipelineCompilerFactory = TFunction<bool(RHI::PPipelineCompiler* pcompiler)>;

    void RegisterCompiler(ETargetRHI rhi, FPipelineCompilerFactory&& rfactory);
    void UnregisterCompiler(ETargetRHI rhi);

    RHI::PPipelineCompiler CreateCompiler();
    RHI::PPipelineCompiler CreateCompiler(ETargetRHI rhi);

    void DestroyCompiler(RHI::PPipelineCompiler& compiler);

    using FFrameGraphFactory = TFunction<bool(RHI::PFrameGraph* pfg, ERHIFeature features, RHI::FWindowHandle mainWindow)>;

    void RegisterFrameGraph(ETargetRHI rhi, FFrameGraphFactory&& rfactory);
    void UnregisterFrameGraph(ETargetRHI rhi);

    RHI::PFrameGraph CreateFrameGraph(ERHIFeature features = ERHIFeature::Default, RHI::FWindowHandle mainWindow = Default);
    RHI::PFrameGraph CreateFrameGraph(ETargetRHI rhi, ERHIFeature features = ERHIFeature::Default, RHI::FWindowHandle mainWindow = Default);

    void DestroyFrameGraph(RHI::PFrameGraph& fg);

    using FRHIEvent = TFunction<void(ETargetRHI)>;
    using FPipelineCompilerEvent = TFunction<void(RHI::IPipelineCompiler&)>;
    using FFrameGraphEvent = TFunction<void(RHI::IFrameGraph&)>;

    PUBLIC_EVENT(OnPipelineCompilerPreCreate, FRHIEvent);
    PUBLIC_EVENT(OnPipelineCompilerPostCreate, FPipelineCompilerEvent);

    PUBLIC_EVENT(OnPipelineCompilerPreDestroy, FPipelineCompilerEvent);
    PUBLIC_EVENT(OnPipelineCompilerPostDestroy, FRHIEvent);

    PUBLIC_EVENT(OnFrameGraphPreCreate, FRHIEvent);
    PUBLIC_EVENT(OnFrameGraphPostCreate, FFrameGraphEvent);

    PUBLIC_EVENT(OnFrameGraphPreDestroy, FFrameGraphEvent);
    PUBLIC_EVENT(OnFrameGraphPostDestroy, FRHIEvent);

private:
    FCriticalSection _barrier;

    ERHIFeature _systemFlags{ Default };

    ASSOCIATIVE_VECTORINSITU(RHIMisc, ETargetRHI, const ITargetRHI*, u32(ETargetRHI::_Count)) _targets;
    ASSOCIATIVE_VECTORINSITU(RHIMisc, ETargetRHI, FPipelineCompilerFactory, u32(ETargetRHI::_Count)) _compilerFactories;
    ASSOCIATIVE_VECTORINSITU(RHIMisc, ETargetRHI, FFrameGraphFactory, u32(ETargetRHI::_Count)) _frameGraphFactories;

    VECTORINSITU(RHIMisc, RHI::SPipelineCompiler, 2) _compilers;
    VECTORINSITU(RHIMisc, RHI::SFrameGraph, 2) _frameGraphs;

    void OnPipelineCompilerPreCreate_(ETargetRHI rhi);
    void OnPipelineCompilerPostCreate_(RHI::IPipelineCompiler& compiler);
    void OnPipelineCompilerPreDestroy_(RHI::IPipelineCompiler& compiler);
    void OnPipelineCompilerPostDestroy_(ETargetRHI rhi);

    void OnFrameGraphPreCreate_(ETargetRHI rhi);
    void OnFrameGraphPostCreate_(RHI::IFrameGraph& fg);
    void OnFrameGraphPreDestroy_(RHI::IFrameGraph& fg);
    void OnFrameGraphPostDestroy_(ETargetRHI rhi);

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
