#include "stdafx.h"

#include "BuildNode.h"

#include "BuildGraph.h"

#include "RTTI/Any.h"
#include "RTTI/Macros-impl.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
RTTI_CLASS_BEGIN(BuildGraph, FBuildNode, Abstract)
RTTI_PROPERTY_PRIVATE_FIELD(_opaqueData)
RTTI_PROPERTY_PRIVATE_READONLY(_staticDeps)
RTTI_PROPERTY_PRIVATE_READONLY(_dynamicDeps)
RTTI_PROPERTY_PRIVATE_READONLY(_runtimeDeps)
RTTI_CLASS_END()
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FBuildNode::FBuildNode() NOEXCEPT
{}
//----------------------------------------------------------------------------
FBuildNode::~FBuildNode()
{}
//----------------------------------------------------------------------------
void FBuildNode::AddStaticDep(FBuildNode* node) {
    Assert(node);

    Add_AssertUnique(_staticDeps, PBuildNode{ node });
}
//----------------------------------------------------------------------------
void FBuildNode::AddDynamicDep(FBuildNode* node) {
    Assert(node);

    Add_AssertUnique(_dynamicDeps, PBuildNode{ node });
}
//----------------------------------------------------------------------------
void FBuildNode::AddRuntimeDep(FBuildNode* node) {
    Assert(node);

    Add_AssertUnique(_runtimeDeps, PBuildNode{ node });
}
//----------------------------------------------------------------------------
void FBuildNode::AddStaticDeps(const TMemoryView<const SBuildNode>& nodes) {
    _staticDeps.reserve_Additional(nodes.size());
    for (const SBuildNode& n : nodes) {
        Assert(n);
        Add_AssertUnique(_staticDeps, PBuildNode{ n.get() });
    }
}
//----------------------------------------------------------------------------
void FBuildNode::AddDynamicDeps(const TMemoryView<const SBuildNode>& nodes) {
    _dynamicDeps.reserve_Additional(nodes.size());
    for (const SBuildNode& n : nodes) {
        Assert(n);
        Add_AssertUnique(_dynamicDeps, PBuildNode{ n.get() });
    }
}
//----------------------------------------------------------------------------
void FBuildNode::AddRuntimeDeps(const TMemoryView<const SBuildNode>& nodes) {
    _runtimeDeps.reserve_Additional(nodes.size());
    for (const SBuildNode& n : nodes) {
        Assert(n);
        Add_AssertUnique(_runtimeDeps, PBuildNode{ n.get() });
    }
}
//----------------------------------------------------------------------------
void FBuildNode::AddStaticDeps(const TMemoryView<const PBuildNode>& nodes) {
    _staticDeps.reserve_Additional(nodes.size());
    for (const PBuildNode& n : nodes) {
        Assert(n);
        Add_AssertUnique(_staticDeps, n);
    }
}
//----------------------------------------------------------------------------
void FBuildNode::AddDynamicDeps(const TMemoryView<const PBuildNode>& nodes) {
    _dynamicDeps.reserve_Additional(nodes.size());
    for (const PBuildNode& n : nodes) {
        Assert(n);
        Add_AssertUnique(_dynamicDeps, n);
    }
}
//----------------------------------------------------------------------------
void FBuildNode::AddRuntimeDeps(const TMemoryView<const PBuildNode>& nodes) {
    _runtimeDeps.reserve_Additional(nodes.size());
    for (const PBuildNode& n : nodes) {
        Assert(n);
        Add_AssertUnique(_runtimeDeps, n);
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
