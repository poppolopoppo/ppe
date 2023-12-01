#pragma once

#include "Texture_fwd.h"

#include "Texture.h"

#include "RHI/SamplerEnums.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_TEXTURE_API FTexture2D final : public FTexture {
public:
    FTexture2D() NOEXCEPT;

    const uint2& Dimensions() const { return _dimensions; }

    virtual u32 SurfaceWidth() const NOEXCEPT override { return _dimensions.x; }
    virtual u32 SurfaceHeight() const NOEXCEPT override { return _dimensions.y; }
    virtual u32 SurfaceDepth() const NOEXCEPT override { return 1; }
    virtual u32 SurfaceArraySize() const NOEXCEPT override { return 1; }

    virtual ETextureAddressMode AddressModeU() const NOEXCEPT override { return _addressModeU; }
    virtual ETextureAddressMode AddressModeV() const NOEXCEPT override { return _addressModeV; }
    virtual ETextureAddressMode AddressModeW() const NOEXCEPT override { return ETextureAddressMode::Unknown; }

protected:
    uint2 _dimensions;

    ETextureAddressMode _addressModeU;
    ETextureAddressMode _addressModeV;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
