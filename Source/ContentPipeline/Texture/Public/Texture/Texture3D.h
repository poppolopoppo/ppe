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

    const uint3& Dimensions() const { return _dimensions; }

    virtual u32 SurfaceWidth() const NOEXCEPT override { return _dimensions.x; }
    virtual u32 SurfaceHeight() const NOEXCEPT override { return _dimensions.y; }
    virtual u32 SurfaceDepth() const NOEXCEPT override { return _dimensions.z; }
    virtual u32 SurfaceArraySize() const NOEXCEPT override { return 1; }

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
