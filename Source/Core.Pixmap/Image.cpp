#include "stdafx.h"

#include "Image.h"

#include "FloatImage.h"
#include "Pixmap_fwd.h"

#include "Core/Allocator/PoolAllocator-impl.h"
#include "Core/Allocator/ThreadLocalHeap.h"
#include "Core/Color/Color.h"
#include "Core/Diagnostic/Logger.h"
#include "Core/IO/FileSystem.h"
#include "Core/IO/FileSystemConstNames.h"
#include "Core/IO/VirtualFileSystem.h"
#include "Core/Maths/ScalarVector.h"
#include "Core/Memory/MemoryStream.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_MALLOC(sz) \
    Core::GetThreadLocalHeap().malloc(sz, MEMORY_DOMAIN_TRACKING_DATA(Image))
#define STBI_REALLOC(p,newsz) \
    Core::GetThreadLocalHeap().realloc(p, newsz, MEMORY_DOMAIN_TRACKING_DATA(Image))
#define STBI_FREE(p) \
    Core::GetThreadLocalHeap().free(p, MEMORY_DOMAIN_TRACKING_DATA(Image))
#define STBI_ASSERT(x) \
    Assert(x)
#define STBI_NO_STDIO
#include "External/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBIW_MALLOC(sz) \
    Core::GetThreadLocalHeap().malloc(sz, MEMORY_DOMAIN_TRACKING_DATA(Image))
#define STBIW_REALLOC(p,newsz) \
    Core::GetThreadLocalHeap().realloc(p, newsz, MEMORY_DOMAIN_TRACKING_DATA(Image))
#define STBIW_FREE(p) \
    Core::GetThreadLocalHeap().free(p, MEMORY_DOMAIN_TRACKING_DATA(Image))
#define STBIW_ASSERT(x) \
    Assert(x)
#define STBI_WRITE_NO_STDIO
#include "External/stb_image_write.h"

namespace Core {
namespace Pixmap {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <ColorDepth _Depth, ColorSpace _Space>
struct ChannelTraits_ {};
//----------------------------------------------------------------------------
template <ColorSpace _Space>
struct ChannelTraits_<ColorDepth::_8bits, _Space> {
    typedef ubyten type;
    static float SRGB_to_Linear(const type& srgb) {
        return Core::SRGB_to_Linear(srgb._data);
    }
};
template <ColorSpace _Space>
struct ChannelTraits_<ColorDepth::_16bits, _Space> {
    typedef ushortn type;
    static float SRGB_to_Linear(const type& srgb) {
        return Core::SRGB_to_Linear(srgb.Normalized());
    }
};
template <ColorSpace _Space>
struct ChannelTraits_<ColorDepth::_32bits, _Space> {
    typedef uwordn type;
    static float SRGB_to_Linear(const type& srgb) {
        return Core::SRGB_to_Linear(srgb.Normalized());
    }
};
//----------------------------------------------------------------------------
template <>
struct ChannelTraits_<ColorDepth::_8bits, ColorSpace::Float> {
    typedef ubyten type;
};
template <>
struct ChannelTraits_<ColorDepth::_16bits, ColorSpace::Float> {
    typedef half type;
};
template <>
struct ChannelTraits_<ColorDepth::_32bits, ColorSpace::Float> {
    typedef float type;
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <ColorDepth _Depth, ColorMask _Mask, ColorSpace _Space>
struct PixelTraits_ {
    typedef typename ChannelTraits_<_Depth, _Space>::type channel_type;
    typedef channel_type raw_type[size_t(_Mask)];

    static void float_to_raw(raw_type* dst, const FloatImage::color_type* src) {
        for (size_t i = 0; i < size_t(_Mask); ++i)
            (*dst)[i] = src->Data()._data[i];
    }

    static void raw_to_float(FloatImage::color_type* dst, const raw_type* src) {
        for (size_t i = 0; i < size_t(_Mask); ++i)
            dst->Data()._data[i] = (*src)[i];
        for (size_t i = size_t(_Mask); i < 3; ++i)
            dst->Data()._data[i] = 0.0f;
        if (size_t(_Mask) < 4)
            dst->Data()._data[3] = 1.0f;
    }
};
//----------------------------------------------------------------------------
template <ColorDepth _Depth, ColorMask _Mask>
struct PixelTraits_<_Depth, _Mask, ColorSpace::sRGB> {
    typedef ChannelTraits_<_Depth, ColorSpace::sRGB> channel_traits;
    typedef typename channel_traits::type channel_type;
    typedef channel_type raw_type[size_t(_Mask)];

    static void float_to_raw(raw_type* dst, const FloatImage::color_type* src) {
        for (size_t i = 0; i < size_t(_Mask); ++i)
            (*dst)[i] = Saturate(Linear_to_SRGB(src->Data()._data[i]));
    }

    static void raw_to_float(FloatImage::color_type* dst, const raw_type* src) {
        for (size_t i = 0; i < size_t(_Mask); ++i)
            dst->Data()._data[i] = channel_traits::SRGB_to_Linear((*src)[i]);
        for (size_t i = size_t(_Mask); i < 3; ++i)
            dst->Data()._data[i] = 0.0f;
        if (size_t(_Mask) < 4)
            dst->Data()._data[3] = 1.0f;
    }
};
//----------------------------------------------------------------------------
template <ColorDepth _Depth>
struct PixelTraits_<_Depth, ColorMask::RGBA, ColorSpace::sRGB> {
    typedef ChannelTraits_<_Depth, ColorSpace::sRGB> channel_traits;
    typedef typename channel_traits::type channel_type;
    typedef channel_type raw_type[4];

    static void float_to_raw(raw_type* dst, const FloatImage::color_type* src) {
        for (size_t i = 0; i < 3; ++i)
            (*dst)[i] = Saturate(Linear_to_SRGB(src->Data()._data[i]));
        (*dst)[3] = Saturate(src->Data()._data[3]);
    }

    static void raw_to_float(FloatImage::color_type* dst, const raw_type* src) {
        for (size_t i = 0; i < 3; ++i)
            dst->Data()._data[i] = channel_traits::SRGB_to_Linear((*src)[i]);
        dst->Data()._data[3] = (*src)[3];
    }
};
//----------------------------------------------------------------------------
template <ColorDepth _Depth>
struct PixelTraits_<_Depth, ColorMask::RGB, ColorSpace::YCoCg> {
    typedef typename ChannelTraits_<_Depth, ColorSpace::YCoCg>::type channel_type;
    typedef channel_type raw_type[3];

    static void float_to_raw(raw_type* dst, const FloatImage::color_type* src) {
        const float3 yCoCg = RGB_to_YCoCg(*reinterpret_cast<const float3*>(src));
        for (size_t i = 0; i < 3; ++i)
            (*dst)[i] = yCoCg._data[i];
    }

    static void raw_to_float(FloatImage::color_type* dst, const raw_type* src) {
        float3 yCoCg;
        for (size_t i = 0; i < 3; ++i)
            yCoCg._data[i] = (*src)[i];

        const float3 rgb = YCoCg_to_RGB(yCoCg);
        for (size_t i = 0; i < 3; ++i)
            dst->Data()._data[i] = rgb._data[i];

        dst->a() = 1.0f;
    }
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <template <typename> class _Functor, typename _Dst, typename _Src, ColorDepth _Depth, ColorMask _Mask, ColorSpace _Space>
static void TypedMaskSpaceConvert_(_Dst* dst, const _Src* src) {
    typedef PixelTraits_<_Depth, _Mask, _Space> pixel_traits;
    _Functor<pixel_traits>()(dst, src);
}
//----------------------------------------------------------------------------
template <template <typename> class _Functor, typename _Dst, typename _Src, ColorDepth _Depth, ColorMask _Mask >
static void TypedMaskConvert_(_Dst* dst, const _Src* src, ColorSpace space ) {
    switch (space)
    {
    case Core::Pixmap::ColorSpace::Linear:
        TypedMaskSpaceConvert_<_Functor, _Dst, _Src, _Depth, _Mask, ColorSpace::Linear>(dst, src);
        break;
    case Core::Pixmap::ColorSpace::sRGB:
        TypedMaskSpaceConvert_<_Functor, _Dst, _Src, _Depth, _Mask, ColorSpace::sRGB>(dst, src);
        break;
    case Core::Pixmap::ColorSpace::YCoCg:
        TypedMaskSpaceConvert_<_Functor, _Dst, _Src, _Depth, _Mask, ColorSpace::YCoCg>(dst, src);
        break;
    default:
        AssertNotImplemented();
        break;
    }
}
//----------------------------------------------------------------------------
template <template <typename> class _Functor, typename _Dst, typename _Src, ColorDepth _Depth >
static void TypedConvert_(_Dst* dst, const _Src* src, ColorMask mask, ColorSpace space ) {
    switch (mask)
    {
    case Core::Pixmap::ColorMask::R:
        TypedMaskConvert_<_Functor, _Dst, _Src, _Depth, ColorMask::R>(dst, src, space);
        break;
    case Core::Pixmap::ColorMask::RG:
        TypedMaskConvert_<_Functor, _Dst, _Src, _Depth, ColorMask::RG>(dst, src, space);
        break;
    case Core::Pixmap::ColorMask::RGB:
        TypedMaskConvert_<_Functor, _Dst, _Src, _Depth, ColorMask::RGB>(dst, src, space);
        break;
    case Core::Pixmap::ColorMask::RGBA:
        TypedMaskConvert_<_Functor, _Dst, _Src, _Depth, ColorMask::RGBA>(dst, src, space);
        break;
    default:
        AssertNotImplemented();
        break;
    }
}
//----------------------------------------------------------------------------
template <template <typename> class _Functor, typename _Dst, typename _Src>
static void Convert_(_Dst* dst, const _Src* src, ColorDepth depth, ColorMask mask, ColorSpace space ) {
    switch (depth)
    {
    case Core::Pixmap::ColorDepth::_8bits:
        TypedConvert_<_Functor, _Dst, _Src, ColorDepth::_8bits>(dst, src, mask, space);
        break;
    case Core::Pixmap::ColorDepth::_16bits:
        TypedConvert_<_Functor, _Dst, _Src, ColorDepth::_16bits>(dst, src, mask, space);
        break;
    case Core::Pixmap::ColorDepth::_32bits:
        TypedConvert_<_Functor, _Dst, _Src, ColorDepth::_32bits>(dst, src, mask, space);
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
struct RawToFloat_ {
    void operator ()(FloatImage* dst, const Image* src) const {
        typedef typename _PixelTraits::raw_type raw_type;
        Assert(sizeof(raw_type) == src->PixelSizeInBytes());

        forrange(y, 0, src->Height()) {
            const raw_type* srcPixel = reinterpret_cast<const raw_type*>(src->Scanline(y).data());

            for (FloatImage::color_type& dstPixel : dst->Scanline(y)) {
                _PixelTraits::raw_to_float(&dstPixel, srcPixel);
                srcPixel++;
            }
        }
    }
};
//----------------------------------------------------------------------------
template <typename _PixelTraits>
struct FloatToRaw_ {
    void operator ()(Image* dst, const FloatImage* src) const {
        typedef typename _PixelTraits::raw_type raw_type;
        Assert(sizeof(raw_type) == dst->PixelSizeInBytes());

        forrange(y, 0, src->Height()) {
            raw_type* dstPixel = reinterpret_cast<raw_type*>(dst->Scanline(y).data());

            for (const FloatImage::color_type& srcPixel : src->Scanline(y)) {
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
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Pixmap, Image, )
//----------------------------------------------------------------------------
Image::Image()
:   _width(0)
,   _height(0)
,   _depth(ColorDepth::_8bits)
,   _mask(ColorMask::RGBA)
,   _space(ColorSpace::sRGB) {
    Assert((0 != _width) == (0 != _height));
}
//----------------------------------------------------------------------------
Image::Image(
    size_t width, size_t height,
    ColorDepth depth /* = ColorDepth::_8bits */,
    ColorMask mask /* = ColorMask::RGBA */,
    ColorSpace space /* = ColorSpace::sRGB */ )
:   _width(width)
,   _height(height)
,   _depth(depth)
,   _mask(mask)
,   _space(space) {
    Assert((0 != _width) == (0 != _height));
    Assert((space == ColorSpace::YCoCg) == (mask == ColorMask::RGBA));

    const size_t sizeInBytes = (PixelSizeInBytes() * _width * _height);
    _data.Resize_DiscardData(sizeInBytes);
}
//----------------------------------------------------------------------------
Image::Image(
    raw_data_type&& rdata,
    size_t width, size_t height,
    ColorDepth depth /* = ColorDepth::_8bits */,
    ColorMask mask /* = ColorMask::RGBA */,
    ColorSpace space /* = ColorSpace::sRGB */ )
:   _width(width)
,   _height(height)
,   _depth(depth)
,   _mask(mask)
,   _space(space)
,   _data(std::move(rdata)) {
    Assert((0 != _width) == (0 != _height));
    Assert((space == ColorSpace::YCoCg) == (mask == ColorMask::RGBA));

#ifdef WITH_CORE_ASSERT_RELEASE
    const size_t sizeInBytes = (PixelSizeInBytes() * _width * _height);
    AssertRelease(_data.size() == sizeInBytes);
#endif
}
//----------------------------------------------------------------------------
Image::~Image() {
#ifdef WITH_CORE_ASSERT
    const size_t sizeInBytes = (PixelSizeInBytes() * _width * _height);
    Assert(_data.size() == sizeInBytes);
#endif
}
//----------------------------------------------------------------------------
size_t Image::PixelSizeInBytes() const {
    const size_t channelCount = size_t(_mask);
    const size_t channelSizeInBytes = (size_t(_depth) >> 3);
    return (channelCount * channelSizeInBytes);
}
//----------------------------------------------------------------------------
MemoryView<u8> Image::Scanline(size_t row) {
    const size_t scanlineSizeInBytes = (PixelSizeInBytes() * _width);
    return _data.MakeView().SubRange(scanlineSizeInBytes * row, scanlineSizeInBytes);
}
//----------------------------------------------------------------------------
MemoryView<const u8> Image::Scanline(size_t row) const {
    const size_t scanlineSizeInBytes = (PixelSizeInBytes() * _width);
    return _data.MakeConstView().SubRange(scanlineSizeInBytes * row, scanlineSizeInBytes);
}
//----------------------------------------------------------------------------
void Image::Resize_DiscardData(size_t width, size_t height) {
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
void Image::Resize_DiscardData(size_t width, size_t height, ColorDepth depth, ColorMask mask, ColorSpace space) {
    Assert((0 != _width) == (0 != _height));
    Assert((space == ColorSpace::YCoCg) == (mask == ColorMask::RGBA));

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
void Image::ConvertFrom(const FloatImage* src) {
    Assert(nullptr != src);

    Resize_DiscardData(src->Width(), src->Height());
    Convert_<FloatToRaw_>(this, src, _depth, _mask, _space);
}
//----------------------------------------------------------------------------
void Image::ConvertTo(FloatImage* dst) const {
    Assert(nullptr != dst);

    dst->Resize_DiscardData(_width, _height);
    Convert_<RawToFloat_>(dst, this, _depth, _mask, _space);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Load(Image* dst, const Filename& filename) {
    RAWSTORAGE_THREAD_LOCAL(FileSystem, u8) content;
    if (false == VFS_ReadAll(&content, filename, AccessPolicy::Binary))
        return false;

    return Load(dst, filename, content.MakeConstView());
}
//----------------------------------------------------------------------------
bool Load(Image* dst, const Filename& filename, const MemoryView<const u8>& content) {
    Assert(dst);
    Assert(filename.HasExtname());

    // supported image file types :
    const Extname ext = filename.Extname();
    if (FileSystemConstNames::BmpExt() != ext &&
        FileSystemConstNames::GifExt() != ext &&
        FileSystemConstNames::HdrExt() != ext &&
        FileSystemConstNames::JpgExt() != ext &&
        FileSystemConstNames::PgmExt() != ext &&
        FileSystemConstNames::PngExt() != ext &&
        FileSystemConstNames::PpmExt() != ext &&
        FileSystemConstNames::PicExt() != ext &&
        FileSystemConstNames::PsdExt() != ext &&
        FileSystemConstNames::TgaExt() != ext )
        return false;

    if (FileSystemConstNames::HdrExt() == ext) {
        // import .hdr images in a 32bits float buffer :
        const int len = checked_cast<int>(content.size());

        int x, y, comp;
        const ThreadLocalPtr<float, MEMORY_DOMAIN_TAG(Image)> decoded(
            ::stbi_loadf_from_memory(content.data(), len, &x, &y, &comp, 0) );

        if (nullptr == decoded)
            return false;

        dst->_width = checked_cast<size_t>(x);
        dst->_height = checked_cast<size_t>(y);
        dst->_depth = ColorDepth::_32bits;
        dst->_space = ColorSpace::Float;

        switch (comp)
        {
        case 1:
            dst->_mask = ColorMask::R;
            break;
        case 2:
            dst->_mask = ColorMask::RG;
            break;
        case 3:
            dst->_mask = ColorMask::RGB;
            break;
        case 4:
            dst->_mask = ColorMask::RGBA;
            break;

        default:
            AssertNotImplemented();
            break;
        }

        const size_t sizeInBytes = (dst->_width*dst->_height*dst->PixelSizeInBytes());

        dst->_data.Resize_DiscardData(sizeInBytes);
        Assert(dst->TotalSizeInBytes() == sizeInBytes);
        ::memcpy(dst->_data.data(), decoded.get(), sizeInBytes);
    }
    else {
        // everything else will always use an 8bits buffer :
        const int len = checked_cast<int>(content.size());

        int x, y, comp;
        const ThreadLocalPtr<::stbi_uc, MEMORY_DOMAIN_TAG(Image)> decoded(
            ::stbi_load_from_memory(content.data(), len, &x, &y, &comp, 0) );
        if (nullptr == decoded)
            return false;

        dst->_width = checked_cast<size_t>(x);
        dst->_height = checked_cast<size_t>(y);
        dst->_depth = ColorDepth::_8bits; // sadly stbi doesn't handle 16 bits per channel...
        dst->_space = ColorSpace::sRGB;

        switch (comp)
        {
        case 1:
            dst->_mask = ColorMask::R;
            break;
        case 2:
            dst->_mask = ColorMask::RG;
            break;
        case 3:
            dst->_mask = ColorMask::RGB;
            break;
        case 4:
            dst->_mask = ColorMask::RGBA;
            break;

        default:
            AssertNotImplemented();
            break;
        }

        const size_t sizeInBytes = (dst->_width*dst->_height*dst->PixelSizeInBytes());

        dst->_data.Resize_DiscardData(sizeInBytes);
        Assert(dst->TotalSizeInBytes() == sizeInBytes);
        ::memcpy(dst->_data.data(), decoded.get(), sizeInBytes);
    }

    LOG(Info, L"[Pixmap] Loaded a {0}_{1}_{2}:{3}x{4} image from '{5}'",
        dst->Mask(), dst->Depth(), dst->Space(), dst->Width(), dst->Height(),
        filename );

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
bool Save(const Image* src, const Filename& filename) {
    MEMORYSTREAM_THREAD_LOCAL(Image) writer;
    if (false == Save(src, filename, &writer))
        return false;

    if (false == VFS_WriteAll(filename, writer.MakeView(), AccessPolicy::Truncate_Binary))
        return false;

    return true;
}
//----------------------------------------------------------------------------
bool Save(const Image* src, const Filename& filename, IStreamWriter* writer) {
    Assert(src);
    Assert(writer);
    Assert(filename.HasExtname());

    LOG(Info, L"[Pixmap] Saving a {0}_{1}_{2}:{3}x{4} image to '{5}'",
        src->Mask(), src->Depth(), src->Space(), src->Width(), src->Height(),
        filename );

    int result = 0;

    const int x = checked_cast<int>(src->_width);
    const int y = checked_cast<int>(src->_height);
    const int comp = int(src->_mask);

    const Extname ext = filename.Extname();
    if (FileSystemConstNames::PngExt() == ext) {
        AssertRelease(ColorDepth::_8bits == src->_depth);
        result = ::stbi_write_png_to_func(&WriteFuncForSTBIW_, writer, x, y, comp, src->_data.data(), 0);
    }
    else if (FileSystemConstNames::TgaExt() == ext) {
        AssertRelease(ColorDepth::_8bits == src->_depth);
        result = ::stbi_write_tga_to_func(&WriteFuncForSTBIW_, writer, x, y, comp, src->_data.data());
    }
    else if (FileSystemConstNames::HdrExt() == ext) {
        AssertRelease(ColorDepth::_32bits == src->_depth);
        AssertRelease(ColorSpace::Float == src->_space);
        result = ::stbi_write_hdr_to_func(&WriteFuncForSTBIW_, writer, x, y, comp, reinterpret_cast<const float*>(src->_data.data()));
    }
    else if (FileSystemConstNames::BmpExt() == ext) {
        AssertRelease(ColorDepth::_8bits == src->_depth);
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
void Image::Start() {
    // Workaround for thread safety of stb_image
    // https://github.com/nothings/stb/issues/309
    ::stbi__init_zdefaults();
}
//----------------------------------------------------------------------------
void Image::Shutdown() {
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Pixmap
} //!namespace Core
