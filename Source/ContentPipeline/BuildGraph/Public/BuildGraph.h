#pragma once

#include "BuildGraph_fwd.h"

#include "Memory/RefPtr.h"
#include "MetaObject.h"
#include "RTTI_Macros.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(BuildGraph);
class PPE_BUILDGRAPH_API FBuildGraph : public RTTI::FMetaObject {
public:
    FBuildGraph();
    ~FBuildGraph();

    FBuildGraph(const FBuildGraph&) = delete;
    FBuildGraph& operator =(const FBuildGraph&) = delete;

    RTTI_CLASS_HEADER(FBuildGraph, RTTI::FMetaObject);

private:

};
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
