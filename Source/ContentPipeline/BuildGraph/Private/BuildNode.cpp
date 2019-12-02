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
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
