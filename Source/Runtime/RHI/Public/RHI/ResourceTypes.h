#pragma once

#include "RHI_fwd.h"

#include "RHI/ResourceEnums.h"
#include "RHI/ResourceId.h"

#include "Container/Stack.h"
#include "Container/FixedSizeHashTable.h"
#include "Maths/Range.h"
#include "Maths/ScalarRectangle.h"

#define PPE_RHI_EACH_DYNAMICSTATE(EACH) \
    EACH(EStencilOp, StencilFailOp, ) \
    EACH(EStencilOp, StencilDepthFailOp, ) \
    EACH(EStencilOp, StencilPassOp, ) \
    EACH(FStencilValue, StencilReference, ) \
    EACH(FStencilValue, StencilWriteMask, ) \
    EACH(FStencilValue, StencilCompareMask, ) \
    EACH(ECullMode, CullMode, ) \
    EACH(ECompareOp, DepthCompareOp, ) \
    EACH(bool, EnableDepthTest, : 1) \
    EACH(bool, EnableDepthWrite, : 1) \
    EACH(bool, EnableStencilTest, : 1) \
    EACH(bool, EnableRasterizerDiscard, : 1) \
    EACH(bool, EnableFrontFaceCCW, : 1)

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
using FColorBuffers = TFixedSizeHashMap<ERenderTargetID, FColorBufferState, MaxViewports>;
//----------------------------------------------------------------------------
using FScissors = TFixedSizeStack<FRectangleI, MaxViewports>;
//----------------------------------------------------------------------------
struct FImageDataRange {
    using FSubRange = TRange<u32>;

    FSubRange Layers;
    FSubRange Mipmaps;

    FImageDataRange() = default;
    FImageDataRange(
        FImageLayer baseLayer, u32 layerCount,
        FMipmapLevel baseLevel, u32 levelCount )
    :   Layers(FSubRange(*baseLayer, *baseLayer + layerCount))
    ,   Mipmaps(FSubRange(*baseLevel, *baseLevel + layerCount))
    {}

    bool Empty() const { return (Layers.Empty() || Mipmaps.Empty()); }

    bool WholeLayers() const { return Layers.Whole(); }
    bool WholeMipmaps() const { return Mipmaps.Whole(); }

    bool Overlaps(const FImageDataRange& other) const {
        return (Layers.Overlaps(other.Layers) && Mipmaps.Overlaps(other.Mipmaps));
    }

    FImageDataRange Intersect(const FImageDataRange& other) const {
        FImageDataRange result(*this);
        result.Layers.Intersect(other.Layers);
        if (result.Layers.Empty())
            result.Layers = FSubRange();
        result.Mipmaps.Intersect(other.Mipmaps);
        return result;
    }

    bool operator ==(const FImageDataRange& other) const { return (Layers == other.Layers && Mipmaps == other.Mipmaps); }
    bool operator !=(const FImageDataRange& other) const { return (not operator ==(other)); }

};
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
        EImageAspect aspectMask = EImageAspect::Auto)
    :   AspectMask(aspectMask)
    ,   MipLevel(mipLevel)
    ,   BaseLayer(baseLayer)
    ,   LayerCount(layerCount)
    {}
};
//----------------------------------------------------------------------------
struct FPushConstantData {
    FPushConstantID Id{ Default };
    u16 Size;
    u8 Data[MaxPushConstantsSize];

    FPushConstantData() = default;
    FPushConstantData(const FPushConstantID& id, const void* p, size_t size)
    :   Id(id), Size(size) {
        Assert(id);
        Assert(p);
        Assert(size);
        Assert(size < MaxPushConstantsSize);
        FPlatformMemory::Memcpy(Data, p, size);
    }
};
using FPushConstantDatas = TFixedSizeStack<FPushConstantData, MaxPushConstantsCount>;
//----------------------------------------------------------------------------
struct FVertexBuffer {
    FRawBufferID Id{ Default };
    u32 Offset{ UMax };
};
//----------------------------------------------------------------------------
struct FDynamicStates {
    FDynamicStates() = default;

#define DECL_DYNAMICSTATE_ACCESSOR(TYPE, NAME, SUFF) \
    CONSTEXPR bool CONCAT(Has, NAME)() const { return CONCAT(_has, NAME); } \
    CONSTEXPR TYPE CONCAT(NAME)() const { Assert(CONCAT(_has, NAME)); return CONCAT(_, NAME); } \
    CONSTEXPR FDynamicStates& CONCAT(Set, NAME)(TYPE value) { \
        CONCAT(_, NAME) = value; \
        CONCAT(_has, NAME) = true; \
    }
    PPE_RHI_EACH_DYNAMICSTATE(DECL_DYNAMICSTATE_ACCESSOR)
#undef DECL_DYNAMICSTATE_ACCESSOR

private:
#define DECL_DYNAMICSTATE_FIELD(TYPE, NAME, SUFF) TYPE CONCAT(_, NAME) SUFF { Default };
    PPE_RHI_EACH_DYNAMICSTATE(DECL_DYNAMICSTATE_FIELD)
#undef DECL_DYNAMICSTATE_FIELD
#define DECL_DYNAMICSTATE_BITFIELD(TYPE, NAME, SUFF) bool CONCAT(_has, NAME) : 1 { false };
    PPE_RHI_EACH_DYNAMICSTATE(DECL_DYNAMICSTATE_BITFIELD)
#undef DECL_DYNAMICSTATE_BITFIELD
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
