#pragma once

#include "Texture_fwd.h"

#include "TextureEnums.h"

#include "IO/BulkData.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_TEXTURE_API FTexture {
public:
    virtual ~FTexture() = default;

    NODISCARD const FBulkData& BulkData() const { return _bulkData; }

    NODISCARD FTextureGroupGuid TextureGroupGuid() const { return _textureGroupGuid; }

    NODISCARD ETextureGammaSpace GammaSpace() const { return _gammaSpace; }
    NODISCARD ETextureImageView ImageView() const { return _imageView; }
    NODISCARD ETexturePixelFormat PixelFormat() const { return _pixelFormat; }

    NODISCARD ETextureMipmapFilter MipmapFilter() const { return _mipmapFilter; }
    NODISCARD ETextureSampleFilter MinFilter() const { return _minFilter; }
    NODISCARD ETextureSampleFilter MagFilter() const { return _magFilter; }

    NODISCARD bool AllowAnisotropy() const { return _allowAnisotropy; }

    virtual u32 SurfaceWidth() const NOEXCEPT = 0;
    virtual u32 SurfaceHeight() const NOEXCEPT = 0;
    virtual u32 SurfaceDepth() const NOEXCEPT = 0;
    virtual u32 SurfaceArraySize() const NOEXCEPT = 0;

    virtual RHI::EAddressMode AddressModeU() const NOEXCEPT = 0;
    virtual RHI::EAddressMode AddressModeV() const NOEXCEPT = 0;
    virtual RHI::EAddressMode AddressModeW() const NOEXCEPT = 0;

protected:
    FTexture() NOEXCEPT;

    FBulkData _bulkData;

    FTextureGroupGuid _textureGroupGuid;

    ETextureGammaSpace _gammaSpace;
    ETextureImageView _imageView;
    ETexturePixelFormat _pixelFormat;

    ETextureMipmapFilter _mipmapFilter;
    ETextureSampleFilter _magFilter;
    ETextureSampleFilter _minFilter;

    bool _allowAnisotropy{ false };
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
