#pragma once

#include "Texture_fwd.h"

#include "Container/Appendable.h"
#include "IO/FileSystem_fwd.h"
#include "Meta/Optional.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ITextureService {
public:
    virtual ~ITextureService() = default;

public: // Interface:

    /** Image formats **/

    virtual size_t AllImageFormats(TAppendable<ContentPipeline::UImageFormat> outImageFormats) const = 0;
    virtual void RegisterImageFormat(ContentPipeline::EImageFormat format, ContentPipeline::UImageFormat&& impl) NOEXCEPT = 0;

    NODISCARD virtual ContentPipeline::UImageFormat ImageFormat(ContentPipeline::EImageFormat format) const NOEXCEPT = 0;
    NODISCARD virtual ContentPipeline::UImageFormat ImageFormat(
        ContentPipeline::EImageFormat format,
        const ContentPipeline::FTextureSourceProperties& properties) const NOEXCEPT = 0;

    /** Texture compressions **/

    virtual size_t AllTextureCompressions(TAppendable<ContentPipeline::UTextureCompression> outTextureCompressions) const = 0;
    virtual void RegisterTextureCompression(RHI::EPixelFormat format, ContentPipeline::UTextureCompression&& impl) NOEXCEPT = 0;

    NODISCARD virtual ContentPipeline::UTextureCompression TextureCompression(RHI::EPixelFormat format) const NOEXCEPT = 0;
    NODISCARD virtual ContentPipeline::UTextureCompression TextureCompression(
        RHI::EPixelFormat format,
        const ContentPipeline::FTextureSourceProperties& properties,
        const ContentPipeline::FTextureCompressionSettings& settings) const NOEXCEPT = 0;

public: // Public helpers:

    PPE_TEXTURE_API NODISCARD ContentPipeline::UImageFormat ImageFormat(const FExtname& extname) const NOEXCEPT;

    PPE_TEXTURE_API NODISCARD Meta::TOptional<ContentPipeline::FTextureSource> ImportTextureSource2D(const FFilename& sourceFile) const;
    PPE_TEXTURE_API NODISCARD Meta::TOptional<ContentPipeline::FTextureSource> ImportTextureSource2DArray(const FFilename& sourceFile) const;
    PPE_TEXTURE_API NODISCARD Meta::TOptional<ContentPipeline::FTextureSource> ImportTextureSource3D(const FFilename& sourceFile) const;
    PPE_TEXTURE_API NODISCARD Meta::TOptional<ContentPipeline::FTextureSource> ImportTextureSourceCube(const FFilename& sourceFile) const;
    PPE_TEXTURE_API NODISCARD Meta::TOptional<ContentPipeline::FTextureSource> ImportTextureSourceCubeArray(const FFilename& sourceFile) const;
    PPE_TEXTURE_API NODISCARD Meta::TOptional<ContentPipeline::FTextureSource> ImportTextureSourceCubeLongLat(const FFilename& sourceFile) const;

    PPE_TEXTURE_API NODISCARD bool ExportTextureSource2D(const FFilename& sourceFile, const ContentPipeline::FTextureSource& sourceData) const;
    PPE_TEXTURE_API NODISCARD bool ExportTextureSource2DArray(const FFilename& sourceFile, const ContentPipeline::FTextureSource& sourceData) const;
    PPE_TEXTURE_API NODISCARD bool ExportTextureSource3D(const FFilename& sourceFile, const ContentPipeline::FTextureSource& sourceData) const;
    PPE_TEXTURE_API NODISCARD bool ExportTextureSourceCube(const FFilename& sourceFile, const ContentPipeline::FTextureSource& sourceData) const;
    PPE_TEXTURE_API NODISCARD bool ExportTextureSourceCubeArray(const FFilename& sourceFile, const ContentPipeline::FTextureSource& sourceData) const;
    PPE_TEXTURE_API NODISCARD bool ExportTextureSourceCubeLongLat(const FFilename& sourceFile, const ContentPipeline::FTextureSource& sourceData) const;

    PPE_TEXTURE_API NODISCARD bool ExportTexture2D(const FFilename& sourceFile, const ContentPipeline::FTextureSourceProperties& properties, const FRawMemoryConst& bulk) const;
    PPE_TEXTURE_API NODISCARD bool ExportTexture2DArray(const FFilename& sourceFile, const ContentPipeline::FTextureSourceProperties& properties, const FRawMemoryConst& bulk) const;
    PPE_TEXTURE_API NODISCARD bool ExportTexture3D(const FFilename& sourceFile, const ContentPipeline::FTextureSourceProperties& properties, const FRawMemoryConst& bulk) const;
    PPE_TEXTURE_API NODISCARD bool ExportTextureCube(const FFilename& sourceFile, const ContentPipeline::FTextureSourceProperties& properties, const FRawMemoryConst& bulk) const;
    PPE_TEXTURE_API NODISCARD bool ExportTextureCubeArray(const FFilename& sourceFile, const ContentPipeline::FTextureSourceProperties& properties, const FRawMemoryConst& bulk) const;
    PPE_TEXTURE_API NODISCARD bool ExportTextureCubeLongLat(const FFilename& sourceFile, const ContentPipeline::FTextureSourceProperties& properties, const FRawMemoryConst& bulk) const;

public:
    static PPE_TEXTURE_API void MakeDefault(UTextureService* texture);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
