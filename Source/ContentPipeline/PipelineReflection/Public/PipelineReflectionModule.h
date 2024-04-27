#pragma once

#include "PipelineReflection_fwd.h"

#include "Modular/ModuleInterface.h"

#include "Diagnostic/Logger_fwd.h"

namespace PPE {
namespace RHI {
EXTERN_LOG_CATEGORY(PPE_PIPELINEREFLECTION_API, PipelineReflection)
} //!namespace RHI
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_PIPELINEREFLECTION_API FPipelineReflectionModule final : public IModuleInterface {
public:
    static const FModuleInfo StaticInfo;

    explicit FPipelineReflectionModule() NOEXCEPT;

    virtual void Start(FModularDomain& domain) override;
    virtual void Shutdown(FModularDomain& domain) override;

    virtual void DutyCycle(FModularDomain& domain) override;
    virtual void ReleaseMemory(FModularDomain& domain) NOEXCEPT override;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
