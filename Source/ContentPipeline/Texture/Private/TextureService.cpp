// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "TextureService.h"

#include "Texture/TextureCompression.h"
#include "Texture/TextureEnums.h"
#include "Texture/TextureSource.h"
#include "Texture/Image/STBImageFormat.h"

#include "RHI/ResourceEnums.h"

#include "Container/HashMap.h"
#include "Container/MultiMap.h"
#include "Diagnostic/Logger.h"
#include "IO/BufferedStream.h"
#include "Thread/ThreadSafe.h"
#include "VirtualFileSystem_fwd.h"

namespace PPE {
//-- --------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace ContentPipeline {
//----------------------------------------------------------------------------
class FDefaultTextureService_ final : public ITextureService {
public:
    FDefaultTextureService_()
    // register default image formats here:
    :   _imageFormats({
            { EImageFormat::PNG, UImageFormat::Make<FSTBImagePNG>() },
            { EImageFormat::BMP, UImageFormat::Make<FSTBImageBMP>() },
            { EImageFormat::TGA, UImageFormat::Make<FSTBImageTGA>() },
            { EImageFormat::JPG, UImageFormat::Make<FSTBImageJPG>() },
            { EImageFormat::HDR, UImageFormat::Make<FSTBImageHDR>() },
        })
    {}

public: // ITextureService

    /** Image formats **/

    virtual size_t AllImageFormats(TAppendable<UImageFormat> outImageFormats) const override {
        u32 numImageFormats = 0;
        for (const TPair<const EImageFormat, UImageFormat>& it : *_imageFormats.LockShared()) {
            outImageFormats.emplace_back(it.second);
            numImageFormats++;
        }
        return numImageFormats;
    }

    virtual void RegisterImageFormat(EImageFormat format, UImageFormat&& impl) NOEXCEPT override {
        _imageFormats.LockExclusive()->emplace(format, std::move(impl));
    }

    NODISCARD virtual UImageFormat ImageFormat(EImageFormat format) const NOEXCEPT override {
        const auto shared = _imageFormats.LockShared();

        if (const auto it = shared->find(format); it != shared->end())
            return it->second;

        return Default;
    }

    NODISCARD virtual UImageFormat ImageFormat(EImageFormat format, const FTextureSourceProperties& properties) const NOEXCEPT override {
        const auto shared = _imageFormats.LockShared();

        for (auto it = shared->find(format); it != shared->end(); ++it) {
            if (it->second->SupportsTextureSource(properties))
                return it->second;
        }

        return Default;
    }

    /** Texture compressions **/

    virtual size_t AllTextureCompressions(TAppendable<UTextureCompression> outTextureCompressions) const override {
        u32 numTextureCompressions = 0;
        for (const TPair<const RHI::EPixelFormat, UTextureCompression>& it : *_textureCompressions.LockShared()) {
            outTextureCompressions.emplace_back(it.second);
            numTextureCompressions++;
        }
        return numTextureCompressions;
    }

    virtual void RegisterTextureCompression(RHI::EPixelFormat format, UTextureCompression&& impl) NOEXCEPT override {
        Assert_NoAssume(format != RHI::EPixelFormat::Unknown);
        Assert_NoAssume(impl->PixelFormat() == format);
        _textureCompressions.LockExclusive()->insert({ format, std::move(impl) });
    }

    NODISCARD virtual UTextureCompression TextureCompression(RHI::EPixelFormat format) const NOEXCEPT override {
        const auto shared = _textureCompressions.LockShared();

        if (const auto it = shared->find(format); it != shared->end())
            return it->second;

        return Default;
    }

    NODISCARD virtual UTextureCompression TextureCompression(
        RHI::EPixelFormat format,
        const FTextureSourceProperties& properties,
        const FTextureCompressionSettings& settings) const NOEXCEPT override {
        const auto shared = _textureCompressions.LockShared();

        forrange(it, shared->find(format), shared->end())
            if (it->second->SupportsTextureSource(properties, settings))
                return it->second;

        return Default;
    }

private:
    TThreadSafe<MULTIMAP(Texture, EImageFormat, UImageFormat), EThreadBarrier::RWLock> _imageFormats;
    TThreadSafe<MULTIMAP(Texture, RHI::EPixelFormat, UTextureCompression), EThreadBarrier::RWLock> _textureCompressions;
};
//----------------------------------------------------------------------------
template <FTextureImporterResult (IImageFormat::*_ImportTexture)(FTextureSourceProperties*, IStreamReader&) const>
static Meta::TOptional<FTextureSource> ImportTextureSourceFromFile_(const ITextureService& textures, const FFilename& sourceFile) {
    const UImageFormat imageFormat = textures.ImageFormat(sourceFile.Extname());
    PPE_LOG_CHECKEX(Texture, Meta::TOptional<FTextureSource>{}, imageFormat.Valid());

    const UStreamReader input = VFS_OpenBinaryReadable(sourceFile);
    PPE_LOG_CHECKEX(Texture, Meta::TOptional<FTextureSource>{}, input.valid());

    FTextureSourceProperties sourceProperties;
    FTextureImporterResult importerResult = (*imageFormat.*_ImportTexture)(&sourceProperties, *input);

    if (not importerResult)
        return Meta::TOptional<FTextureSource>{};

    FTextureSource sourceData;
    sourceData.Construct(sourceProperties, std::move(importerResult));

    return Meta::MakeOptional(std::move(sourceData));
}
//----------------------------------------------------------------------------
template <bool (IImageFormat::* _ExportTexture)(IStreamWriter*, const FTextureSourceProperties&, const FRawMemoryConst&) const>
static bool ExportTextureToFile_(const ITextureService& textures, const FFilename& sourceFile, const FTextureSourceProperties& properties, const FRawMemoryConst& bulk) {
    const UImageFormat imageFormat = textures.ImageFormat(sourceFile.Extname());
    PPE_LOG_CHECK(Texture, imageFormat.Valid());

    const UStreamWriter output = VFS_OpenBinaryWritable(sourceFile, EAccessPolicy::Truncate);
    PPE_LOG_CHECK(Texture, output.valid());

    return (*imageFormat.*_ExportTexture)(output.get(), properties, bulk);
}
//----------------------------------------------------------------------------
template <bool (IImageFormat::* _ExportTexture)(IStreamWriter*, const FTextureSourceProperties&, const FRawMemoryConst&) const>
static bool ExportTextureSourceToFile_(const ITextureService& textures, const FFilename& sourceFile, const FTextureSource& sourceData) {
    return ExportTextureToFile_<_ExportTexture>(
        textures,
        sourceFile,
        sourceData.Properties(),
        sourceData.LockRead().MakeView());
}
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void ITextureService::MakeDefault(UTextureService* texture) {
    texture->create<ContentPipeline::FDefaultTextureService_>();
}
//----------------------------------------------------------------------------
// Deduce image format interface from file extension:
//----------------------------------------------------------------------------
ContentPipeline::UImageFormat ITextureService::ImageFormat(const FExtname& extname) const NOEXCEPT {
    const ContentPipeline::EImageFormat imageFormat = ContentPipeline::EImageFormat_FromExtname(extname);
    if (imageFormat != ContentPipeline::EImageFormat::Unknown)
        return ImageFormat(imageFormat);
    return Default;
}
//----------------------------------------------------------------------------
// Import texture source from file:
//----------------------------------------------------------------------------
Meta::TOptional<ContentPipeline::FTextureSource> ITextureService::ImportTextureSource2D(const FFilename& sourceFile) const {
    return ContentPipeline::ImportTextureSourceFromFile_<&ContentPipeline::IImageFormat::ImportTexture2D>(*this, sourceFile);
}
//----------------------------------------------------------------------------
Meta::TOptional<ContentPipeline::FTextureSource> ITextureService::ImportTextureSource2DArray(const FFilename& sourceFile) const {
    return ContentPipeline::ImportTextureSourceFromFile_<&ContentPipeline::IImageFormat::ImportTexture2DArray>(*this, sourceFile);
}
//----------------------------------------------------------------------------
Meta::TOptional<ContentPipeline::FTextureSource> ITextureService::ImportTextureSource3D(const FFilename& sourceFile) const {
    return ContentPipeline::ImportTextureSourceFromFile_<&ContentPipeline::IImageFormat::ImportTexture3D>(*this, sourceFile);
}
//----------------------------------------------------------------------------
Meta::TOptional<ContentPipeline::FTextureSource> ITextureService::ImportTextureSourceCube(const FFilename& sourceFile) const {
    return ContentPipeline::ImportTextureSourceFromFile_<&ContentPipeline::IImageFormat::ImportTextureCube>(*this, sourceFile);
}
//----------------------------------------------------------------------------
Meta::TOptional<ContentPipeline::FTextureSource> ITextureService::ImportTextureSourceCubeArray(const FFilename& sourceFile) const {
    return ContentPipeline::ImportTextureSourceFromFile_<&ContentPipeline::IImageFormat::ImportTextureCubeArray>(*this, sourceFile);
}
//----------------------------------------------------------------------------
Meta::TOptional<ContentPipeline::FTextureSource> ITextureService::ImportTextureSourceCubeLongLat(const FFilename& sourceFile) const {
    return ContentPipeline::ImportTextureSourceFromFile_<&ContentPipeline::IImageFormat::ImportTextureCubeLongLat>(*this, sourceFile);
}
//----------------------------------------------------------------------------
// Export texture source to file:
//----------------------------------------------------------------------------
bool ITextureService::ExportTextureSource2D(const FFilename& sourceFile, const ContentPipeline::FTextureSource& sourceData) const {
    return ContentPipeline::ExportTextureSourceToFile_<&ContentPipeline::IImageFormat::ExportTexture2D>(*this, sourceFile, sourceData);
}
//----------------------------------------------------------------------------
bool ITextureService::ExportTextureSource2DArray(const FFilename& sourceFile, const ContentPipeline::FTextureSource& sourceData) const {
    return ContentPipeline::ExportTextureSourceToFile_<&ContentPipeline::IImageFormat::ExportTexture2DArray>(*this, sourceFile, sourceData);
}
//----------------------------------------------------------------------------
bool ITextureService::ExportTextureSource3D(const FFilename& sourceFile, const ContentPipeline::FTextureSource& sourceData) const {
    return ContentPipeline::ExportTextureSourceToFile_<&ContentPipeline::IImageFormat::ExportTexture3D>(*this, sourceFile, sourceData);
}
//----------------------------------------------------------------------------
bool ITextureService::ExportTextureSourceCube(const FFilename& sourceFile, const ContentPipeline::FTextureSource& sourceData) const {
    return ContentPipeline::ExportTextureSourceToFile_<&ContentPipeline::IImageFormat::ExportTextureCube>(*this, sourceFile, sourceData);
}
//----------------------------------------------------------------------------
bool ITextureService::ExportTextureSourceCubeArray(const FFilename& sourceFile, const ContentPipeline::FTextureSource& sourceData) const {
    return ContentPipeline::ExportTextureSourceToFile_<&ContentPipeline::IImageFormat::ExportTextureCubeArray>(*this, sourceFile, sourceData);
}
//----------------------------------------------------------------------------
bool ITextureService::ExportTextureSourceCubeLongLat(const FFilename& sourceFile, const ContentPipeline::FTextureSource& sourceData) const {
    return ContentPipeline::ExportTextureSourceToFile_<&ContentPipeline::IImageFormat::ExportTextureCubeLongLat>(*this, sourceFile, sourceData);
}
//----------------------------------------------------------------------------
// Export texture data to file:
//----------------------------------------------------------------------------
bool ITextureService::ExportTexture2D(const FFilename& sourceFile, const ContentPipeline::FTextureSourceProperties& properties, const FRawMemoryConst& bulk) const {
    return ContentPipeline::ExportTextureToFile_<&ContentPipeline::IImageFormat::ExportTexture2D>(*this, sourceFile, properties, bulk);
}
bool ITextureService::ExportTexture2DArray(const FFilename& sourceFile, const ContentPipeline::FTextureSourceProperties& properties, const FRawMemoryConst& bulk) const {
    return ContentPipeline::ExportTextureToFile_<&ContentPipeline::IImageFormat::ExportTexture2DArray>(*this, sourceFile, properties, bulk);
}
bool ITextureService::ExportTexture3D(const FFilename& sourceFile, const ContentPipeline::FTextureSourceProperties& properties, const FRawMemoryConst& bulk) const {
    return ContentPipeline::ExportTextureToFile_<&ContentPipeline::IImageFormat::ExportTexture3D>(*this, sourceFile, properties, bulk);
}
bool ITextureService::ExportTextureCube(const FFilename& sourceFile, const ContentPipeline::FTextureSourceProperties& properties, const FRawMemoryConst& bulk) const {
    return ContentPipeline::ExportTextureToFile_<&ContentPipeline::IImageFormat::ExportTextureCube>(*this, sourceFile, properties, bulk);
}
bool ITextureService::ExportTextureCubeArray(const FFilename& sourceFile, const ContentPipeline::FTextureSourceProperties& properties, const FRawMemoryConst& bulk) const {
    return ContentPipeline::ExportTextureToFile_<&ContentPipeline::IImageFormat::ExportTextureCubeArray>(*this, sourceFile, properties, bulk);
}
bool ITextureService::ExportTextureCubeLongLat(const FFilename& sourceFile, const ContentPipeline::FTextureSourceProperties& properties, const FRawMemoryConst& bulk) const {
    return ContentPipeline::ExportTextureToFile_<&ContentPipeline::IImageFormat::ExportTextureCubeLongLat>(*this, sourceFile, properties, bulk);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
