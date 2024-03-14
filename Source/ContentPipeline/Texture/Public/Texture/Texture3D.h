#pragma once

#include "Texture_fwd.h"

#include "Texture.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_TEXTURE_API FTexture3D final : public FTexture {
public:
    FTexture3D() NOEXCEPT;
    FTexture3D(
        const uint3& dimensions,
        const FTextureProperties& properties, FBulkData&& data,
        ETextureAddressMode addressModeU = Default,
        ETextureAddressMode addressModeV = Default,
        ETextureAddressMode addressModeW = Default) NOEXCEPT;

    FTexture3D(FTexture3D&& ) = default;
    FTexture3D& operator =(FTexture3D&& ) = default;

    const uint3& Dimensions() const { return _dimensions; }

    virtual u32 SurfaceWidth() const NOEXCEPT override { return _dimensions.x; }
    virtual u32 SurfaceHeight() const NOEXCEPT override { return _dimensions.y; }
    virtual u32 SurfaceDepth() const NOEXCEPT override { return _dimensions.z; }
    virtual u32 ArraySize() const NOEXCEPT override { return 1; }

    virtual ETextureAddressMode AddressModeU() const NOEXCEPT override { return _addressModeU; }
    virtual ETextureAddressMode AddressModeV() const NOEXCEPT override { return _addressModeV; }
    virtual ETextureAddressMode AddressModeW() const NOEXCEPT override { return _addressModeW; }

protected:
    uint3 _dimensions;

    ETextureAddressMode _addressModeU;
    ETextureAddressMode _addressModeV;
    ETextureAddressMode _addressModeW;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
