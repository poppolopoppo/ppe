#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Maths/Geometry/ScalarBoundingBox.h"
#include "Core/Memory/MemoryView.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
}

namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class MeshName;
FWD_REFPTR(ModelMesh);
FWD_REFPTR(ModelSubPart);
//----------------------------------------------------------------------------
FWD_REFPTR(Model);
class Model : public RefCountable {
public:
    Model(  const AABB3f& boundingBox,
            VECTOR(Mesh, PModelMesh)&& meshes,
            VECTOR(Mesh, PModelSubPart)&& subParts );
    ~Model();

    Model(const Model& ) = delete;
    Model& operator =(const Model& ) = delete;

    const AABB3f& BoundingBox() const { return _boundingBox; }
    const VECTOR(Mesh, PModelMesh)& Meshes() const { return _meshes; }
    const VECTOR(Mesh, PModelSubPart)& SubParts() const { return _subParts; }

    bool TryGetSubPart(const ModelSubPart **psubpart, const MeshName& name) const;
    const ModelSubPart *SubPart(const MeshName& name) const;

    void Create(Graphics::IDeviceAPIEncapsulator *device);
    void Destroy(Graphics::IDeviceAPIEncapsulator *device);

    SINGLETON_POOL_ALLOCATED_DECL(Model);

private:
    AABB3f _boundingBox;
    VECTOR(Mesh, PModelMesh) _meshes;
    VECTOR(Mesh, PModelSubPart) _subParts;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
