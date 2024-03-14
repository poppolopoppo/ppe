#pragma once

#include "Texture_fwd.h"

#include "Texture.h"
#include "RHI/SamplerEnums.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_TEXTURE_API FTextureCube final : public FTexture {
public:
    FTextureCube() NOEXCEPT;
    FTextureCube(
        const uint2& dimensions,
        const FTextureProperties& properties, FBulkData&& data,
        ETextureAddressMode addressModeU = Default,
        ETextureAddressMode addressModeV = Default) NOEXCEPT;

    FTextureCube(FTextureCube&& ) = default;
    FTextureCube& operator =(FTextureCube&& ) = default;

    const uint2& Dimensions() const { return _dimensions; }

    virtual u32 SurfaceWidth() const NOEXCEPT override { return _dimensions.x; }
    virtual u32 SurfaceHeight() const NOEXCEPT override { return _dimensions.y; }
    virtual u32 SurfaceDepth() const NOEXCEPT override { return 1; }
    virtual u32 ArraySize() const NOEXCEPT override { return 6; }

    virtual RHI::EAddressMode AddressModeU() const NOEXCEPT override { return _addressModeU; }
    virtual RHI::EAddressMode AddressModeV() const NOEXCEPT override { return _addressModeV; }
    virtual RHI::EAddressMode AddressModeW() const NOEXCEPT override { return RHI::EAddressMode::Unknown; }

protected:
    uint2 _dimensions;

    RHI::EAddressMode _addressModeU;
    RHI::EAddressMode _addressModeV;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
