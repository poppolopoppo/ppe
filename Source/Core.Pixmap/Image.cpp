#include "stdafx.h"

#include "Image.h"

#include "FloatImage.h"
#include "Pixmap_fwd.h"

#include "Core/Allocator/PoolAllocator-impl.h"
#include "Core/Allocator/TrackingMalloc.h"
#include "Core/Color/Color.h"
#include "Core/Diagnostic/Logger.h"
#include "Core/IO/FileSystem.h"
#include "Core/IO/FS/ConstNames.h"
#include "Core/IO/VirtualFileSystem.h"
#include "Core/Maths/PackedVectors.h"
#include "Core/Maths/ScalarVector.h"
#include "Core/Memory/MemoryStream.h"

PRAGMA_MSVC_WARNING_PUSH()
PRAGMA_MSVC_WARNING_DISABLE(4996) // 'sprintf': This function or variable may be unsafe. Consider using sprintf_s instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details.
PRAGMA_MSVC_WARNING_DISABLE(6001) // warning C6001: Using uninitialized memory 'coutput'

#define STB_IMAGE_IMPLEMENTATION
#define STBI_MALLOC(sz) \
    Core::tracking_malloc_thread_local<MEMORY_DOMAIN_TAG(STBImage)>(sz)
#define STBI_REALLOC(p,newsz) \
    Core::tracking_realloc_thread_local<MEMORY_DOMAIN_TAG(STBImage)>(p, newsz)
#define STBI_FREE(p) \
    Core::tracking_free_thread_local<MEMORY_DOMAIN_TAG(STBImage)>(p)
#define STBI_ASSERT(x) \
    Assert(NOOP("stb_image: "), (x))
#define STBI_NO_STDIO
#include "Core.External/stb/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBIW_MALLOC(sz) \
    Core::tracking_malloc_thread_local<MEMORY_DOMAIN_TAG(STBImage)>(sz)
#define STBIW_REALLOC(p,newsz) \
    Core::tracking_realloc_thread_local<MEMORY_DOMAIN_TAG(STBImage)>(p, newsz)
#define STBIW_FREE(p) \
    Core::tracking_free_thread_local<MEMORY_DOMAIN_TAG(STBImage)>(p)
#define STBIW_ASSERT(x) \
    Assert(NOOP("stb_image_write: "), (x))
#define STBI_WRITE_NO_STDIO
#include "Core.External/stb/stb_image_write.h"

PRAGMA_MSVC_WARNING_POP()

namespace Core {
namespace Pixmap {
EXTERN_LOG_CATEGORY(CORE_PIXMAP_API, Pixmap);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <EColorDepth _Depth, EColorSpace _Space>
struct TChannelTraits_ {};
//----------------------------------------------------------------------------
template <EColorSpace _Space>
struct TChannelTraits_<EColorDepth::_8bits, _Space> {
    typedef ubyten type;
    static float SRGB_to_Linear(const type& srgb) {
        return Core::SRGB_to_Linear(srgb._data);
    }
};
template <EColorSpace _Space>
struct TChannelTraits_<EColorDepth::_16bits, _Space> {
    typedef ushortn type;
    static float SRGB_to_Linear(const type& srgb) {
        return Core::SRGB_to_Linear(srgb.Normalized());
    }
};
template <EColorSpace _Space>
struct TChannelTraits_<EColorDepth::_32bits, _Space> {
    typedef uwordn type;
    static float SRGB_to_Linear(const type& srgb) {
        return Core::SRGB_to_Linear(srgb.Normalized());
    }
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <EColorMask _Mask>
struct THomogenizeColorF_ {
    static void _0001(FFloatImage::color_type&) {}
};
template <> struct THomogenizeColorF_<EColorMask::R> {
    static void _0001(FFloatImage::color_type& dst) {
        dst[1] = dst[2] = 0.f; dst[3] = 1.f;
    }
};
template <> struct THomogenizeColorF_<EColorMask::RG> {
    static void _0001(FFloatImage::color_type& dst) {
        dst[2] = 0.f; dst[3] = 1.f;
    }
};
template <> struct THomogenizeColorF_<EColorMask::RGB> {
    static void _0001(FFloatImage::color_type& dst) {
        dst[3] = 1.f;
    }
};
//----------------------------------------------------------------------------
template <EColorDepth _Depth, EColorMask _Mask, EColorSpace _Space>
struct TPixelTraits_ {
    typedef typename TChannelTraits_<_Depth, _Space>::type channel_type;
    typedef channel_type raw_type[size_t(_Mask)];

    static void float_to_raw(raw_type* dst, const FFloatImage::color_type* src) {
        for (size_t i = 0; i < size_t(_Mask); ++i)
            (*dst)[i] = (*src)[i];
    }

    static void raw_to_float(FFloatImage::color_type* dst, const raw_type* src) {
        for (size_t i = 0; i < size_t(_Mask); ++i)
            (*dst)[i] = (*src)[i];
        THomogenizeColorF_<_Mask>::_0001(*dst);
    }
};
//----------------------------------------------------------------------------
template <EColorDepth _Depth, EColorMask _Mask>
struct TPixelTraits_<_Depth, _Mask, EColorSpace::sRGB> {
    typedef TChannelTraits_<_Depth, EColorSpace::sRGB> channel_traits;
    typedef typename channel_traits::type channel_type;
    typedef channel_type raw_type[size_t(_Mask)];

    static void float_to_raw(raw_type* dst, const FFloatImage::color_type* src) {
        for (size_t i = 0; i < size_t(_Mask); ++i)
            (*dst)[i] = Saturate(Linear_to_SRGB((*src)[i]));
    }

    static void raw_to_float(FFloatImage::color_type* dst, const raw_type* src) {
        for (size_t i = 0; i < size_t(_Mask); ++i)
            (*dst)[i] = channel_traits::SRGB_to_Linear((*src)[i]);
        THomogenizeColorF_<_Mask>::_0001(*dst);
    }
};
//----------------------------------------------------------------------------
template <EColorDepth _Depth>
struct TPixelTraits_<_Depth, EColorMask::RGBA, EColorSpace::sRGB> {
    typedef TChannelTraits_<_Depth, EColorSpace::sRGB> channel_traits;
    typedef typename channel_traits::type channel_type;
    typedef channel_type raw_type[4];

    static void float_to_raw(raw_type* dst, const FFloatImage::color_type* src) {
        for (size_t i = 0; i < 3; ++i)
            (*dst)[i] = Saturate(Linear_to_SRGB((*src)[i]));
        (*dst)[3] = Saturate((*src)[3]);
    }

    static void raw_to_float(FFloatImage::color_type* dst, const raw_type* src) {
        for (size_t i = 0; i < 3; ++i)
            (*dst)[i] = channel_traits::SRGB_to_Linear((*src)[i]);
        (*dst)[3] = (*src)[3];
    }
};
//----------------------------------------------------------------------------
template <EColorDepth _Depth>
struct TPixelTraits_<_Depth, EColorMask::RGB, EColorSpace::YCoCg> {
    typedef typename TChannelTraits_<_Depth, EColorSpace::YCoCg>::type channel_type;
    typedef channel_type raw_type[3];

    static void float_to_raw(raw_type* dst, const FFloatImage::color_type* src) {
        const float3 yCoCg = RGB_to_YCoCg(*reinterpret_cast<const float3*>(src));
        for (size_t i = 0; i < 3; ++i)
            (*dst)[i] = yCoCg._data[i];
    }

    static void raw_to_float(FFloatImage::color_type* dst, const raw_type* src) {
        float3 yCoCg;
        for (size_t i = 0; i < 3; ++i)
            yCoCg._data[i] = (*src)[i];

        const float3 rgb = YCoCg_to_RGB(yCoCg);
        for (size_t i = 0; i < 3; ++i)
            (*dst)[i] = rgb._data[i];

        dst->A = 1.0f;
    }
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <template <typename> class _Functor, typename _Dst, typename _Src, EColorDepth _Depth, EColorMask _Mask, EColorSpace _Space>
static void TypedMaskSpaceConvert_(_Dst* dst, const _Src* src) {
    typedef TPixelTraits_<_Depth, _Mask, _Space> pixel_traits;
    _Functor<pixel_traits>()(dst, src);
}
//----------------------------------------------------------------------------
template <template <typename> class _Functor, typename _Dst, typename _Src, EColorDepth _Depth, EColorMask _Mask >
static void TypedMaskConvert_(_Dst* dst, const _Src* src, EColorSpace space ) {
    switch (space)
    {
    case Core::Pixmap::EColorSpace::Linear:
        TypedMaskSpaceConvert_<_Functor, _Dst, _Src, _Depth, _Mask, EColorSpace::Linear>(dst, src);
        break;
    case Core::Pixmap::EColorSpace::sRGB:
        TypedMaskSpaceConvert_<_Functor, _Dst, _Src, _Depth, _Mask, EColorSpace::sRGB>(dst, src);
        break;
    case Core::Pixmap::EColorSpace::YCoCg:
        TypedMaskSpaceConvert_<_Functor, _Dst, _Src, _Depth, _Mask, EColorSpace::YCoCg>(dst, src);
        break;
    default:
        AssertNotImplemented();
        break;
    }
}
//----------------------------------------------------------------------------
template <template <typename> class _Functor, typename _Dst, typename _Src, EColorDepth _Depth >
static void TypedConvert_(_Dst* dst, const _Src* src, EColorMask mask, EColorSpace space ) {
    switch (mask)
    {
    case Core::Pixmap::EColorMask::R:
        TypedMaskConvert_<_Functor, _Dst, _Src, _Depth, EColorMask::R>(dst, src, space);
        break;
    case Core::Pixmap::EColorMask::RG:
        TypedMaskConvert_<_Functor, _Dst, _Src, _Depth, EColorMask::RG>(dst, src, space);
        break;
    case Core::Pixmap::EColorMask::RGB:
        TypedMaskConvert_<_Functor, _Dst, _Src, _Depth, EColorMask::RGB>(dst, src, space);
        break;
    case Core::Pixmap::EColorMask::RGBA:
        TypedMaskConvert_<_Functor, _Dst, _Src, _Depth, EColorMask::RGBA>(dst, src, space);
        break;
    default:
        AssertNotImplemented();
        break;
    }
}
//----------------------------------------------------------------------------
template <template <typename> class _Functor, typename _Dst, typename _Src>
static void Convert_(_Dst* dst, const _Src* src, EColorDepth depth, EColorMask mask, EColorSpace space ) {
    switch (depth)
    {
    case Core::Pixmap::EColorDepth::_8bits:
        TypedConvert_<_Functor, _Dst, _Src, EColorDepth::_8bits>(dst, src, mask, space);
        break;
    case Core::Pixmap::EColorDepth::_16bits:
        TypedConvert_<_Functor, _Dst, _Src, EColorDepth::_16bits>(dst, src, mask, space);
        break;
    case Core::Pixmap::EColorDepth::_32bits:
        TypedConvert_<_Functor, _Dst, _Src, EColorDepth::_32bits>(dst, src, mask, space);
        break;
    default:
        AssertNotImplemented();
        break;
    }
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename _PixelTraits>
struct FRawToFloat_ {
    void operator ()(FFloatImage* dst, const FImage* src) const {
        typedef typename _PixelTraits::raw_type raw_type;
        Assert(sizeof(raw_type) == src->PixelSizeInBytes());

        forrange(y, 0, src->Height()) {
            const raw_type* srcPixel = reinterpret_cast<const raw_type*>(src->Scanline(y).data());

            for (FFloatImage::color_type& dstPixel : dst->Scanline(y)) {
                _PixelTraits::raw_to_float(&dstPixel, srcPixel);
                srcPixel++;
            }
        }
    }
};
//----------------------------------------------------------------------------
template <typename _PixelTraits>
struct FFloatToRaw_ {
    void operator ()(FImage* dst, const FFloatImage* src) const {
        typedef typename _PixelTraits::raw_type raw_type;
        Assert(sizeof(raw_type) == dst->PixelSizeInBytes());

        forrange(y, 0, src->Height()) {
            raw_type* dstPixel = reinterpret_cast<raw_type*>(dst->Scanline(y).data());

            for (const FFloatImage::color_type& srcPixel : src->Scanline(y)) {
                _PixelTraits::float_to_raw(dstPixel, &srcPixel);
                dstPixel++;
            }
        }
    }
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Pixmap, FImage, )
//----------------------------------------------------------------------------
FImage::FImage()
:   _width(0)
,   _height(0)
,   _depth(EColorDepth::_8bits)
,   _mask(EColorMask::RGBA)
,   _space(EColorSpace::sRGB) {
    Assert((0 != _width) == (0 != _height));
}
//----------------------------------------------------------------------------
FImage::FImage(
    size_t width, size_t height,
    EColorDepth depth /* = EColorDepth::_8bits */,
    EColorMask mask /* = EColorMask::RGBA */,
    EColorSpace space /* = EColorSpace::sRGB */ )
:   _width(width)
,   _height(height)
,   _depth(depth)
,   _mask(mask)
,   _space(space) {
    Assert((0 != _width) == (0 != _height));
    Assert((space == EColorSpace::YCoCg) == (mask == EColorMask::RGBA));

    const size_t sizeInBytes = (PixelSizeInBytes() * _width * _height);
    _data.Resize_DiscardData(sizeInBytes);
}
//----------------------------------------------------------------------------
FImage::FImage(
    raw_data_type&& rdata,
    size_t width, size_t height,
    EColorDepth depth /* = EColorDepth::_8bits */,
    EColorMask mask /* = EColorMask::RGBA */,
    EColorSpace space /* = EColorSpace::sRGB */ )
:   _width(width)
,   _height(height)
,   _depth(depth)
,   _mask(mask)
,   _space(space)
,   _data(std::move(rdata)) {
    Assert((0 != _width) == (0 != _height));
    Assert((space == EColorSpace::YCoCg) == (mask == EColorMask::RGBA));

#ifdef WITH_CORE_ASSERT_RELEASE
    const size_t sizeInBytes = (PixelSizeInBytes() * _width * _height);
    AssertRelease(_data.size() == sizeInBytes);
#endif
}
//----------------------------------------------------------------------------
FImage::~FImage() {
#ifdef WITH_CORE_ASSERT
    const size_t sizeInBytes = (PixelSizeInBytes() * _width * _height);
    Assert(_data.size() == sizeInBytes);
#endif
}
//----------------------------------------------------------------------------
size_t FImage::PixelSizeInBytes() const {
    const size_t channelCount = size_t(_mask);
    const size_t channelSizeInBytes = (size_t(_depth) >> 3);
    return (channelCount * channelSizeInBytes);
}
//----------------------------------------------------------------------------
TMemoryView<u8> FImage::Scanline(size_t row) {
    const size_t scanlineSizeInBytes = (PixelSizeInBytes() * _width);
    return _data.MakeView().SubRange(scanlineSizeInBytes * row, scanlineSizeInBytes);
}
//----------------------------------------------------------------------------
TMemoryView<const u8> FImage::Scanline(size_t row) const {
    const size_t scanlineSizeInBytes = (PixelSizeInBytes() * _width);
    return _data.MakeConstView().SubRange(scanlineSizeInBytes * row, scanlineSizeInBytes);
}
//----------------------------------------------------------------------------
void FImage::Resize_DiscardData(size_t width, size_t height) {
    Assert((0 != _width) == (0 != _height));

    if (_width == width &&
        _height == height)
        return;

    _width = width;
    _height = height;

    const size_t sizeInBytes = (PixelSizeInBytes() * _width * _height);
    _data.Resize_DiscardData(sizeInBytes);
}
//----------------------------------------------------------------------------
void FImage::Resize_DiscardData(size_t width, size_t height, EColorDepth depth, EColorMask mask, EColorSpace space) {
    Assert((0 != _width) == (0 != _height));
    Assert((space == EColorSpace::YCoCg) == (mask == EColorMask::RGBA));

    if (_width == width &&
        _height == height &&
        _depth == depth &&
        _mask == mask &&
        _space == space)
        return;

    _width = width;
    _height = height;
    _depth = depth;
    _mask = mask;
    _space =space;

    const size_t sizeInBytes = (PixelSizeInBytes() * _width * _height);
    _data.Resize_DiscardData(sizeInBytes);
}
//----------------------------------------------------------------------------
void FImage::ConvertFrom(const FFloatImage* src) {
    Assert(nullptr != src);

    Resize_DiscardData(src->Width(), src->Height());
    Convert_<FFloatToRaw_>(this, src, _depth, _mask, _space);
}
//----------------------------------------------------------------------------
void FImage::ConvertTo(FFloatImage* dst) const {
    Assert(nullptr != dst);

    dst->Resize_DiscardData(_width, _height);
    Convert_<FRawToFloat_>(dst, this, _depth, _mask, _space);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Load(FImage* dst, EColorDepth depth, EColorSpace space, const FFilename& filename) {
    Assert(dst);
    Assert(filename.HasExtname());

    // supported image file types :
    const FExtname ext = filename.Extname();
    if (FFSConstNames::Bmp() != ext &&
        FFSConstNames::Gif() != ext &&
        FFSConstNames::Hdr() != ext &&
        FFSConstNames::Jpg() != ext &&
        FFSConstNames::Pgm() != ext &&
        FFSConstNames::Png() != ext &&
        FFSConstNames::Ppm() != ext &&
        FFSConstNames::Pic() != ext &&
        FFSConstNames::Psd() != ext &&
        FFSConstNames::Tga() != ext)
        return false;

    RAWSTORAGE_THREAD_LOCAL(FileSystem, u8) content;
    if (false == VFS_ReadAll(&content, filename, EAccessPolicy::Binary))
        return false;

    return Load(dst, depth, space, content.MakeConstView());
}
//----------------------------------------------------------------------------
bool Load(FImage* dst, EColorDepth depth, EColorSpace space, const TMemoryView<const u8>& content) {
    Assert(dst);

    int channelCount = 0;
    int width = 0;
    int height = 0;

    dst->_depth = depth;
    dst->_space = space;

    const int contentSizeInBytes = checked_cast<int>(content.size());

    void* decoded;
    switch (depth)
    {
    case Core::Pixmap::EColorDepth::_8bits:
        decoded = ::stbi_load_from_memory(content.data(), contentSizeInBytes, &width, &height, &channelCount, 0);
        break;
    case Core::Pixmap::EColorDepth::_16bits:
        decoded = ::stbi_load_16_from_memory(content.data(), contentSizeInBytes, &width, &height, &channelCount, 0);
        break;
    case Core::Pixmap::EColorDepth::_32bits:
        decoded = ::stbi_loadf_from_memory(content.data(), contentSizeInBytes, &width, &height, &channelCount, 0);
        break;

    default:
        AssertNotImplemented();
        return false;
    }

    if (nullptr == decoded)
        return false;

    AssertRelease(space != EColorSpace::YCoCg || channelCount > 2);

    switch (channelCount)
    {
    case 1:
        dst->_mask = EColorMask::R;
        break;
    case 2:
        dst->_mask = EColorMask::RG;
        break;
    case 3:
        dst->_mask = EColorMask::RGB;
        break;
    case 4:
        dst->_mask = EColorMask::RGBA;
        break;

    default:
        AssertNotImplemented();
        return false;
    }

    dst->_width = checked_cast<size_t>(width);
    dst->_height = checked_cast<size_t>(height);

    LOG(Pixmap, Info, L"loaded a {0}_{1}_{2}:{3}x{4} image",
        dst->Mask(), dst->Depth(), dst->Space(), dst->Width(), dst->Height() );

    const size_t decodedSizeInBytes = (dst->_width*dst->_height*dst->PixelSizeInBytes());

    dst->_data.Resize_DiscardData(decodedSizeInBytes);
    Assert(dst->TotalSizeInBytes() == decodedSizeInBytes);
    ::memcpy(dst->_data.data(), decoded, decodedSizeInBytes);

    STBI_FREE(decoded);

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
static void WriteFuncForSTBIW_(void *context, void *data, int size) {
    reinterpret_cast<IStreamWriter*>(context)->Write(data, size);
}
} //!namespace
//----------------------------------------------------------------------------
bool Save(const FImage* src, const FFilename& filename) {
    MEMORYSTREAM_THREAD_LOCAL(Image) writer;
    if (false == Save(src, filename, &writer))
        return false;

    if (false == VFS_WriteAll(filename, writer.MakeView(), EAccessPolicy::Create_Binary))
        return false;

    return true;
}
//----------------------------------------------------------------------------
bool Save(const FImage* src, const FFilename& filename, IStreamWriter* writer) {
    Assert(src);
    Assert(writer);
    Assert(filename.HasExtname());

    LOG(Pixmap, Info, L"saving a {0}_{1}_{2}:{3}x{4} image to '{5}'",
        src->Mask(), src->Depth(), src->Space(), src->Width(), src->Height(),
        filename );

    int result = 0;

    const int x = checked_cast<int>(src->_width);
    const int y = checked_cast<int>(src->_height);
    const int comp = int(src->_mask);

    const FExtname ext = filename.Extname();
    if (FFSConstNames::Png() == ext) {
        AssertRelease(EColorDepth::_8bits == src->_depth);
        result = ::stbi_write_png_to_func(&WriteFuncForSTBIW_, writer, x, y, comp, src->_data.data(), 0);
    }
    else if (FFSConstNames::Tga() == ext) {
        AssertRelease(EColorDepth::_8bits == src->_depth);
        result = ::stbi_write_tga_to_func(&WriteFuncForSTBIW_, writer, x, y, comp, src->_data.data());
    }
    else if (FFSConstNames::Hdr() == ext) {
        AssertRelease(EColorDepth::_32bits == src->_depth);
        AssertRelease(EColorSpace::Linear == src->_space);
        result = ::stbi_write_hdr_to_func(&WriteFuncForSTBIW_, writer, x, y, comp, reinterpret_cast<const float*>(src->_data.data()));
    }
    else if (FFSConstNames::Bmp() == ext) {
        AssertRelease(EColorDepth::_8bits == src->_depth);
        result = ::stbi_write_bmp_to_func(&WriteFuncForSTBIW_, writer, x, y, comp, src->_data.data());
    }
    else {
        AssertNotImplemented();
        return false;
    }

    return (0 != result);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FImage::Start() {
#if 0 // seems fixed
    // TODO: remove when fixed in an updated version
    // Workaround for thread safety of stb_image
    // https://github.com/nothings/stb/issues/309
    ::stbi__init_zdefaults();
#endif
}
//----------------------------------------------------------------------------
void FImage::Shutdown() {
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Pixmap
} //!namespace Core
