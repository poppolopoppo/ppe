#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Maths/Geometry/ScalarBoundingBox.h"
#include "Core/Memory/RefPtr.h"

#include "Core.Engine/Mesh/MeshName.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(Material);
//----------------------------------------------------------------------------
FWD_REFPTR(ModelMeshSubPart);
class ModelMeshSubPart : public RefCountable {
public:
    ModelMeshSubPart(   const MeshName& name,
                        u32 boneIndex,
                        u32 baseVertex,
                        u32 firstIndex,
                        u32 indexCount,
                        const AABB3f& boundingBox,
                        const Engine::Material *material );
    ~ModelMeshSubPart();

    ModelMeshSubPart(const ModelMeshSubPart& ) = delete;
    ModelMeshSubPart& operator =(const ModelMeshSubPart& ) = delete;

    const MeshName& Name() const { return _name; }

    u32 BoneIndex() const { return _boneIndex; }
    u32 BaseVertex() const { return _baseVertex; }
    u32 FirstIndex() const { return _firstIndex; }
    u32 IndexCount() const { return _indexCount; }

    const AABB3f& BoundingBox() const { return _boundingBox; }

    const PCMaterial& Material() const { return _material; }

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    MeshName _name;

    u32 _boneIndex;
    u32 _baseVertex;
    u32 _firstIndex;
    u32 _indexCount;

    AABB3f _boundingBox;

    PCMaterial _material;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
