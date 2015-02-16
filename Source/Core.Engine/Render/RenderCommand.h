#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Memory/RefPtr.h"
#include "Core/Memory/UniquePtr.h"

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
FWD_REFPTR(IndexBuffer);
FWD_REFPTR(VertexBuffer);
enum class PrimitiveType;
}

namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(Material);
FWD_REFPTR(MaterialEffect);
class RenderBatch;
class RenderTree;
//----------------------------------------------------------------------------
struct RenderCommand {
    RenderBatch *Batch; // to remove from batch before dying

    Engine::MaterialEffect *MaterialEffect;
    const Graphics::IndexBuffer *Indices;
    const Graphics::VertexBuffer *Vertices;

    u32 BaseVertex;
    u32 StartIndex;
    u32 PrimitiveCount  : 27; // 134 217 727 max prim count
    u32 PrimitiveType   : 4;
    mutable u32 Ready   : 1;

    const RenderCommand *Next;

    bool operator ==(const RenderCommand& other) const;
    bool operator !=(const RenderCommand& other) const { return !operator ==(other); }

    bool operator <(const RenderCommand& other) const;
    bool operator >=(const RenderCommand& other) const { return !operator <(other); }

    SINGLETON_POOL_ALLOCATED_DECL(RenderCommand);
};
STATIC_ASSERT(std::is_pod<RenderCommand>::value);
//----------------------------------------------------------------------------
typedef UniquePtr<const RenderCommand> URenderCommand;
//----------------------------------------------------------------------------
bool AcquireRenderCommand(
    URenderCommand& pOutCommand,
    RenderTree *renderTree,
    const char *renderLayerName,
    const Material *material,
    const Graphics::IndexBuffer *indices,
    const Graphics::VertexBuffer *vertices,
    Graphics::PrimitiveType primitiveType,
    size_t baseVertex,
    size_t startIndex,
    size_t primitiveCount );
//----------------------------------------------------------------------------
void ReleaseRenderCommand(  URenderCommand& pcommand,
                            Graphics::IDeviceAPIEncapsulator *device );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
