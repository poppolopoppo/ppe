#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Maths/Geometry/ScalarBoundingBox.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(ModelMesh);
//----------------------------------------------------------------------------
FWD_REFPTR(ModelMeshSubPart);
class ModelMeshSubPart : public RefCountable {
public:
    ModelMeshSubPart(   u32 baseVertex,
                        u32 firstIndex,
                        u32 indexCount,
                        const AABB3f& boundingBox,
                        const Engine::ModelMesh *modelMesh );
    ~ModelMeshSubPart();

    ModelMeshSubPart(const ModelMeshSubPart& ) = delete;
    ModelMeshSubPart& operator =(const ModelMeshSubPart& ) = delete;

    u32 BaseVertex() const { return _baseVertex; }
    u32 FirstIndex() const { return _firstIndex; }
    u32 IndexCount() const { return _indexCount; }
    const AABB3f& BoundingBox() const { return _boundingBox; }
    const Engine::ModelMesh *ModelMesh() const { return _modelMesh.get(); }

    SINGLETON_POOL_ALLOCATED_DECL(ModelMeshSubPart);

private:
    u32 _baseVertex;
    u32 _firstIndex;
    u32 _indexCount;
    AABB3f _boundingBox;
    SCModelMesh _modelMesh;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
