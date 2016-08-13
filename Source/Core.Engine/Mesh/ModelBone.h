#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Maths/ScalarMatrix.h"
#include "Core/Memory/RefPtr.h"

#include "Core.Engine/Mesh/MeshName.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(ModelBone);
class ModelBone : public RefCountable {
public:
    ModelBone(  const MeshName& name,
                const MeshName& group,
                const float4x4& transform,
                const AABB3f& boundingBox );
    ~ModelBone();

    ModelBone(const ModelBone& ) = delete;
    ModelBone& operator =(const ModelBone& ) = delete;

    const MeshName& Name() const { return _name; }
    const MeshName& Group() const { return _group; }
    const float4x4& Transform() const { return _transform; }
    const AABB3f& BoundingBox() const { return _boundingBox; }

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    MeshName _name;
    MeshName _group;
    float4x4 _transform;
    AABB3f _boundingBox;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
