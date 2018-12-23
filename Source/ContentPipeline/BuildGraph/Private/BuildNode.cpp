#include "stdafx.h"

#include "BuildNode.h"

#include "BuildGraph.h"

#include "RTTI_Macros-impl.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FBuildNode::FBuildNode(RTTI::FConstructorTag)
:   _pass(FBuildPass::Zero())
{}
//----------------------------------------------------------------------------
FBuildNode::FBuildNode(RTTI::FName&& name)
:   _name(std::move(name))
,   _state(EBuildState::Unbuilt)
,   _pass(FBuildPass::Zero())
,   _lastBuilt(0)
,   _fingerprint(FBuildFingerprint::Zero()) {
    Assert(not _name.empty());
}
//----------------------------------------------------------------------------
FBuildNode::~FBuildNode() {

}
//----------------------------------------------------------------------------
RTTI_CLASS_BEGIN(BuildGraph, FBuildNode, RTTI::EClassFlags::Abstract)
RTTI_PROPERTY_PRIVATE_READONLY(_name)
RTTI_PROPERTY_PRIVATE_READONLY(_state)
RTTI_PROPERTY_PRIVATE_READONLY(_lastBuilt)
RTTI_PROPERTY_PRIVATE_READONLY(_fingerprint)
RTTI_PROPERTY_PRIVATE_READONLY(_staticDeps)
RTTI_PROPERTY_PRIVATE_READONLY(_dynamicDeps)
RTTI_PROPERTY_PRIVATE_READONLY(_runtimeDeps)
RTTI_CLASS_END()
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
