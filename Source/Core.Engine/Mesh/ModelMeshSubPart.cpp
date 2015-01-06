#include "stdafx.h"

#include "ModelMeshSubPart.h"

#include "Material/Material.h"

#include "Core/Allocator/PoolAllocator-impl.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_DEF(ModelMeshSubPart, );
//----------------------------------------------------------------------------
ModelMeshSubPart::ModelMeshSubPart(
    const Engine::Material *material,
    u32 boneIndex,
    u32 baseVertex,
    u32 firstIndex,
    u32 indexCount,
    const AABB3f& boundingBox )
:   _material(material)
,   _boneIndex(boneIndex)
,   _baseVertex(baseVertex)
,   _firstIndex(firstIndex)
,   _indexCount(indexCount)
,   _boundingBox(boundingBox) {
    Assert(material);
    Assert(indexCount > 0);
    Assert(boundingBox.HasPositiveExtents());
}
//----------------------------------------------------------------------------
ModelMeshSubPart::~ModelMeshSubPart() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
