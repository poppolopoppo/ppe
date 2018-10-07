#pragma once

#include "BuildGraph_fwd.h"

#ifndef PPE_STATICMODULES_STARTUP
#   error "ContentPipeline/BuildGraph/ModuleExport.h can't be included first !"
#endif

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_BUILDGRAPH_API FBuildGraphModule : public FModule {
public:
    FBuildGraphModule();
    virtual ~FBuildGraphModule();

protected:
    virtual void Start(FModuleManager& manager) override final;
    virtual void Shutdown() override final;
    virtual void ReleaseMemory() override final;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE

using FBuildGraphModule = PPE::ContentPipeline::FBuildGraphModule;
PPE_STATICMODULE_STARTUP_DEF(BuildGraph);

#undef PPE_STATICMODULES_STARTUP
#define PPE_STATICMODULES_STARTUP PPE_STATICMODULE_STARTUP_NAME(BuildGraph)
