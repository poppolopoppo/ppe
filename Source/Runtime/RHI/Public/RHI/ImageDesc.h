#pragma once

#include "RHI_fwd.h"

#include "RHI/ImageHelpers.h"
#include "RHI/ResourceEnums.h"

#include "Container/Hash.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FImageDesc {
    EImageDim Type{ Default };
    EImageView View{ Default }; // optional
    EImageFlags Flags{ Default };
    uint3 Dimensions{ 0 };
    EPixelFormat Format{ Default };
    EImageUsage Usage{ Default };
    FImageLayer ArrayLayers;
    FMipmapLevel MaxLevel;
    FMultiSamples Samples; // enable multisampling if > 1
    EQueueUsage Queues{ Default };
    bool IsExternal{ false };

    FImageDesc() = default;
    FImageDesc(
        EImageDim type,
        const uint3& dimensions,
        EPixelFormat format,
        EImageUsage usage,
        FImageLayer arrayLayers = Default,
        FMipmapLevel maxLevel = Default,
        EQueueUsage queues = Default )
    :   Type(type)
    ,   Dimensions(dimensions)
    ,   Format(format)
    ,   Usage(usage)
    ,   ArrayLayers(arrayLayers)
    ,   MaxLevel(maxLevel)
    ,   Queues(queues)
    {}

    FImageDesc& SetType(EImageDim value) { Type = value;  return *this; }
    FImageDesc& SetView(EImageView value) NOEXCEPT;
    FImageDesc& SetFlag(EImageFlags value) { Flags |= value; return *this; }
    FImageDesc& SetDimension(u32 value) NOEXCEPT;
    FImageDesc& SetDimension(const uint2& value) NOEXCEPT;
    FImageDesc& SetDimension(const uint3& value) NOEXCEPT;
    FImageDesc& SetUsage(EImageUsage value) { Usage = value;  return *this; }
    FImageDesc& SetFormat(EPixelFormat value) { Format = value;  return *this; }
    FImageDesc& SetQueues(EQueueUsage value) { Queues = value;  return *this; }
    FImageDesc& SetArrayLayers(u32 value) { ArrayLayers = FImageLayer{value};  return *this; }
    FImageDesc& SetMaxMipmaps(u32 value) { MaxLevel = FMipmapLevel{value};  return *this; }
    FImageDesc& SetAllMipmaps() { MaxLevel = FMipmapLevel{~0u};  return *this; }
    FImageDesc& SetSamples(u32 value) { Samples = FMultiSamples{value};  return *this; }

    PPE_RHI_API void Validate();

    friend hash_t hash_value(const FImageDesc& it) {
        return hash_tuple(it.Type, it.View, it.Flags, it.Dimensions, it.Format, it.Usage, it.ArrayLayers, it.MaxLevel, it.Queues);
    }
};
//----------------------------------------------------------------------------
struct FImageViewDesc {
    EImageView View{ Default };
    EPixelFormat Format{ Default };
    FMipmapLevel BaseLevel;
    u32 LevelCount{ 1 };
    FImageLayer BaseLayer;
    u32 LayerCount{ 1 };
    FImageSwizzle Swizzle;
    EImageAspect AspectMask{ Default };

    FImageViewDesc() = default;
    FImageViewDesc(
        EImageView view,
        EPixelFormat format,
        FMipmapLevel baseLevel = Default,
        u32 numLevels = 1,
        FImageLayer baseLayer = Default,
        u32 numLayers = 1,
        FImageSwizzle swizzle = Default,
        EImageAspect aspectMask = Default )
    :   View(view)
    ,   Format(format)
    ,   BaseLevel(baseLevel)
    ,   LevelCount(numLevels)
    ,   BaseLayer(baseLayer)
    ,   LayerCount(numLayers)
    ,   Swizzle(swizzle)
    ,   AspectMask(aspectMask)
    {}

    PPE_RHI_API explicit FImageViewDesc(const FImageDesc& desc);

    FImageViewDesc& SetType(EImageView value) { View = value;  return *this; }
    FImageViewDesc& SetFormat(EPixelFormat value) { Format = value;  return *this; }
    FImageViewDesc& SetBaseLevel(u32 value) { BaseLevel = FMipmapLevel{value};  return *this; }
    FImageViewDesc& SetLevels(u32 base, u32 count) { BaseLevel = FMipmapLevel{base}; LevelCount = count;  return *this; }
    FImageViewDesc& SetBaseLayer(u32 value) { BaseLayer = FImageLayer{value};  return *this; }
    FImageViewDesc& SetArrayLayers(u32 base, u32 count) { BaseLayer = FImageLayer(base); LayerCount = count;  return *this; }
    FImageViewDesc& SetSwizzle(FImageSwizzle value) { Swizzle = value;  return *this; }
    FImageViewDesc& SetAspect(EImageAspect value) { AspectMask = value;  return *this; }

    PPE_RHI_API void Validate(const FImageDesc& desc);

    bool operator ==(const FImageViewDesc& other) const {
        return (
            View == other.View &&
            Format == other.Format &&
            BaseLevel == other.BaseLevel &&
            LevelCount == other.LevelCount &&
            BaseLayer == other.BaseLayer &&
            LayerCount == other.LayerCount &&
            Swizzle == other.Swizzle &&
            AspectMask == other.AspectMask );
    }
    bool operator !=(const FImageViewDesc& other) const {
        return (not operator ==(other));
    }

    friend hash_t hash_value(const FImageViewDesc& it) {
        return hash_tuple(it.View, it.Format, it.BaseLevel, it.LevelCount, it.BaseLayer, it.LayerCount, it.Swizzle, it.AspectMask);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
