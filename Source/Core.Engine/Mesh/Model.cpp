#include "stdafx.h"

#include "Model.h"

#include "ModelBone.h"
#include "ModelMesh.h"
#include "ModelMeshSubPart.h"

#include "Core/Allocator/PoolAllocator-impl.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_DEF(Model, );
//----------------------------------------------------------------------------
Model::Model(
    const MeshName& name,
    const AABB3f& boundingBox,
    VECTOR(Mesh, PModelBone)&& bones,
    VECTOR(Mesh, PModelMesh)&& meshes )
:   _boundingBox(boundingBox)
,   _bones(std::move(bones))
,   _meshes(std::move(meshes)) {
    Assert(!name.empty());
    Assert(_boundingBox.HasPositiveExtents());
#ifdef WITH_CORE_ASSERT
    for (const PModelBone& modelBone : _bones) {
        Assert(modelBone);
        Assert(_boundingBox.Contains(modelBone->BoundingBox()));
    }
    for (const PModelMesh& modelMesh : _meshes) {
        Assert(modelMesh);
        for (const PModelMeshSubPart& subPart : modelMesh->SubParts()) {
            Assert(subPart);
            Assert(_boundingBox.Contains(subPart->BoundingBox()));
            Assert(subPart->BaseVertex() < modelMesh->VertexCount());
            Assert(subPart->FirstIndex() + subPart->IndexCount() <= modelMesh->IndexCount());
        }
    }
#endif
}
//----------------------------------------------------------------------------
Model::~Model() {}
//----------------------------------------------------------------------------
bool Model::TryGetBone(const ModelBone **pbone, const MeshName& name) const {
    Assert(pbone);
    Assert(!name.empty());

    for (const PModelBone& bone : _bones)
        if (bone->Name() == name) {
            *pbone = bone.get();
            return true;
        }

    return false;
}
//----------------------------------------------------------------------------
const ModelBone *Model::Bone(const MeshName& name) const {
    const ModelBone *bone = nullptr;
    if (!TryGetBone(&bone, name))
        AssertNotReached();
    Assert(bone);
    return bone;
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
