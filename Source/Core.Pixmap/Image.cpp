#include "stdafx.h"

#include "Image.h"

#include "FloatImage.h"
#include "Pixmap_fwd.h"

#include "Core/Allocator/PoolAllocator-impl.h"
#include "Core/Color/Color.h"
#include "Core/IO/FileSystem.h"
#include "Core/IO/FileSystemConstNames.h"
#include "Core/Maths/Geometry/ScalarVector.h"

namespace Core {
namespace Pixmap {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <ColorDepth _Depth, ColorSpace _Space>
struct ChannelType_ {};
//----------------------------------------------------------------------------
template <ColorSpace _Space>
struct ChannelType_<ColorDepth::_8bits, _Space> {
    typedef ubyten type;
};
template <ColorSpace _Space>
struct ChannelType_<ColorDepth::_16bits, _Space> {
    typedef ushortn type;
};
template <ColorSpace _Space>
struct ChannelType_<ColorDepth::_32bits, _Space> {
    typedef uwordn type;
};
//----------------------------------------------------------------------------
template <>
struct ChannelType_<ColorDepth::_8bits, ColorSpace::Float> {
    typedef ubyten type;
};
template <>
struct ChannelType_<ColorDepth::_16bits, ColorSpace::Float> {
    typedef half type;
};
template <>
struct ChannelType_<ColorDepth::_32bits, ColorSpace::Float> {
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
    typedef typename ChannelType_<_Depth, _Space>::type channel_type;
    typedef channel_type raw_type[size_t(_Mask)];

    static void float_to_raw(raw_type* dst, const FloatImage::color_type* src) {
        for (size_t i = 0; i < size_t(_Mask); ++i)
            (*dst)[i] = src->Data()._data[i];
    }

    static void raw_to_float(FloatImage::color_type* dst, const raw_type* src) {
        for (size_t i = 0; i < size_t(_Mask); ++i)
            dst->Data()._data[i] = (*src)[i];
    }
};
//----------------------------------------------------------------------------
template <ColorDepth _Depth, ColorMask _Mask>
struct PixelTraits_<_Depth, _Mask, ColorSpace::sRGB> {
    typedef typename ChannelType_<_Depth, ColorSpace::sRGB>::type channel_type;
    typedef channel_type raw_type[size_t(_Mask)];

    static void float_to_raw(raw_type* dst, const FloatImage::color_type* src) {
        for (size_t i = 0; i < size_t(_Mask); ++i)
            (*dst)[i] = Linear_to_SRGB(src->Data()._data[i]);
    }

    static void raw_to_float(FloatImage::color_type* dst, const raw_type* src) {
        for (size_t i = 0; i < size_t(_Mask); ++i)
            dst->Data()._data[i] = SRGB_to_Linear((*src)[i]);
    }
};
//----------------------------------------------------------------------------
template <ColorDepth _Depth>
struct PixelTraits_<_Depth, ColorMask::RGBA, ColorSpace::sRGB> {
    typedef typename ChannelType_<_Depth, ColorSpace::sRGB>::type channel_type;
    typedef channel_type raw_type[4];

    static void float_to_raw(raw_type* dst, const FloatImage::color_type* src) {
        for (size_t i = 0; i < 3; ++i)
            (*dst)[i] = Linear_to_SRGB(src->Data()._data[i]);
        (*dst)[3] = src->Data()._data[3];
    }

    static void raw_to_float(FloatImage::color_type* dst, const raw_type* src) {
        for (size_t i = 0; i < 3; ++i)
            dst->Data()._data[i] = SRGB_to_Linear((*src)[i]);
        dst->Data()._data[3] = (*src)[3];
    }
};
//----------------------------------------------------------------------------
template <ColorDepth _Depth>
struct PixelTraits_<_Depth, ColorMask::RGB, ColorSpace::YCoCg> {
    typedef typename ChannelType_<_Depth, ColorSpace::YCoCg>::type channel_type;
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
    }
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <template <typename> typename _Functor, typename _Dst, typename _Src, ColorDepth _Depth, ColorMask _Mask, ColorSpace _Space>
static void TypedMaskSpaceConvert_(_Dst* dst, const _Src* src) {
    typedef PixelTraits_<_Depth, _Mask, _Space> pixel_traits;
    _Functor<pixel_traits>()(dst, src);
}
//----------------------------------------------------------------------------
template <template <typename> typename _Functor, typename _Dst, typename _Src, ColorDepth _Depth, ColorMask _Mask >
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
template <template <typename> typename _Functor, typename _Dst, typename _Src, ColorDepth _Depth >
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
template <template <typename> typename _Functor, typename _Dst, typename _Src>
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
    rawdata_type&& rdata,
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
    AssertRelease(_data.SizeInBytes() == sizeInBytes);
#endif
}
//----------------------------------------------------------------------------
Image::~Image() {
#ifdef WITH_CORE_ASSERT
    const size_t sizeInBytes = (PixelSizeInBytes() * _width * _height);
    Assert(_data.SizeInBytes() == sizeInBytes);
#endif
}
//----------------------------------------------------------------------------
size_t Image::PixelSizeInBytes() const {
    const size_t channelCount = Meta::CountBitsSet(size_t(_mask));
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

    _width = width;
    _height = height;

    const size_t sizeInBytes = (PixelSizeInBytes() * _width * _height);
    _data.Resize_DiscardData(sizeInBytes);
}
//----------------------------------------------------------------------------
void Image::Resize_DiscardData(size_t width, size_t height, ColorDepth depth, ColorMask mask, ColorSpace space) {
    Assert((0 != _width) == (0 != _height));
    Assert((space == ColorSpace::YCoCg) == (mask == ColorMask::RGBA));

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
bool LoadImage(Image* dst, const Filename& filename) {
    Assert(filename.HasExtname());
    const Extname ext = filename.Extname();

    if (ext == FileSystemConstNames::PngExt()) {
        AssertNotImplemented(); // TODO
        return true;
    }
    else if (ext == FileSystemConstNames::RawExt()) {
        AssertNotImplemented(); // TODO
        return true;
    }
    else {
        AssertNotImplemented(); // unrecognized format !
        return false;
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Pixmap
} //!namespace Core
