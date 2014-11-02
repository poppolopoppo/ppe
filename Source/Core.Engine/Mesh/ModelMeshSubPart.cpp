#include "stdafx.h"

#include "ModelMeshSubPart.h"

#include "ModelMesh.h"

#include "Core/Allocator/PoolAllocator-impl.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_DEF(ModelMeshSubPart, );
//----------------------------------------------------------------------------
ModelMeshSubPart::ModelMeshSubPart(
    u32 baseVertex,
    u32 firstIndex,
    u32 indexCount,
    const AABB3f& boundingBox,
    const Engine::ModelMesh *modelMesh )
:   _baseVertex(baseVertex)
,   _firstIndex(firstIndex)
,   _indexCount(indexCount)
,   _boundingBox(boundingBox)
,   _modelMesh(modelMesh) {
    Assert(indexCount > 0);
    Assert(boundingBox.HasPositiveExtents());
    Assert(modelMesh);
}
//----------------------------------------------------------------------------
ModelMeshSubPart::~ModelMeshSubPart() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
