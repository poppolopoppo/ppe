#pragma once

#include "Texture_fwd.h"

#include "TextureEnums.h"

#include "IO/BulkData.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FTextureProperties {
public:
    FTextureGroupId TextureGroup{ Default };

    ETextureGammaSpace Gamma{ Default };
    ETextureImageView ImageView{ Default };
    ETexturePixelFormat Format{ Default };

    ETextureMipmapFilter MipmapFilter{ Default };
    ETextureSampleFilter MagFilter{ Default };
    ETextureSampleFilter MinFilter{ Default };

    u8 NumMips{ 1 };

    bool AllowAnisotropy{ true };
};
//----------------------------------------------------------------------------
class PPE_TEXTURE_API FTexture : public FRefCountable {
public:
    virtual ~FTexture() = default;

    NODISCARD const FBulkData& Data() const { return _data; }

    NODISCARD FTextureGroupId TextureGroup() const { return _properties.TextureGroup; }

    NODISCARD ETextureGammaSpace Gamma() const { return _properties.Gamma; }
    NODISCARD ETextureImageView ImageView() const { return _properties.ImageView; }
    NODISCARD ETexturePixelFormat Format() const { return _properties.Format; }

    NODISCARD ETextureMipmapFilter MipmapFilter() const { return _properties.MipmapFilter; }
    NODISCARD ETextureSampleFilter MinFilter() const { return _properties.MinFilter; }
    NODISCARD ETextureSampleFilter MagFilter() const { return _properties.MagFilter; }

    NODISCARD u32 NumMips() const { return _properties.NumMips; }

    NODISCARD bool AllowAnisotropy() const { return _properties.AllowAnisotropy; }

    NODISCARD uint3 Dimensions() const { return { SurfaceWidth(), SurfaceHeight(), SurfaceDepth() }; }

    virtual u32 SurfaceWidth() const NOEXCEPT = 0;
    virtual u32 SurfaceHeight() const NOEXCEPT = 0;
    virtual u32 SurfaceDepth() const NOEXCEPT = 0;
    virtual u32 ArraySize() const NOEXCEPT = 0;

    virtual RHI::EAddressMode AddressModeU() const NOEXCEPT = 0;
    virtual RHI::EAddressMode AddressModeV() const NOEXCEPT = 0;
    virtual RHI::EAddressMode AddressModeW() const NOEXCEPT = 0;

    NODISCARD RHI::FImageID CreateTextureRHI(RHI::IFrameGraph& fg) const;

protected:
    FTexture() NOEXCEPT;
    FTexture(const FTextureProperties& properties, FBulkData&& data) NOEXCEPT;

    FTexture(FTexture&& ) = default;
    FTexture& operator =(FTexture&& ) = default;

    FBulkData _data;
    FTextureProperties _properties;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
