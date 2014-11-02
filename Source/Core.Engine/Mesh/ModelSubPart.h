#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Container/Vector.h"
#include "Core/Maths/Geometry/ScalarBoundingBox.h"
#include "Core/Memory/MemoryView.h"
#include "Core/Memory/RefPtr.h"

#include "Core.Engine/Mesh/MeshName.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(ModelMeshSubPart);
//----------------------------------------------------------------------------
FWD_REFPTR(ModelSubPart);
class ModelSubPart : public RefCountable {
public:
    ModelSubPart(   const MeshName& name,
                    const MeshName& group,
                    const AABB3f& boundinbBox,
                    VECTOR(Mesh, PModelMeshSubPart)&& meshSubParts );
    ~ModelSubPart();

    ModelSubPart(const ModelSubPart& ) = delete;
    ModelSubPart& operator =(const ModelSubPart& ) = delete;

    const MeshName& Name() const { return _name; }
    const MeshName& Group() const { return _group; }
    const AABB3f& BoundingBox() const { return _boundingBox; }
    const VECTOR(Mesh, PModelMeshSubPart)& MeshSubParts() const { return _meshSubParts; }

    SINGLETON_POOL_ALLOCATED_DECL(ModelSubPart);

private:
    MeshName _name;
    MeshName _group;
    AABB3f _boundingBox;
    VECTOR(Mesh, PModelMeshSubPart) _meshSubParts;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
