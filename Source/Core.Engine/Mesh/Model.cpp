#include "stdafx.h"

#include "Model.h"

#include "ModelMesh.h"
#include "ModelMeshSubPart.h"
#include "ModelSubPart.h"

#include "Core/Allocator/PoolAllocator-impl.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_DEF(Model, );
//----------------------------------------------------------------------------
Model::Model(
    const AABB3f& boundingBox,
    VECTOR(Mesh, PModelMesh)&& meshes,
    VECTOR(Mesh, PModelSubPart)&& subParts )
:   _boundingBox(boundingBox)
,   _meshes(std::move(meshes))
,   _subParts(std::move(subParts)) {
    Assert(_boundingBox.HasPositiveExtents());
#ifdef WITH_CORE_ASSERT
    for (const PModelSubPart& modelSubPart : subParts) {
        Assert(_boundingBox.Contains(modelSubPart->BoundingBox()));
        for (const PModelMeshSubPart& meshSubPart : modelSubPart->MeshSubParts()) {
            Assert(modelSubPart->BoundingBox().Contains(meshSubPart->BoundingBox()));
            Assert(meshSubPart->BaseVertex() < meshSubPart->ModelMesh()->VertexCount());
            Assert(meshSubPart->FirstIndex() + meshSubPart->IndexCount() <= meshSubPart->ModelMesh()->IndexCount());
        }
    }
#endif
}
//----------------------------------------------------------------------------
Model::~Model() {}
//----------------------------------------------------------------------------
bool Model::TryGetSubPart(const ModelSubPart **psubpart, const MeshName& name) const {
    Assert(psubpart);
    Assert(!name.empty());

    for (const PModelSubPart& s : _subParts)
        if (s->Name() == name)
        {
            *psubpart = s.get();
            return true;
        }

    return false;
}
//----------------------------------------------------------------------------
const ModelSubPart *Model::SubPart(const MeshName& name) const {
    const ModelSubPart *psubpart = nullptr;
    if (!TryGetSubPart(&psubpart, name))
        AssertNotReached();
    return psubpart;
}
//----------------------------------------------------------------------------
void Model::Create(Graphics::IDeviceAPIEncapsulator *device) {
    for (const PModelMesh& pmesh : _meshes)
        pmesh->Create(device);
}
//----------------------------------------------------------------------------
void Model::Destroy(Graphics::IDeviceAPIEncapsulator *device) {
    for (const PModelMesh& pmesh : _meshes)
        pmesh->Destroy(device);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
