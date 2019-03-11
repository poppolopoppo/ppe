#pragma once

#include "BuildGraph_fwd.h"

#include "Container/Vector.h"
#include "RTTI/Macros.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_BUILDGRAPH_API FBuildDependencies {
    FBuildFingerprint Fingerprint;
    VECTOR(BuildGraph, PBuildNode) Links;

    void Add(FBuildNode* node);
    bool DependsOn(const FBuildNode& node) const;
    bool Remove(const FBuildNode& node);
    void Reset();

    EBuildResult Build(FBuildContext& ctx) const;
};
RTTI_STRUCT_DECL(PPE_BUILDGRAPH_API, FBuildDependencies);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE