#include "stdafx.h"

#include "CompressedImage.h"

#include "Image.h"
#include "Pixmap_fwd.h"

#include "Core/Allocator/PoolAllocator-impl.h"
#include "Core/Diagnostic/Logger.h"
#include "Core/HAL/PlatformMemory.h"
#include "Core/Maths/ScalarVector.h"
#include "Core/Maths/MathHelpers.h"

#define STB_DXT_IMPLEMENTATION
#define STBD_ABS(_VAL) std::abs(_VAL)
#define STBD_FABS(_VAL) std::abs(_VAL)
#define STBD_MEMSET(_DST, _VAL, _SIZE) ::Core::FPlatformMemory::Memset((_DST), (_VAL), (_SIZE))

PRAGMA_MSVC_WARNING_PUSH()
PRAGMA_MSVC_WARNING_DISABLE(4244) // 'return': conversion from 'XXX' to 'YYY', possible loss of data

#include "Core.External/stb/stb_dxt.h"

PRAGMA_MSVC_WARNING_POP()

namespace Core {
namespace Pixmap {
EXTERN_LOG_CATEGORY(CORE_PIXMAP_API, Pixmap)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
struct rgb8_type { u8 _data[3]; };
STATIC_ASSERT(sizeof(rgb8_type) == 3);
struct rgba8_type { u8 _data[4]; };
STATIC_ASSERT(sizeof(rgba8_type) == 4);
struct rawblock_type { rgba8_type _data[4][4]; };
STATIC_ASSERT(sizeof(rawblock_type) == 4*4*4);
//----------------------------------------------------------------------------
template <EBlockFormat _Format, EColorSpace _Space>
struct TBlockTraits_ {
    static void fill_block(rawblock_type& dst, const FImage* src, size_t x, size_t y) {
        AssertNotImplemented();
    }
};
//----------------------------------------------------------------------------
template <EColorSpace _Space>
struct TBlockTraits_<EBlockFormat::DXT1, _Space> {
    static void fill_block(rawblock_type& dst, const FImage* src, size_t x, size_t y) {
        Assert(src->PixelSizeInBytes() == sizeof(rgb8_type));
        const auto* input = reinterpret_cast<const rgb8_type*>(src->MakeConstView().data());

        forrange(j, 0, 4) {
            const size_t offset = (y + j) * src->Width() + x;
            forrange(i, 0, 4) {
                dst._data[i][j]._data[0] = input[offset + i]._data[0];
                dst._data[i][j]._data[1] = input[offset + i]._data[1];
                dst._data[i][j]._data[2] = input[offset + i]._data[2];
                dst._data[i][j]._data[3] = 0;
            }
        }
    }
};
//----------------------------------------------------------------------------
template <EColorSpace _Space>
struct TBlockTraits_<EBlockFormat::DXT5, _Space> {
    static void fill_block(rawblock_type& dst, const FImage* src, size_t x, size_t y) {
        Assert(src->PixelSizeInBytes() == sizeof(rgba8_type));
        const auto* input = reinterpret_cast<const rgba8_type*>(src->MakeConstView().data());

        forrange(j, 0, 4) {
            const size_t offset = (y + j) * src->Width() + x;
            forrange(i, 0, 4) {
                dst._data[i][j]._data[0] = input[offset + i]._data[0];
                dst._data[i][j]._data[1] = input[offset + i]._data[1];
                dst._data[i][j]._data[2] = input[offset + i]._data[2];
                dst._data[i][j]._data[3] = input[offset + i]._data[3];
            }
        }
    }
};
//----------------------------------------------------------------------------
template <>
struct TBlockTraits_<EBlockFormat::DXT5, EColorSpace::YCoCg> {
    static void fill_block(rawblock_type& dst, const FImage* src, size_t x, size_t y) {
        Assert(src->PixelSizeInBytes() == sizeof(rgb8_type));
        const auto* input = reinterpret_cast<const rgb8_type*>(src->MakeConstView().data());

        forrange(j, 0, 4) {
            const size_t offset = (y + j) * src->Width() + x;
            forrange(i, 0, 4) {
                dst._data[i][j]._data[0] = input[offset + i]._data[1]; // Co in R
                dst._data[i][j]._data[1] = input[offset + i]._data[2]; // Cg in B
                dst._data[i][j]._data[2] = 0;
                dst._data[i][j]._data[3] = input[offset + i]._data[0]; // Y in A
            }
        }
    }
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <EBlockFormat _Format, EColorSpace _Space>
static void BlockSpaceCompress_(FCompressedImage* dst, const FImage* src, FCompressedImage::EQuality quality) {
    typedef TBlockTraits_<_Format, _Space> traits_type;

    STATIC_ASSERT(STB_DXT_NORMAL == size_t(FCompressedImage::EQuality::Default));
    STATIC_ASSERT(STB_DXT_DITHER == size_t(FCompressedImage::EQuality::Dithering));
    STATIC_ASSERT(STB_DXT_HIGHQUAL == size_t(FCompressedImage::EQuality::HighQuality));

    const int alpha = (_Format == EBlockFormat::DXT5 ? 1 : 0);
    const size_t blockSizeInBytes = size_t(_Format);

    const TMemoryView<u8> dstPixels = dst->MakeView();
    const size_t strideInBytes = (src->Width() * blockSizeInBytes)>>2;
    Assert((blockSizeInBytes * (src->Width()/4) * (src->Height()/4)) == dst->TotalSizeInBytes());

    rawblock_type block;
    forrange(y, 0, src->Height()>>2) {
        const TMemoryView<u8> scanline = dstPixels.SubRange(y * strideInBytes, strideInBytes);
        forrange(x, 0, src->Width()>>2)
        {
            traits_type::fill_block(block, src, x<<2, y<<2);

            ::stb_compress_dxt_block(
                reinterpret_cast<unsigned char*>(scanline.data() + x * blockSizeInBytes),
                reinterpret_cast<const unsigned char*>(&block),
                alpha,
                int(quality) );
        }
    }
}
//----------------------------------------------------------------------------
template <EBlockFormat _Format>
static void BlockCompress_(FCompressedImage* dst, const FImage* src, FCompressedImage::EQuality quality) {
    switch (dst->Space())
    {
    case EColorSpace::Linear:
        BlockSpaceCompress_<_Format, EColorSpace::Linear>(dst, src, quality);
        break;
    case EColorSpace::sRGB:
        BlockSpaceCompress_<_Format, EColorSpace::sRGB>(dst, src, quality);
        break;
    case EColorSpace::YCoCg:
        BlockSpaceCompress_<_Format, EColorSpace::YCoCg>(dst, src, quality);
        break;
    default:
        AssertNotImplemented();
        break;
    }
}
//----------------------------------------------------------------------------
static void Compress_(FCompressedImage* dst, const FImage* src, FCompressedImage::EQuality quality) {
    switch (dst->Format())
    {
    case EBlockFormat::DXT1:
        BlockCompress_<EBlockFormat::DXT1>(dst, src, quality);
        break;
    case EBlockFormat::DXT5:
        BlockCompress_<EBlockFormat::DXT5>(dst, src, quality);
        break;
    default:
        break;
    }
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Pixmap, FCompressedImage, );
//----------------------------------------------------------------------------
FCompressedImage::FCompressedImage()
:   _width(0)
,   _height(0)
,   _format(EBlockFormat::DXT1)
,   _space(EColorSpace::sRGB) {}
//----------------------------------------------------------------------------
FCompressedImage::FCompressedImage(size_t width, size_t height, EBlockFormat format, EColorSpace space)
:   FCompressedImage() {
    Resize_DiscardData(width, height, format, space);
}
//----------------------------------------------------------------------------
FCompressedImage::~FCompressedImage() {}
//----------------------------------------------------------------------------
uint2 FCompressedImage::WidthHeight() const {
    return uint2(checked_cast<unsigned>(_width), checked_cast<unsigned>(_height));
}
//----------------------------------------------------------------------------
void FCompressedImage::CopyTo(FCompressedImage* dst) const {
    Assert(dst);

    dst->Resize_DiscardData(_width, _height, _format, _space);
    Assert(dst->_data.size() == _data.size());
    Assert(((BlockSizeInBytes()*_width*_height)>>4) == _data.size());

    FPlatformMemory::MemcpyLarge(dst->_data.data(), _data.data(), _data.size());
}
//----------------------------------------------------------------------------
void FCompressedImage::Resize_DiscardData(const uint2& size, EBlockFormat format, EColorSpace space) {
    Resize_DiscardData(size.x(), size.y(), format, space);
}
//----------------------------------------------------------------------------
void FCompressedImage::Resize_DiscardData(size_t width, size_t height, EBlockFormat format, EColorSpace space) {
    AssertRelease( // DXT images must be aligned on 4 !
        (width % 4) == 0 &&
        (height % 4) == 0 );
    AssertRelease(
        (EColorSpace::sRGB == space) ||
        (EColorSpace::Linear == space) ||
        (EColorSpace::YCoCg == space) );
    Assert((EColorSpace::YCoCg != space) || (EBlockFormat::DXT5 == format));

    _width = width;
    _height = height;
    _format = format;
    _space = space;

    const size_t blockSizeInBytes = BlockSizeInBytes();
    const size_t blockCount = (_width * _height)>>4;

    _data.Resize_DiscardData(blockCount * blockSizeInBytes);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void Compress(FCompressedImage* dst, const FImage* src, FCompressedImage::EQuality quality) {
    Assert(dst);
    Assert(src);
    AssertRelease(src->Depth() == EColorDepth::_8bits);
    AssertRelease(src->Width() % 4 == 0);
    AssertRelease(src->Height() % 4 == 0);

    EBlockFormat format;
    switch (src->Mask())
    {
    case Core::Pixmap::EColorMask::R:
    case Core::Pixmap::EColorMask::RGB:
        format = EBlockFormat::DXT1;
        break;
    case Core::Pixmap::EColorMask::RG:
    case Core::Pixmap::EColorMask::RGBA:
        format = EBlockFormat::DXT5;
        break;
    default:
        AssertNotImplemented();
        break;
    }

    LOG(Pixmap, Info, L"Compress {0}_{1}_{2}:{3}x{4} image to {5}",
        src->Mask(), src->Depth(), src->Space(), src->Width(), src->Height(),
        format );

    dst->Resize_DiscardData(src->Width(), src->Height(), format, src->Space());

    Compress_(dst, src, quality);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Pixmap
} //!namespace Core
