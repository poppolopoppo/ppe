// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "ModelBone.h"

#include "Core/Allocator/PoolAllocator-impl.h"
#include "Core/Maths/ScalarMatrixHelpers.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_TAGGED_DEF(Engine, FModelBone, );
//----------------------------------------------------------------------------
FModelBone::FModelBone(
    const FMeshName& name,
    const FMeshName& group,
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
FModelBone::~FModelBone() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
