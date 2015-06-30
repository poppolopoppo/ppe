#include "stdafx.h"

#include "ModelMeshSubPart.h"

#include "Material/Material.h"

#include "Core/Allocator/PoolAllocator-impl.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_TAGGED_DEF(Engine, ModelMeshSubPart, );
//----------------------------------------------------------------------------
ModelMeshSubPart::ModelMeshSubPart(
    const MeshName& name,
    u32 boneIndex,
    u32 baseVertex,
    u32 firstIndex,
    u32 indexCount,
    const AABB3f& boundingBox,
    const Engine::Material *material )
:   _name(name)
,   _boneIndex(boneIndex)
,   _baseVertex(baseVertex)
,   _firstIndex(firstIndex)
,   _indexCount(indexCount)
,   _boundingBox(boundingBox)
,   _material(material) {
    Assert(!name.empty());
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
