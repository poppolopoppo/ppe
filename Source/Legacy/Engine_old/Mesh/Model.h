#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Maths/ScalarBoundingBox.h"
#include "Core/Memory/MemoryView.h"
#include "Core/Memory/RefPtr.h"
#include "Core/Memory/UniquePtr.h"

#include "Core.Engine/Mesh/MeshName.h"

namespace Core {
namespace Graphics {
class FBindName;
class IDeviceAPIEncapsulator;
}

namespace Engine {
struct FRenderCommandRegistration;
typedef TUniquePtr<const FRenderCommandRegistration> URenderCommand;
class FRenderTree;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(ModelBone);
FWD_REFPTR(ModelMesh);
//----------------------------------------------------------------------------
FWD_REFPTR(Model);
class FModel : public FRefCountable {
public:
    FModel(  const FMeshName& name,
            const AABB3f& boundingBox,
            VECTOR(Mesh, PModelBone)&& bones,
            VECTOR(Mesh, PModelMesh)&& meshes );
    ~FModel();

    FModel(const FModel& ) = delete;
    FModel& operator =(const FModel& ) = delete;

    const FMeshName& Name() const { return _name; }
    const AABB3f& BoundingBox() const { return _boundingBox; }
    const VECTOR(Mesh, PModelBone)& Bones() const { return _bones; }
    const VECTOR(Mesh, PModelMesh)& Meshes() const { return _meshes; }

    bool TryGetBone(const FModelBone **pbone, const FMeshName& name) const;
    const FModelBone *FBone(const FMeshName& name) const;

    void Create(Graphics::IDeviceAPIEncapsulator *device);
    void Destroy(Graphics::IDeviceAPIEncapsulator *device);

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    FMeshName _name;
    AABB3f _boundingBox;
    VECTOR(Mesh, PModelBone) _bones;
    VECTOR(Mesh, PModelMesh) _meshes;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FModelRenderCommand {
    SCModel FModel;
    VECTOR(Mesh, URenderCommand) RenderCommands;
    SINGLETON_POOL_ALLOCATED_DECL();
};
//----------------------------------------------------------------------------
typedef TUniquePtr<const FModelRenderCommand> UModelRenderCommand;
//----------------------------------------------------------------------------
bool AcquireModelRenderCommand( UModelRenderCommand& pModelCommand,
                                Graphics::IDeviceAPIEncapsulator *device,
                                FRenderTree *renderTree,
                                const TMemoryView<const TPair<Graphics::FBindName, const char *>>& parTagToRenderLayerName,
                                const char *parFallbackRenderLayerName,
                                const FModel *model );
//----------------------------------------------------------------------------
bool AcquireModelRenderCommand( UModelRenderCommand& pModelCommand,
                                Graphics::IDeviceAPIEncapsulator *device,
                                FRenderTree *renderTree,
                                const char *parRenderLayerName,
                                const FModel *model );
//----------------------------------------------------------------------------
void ReleaseModelRenderCommand( UModelRenderCommand& pModelCommand,
                                Graphics::IDeviceAPIEncapsulator *device,
                                const FModel *model );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
