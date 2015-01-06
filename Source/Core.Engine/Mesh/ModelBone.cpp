#include "stdafx.h"

#include "ModelBone.h"

#include "Core/Allocator/PoolAllocator-impl.h"
#include "Core/Maths/Transform/ScalarMatrixHelpers.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_DEF(ModelBone, );
//----------------------------------------------------------------------------
ModelBone::ModelBone(
    const MeshName& name,
    const MeshName& group,
    const float4x4& transform,
    const AABB3f& boundingBox )
:   _name(name)
,   _group(group)
,   _transform(transform)
,   _boundingBox(boundingBox) {
    Assert(!name.empty());
    Assert(!group.empty());
    Assert(IsHomogeneous(_transform));
    Assert(boundingBox.HasPositiveExtents());
}
//----------------------------------------------------------------------------
ModelBone::~ModelBone() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
