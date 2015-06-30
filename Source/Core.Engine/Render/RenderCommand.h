#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Memory/RefPtr.h"
#include "Core/Memory/UniquePtr.h"
#include "Core/Meta/BitField.h"

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
FWD_REFPTR(IndexBuffer);
FWD_REFPTR(VertexBuffer);
enum class PrimitiveType;
}

namespace Engine {
FWD_REFPTR(Effect);
FWD_REFPTR(Material);
FWD_REFPTR(MaterialEffect);
class RenderBatch;
class RenderTree;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// TODO : better sorting
// http://realtimecollisiondetection.net/blog/?p=86
// http://aras-p.info/blog/2014/01/16/rough-sorting-by-depth/
struct RenderCommandCriteria {

    size_t MaterialEffectAndReady;

    bool Ready() const { return 0 != (MaterialEffectAndReady & 1); }
    void SetReady() { MaterialEffectAndReady |= 1; }

    Engine::MaterialEffect *MaterialEffect() const { return reinterpret_cast<Engine::MaterialEffect *>(MaterialEffectAndReady&(~1)); }
    void SetMaterialEffect(Engine::MaterialEffect *materialEffect) {
        Assert(materialEffect);
        Assert(0 == (size_t(materialEffect) & 1)); // must be aligned on 4 (guaranteed on every architecture)
        MaterialEffectAndReady = size_t(materialEffect);
        Assert(size_t(materialEffect) == MaterialEffectAndReady);
    }

    const Graphics::VertexBuffer *Vertices;
    const Graphics::IndexBuffer *Indices;
    const Engine::Effect *Effect;

    bool operator !=(const RenderCommandCriteria& other) const { return !operator ==(other); }
    bool operator ==(const RenderCommandCriteria& other) const {
        return (//return MaterialEffect() == other.MaterialEffect() // must be equivalent to next line :
                MaterialEffectAndReady == other.MaterialEffectAndReady &&
                Indices == other.Indices && 
                Vertices == other.Vertices);
    }

    bool operator >=(const RenderCommandCriteria& other) const { return operator <(other); }
    bool operator < (const RenderCommandCriteria& other) const { 
        if (Vertices < other.Vertices) return true;
        if (Vertices > other.Vertices) return false;
        if (Indices  < other.Indices ) return true;
        if (Indices  > other.Indices ) return false;
        if (Effect   < other.Effect  ) return true;
        if (Effect   > other.Effect  ) return false;
        //return MaterialEffect() < other.MaterialEffect() // must be equivalent to next line :
        return MaterialEffectAndReady < other.MaterialEffectAndReady;
    }
};
STATIC_ASSERT(std::is_pod<RenderCommandCriteria>::value);
//----------------------------------------------------------------------------
inline void swap(RenderCommandCriteria& lhs, RenderCommandCriteria& rhs) {
    std::swap(lhs.MaterialEffectAndReady, rhs.MaterialEffectAndReady);
    std::swap(lhs.Indices, rhs.Indices);
    std::swap(lhs.Vertices, rhs.Vertices);
    std::swap(lhs.Effect, rhs.Effect);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct RenderCommandParams {
    u32 BaseVertex;
    u32 StartIndex;
    u32 PrimitiveCountAndType;

    typedef Meta::Bit<u32>::First<5>::type primitivetype_type;
    typedef Meta::Bit<u32>::After<primitivetype_type>::Remain::type primitivecount_type; // 134 217 727 max prim count

    u32 PrimitiveCount() const { return primitivecount_type::Get(PrimitiveCountAndType); }
    void SetPrimitiveCount(u32 value) { primitivecount_type::InplaceSet(PrimitiveCountAndType, value); }

    Graphics::PrimitiveType PrimitiveType() const { return Graphics::PrimitiveType(primitivetype_type::Get(PrimitiveCountAndType)); }
    void SetPrimitiveType(Graphics::PrimitiveType value) { primitivetype_type::InplaceSet(PrimitiveCountAndType, u32(value)); }
};
STATIC_ASSERT(std::is_pod<RenderCommandParams>::value);
//----------------------------------------------------------------------------
inline void swap(RenderCommandParams& lhs, RenderCommandParams& rhs) {
    std::swap(lhs.BaseVertex, rhs.BaseVertex);
    std::swap(lhs.StartIndex, rhs.StartIndex);
    std::swap(lhs.PrimitiveCountAndType, rhs.PrimitiveCountAndType);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct RenderCommandRegistration {
    RenderBatch *Batch; // to remove from batch before dying
    const RenderCommandRegistration *Next;

    SINGLETON_POOL_ALLOCATED_DECL(RenderCommandRegistration);
};
STATIC_ASSERT(std::is_pod<RenderCommandRegistration>::value);
//----------------------------------------------------------------------------
typedef UniquePtr<const RenderCommandRegistration> URenderCommand;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
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
