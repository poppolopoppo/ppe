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
// PNG
//----------------------------------------------------------------------------
template <> bool TSTBImageFormat<EImageFormat::PNG>::SupportsTextureFormat(ETextureSourceFormat fmt) const NOEXCEPT;
template <> bool TSTBImageFormat<EImageFormat::PNG>::ExportTexture2D(IStreamWriter* output, const FTextureSourceProperties& properties, const FRawMemoryConst& bulk) const;
//----------------------------------------------------------------------------
// BMP
//----------------------------------------------------------------------------
template <> bool TSTBImageFormat<EImageFormat::BMP>::SupportsTextureFormat(ETextureSourceFormat fmt) const NOEXCEPT;
template <> bool TSTBImageFormat<EImageFormat::BMP>::ExportTexture2D(IStreamWriter* output, const FTextureSourceProperties& properties, const FRawMemoryConst& bulk) const;
//----------------------------------------------------------------------------
// TGA
//----------------------------------------------------------------------------
template <> bool TSTBImageFormat<EImageFormat::TGA>::SupportsTextureFormat(ETextureSourceFormat fmt) const NOEXCEPT;
template <> bool TSTBImageFormat<EImageFormat::TGA>::ExportTexture2D(IStreamWriter* output, const FTextureSourceProperties& properties, const FRawMemoryConst& bulk) const;
//----------------------------------------------------------------------------
// JPG
//----------------------------------------------------------------------------
template <> bool TSTBImageFormat<EImageFormat::JPG>::SupportsTextureFormat(ETextureSourceFormat fmt) const NOEXCEPT;
template <> bool TSTBImageFormat<EImageFormat::JPG>::ExportTexture2D(IStreamWriter* output, const FTextureSourceProperties& properties, const FRawMemoryConst& bulk) const;
//----------------------------------------------------------------------------
// HDR
//----------------------------------------------------------------------------
template <> bool TSTBImageFormat<EImageFormat::HDR>::SupportsTextureFormat(ETextureSourceFormat fmt) const NOEXCEPT;
template <> bool TSTBImageFormat<EImageFormat::HDR>::ExportTexture2D(IStreamWriter* output, const FTextureSourceProperties& properties, const FRawMemoryConst& bulk) const;
//----------------------------------------------------------------------------
extern template class TSTBImageFormat<EImageFormat::PNG>;
extern template class TSTBImageFormat<EImageFormat::BMP>;
extern template class TSTBImageFormat<EImageFormat::TGA>;
extern template class TSTBImageFormat<EImageFormat::JPG>;
extern template class TSTBImageFormat<EImageFormat::HDR>;
//----------------------------------------------------------------------------
using FSTBImagePNG = TSTBImageFormat<EImageFormat::PNG>;
using FSTBImageBMP = TSTBImageFormat<EImageFormat::BMP>;
using FSTBImageTGA = TSTBImageFormat<EImageFormat::TGA>;
using FSTBImageJPG = TSTBImageFormat<EImageFormat::JPG>;
using FSTBImageHDR = TSTBImageFormat<EImageFormat::HDR>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
