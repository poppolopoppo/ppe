#pragma once

#include "RHI_fwd.h"

#include "RHI/ResourceEnums.h"
#include "RHI/ResourceId.h"

#include "Container/Stack.h"
#include "Container/FixedSizeHashTable.h"
#include "Maths/Range.h"
#include "Maths/ScalarRectangle.h"

#define PPE_RHI_EACH_DYNAMICSTATE(EACH) \
    EACH( 0, EStencilOp, StencilFailOp, ) \
    EACH( 1, EStencilOp, StencilDepthFailOp, ) \
    EACH( 2, EStencilOp, StencilPassOp, ) \
    EACH( 3, FStencilValue, StencilReference, ) \
    EACH( 4, FStencilValue, StencilWriteMask, ) \
    EACH( 5, FStencilValue, StencilCompareMask, ) \
    EACH( 6, ECullMode, CullMode, ) \
    EACH( 7, ECompareOp, DepthCompareOp, ) \
    EACH( 8, bool, EnableDepthTest, : 1) \
    EACH( 9, bool, EnableDepthWrite, : 1) \
    EACH(10, bool, EnableStencilTest, : 1) \
    EACH(11, bool, EnableRasterizerDiscard, : 1) \
    EACH(12, bool, EnableFrontFaceCCW, : 1)

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
using FColorBuffers = TFixedSizeHashMap<ERenderTargetID, FColorBufferState, MaxViewports>;
//----------------------------------------------------------------------------
using FScissors = TFixedSizeStack<FRectangleU, MaxViewports>;
//----------------------------------------------------------------------------
struct FImageDataRange {
    using FSubRange = TRange<u32>;

    FSubRange Layers;
    FSubRange Mipmaps;

    FImageDataRange() = default;
    FImageDataRange(
        FImageLayer baseLayer, u32 layerCount,
        FMipmapLevel baseLevel, u32 levelCount ) NOEXCEPT
    :   FImageDataRange(
            FSubRange(*baseLayer, *baseLayer + layerCount),
            FSubRange(*baseLevel, *baseLevel + levelCount) )
    {}
    FImageDataRange(const FSubRange& layers, const FSubRange& mipmaps) NOEXCEPT
    :   Layers(layers)
    ,   Mipmaps(mipmaps)
    {}

    bool Empty() const { return (Layers.Empty() || Mipmaps.Empty()); }

    bool WholeLayers() const { return Layers.Whole(); }
    bool WholeMipmaps() const { return Mipmaps.Whole(); }

    bool Overlaps(const FImageDataRange& other) const {
        return (Layers.Overlaps(other.Layers) && Mipmaps.Overlaps(other.Mipmaps));
    }

    FImageDataRange Intersect(const FImageDataRange& other) const {
        const FSubRange layers = Layers.Intersect(other.Layers);
        return { layers, layers.Empty() ? FSubRange{} : Mipmaps.Intersect(other.Mipmaps) };
    }

    bool operator ==(const FImageDataRange& other) const { return (Layers == other.Layers && Mipmaps == other.Mipmaps); }
    bool operator !=(const FImageDataRange& other) const { return (not operator ==(other)); }

};
PPE_ASSUME_TYPE_AS_POD(FImageDataRange);
//----------------------------------------------------------------------------
struct FImageSubresourceRange {
    EImageAspect AspectMask{ EImageAspect::Auto };
    FMipmapLevel MipLevel;
    FImageLayer BaseLayer;
    u32 LayerCount{ 1 };

    FImageSubresourceRange() = default;
    FImageSubresourceRange(
        FMipmapLevel mipLevel,
        FImageLayer baseLayer = Default,
        u32 layerCount = 1,
        EImageAspect aspectMask = EImageAspect::Auto) NOEXCEPT
    :   AspectMask(aspectMask)
    ,   MipLevel(mipLevel)
    ,   BaseLayer(baseLayer)
    ,   LayerCount(layerCount)
    {}
};
PPE_ASSUME_TYPE_AS_POD(FImageSubresourceRange);
//----------------------------------------------------------------------------
struct FPushConstantData {
    FPushConstantID Id{ Default };
    u16 Size;
    u8 Data[MaxPushConstantsSize];

    FPushConstantData() = default;
    FPushConstantData(const FPushConstantID& id, const void* p, size_t size) NOEXCEPT
    :   Id(id), Size(checked_cast<u16>(size)) {
        Assert(Id);
        Assert(p);
        Assert(Size);
        Assert(size < MaxPushConstantsSize);
        FPlatformMemory::Memcpy(Data, p, size);
    }
};
PPE_ASSUME_TYPE_AS_POD(FPushConstantData);
using FPushConstantDatas = TFixedSizeStack<FPushConstantData, MaxPushConstantsCount>;
//----------------------------------------------------------------------------
struct FVertexBuffer {
    FRawBufferID Id{ Default };
    size_t Offset{ UMax };
};
PPE_ASSUME_TYPE_AS_POD(FVertexBuffer);
//----------------------------------------------------------------------------
// Dynamic states
//----------------------------------------------------------------------------
enum class EDrawDynamicState : u32 {
    Unknown = 0,
#define DECL_DYNAMICSTATE_FLAG(ID, TYPE, NAME, SUFF) NAME = 1 << ID,
    PPE_RHI_EACH_DYNAMICSTATE(DECL_DYNAMICSTATE_FLAG)
#undef DECL_DYNAMICSTATE_FLAG
};
ENUM_FLAGS(EDrawDynamicState);
//----------------------------------------------------------------------------
struct  FDrawDynamicStates {
    FDrawDynamicStates() NOEXCEPT
    :   _states{ Default }
#define INIT_DYNAMICSTATE_FIELD(ID, TYPE, NAME, SUFF) , CONCAT(_, NAME){ Default }
    PPE_RHI_EACH_DYNAMICSTATE(INIT_DYNAMICSTATE_FIELD)
#undef INIT_DYNAMICSTATE_FIELD
    {}

    CONSTEXPR bool Has(EDrawDynamicState flag) const NOEXCEPT { return (_states & flag); }

#define DECL_DYNAMICSTATE_ACCESSOR(ID, TYPE, NAME, SUFF) \
    using CONCAT(F, NAME) = TYPE; \
    CONSTEXPR bool CONCAT(Has, NAME)() const { return Has(EDrawDynamicState::NAME); } \
    CONSTEXPR TYPE NAME() const { Assert_NoAssume(CONCAT(Has, NAME)()); return CONCAT(_, NAME); } \
    CONSTEXPR FDrawDynamicStates& CONCAT(Set, NAME)(TYPE value) { \
        CONCAT(_, NAME) = value; \
        _states += EDrawDynamicState::NAME; \
        return (*this); \
    }
    PPE_RHI_EACH_DYNAMICSTATE(DECL_DYNAMICSTATE_ACCESSOR)
#undef DECL_DYNAMICSTATE_ACCESSOR

private:
    EDrawDynamicState _states{ Default };

#define DECL_DYNAMICSTATE_FIELD(ID, TYPE, NAME, SUFF) TYPE CONCAT(_, NAME) SUFF;
    PPE_RHI_EACH_DYNAMICSTATE(DECL_DYNAMICSTATE_FIELD)
#undef DECL_DYNAMICSTATE_FIELD
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
