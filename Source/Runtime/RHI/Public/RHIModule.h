#pragma once

#include "RHI_fwd.h"

#include "HAL/TargetRHI.h"

#include "Modular/ModuleInterface.h"

#include "Container/Appendable.h"
#include "Container/AssociativeVector.h"
#include "Diagnostic/Logger_fwd.h"
#include "Misc/Event.h"
#include "Misc/Function.h"
#include "Thread/ReadWriteLock.h"

namespace PPE {
namespace RHI {
EXTERN_LOG_CATEGORY(PPE_RHI_API, RHI)
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

    ERHIFeature RecommendedFeatures(ERHIFeature) const NOEXCEPT;

    size_t MaxStagingBufferMemory() const { return _maxStagingBufferMemory; }
    size_t StagingBufferSize() const { return _stagingBufferSize; }

    // you can limit max size of host visible memory that may be used by FrameGraph, by default used max available size.
    void SetMaxStagingBufferMemory(size_t value) { _maxStagingBufferMemory = value; }

    // max size of single staging buffer (needed for tests), 0 - auto
    void SetStagingBufferSize(size_t value) { _stagingBufferSize = value; }

    // --- TargetRHI ---

    using FTargetEvent = TFunction<void(ETargetRHI rhi, TPtrRef<const ITargetRHI>)>;

    PUBLIC_EVENT(OnRegisterTarget, FTargetEvent);
    PUBLIC_EVENT(OnUnregisterTarget, FTargetEvent);

    TPtrRef<const ITargetRHI> Target() const NOEXCEPT; // pick first available
    TPtrRef<const ITargetRHI> Target(ETargetRHI rhi) const NOEXCEPT;

    void RegisterTarget(ETargetRHI rhi, TPtrRef<const ITargetRHI>&& rtarget);
    void UnregisterTarget(ETargetRHI rhi);

    // --- PipelineCompiler --

    using FCompilerEvent = TFunction<void(RHI::EShaderLangFormat, const RHI::PPipelineCompiler&)>;

    PUBLIC_EVENT(OnRegisterCompiler, FCompilerEvent);
    PUBLIC_EVENT(OnUnregisterCompiler, FCompilerEvent);

    RHI::SPipelineCompiler Compiler(RHI::EShaderLangFormat lang) const NOEXCEPT;
    void ListCompilers(TAppendable<RHI::SPipelineCompiler>&& compilers) const NOEXCEPT;

    void RegisterCompiler(RHI::EShaderLangFormat lang, RHI::PPipelineCompiler&& rcompiler);
    void UnregisterCompiler(RHI::EShaderLangFormat lang);

private:
    FReadWriteLock _barrierRW;

    ASSOCIATIVE_VECTORINSITU(RHIMisc, ETargetRHI, TPtrRef<const ITargetRHI>, static_cast<u32>(ETargetRHI::_Count)) _targets;
    ASSOCIATIVE_VECTORINSITU(RHIMisc, RHI::EShaderLangFormat, RHI::PPipelineCompiler, static_cast<u32>(ETargetRHI::_Count)) _compilers;

    size_t _maxStagingBufferMemory;
    size_t _stagingBufferSize;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
