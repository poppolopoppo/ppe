#pragma once

#include "BuildGraph_fwd.h"

#include "Container/HashMap.h"
#include "IO/Filename.h"

#include "RTTI/Namespace.h"

namespace PPE {
namespace ContentPipeline {
RTTI_NAMESPACE_DECL(PPE_BUILDGRAPH_API, BuildGraph);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_BUILDGRAPH_API FBuildGraph : Meta::FNonCopyableNorMovable {
public:
    FBuildGraph();
    virtual ~FBuildGraph();

    const HASHMAP(BuildGraph, RTTI::FName, PBuildNode)& Nodes() const { return _nodes; }

    void AddNode(const PBuildNode& node);
    void RemoveNode(const PBuildNode& node);

private:
    FFilename _sourceFile;
    HASHMAP(BuildGraph, RTTI::FName, PBuildNode) _nodes;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
