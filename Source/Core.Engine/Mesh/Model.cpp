#include "stdafx.h"

#include "Model.h"

#include "ModelBone.h"
#include "ModelMesh.h"
#include "ModelMeshSubPart.h"

#include "Core.Graphics/Device/BindName.h"
#include "Core.Graphics/Device/Geometry/PrimitiveType.h"

#include "Core.Engine/Material/Material.h"
#include "Core.Engine/Render/RenderCommand.h"

#include "Core/Allocator/PoolAllocator-impl.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static const char *SelectRenderLayerName_(
    const Material *parMaterial,
    const MemoryView<const Pair<Graphics::BindName, const char *>>& parTagToRenderLayerName,
    const char *parFallbackRenderLayerName ) {
    Assert(parMaterial);

    for (const Pair<Graphics::BindName, const char *>& it : parTagToRenderLayerName)
        if (Contains(parMaterial->Tags(), it.first)) {
            Assert(it.second);
            return it.second;
        }

    Assert(parFallbackRenderLayerName);
    return parFallbackRenderLayerName;
}
//----------------------------------------------------------------------------
} //!namespace
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
:   _name(name)
,   _boundingBox(boundingBox)
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
SINGLETON_POOL_ALLOCATED_DEF(ModelRenderCommand, );
//----------------------------------------------------------------------------
bool AcquireModelRenderCommand( UModelRenderCommand& pModelCommand,
                                Graphics::IDeviceAPIEncapsulator *device,
                                RenderTree *renderTree,
                                const MemoryView<const Pair<Graphics::BindName, const char *>>& parTagToRenderLayerName,
                                const char *parFallbackRenderLayerName,
                                const Model *model ) {
    Assert(!pModelCommand);
    Assert(device);
    Assert(model);
    Assert(parTagToRenderLayerName.size() || parFallbackRenderLayerName);

    ModelRenderCommand *result = new ModelRenderCommand();
    result->Model = model;

    size_t subPartCount = 0;
    for (const PModelMesh& modelMesh : model->Meshes())
        subPartCount += modelMesh->SubParts().size();

    result->RenderCommands.reserve(subPartCount);

    for (const PModelMesh& modelMesh : model->Meshes())
        for (const PModelMeshSubPart& modelMeshSubPart : modelMesh->SubParts()) {
            const size_t primitiveCount = Graphics::PrimitiveCount(modelMesh->PrimitiveType(), modelMeshSubPart->IndexCount());

            const char *renderLayerName = SelectRenderLayerName_(   modelMeshSubPart->Material().get(), 
                                                                    parTagToRenderLayerName, 
                                                                    parFallbackRenderLayerName );
            Assert(renderLayerName);

            URenderCommand pCommand;
            if (!AcquireRenderCommand(  pCommand, 
                                        renderTree, 
                                        renderLayerName,
                                        modelMeshSubPart->Material().get(),
                                        modelMesh->IndexBuffer().get(),
                                        modelMesh->VertexBuffer().get(),
                                        modelMesh->PrimitiveType(),
                                        modelMeshSubPart->BaseVertex(),
                                        modelMeshSubPart->FirstIndex(),
                                        primitiveCount) ) {
                Assert(result->RenderCommands.empty());
                delete result;
                return false;
            }
            Assert(pCommand);

            result->RenderCommands.push_back(std::move(pCommand));
        }

    pModelCommand.reset(result);
    return true;
}
//----------------------------------------------------------------------------
void ReleaseModelRenderCommand( UModelRenderCommand& pModelCommand,
                                Graphics::IDeviceAPIEncapsulator *device,
                                const Model *model ) {
    Assert(pModelCommand);
    Assert(model);
    Assert(pModelCommand->Model.get() == model);

    for (URenderCommand& pCommand : const_cast<ModelRenderCommand *>(pModelCommand.get())->RenderCommands) {
        Assert(pCommand);
        ReleaseRenderCommand(pCommand, device);
        Assert(!pCommand);
    }

    pModelCommand.reset(nullptr);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
