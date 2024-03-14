#pragma once

#include "Texture_fwd.h"

#include "Texture/TextureCompression.h"
#include "Texture/TextureEnums.h"

#include "RHI/ResourceEnums.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <RHI::EPixelFormat _PixelFormat, ETextureSourceFormat _SourceFormat, ETextureGammaSpace _GammaSpace>
class TPassthroughCompression final : public ITextureCompression {
public:
    NODISCARD virtual RHI::EPixelFormat Format() const NOEXCEPT override { return _PixelFormat; }

    NODISCARD virtual bool SupportsTextureSource(const FTextureSourceProperties& src, const FTextureCompressionSettings& settings) const NOEXCEPT override;

    NODISCARD virtual bool CompressTexture(FTexture2D& dst, const FTextureSource& src, const FTextureCompressionSettings& settings) const override;
    NODISCARD virtual bool CompressTexture(FTexture2DArray& dst, const FTextureSource& src, const FTextureCompressionSettings& settings) const override;
    NODISCARD virtual bool CompressTexture(FTexture3D& dst, const FTextureSource& src, const FTextureCompressionSettings& settings) const override;
    NODISCARD virtual bool CompressTexture(FTextureCube& dst, const FTextureSource& src, const FTextureCompressionSettings& settings) const override;
    NODISCARD virtual bool CompressTexture(FTextureCubeArray& dst, const FTextureSource& src, const FTextureCompressionSettings& settings) const override;
};
//----------------------------------------------------------------------------
// Template instantiations
//----------------------------------------------------------------------------
#define PPE_PASSTHROUGH_COMPRESSION_DECL(_PIXEL_FORMAT, _SOURCE_FORMAT, _GAMMA_SPACE) \
    extern template class TPassthroughCompression<RHI::EPixelFormat::_PIXEL_FORMAT, ETextureSourceFormat::_SOURCE_FORMAT, ETextureGammaSpace::_GAMMA_SPACE>; \
    using CONCAT(FPassthroughCompression_, _PIXEL_FORMAT) = TPassthroughCompression<RHI::EPixelFormat::_PIXEL_FORMAT, ETextureSourceFormat::_SOURCE_FORMAT, ETextureGammaSpace::_GAMMA_SPACE>;
//----------------------------------------------------------------------------
PPE_PASSTHROUGH_COMPRESSION_DECL(BGRA8_UNorm,    BGRA8,      Linear  )
PPE_PASSTHROUGH_COMPRESSION_DECL(sBGR8_A8,       BGRA8,      sRGB    )
PPE_PASSTHROUGH_COMPRESSION_DECL(R16_UNorm,      G16,        Linear  )
PPE_PASSTHROUGH_COMPRESSION_DECL(R8_UNorm,       G8,         Linear  )
PPE_PASSTHROUGH_COMPRESSION_DECL(R16f,           R16f,       Linear  )
PPE_PASSTHROUGH_COMPRESSION_DECL(RG16_UNorm,     RG16,       Linear  )
PPE_PASSTHROUGH_COMPRESSION_DECL(RG8_UNorm,      RG8,        Linear  )
PPE_PASSTHROUGH_COMPRESSION_DECL(RGBA16_UNorm,   RGBA16,     Linear  )
PPE_PASSTHROUGH_COMPRESSION_DECL(RGBA16f,        RGBA16f,    Linear  )
PPE_PASSTHROUGH_COMPRESSION_DECL(RGBA32f,        RGBA32f,    Linear  )
PPE_PASSTHROUGH_COMPRESSION_DECL(RGBA8_UNorm,    RGBA8,      Linear  )
PPE_PASSTHROUGH_COMPRESSION_DECL(sRGB8_A8,       RGBA8,      sRGB    )
//----------------------------------------------------------------------------
#undef PPE_PASSTHROUGH_COMPRESSION_DECL
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
