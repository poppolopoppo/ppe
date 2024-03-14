#pragma once

#include "Texture_fwd.h"

#include "Texture/ImageFormat.h"
#include "Texture/TextureEnums.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <EImageFormat _ImageFormat>
class TSTBImageFormat final : public IImageFormat {
public:
    NODISCARD virtual EImageFormat ImageFormat() const NOEXCEPT override { return _ImageFormat; }

    NODISCARD virtual bool SupportsImageView(RHI::EImageView view) const NOEXCEPT override;
    NODISCARD virtual bool SupportsTextureFormat(ETextureSourceFormat fmt) const NOEXCEPT override;
    NODISCARD virtual bool SupportsTextureSource(const FTextureSourceProperties& properties) const NOEXCEPT override;

    NODISCARD virtual bool ExportTexture2D(IStreamWriter* output, const FTextureSourceProperties& properties, const FRawMemoryConst& bulk) const override;
    NODISCARD virtual bool ExportTexture2DArray(IStreamWriter* output, const FTextureSourceProperties& properties, const FRawMemoryConst& bulk) const override;
    NODISCARD virtual bool ExportTexture3D(IStreamWriter* output, const FTextureSourceProperties& properties, const FRawMemoryConst& bulk) const override;
    NODISCARD virtual bool ExportTextureCube(IStreamWriter* output, const FTextureSourceProperties& properties, const FRawMemoryConst& bulk) const override;
    NODISCARD virtual bool ExportTextureCubeArray(IStreamWriter* output, const FTextureSourceProperties& properties, const FRawMemoryConst& bulk) const override;
    NODISCARD virtual bool ExportTextureCubeLongLat(IStreamWriter* output, const FTextureSourceProperties& properties, const FRawMemoryConst& bulk) const override;

    NODISCARD virtual FTextureImporterResult ImportTexture2D(FTextureSourceProperties* outProperties, const FRawMemoryConst& memory) const override;
    NODISCARD virtual FTextureImporterResult ImportTexture2DArray(FTextureSourceProperties* outProperties, const FRawMemoryConst& memory) const override;
    NODISCARD virtual FTextureImporterResult ImportTexture3D(FTextureSourceProperties* outProperties, const FRawMemoryConst& memory) const override;
    NODISCARD virtual FTextureImporterResult ImportTextureCube(FTextureSourceProperties* outProperties, const FRawMemoryConst& memory) const override;
    NODISCARD virtual FTextureImporterResult ImportTextureCubeArray(FTextureSourceProperties* outProperties, const FRawMemoryConst& memory) const override;
    NODISCARD virtual FTextureImporterResult ImportTextureCubeLongLat(FTextureSourceProperties* outProperties, const FRawMemoryConst& memory) const override;

    NODISCARD virtual FTextureImporterResult ImportTexture2D(FTextureSourceProperties* outProperties, IStreamReader& input) const override;
    NODISCARD virtual FTextureImporterResult ImportTexture2DArray(FTextureSourceProperties* outProperties, IStreamReader& input) const override;
    NODISCARD virtual FTextureImporterResult ImportTexture3D(FTextureSourceProperties* outProperties, IStreamReader& input) const override;
    NODISCARD virtual FTextureImporterResult ImportTextureCube(FTextureSourceProperties* outProperties, IStreamReader& input) const override;
    NODISCARD virtual FTextureImporterResult ImportTextureCubeArray(FTextureSourceProperties* outProperties, IStreamReader& input) const override;
    NODISCARD virtual FTextureImporterResult ImportTextureCubeLongLat(FTextureSourceProperties* outProperties, IStreamReader& input) const override;
};
//----------------------------------------------------------------------------
// Template instantiations
//----------------------------------------------------------------------------
#define PPE_STB_IMAGE_FORMAT_DECL(_IMAGE_FORMAT) \
    template <> bool TSTBImageFormat<EImageFormat::_IMAGE_FORMAT>::SupportsTextureFormat(ETextureSourceFormat fmt) const NOEXCEPT; \
    template <> bool TSTBImageFormat<EImageFormat::_IMAGE_FORMAT>::ExportTexture2D(IStreamWriter* output, const FTextureSourceProperties& properties, const FRawMemoryConst& bulk) const; \
    extern template class TSTBImageFormat<EImageFormat::_IMAGE_FORMAT>; \
    using CONCAT(FSTBImage_, _IMAGE_FORMAT) = TSTBImageFormat<EImageFormat::_IMAGE_FORMAT>;
//----------------------------------------------------------------------------
PPE_STB_IMAGE_FORMAT_DECL(PNG)
PPE_STB_IMAGE_FORMAT_DECL(BMP)
PPE_STB_IMAGE_FORMAT_DECL(TGA)
PPE_STB_IMAGE_FORMAT_DECL(JPG)
PPE_STB_IMAGE_FORMAT_DECL(HDR)
//----------------------------------------------------------------------------
#undef PPE_STB_IMAGE_FORMAT_DECL
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
