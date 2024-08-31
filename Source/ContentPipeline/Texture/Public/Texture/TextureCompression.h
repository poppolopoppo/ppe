#pragma once

#include "Texture_fwd.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class ETextureCompressionQuality : u8 {
    High    = 0,
    Medium,
    Low,

    Unknown = High,
};
//----------------------------------------------------------------------------
class FTextureCompressionSettings {
public:
    ETextureCompressionQuality Quality{ Default };
};
//----------------------------------------------------------------------------
class ITextureCompression {
public:
    virtual ~ITextureCompression() = default;

    NODISCARD virtual RHI::EPixelFormat Format() const NOEXCEPT = 0;

    NODISCARD virtual bool SupportsTextureSource(const FTextureSourceProperties& src, const FTextureCompressionSettings& settings) const NOEXCEPT = 0;

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
