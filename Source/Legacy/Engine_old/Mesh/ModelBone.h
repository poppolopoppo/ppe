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
class FModelBone : public FRefCountable {
public:
    FModelBone(  const FMeshName& name,
                const FMeshName& group,
                const float4x4& transform,
                const AABB3f& boundingBox );
    ~FModelBone();

    FModelBone(const FModelBone& ) = delete;
    FModelBone& operator =(const FModelBone& ) = delete;

    const FMeshName& Name() const { return _name; }
    const FMeshName& Group() const { return _group; }
    const float4x4& Transform() const { return _transform; }
    const AABB3f& BoundingBox() const { return _boundingBox; }

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    FMeshName _name;
    FMeshName _group;
    float4x4 _transform;
    AABB3f _boundingBox;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
