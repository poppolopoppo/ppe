#pragma once

#include "Texture_fwd.h"

#include "Texture/TextureCompression.h"

#include "RHI/ResourceEnums.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <RHI::EPixelFormat _PixelFormat>
class TSTBDxtCompression final : public ITextureCompression {
public:
    NODISCARD virtual RHI::EPixelFormat Format() const NOEXCEPT override { return _PixelFormat; }

    NODISCARD virtual bool SupportsTextureSource(const FTextureSourceProperties& src, const FTextureCompressionSettings& settings) const NOEXCEPT override;

    NODISCARD virtual bool CompressTexture(FTexture2D& dst, const FTextureSource& src, const FTextureCompressionSettings& settings) const override;
    NODISCARD virtual bool CompressTexture(FTexture2DArray& dst, const FTextureSource& src, const FTextureCompressionSettings& settings) const override;
    NODISCARD virtual bool CompressTexture(FTexture3D& dst, const FTextureSource& src, const FTextureCompressionSettings& settings) const override;
    NODISCARD virtual bool CompressTexture(FTextureCube& dst, const FTextureSource& src, const FTextureCompressionSettings& settings) const override;
    NODISCARD virtual bool CompressTexture(FTextureCubeArray& dst, const FTextureSource& src, const FTextureCompressionSettings& settings) const override;

    NODISCARD static void CompressDxtBlock(
        const FRawMemory& dst, const FRawMemoryConst (&src)[4],
        const FTextureCompressionSettings& settings) NOEXCEPT;
};
//----------------------------------------------------------------------------
// Template instantiations
//----------------------------------------------------------------------------
#define PPE_STB_DXT_COMPRESSION_DECL(_PIXEL_FORMAT) \
    template <> void TSTBDxtCompression<RHI::EPixelFormat::_PIXEL_FORMAT>::CompressDxtBlock( \
        const FRawMemory& dst, const FRawMemoryConst (&src)[4], \
        const FTextureCompressionSettings& settings) NOEXCEPT; \
    extern template class TSTBDxtCompression<RHI::EPixelFormat::_PIXEL_FORMAT>; \
    using CONCAT(FSTBDxtCompression_, _PIXEL_FORMAT) = TSTBDxtCompression<RHI::EPixelFormat::_PIXEL_FORMAT>;
//----------------------------------------------------------------------------
PPE_STB_DXT_COMPRESSION_DECL(BC1_RGB8_UNorm     )
PPE_STB_DXT_COMPRESSION_DECL(BC1_sRGB8          )
PPE_STB_DXT_COMPRESSION_DECL(BC3_RGBA8_UNorm    )
PPE_STB_DXT_COMPRESSION_DECL(BC3_sRGB8_A8       )
PPE_STB_DXT_COMPRESSION_DECL(BC4_R8_UNorm       )
PPE_STB_DXT_COMPRESSION_DECL(BC5_RG8_SNorm      )
//----------------------------------------------------------------------------
#undef PPE_STB_DXT_COMPRESSION_DECL
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
