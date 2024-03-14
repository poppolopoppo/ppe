#pragma once

#include "Texture_fwd.h"

#include "Texture.h"
#include "RHI/SamplerEnums.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_TEXTURE_API FTextureCubeArray final : public FTexture {
public:
    FTextureCubeArray() NOEXCEPT;
    FTextureCubeArray(
        const uint2& dimensions,
        u32 numCubes,
        const FTextureProperties& properties, FBulkData&& data,
        ETextureAddressMode addressModeU = Default,
        ETextureAddressMode addressModeV = Default) NOEXCEPT;

    FTextureCubeArray(FTextureCubeArray&& ) = default;
    FTextureCubeArray& operator =(FTextureCubeArray&& ) = default;

    const uint2& Dimensions() const { return _dimensions; }
    u32 NumCubes() const { return _numCubes; }

    virtual u32 SurfaceWidth() const NOEXCEPT override { return _dimensions.x; }
    virtual u32 SurfaceHeight() const NOEXCEPT override { return _dimensions.y; }
    virtual u32 SurfaceDepth() const NOEXCEPT override { return 1; }
    virtual u32 ArraySize() const NOEXCEPT override { return _numCubes * 6; }

    virtual ETextureAddressMode AddressModeU() const NOEXCEPT override { return _addressModeU; }
    virtual ETextureAddressMode AddressModeV() const NOEXCEPT override { return _addressModeV; }
    virtual ETextureAddressMode AddressModeW() const NOEXCEPT override { return ETextureAddressMode::Unknown; }

protected:
    uint2 _dimensions;
    u32 _numCubes{ 0 };

    ETextureAddressMode _addressModeU;
    ETextureAddressMode _addressModeV;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
