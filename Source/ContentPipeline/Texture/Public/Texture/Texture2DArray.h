﻿#pragma once

#include "Texture_fwd.h"

#include "Texture.h"

#include "RHI/SamplerEnums.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_TEXTURE_API FTexture2DArray final : public FTexture {
public:
    FTexture2DArray() NOEXCEPT;
    FTexture2DArray(
        const uint2& dimensions,
        u32 numSlices,
        const FTextureProperties& properties, FBulkData&& data,
        ETextureAddressMode addressModeU = Default,
        ETextureAddressMode addressModeV = Default) NOEXCEPT;

    FTexture2DArray(FTexture2DArray&& ) = default;
    FTexture2DArray& operator =(FTexture2DArray&& ) = default;

    const uint2& Dimensions() const { return _dimensions; }

    virtual u32 SurfaceWidth() const NOEXCEPT override { return _dimensions.x; }
    virtual u32 SurfaceHeight() const NOEXCEPT override { return _dimensions.y; }
    virtual u32 SurfaceDepth() const NOEXCEPT override { return 1; }
    virtual u32 ArraySize() const NOEXCEPT override { return _numSlices; }

    virtual ETextureAddressMode AddressModeU() const NOEXCEPT override { return _addressModeU; }
    virtual ETextureAddressMode AddressModeV() const NOEXCEPT override { return _addressModeV; }
    virtual ETextureAddressMode AddressModeW() const NOEXCEPT override { return ETextureAddressMode::Unknown; }

protected:
    uint2 _dimensions;
    u32 _numSlices{ 0 };

    ETextureAddressMode _addressModeU;
    ETextureAddressMode _addressModeV;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
