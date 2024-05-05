// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "TextureService.h"

#include "Texture/EnumToString.h"
#include "Texture/TextureCompression.h"
#include "Texture/TextureEnums.h"
#include "Texture/TextureSource.h"

#include "Texture/Texture.h"

#include "Texture/Compression/PassthroughCompression.h"
#include "Texture/Compression/STBDxtCompression.h"
#include "Texture/Image/STBImageFormat.h"

#include "RHI/ResourceEnums.h"

#include "Container/HashMap.h"
#include "Container/MultiMap.h"
#include "Diagnostic/Logger.h"
#include "IO/BufferedStream.h"
#include "IO/Format.h"
#include "IO/ObservableStream.h"
#include "IO/StringBuilder.h"
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
            // common DCC image formats
            { EImageFormat::PNG, UImageFormat::Make<FSTBImage_PNG>() },
            { EImageFormat::BMP, UImageFormat::Make<FSTBImage_BMP>() },
            { EImageFormat::TGA, UImageFormat::Make<FSTBImage_TGA>() },
            { EImageFormat::JPG, UImageFormat::Make<FSTBImage_JPG>() },
            { EImageFormat::HDR, UImageFormat::Make<FSTBImage_HDR>() },
        })
    // register default texture compressions here:
    ,   _textureCompressions({
        // DXT block compression
            { RHI::EPixelFormat::BC1_RGB8_UNorm,    UTextureCompression::Make<FSTBDxtCompression_BC1_RGB8_UNorm>() },
            { RHI::EPixelFormat::BC1_sRGB8,         UTextureCompression::Make<FSTBDxtCompression_BC1_sRGB8>() },
            { RHI::EPixelFormat::BC3_RGBA8_UNorm,   UTextureCompression::Make<FSTBDxtCompression_BC3_RGBA8_UNorm>() },
            { RHI::EPixelFormat::BC3_sRGB8_A8,      UTextureCompression::Make<FSTBDxtCompression_BC3_sRGB8_A8>() },
            { RHI::EPixelFormat::BC4_R8_UNorm,      UTextureCompression::Make<FSTBDxtCompression_BC4_R8_UNorm>() },
            { RHI::EPixelFormat::BC5_RG8_SNorm,     UTextureCompression::Make<FSTBDxtCompression_BC5_RG8_SNorm>() },
            // uncompressed
            { RHI::EPixelFormat::BGRA8_UNorm,       UTextureCompression::Make<FPassthroughCompression_BGRA8_UNorm>() },
            { RHI::EPixelFormat::sBGR8_A8,          UTextureCompression::Make<FPassthroughCompression_sBGR8_A8>() },
            { RHI::EPixelFormat::R16_UNorm,         UTextureCompression::Make<FPassthroughCompression_R16_UNorm>() },
            { RHI::EPixelFormat::R8_UNorm,          UTextureCompression::Make<FPassthroughCompression_R8_UNorm>() },
            { RHI::EPixelFormat::R16f,              UTextureCompression::Make<FPassthroughCompression_R16f>() },
            { RHI::EPixelFormat::RG16_UNorm,        UTextureCompression::Make<FPassthroughCompression_RG16_UNorm>() },
            { RHI::EPixelFormat::RG8_UNorm,         UTextureCompression::Make<FPassthroughCompression_RG8_UNorm>() },
            { RHI::EPixelFormat::RGBA16_UNorm,      UTextureCompression::Make<FPassthroughCompression_RGBA16_UNorm>() },
            { RHI::EPixelFormat::RGBA16f,           UTextureCompression::Make<FPassthroughCompression_RGBA16f>() },
            { RHI::EPixelFormat::RGBA32f,           UTextureCompression::Make<FPassthroughCompression_RGBA32f>() },
            { RHI::EPixelFormat::RGBA8_UNorm,       UTextureCompression::Make<FPassthroughCompression_RGBA8_UNorm>() },
            { RHI::EPixelFormat::sRGB8_A8,          UTextureCompression::Make<FPassthroughCompression_sRGB8_A8>() },
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
        Assert_NoAssume(impl->Format() == format);
        _textureCompressions.LockExclusive()->insert({ format, std::move(impl) });
    }

    NODISCARD virtual UTextureCompression TextureCompression(RHI::EPixelFormat format) const NOEXCEPT override {
        const auto shared = _textureCompressions.LockShared();

        if (const auto it = shared->find(format); it != shared->end())
            return it->second;

        return Default;
    }

    NODISCARD virtual bool TextureCompression(
        TAppendable<UTextureCompression> outTextureCompressions,
        const FTextureSourceProperties& src,
        const FTextureCompressionSettings& settings) const NOEXCEPT override {
        const auto shared = _textureCompressions.LockShared();

        bool found = false;
        for (const auto& it : *shared) {
            if (it.second->SupportsTextureSource(src, settings)) {
                found = true;
                outTextureCompressions.emplace_back(it.second);
            }
        }

        return found;
    }

private:
    TThreadSafe<MULTIMAP(Texture, EImageFormat, UImageFormat), EThreadBarrier::RWLock> _imageFormats;
    TThreadSafe<MULTIMAP(Texture, RHI::EPixelFormat, UTextureCompression), EThreadBarrier::RWLock> _textureCompressions;
};
//----------------------------------------------------------------------------
template <FTextureImporterResult (IImageFormat::*_ImportTexture)(FTextureSourceProperties*, IStreamReader&) const>
static Meta::TOptional<FTextureSource> ImportTextureSourceFromFile_(const ITextureService& textures, const FFilename& sourceFile) {
    MEMORYDOMAIN_THREAD_SCOPE(Texture);

    const UImageFormat imageFormat = textures.ImageFormat(sourceFile.Extname());
    PPE_LOG_CHECKEX(Texture, Meta::TOptional<FTextureSource>{}, imageFormat.Valid());

    const UStreamReader input = VFS_OpenBinaryReadable(sourceFile);
    PPE_LOG_CHECKEX(Texture, Meta::TOptional<FTextureSource>{}, input.valid());

    FTextureSourceProperties sourceProperties;
    FTextureImporterResult importerResult = UsingStreamWithProgress(*input,
        INLINE_FORMAT(50 + FileSystem::MaxPathLength, "Building texture: \"{}\"", sourceFile),
        [&imageFormat, &sourceProperties](TPtrRef<IStreamReader> rd) {
            return (*imageFormat.*_ImportTexture)(&sourceProperties, rd);
        });
    if (not importerResult)
        return Meta::TOptional<FTextureSource>{};

    importerResult->AttachSourceFile(sourceFile);

    FTextureSource sourceData;
    sourceData.Construct(sourceProperties, std::move(importerResult));

    PPE_SLOG(Texture, Info, "imported texture source from file", {
        {"Width", sourceData.Width()},
        {"Height", sourceData.Height()},
        {"Format", ToString(sourceData.Format())},
        {"Gamma", ToString(sourceData.Gamma())},
        {"SourceFile", ToString(sourceFile)},
    });

    return Meta::MakeOptional(std::move(sourceData));
}
//----------------------------------------------------------------------------
template <bool (IImageFormat::* _ExportTexture)(IStreamWriter*, const FTextureSourceProperties&, const FRawMemoryConst&) const>
static bool ExportTextureToFile_(const ITextureService& textures, const FFilename& sourceFile, const FTextureSourceProperties& properties, const FRawMemoryConst& bulk) {
    MEMORYDOMAIN_THREAD_SCOPE(Texture);

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
