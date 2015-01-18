#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Maths/Geometry/ScalarBoundingBox.h"
#include "Core/Memory/MemoryView.h"
#include "Core/Memory/RefPtr.h"
#include "Core/Memory/UniquePtr.h"

#include "Core.Engine/Mesh/MeshName.h"

namespace Core {
namespace Graphics {
class BindName;
class IDeviceAPIEncapsulator;
}

namespace Engine {
struct RenderCommand;
typedef UniquePtr<const RenderCommand> URenderCommand;
class RenderTree;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(ModelBone);
FWD_REFPTR(ModelMesh);
//----------------------------------------------------------------------------
FWD_REFPTR(Model);
class Model : public RefCountable {
public:
    Model(  const MeshName& name,
            const AABB3f& boundingBox,
            VECTOR(Mesh, PModelBone)&& bones,
            VECTOR(Mesh, PModelMesh)&& meshes );
    ~Model();

    Model(const Model& ) = delete;
    Model& operator =(const Model& ) = delete;

    const MeshName& Name() const { return _name; }
    const AABB3f& BoundingBox() const { return _boundingBox; }
    const VECTOR(Mesh, PModelBone)& Bones() const { return _bones; }
    const VECTOR(Mesh, PModelMesh)& Meshes() const { return _meshes; }

    bool TryGetBone(const ModelBone **pbone, const MeshName& name) const;
    const ModelBone *Bone(const MeshName& name) const;

    void Create(Graphics::IDeviceAPIEncapsulator *device);
    void Destroy(Graphics::IDeviceAPIEncapsulator *device);

    SINGLETON_POOL_ALLOCATED_DECL(Model);

private:
    MeshName _name;
    AABB3f _boundingBox;
    VECTOR(Mesh, PModelBone) _bones;
    VECTOR(Mesh, PModelMesh) _meshes;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct ModelRenderCommand {
    SCModel Model;
    VECTOR(Mesh, URenderCommand) RenderCommands;
    SINGLETON_POOL_ALLOCATED_DECL(ModelRenderCommand);
};
//----------------------------------------------------------------------------
typedef UniquePtr<const ModelRenderCommand> UModelRenderCommand;
//----------------------------------------------------------------------------
bool AcquireModelRenderCommand( UModelRenderCommand& pModelCommand,
                                Graphics::IDeviceAPIEncapsulator *device,
                                RenderTree *renderTree,
                                const MemoryView<const Pair<Graphics::BindName, const char *>>& parTagToRenderLayerName,
                                const char *parFallbackRenderLayerName,
                                const Model *model );
//----------------------------------------------------------------------------
void ReleaseModelRenderCommand( UModelRenderCommand& pModelCommand,
                                Graphics::IDeviceAPIEncapsulator *device,
                                const Model *model );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
