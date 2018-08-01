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
enum class EPrimitiveType;
}

namespace Engine {
FWD_REFPTR(Effect);
FWD_REFPTR(Material);
FWD_REFPTR(MaterialEffect);
class FRenderBatch;
class FRenderTree;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// TODO : better sorting
// http://realtimecollisiondetection.net/blog/?p=86
// http://aras-p.info/blog/2014/01/16/rough-sorting-by-depth/
struct FRenderCommandCriteria {

    size_t MaterialEffectAndReady;

    bool Ready() const { return 0 != (MaterialEffectAndReady & 1); }
    void SetReady() { MaterialEffectAndReady |= 1; }

    Engine::FMaterialEffect *FMaterialEffect() const { return reinterpret_cast<Engine::FMaterialEffect *>(MaterialEffectAndReady&(~1)); }
    void SetMaterialEffect(Engine::FMaterialEffect *materialEffect) {
        Assert(materialEffect);
        Assert(0 == (size_t(materialEffect) & 1)); // must be aligned on 4 (guaranteed on every architecture)
        MaterialEffectAndReady = size_t(materialEffect);
        Assert(size_t(materialEffect) == MaterialEffectAndReady);
    }

    const Graphics::FVertexBuffer *Vertices;
    const Graphics::IndexBuffer *Indices;
    const Engine::FEffect *FEffect;

    bool operator !=(const FRenderCommandCriteria& other) const { return !operator ==(other); }
    bool operator ==(const FRenderCommandCriteria& other) const {
        return (//return FMaterialEffect() == other.MaterialEffect() // must be equivalent to next line :
                MaterialEffectAndReady == other.MaterialEffectAndReady &&
                Indices == other.Indices && 
                Vertices == other.Vertices);
    }

    bool operator >=(const FRenderCommandCriteria& other) const { return operator <(other); }
    bool operator < (const FRenderCommandCriteria& other) const { 
        if (Vertices < other.Vertices) return true;
        if (Vertices > other.Vertices) return false;
        if (Indices  < other.Indices ) return true;
        if (Indices  > other.Indices ) return false;
        if (FEffect   < other.Effect  ) return true;
        if (FEffect   > other.Effect  ) return false;
        //return FMaterialEffect() < other.MaterialEffect() // must be equivalent to next line :
        return MaterialEffectAndReady < other.MaterialEffectAndReady;
    }
};
STATIC_ASSERT(std::is_pod<FRenderCommandCriteria>::value);
//----------------------------------------------------------------------------
inline void swap(FRenderCommandCriteria& lhs, FRenderCommandCriteria& rhs) {
    std::swap(lhs.MaterialEffectAndReady, rhs.MaterialEffectAndReady);
    std::swap(lhs.Indices, rhs.Indices);
    std::swap(lhs.Vertices, rhs.Vertices);
    std::swap(lhs.Effect, rhs.Effect);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FRenderCommandParams {
    u32 BaseVertex;
    u32 StartIndex;
    u32 PrimitiveCountAndType;

    typedef Meta::TBit<u32>::TFirst<5>::type primitivetype_type;
    typedef Meta::TBit<u32>::TAfter<primitivetype_type>::FRemain::type primitivecount_type; // 134 217 727 max prim count

    u32 PrimitiveCount() const { return primitivecount_type::Get(PrimitiveCountAndType); }
    void SetPrimitiveCount(u32 value) { primitivecount_type::InplaceSet(PrimitiveCountAndType, value); }

    Graphics::EPrimitiveType EPrimitiveType() const { return Graphics::EPrimitiveType(primitivetype_type::Get(PrimitiveCountAndType)); }
    void SetPrimitiveType(Graphics::EPrimitiveType value) { primitivetype_type::InplaceSet(PrimitiveCountAndType, u32(value)); }
};
STATIC_ASSERT(std::is_pod<FRenderCommandParams>::value);
//----------------------------------------------------------------------------
inline void swap(FRenderCommandParams& lhs, FRenderCommandParams& rhs) {
    std::swap(lhs.BaseVertex, rhs.BaseVertex);
    std::swap(lhs.StartIndex, rhs.StartIndex);
    std::swap(lhs.PrimitiveCountAndType, rhs.PrimitiveCountAndType);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FRenderCommandRegistration {
    FRenderBatch *Batch; // to remove from batch before dying
    const FRenderCommandRegistration *Next;

    SINGLETON_POOL_ALLOCATED_DECL();
};
STATIC_ASSERT(std::is_pod<FRenderCommandRegistration>::value);
//----------------------------------------------------------------------------
typedef TUniquePtr<const FRenderCommandRegistration> URenderCommand;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool AcquireRenderCommand(
    URenderCommand& pOutCommand,
    FRenderTree *renderTree,
    const char *renderLayerName,
    const FMaterial *material,
    const Graphics::IndexBuffer *indices,
    const Graphics::FVertexBuffer *vertices,
    Graphics::EPrimitiveType primitiveType,
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
