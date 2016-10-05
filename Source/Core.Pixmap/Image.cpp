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
    Core::GetThreadLocalHeap().Malloc(sz, MEMORY_DOMAIN_TRACKING_DATA(Image))
#define STBI_REALLOC(p,newsz) \
    Core::GetThreadLocalHeap().Realloc(p, newsz, MEMORY_DOMAIN_TRACKING_DATA(Image))
#define STBI_FREE(p) \
    Core::GetThreadLocalHeap().Free(p, MEMORY_DOMAIN_TRACKING_DATA(Image))
#define STBI_ASSERT(x) \
    Assert(x)
#define STBI_NO_STDIO
#include "External/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBIW_MALLOC(sz) \
    Core::GetThreadLocalHeap().Malloc(sz, MEMORY_DOMAIN_TRACKING_DATA(Image))
#define STBIW_REALLOC(p,newsz) \
    Core::GetThreadLocalHeap().Realloc(p, newsz, MEMORY_DOMAIN_TRACKING_DATA(Image))
#define STBIW_FREE(p) \
    Core::GetThreadLocalHeap().Free(p, MEMORY_DOMAIN_TRACKING_DATA(Image))
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
template <>
struct TChannelTraits_<EColorDepth::_8bits, EColorSpace::Float> {
    typedef ubyten type;
};
template <>
struct TChannelTraits_<EColorDepth::_16bits, EColorSpace::Float> {
    typedef half type;
};
template <>
struct TChannelTraits_<EColorDepth::_32bits, EColorSpace::Float> {
    typedef float type;
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <EColorDepth _Depth, EColorMask _Mask, EColorSpace _Space>
struct TPixelTraits_ {
    typedef typename TChannelTraits_<_Depth, _Space>::type channel_type;
    typedef channel_type raw_type[size_t(_Mask)];

    static void float_to_raw(raw_type* dst, const FFloatImage::color_type* src) {
        for (size_t i = 0; i < size_t(_Mask); ++i)
            (*dst)[i] = src->Data()._data[i];
    }

    static void raw_to_float(FFloatImage::color_type* dst, const raw_type* src) {
        for (size_t i = 0; i < size_t(_Mask); ++i)
            dst->Data()._data[i] = (*src)[i];
        for (size_t i = size_t(_Mask); i < 3; ++i)
            dst->Data()._data[i] = 0.0f;
        if (size_t(_Mask) < 4)
            dst->Data()._data[3] = 1.0f;
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
            (*dst)[i] = Saturate(Linear_to_SRGB(src->Data()._data[i]));
    }

    static void raw_to_float(FFloatImage::color_type* dst, const raw_type* src) {
        for (size_t i = 0; i < size_t(_Mask); ++i)
            dst->Data()._data[i] = channel_traits::SRGB_to_Linear((*src)[i]);
        for (size_t i = size_t(_Mask); i < 3; ++i)
            dst->Data()._data[i] = 0.0f;
        if (size_t(_Mask) < 4)
            dst->Data()._data[3] = 1.0f;
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
            (*dst)[i] = Saturate(Linear_to_SRGB(src->Data()._data[i]));
        (*dst)[3] = Saturate(src->Data()._data[3]);
    }

    static void raw_to_float(FFloatImage::color_type* dst, const raw_type* src) {
        for (size_t i = 0; i < 3; ++i)
            dst->Data()._data[i] = channel_traits::SRGB_to_Linear((*src)[i]);
        dst->Data()._data[3] = (*src)[3];
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
    void operator ()(FFloatImage* dst, const Image* src) const {
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
    void operator ()(Image* dst, const FFloatImage* src) const {
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
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Pixmap, Image, )
//----------------------------------------------------------------------------
Image::Image()
:   _width(0)
,   _height(0)
,   _depth(EColorDepth::_8bits)
,   _mask(EColorMask::RGBA)
,   _space(EColorSpace::sRGB) {
    Assert((0 != _width) == (0 != _height));
}
//----------------------------------------------------------------------------
Image::Image(
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
Image::Image(
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
TMemoryView<u8> Image::Scanline(size_t row) {
    const size_t scanlineSizeInBytes = (PixelSizeInBytes() * _width);
    return _data.MakeView().SubRange(scanlineSizeInBytes * row, scanlineSizeInBytes);
}
//----------------------------------------------------------------------------
TMemoryView<const u8> Image::Scanline(size_t row) const {
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
void Image::Resize_DiscardData(size_t width, size_t height, EColorDepth depth, EColorMask mask, EColorSpace space) {
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
void Image::ConvertFrom(const FFloatImage* src) {
    Assert(nullptr != src);

    Resize_DiscardData(src->Width(), src->Height());
    Convert_<FFloatToRaw_>(this, src, _depth, _mask, _space);
}
//----------------------------------------------------------------------------
void Image::ConvertTo(FFloatImage* dst) const {
    Assert(nullptr != dst);

    dst->Resize_DiscardData(_width, _height);
    Convert_<FRawToFloat_>(dst, this, _depth, _mask, _space);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Load(Image* dst, const FFilename& filename) {
    RAWSTORAGE_THREAD_LOCAL(FileSystem, u8) content;
    if (false == VFS_ReadAll(&content, filename, AccessPolicy::Binary))
        return false;

    return Load(dst, filename, content.MakeConstView());
}
//----------------------------------------------------------------------------
bool Load(Image* dst, const FFilename& filename, const TMemoryView<const u8>& content) {
    Assert(dst);
    Assert(filename.HasExtname());

    // supported image file types :
    const FExtname ext = filename.Extname();
    if (FFileSystemConstNames::BmpExt() != ext &&
        FFileSystemConstNames::GifExt() != ext &&
        FFileSystemConstNames::HdrExt() != ext &&
        FFileSystemConstNames::JpgExt() != ext &&
        FFileSystemConstNames::PgmExt() != ext &&
        FFileSystemConstNames::PngExt() != ext &&
        FFileSystemConstNames::PpmExt() != ext &&
        FFileSystemConstNames::PicExt() != ext &&
        FFileSystemConstNames::PsdExt() != ext &&
        FFileSystemConstNames::TgaExt() != ext )
        return false;

    if (FFileSystemConstNames::HdrExt() == ext) {
        // import .hdr images in a 32bits float buffer :
        const int len = checked_cast<int>(content.size());

        int x, y, comp;
        const TThreadLocalPtr<float, MEMORY_DOMAIN_TAG(Image)> decoded(
            ::stbi_loadf_from_memory(content.data(), len, &x, &y, &comp, 0) );

        if (nullptr == decoded)
            return false;

        dst->_width = checked_cast<size_t>(x);
        dst->_height = checked_cast<size_t>(y);
        dst->_depth = EColorDepth::_32bits;
        dst->_space = EColorSpace::Float;

        switch (comp)
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
        const TThreadLocalPtr<::stbi_uc, MEMORY_DOMAIN_TAG(Image)> decoded(
            ::stbi_load_from_memory(content.data(), len, &x, &y, &comp, 0) );
        if (nullptr == decoded)
            return false;

        dst->_width = checked_cast<size_t>(x);
        dst->_height = checked_cast<size_t>(y);
        dst->_depth = EColorDepth::_8bits; // sadly stbi doesn't handle 16 bits per channel...
        dst->_space = EColorSpace::sRGB;

        switch (comp)
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
bool Save(const Image* src, const FFilename& filename) {
    MEMORYSTREAM_THREAD_LOCAL(Image) writer;
    if (false == Save(src, filename, &writer))
        return false;

    if (false == VFS_WriteAll(filename, writer.MakeView(), AccessPolicy::Truncate_Binary))
        return false;

    return true;
}
//----------------------------------------------------------------------------
bool Save(const Image* src, const FFilename& filename, IStreamWriter* writer) {
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

    const FExtname ext = filename.Extname();
    if (FFileSystemConstNames::PngExt() == ext) {
        AssertRelease(EColorDepth::_8bits == src->_depth);
        result = ::stbi_write_png_to_func(&WriteFuncForSTBIW_, writer, x, y, comp, src->_data.data(), 0);
    }
    else if (FFileSystemConstNames::TgaExt() == ext) {
        AssertRelease(EColorDepth::_8bits == src->_depth);
        result = ::stbi_write_tga_to_func(&WriteFuncForSTBIW_, writer, x, y, comp, src->_data.data());
    }
    else if (FFileSystemConstNames::HdrExt() == ext) {
        AssertRelease(EColorDepth::_32bits == src->_depth);
        AssertRelease(EColorSpace::Float == src->_space);
        result = ::stbi_write_hdr_to_func(&WriteFuncForSTBIW_, writer, x, y, comp, reinterpret_cast<const float*>(src->_data.data()));
    }
    else if (FFileSystemConstNames::BmpExt() == ext) {
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
void Image::Start() {
    // TODO: remove when fixed in an updated version
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
