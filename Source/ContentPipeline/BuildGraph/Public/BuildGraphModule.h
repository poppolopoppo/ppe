#pragma once

#include "BuildGraph_fwd.h"

#include "Modular/ModuleInterface.h"

#include "Diagnostic/Logger_fwd.h"

namespace PPE {
namespace ContentPipeline {
EXTERN_LOG_CATEGORY(PPE_BUILDGRAPH_API, BuildGraph)
} //!namespace ContentPipeline
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_BUILDGRAPH_API FBuildGraphModule final : public IModuleInterface {
public:
    static const FModuleInfo StaticInfo;

    explicit FBuildGraphModule() NOEXCEPT;

    virtual void Start(FModularDomain& domain) override;
    virtual void Shutdown(FModularDomain& domain) override;

    virtual void DutyCycle(FModularDomain& domain) override;
    virtual void ReleaseMemory(FModularDomain& domain) NOEXCEPT override;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
