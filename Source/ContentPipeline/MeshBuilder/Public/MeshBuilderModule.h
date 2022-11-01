#pragma once

#include "MeshBuilder_fwd.h"

#include "Modular/ModuleInterface.h"

#include "Diagnostic/Logger_fwd.h"
#include "Misc/EventHandle.h"

namespace PPE {
namespace ContentPipeline {
EXTERN_LOG_CATEGORY(PPE_MESHBUILDER_API, MeshBuilder)
} //!namespace ContentPipeline
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_PIPELINECOMPILER_API FMeshBuilderModule final : public IModuleInterface {
public:
    static const FModuleInfo StaticInfo;

    explicit FMeshBuilderModule() NOEXCEPT;

    virtual void Start(FModularDomain& domain) override;
    virtual void Shutdown(FModularDomain& domain) override;

    virtual void DutyCycle(FModularDomain& domain) override;
    virtual void ReleaseMemory(FModularDomain& domain) NOEXCEPT override;

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
