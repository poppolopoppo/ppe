// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

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
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
