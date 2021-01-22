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
    EImageType Type{ Default };
    uint3 Dimensions{ 0 };
    EPixelFormat Format{ Default };
    EImageUsage Usage{ Default };
    FImageLayer ArrayLayers;
    FMipmapLevel MaxLevel;
    FMultiSamples Samples;
    EQueueUsage Queues{ Default };
    bool IsExternal{ false };

    FImageDesc() = default;
    FImageDesc(
        EImageType type,
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

    PPE_RHI_API bool Validate() const;

    friend hash_t hash_value(const FImageDesc& it) {
        return hash_tuple(it.Type, it.Dimensions, it.Format, it.Usage, it.ArrayLayers, it.MaxLevel, it.Queues);
    }
};
//----------------------------------------------------------------------------
struct FImageViewDesc {
    EImageType Type{ Default };
    EPixelFormat Format{ Default };
    FMipmapLevel BaseLevel;
    u32 NumLevels{ 1 };
    FImageLayer BaseLayer;
    u32 NumLayers{ 1 };
    FImageSwizzle Swizzle;
    EImageAspect AspectMask{ Default };

    FImageViewDesc() = default;
    FImageViewDesc(
        EImageType type,
        EPixelFormat format,
        FMipmapLevel baseLevel = Default,
        u32 numLevels = 1,
        FImageLayer baseLayer = Default,
        u32 numLayers = 1,
        FImageSwizzle swizzle = Default,
        EImageAspect aspectMask = Default )
    :   Type(type)
    ,   Format(format)
    ,   BaseLevel(baseLevel)
    ,   NumLevels(numLevels)
    ,   BaseLayer(baseLayer)
    ,   NumLayers(numLayers)
    ,   Swizzle(swizzle)
    ,   AspectMask(aspectMask)
    {}

    PPE_RHI_API explicit FImageViewDesc(const FImageDesc& desc);
    PPE_RHI_API bool Validate(const FImageDesc& desc) const;
    PPE_RHI_API bool operator ==(const FImageViewDesc& other) const;

    bool operator !=(const FImageViewDesc& other) const {
        return (not operator ==(other));
    }

    friend hash_t hash_value(const FImageViewDesc& it) {
        return hash_tuple(it.Type, it.Format, it.BaseLevel, it.NumLevels, it.BaseLayer, it.NumLayers, it.Swizzle, it.AspectMask);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
