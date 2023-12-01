#pragma once

#include "Texture_fwd.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FTextureCompressionSettings {
public:

};
//----------------------------------------------------------------------------
class ITextureCompression : Meta::FNonCopyableNorMovable {
public:
    virtual ~ITextureCompression() = default;

    NODISCARD virtual RHI::EPixelFormat PixelFormat() const NOEXCEPT = 0;

    NODISCARD virtual bool SupportsImageView(RHI::EImageView view) const NOEXCEPT = 0;
    NODISCARD virtual bool SupportsTextureFormat(ETextureSourceFormat fmt) const NOEXCEPT = 0;
    NODISCARD virtual bool SupportsTextureSource(const FTextureSourceProperties& properties, const FTextureCompressionSettings& settings) const NOEXCEPT = 0;

    NODISCARD virtual bool CompressTexture(FTexture2D& dst, const FTextureSource& src, const FTextureCompressionSettings& settings) const = 0;
    NODISCARD virtual bool CompressTexture(FTexture2DArray& dst, const FTextureSource& src, const FTextureCompressionSettings& settings) const = 0;
    NODISCARD virtual bool CompressTexture(FTexture3D& dst, const FTextureSource& src, const FTextureCompressionSettings& settings) const = 0;
    NODISCARD virtual bool CompressTexture(FTextureCube& dst, const FTextureSource& src, const FTextureCompressionSettings& settings) const = 0;
    NODISCARD virtual bool CompressTexture(FTextureCubeArray& dst, const FTextureSource& src, const FTextureCompressionSettings& settings) const = 0;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
