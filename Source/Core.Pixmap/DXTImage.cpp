#include "stdafx.h"

#include "DXTImage.h"

#include "Image.h"
#include "Pixmap_fwd.h"

#include "Core/Allocator/PoolAllocator-impl.h"
#include "Core/Diagnostic/Logger.h"
#include "Core/Maths/Geometry/ScalarVector.h"
#include "Core/Maths/MathHelpers.h"

#define STB_DXT_IMPLEMENTATION

#include "External/stb_dxt.h"

namespace Core {
namespace Pixmap {
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
template <BlockFormat _Format, ColorSpace _Space>
struct BlockTraits_ {
    static void fill_block(rawblock_type& dst, const Image* src, size_t x, size_t y) {
        AssertNotImplemented();
    }
};
//----------------------------------------------------------------------------
template <ColorSpace _Space>
struct BlockTraits_<BlockFormat::DXT1, _Space> {
    static void fill_block(rawblock_type& dst, const Image* src, size_t x, size_t y) {
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
template <ColorSpace _Space>
struct BlockTraits_<BlockFormat::DXT5, _Space> {
    static void fill_block(rawblock_type& dst, const Image* src, size_t x, size_t y) {
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
struct BlockTraits_<BlockFormat::DXT5, ColorSpace::YCoCg> {
    static void fill_block(rawblock_type& dst, const Image* src, size_t x, size_t y) {
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
template <BlockFormat _Format, ColorSpace _Space>
static void BlockSpaceCompress_(DXTImage* dst, const Image* src, DXTImage::Quality quality) {
    typedef BlockTraits_<_Format, _Space> traits_type;

    STATIC_ASSERT(STB_DXT_NORMAL == size_t(DXTImage::Quality::Default));
    STATIC_ASSERT(STB_DXT_DITHER == size_t(DXTImage::Quality::Dithering));
    STATIC_ASSERT(STB_DXT_HIGHQUAL == size_t(DXTImage::Quality::HighQuality));

    const int alpha = (_Format == BlockFormat::DXT5 ? 1 : 0);
    const size_t blockSizeInBytes = size_t(_Format);

    const MemoryView<u8> dstPixels = dst->MakeView();
    const size_t strideInBytes = (src->Width() * blockSizeInBytes)>>2;
    Assert((blockSizeInBytes * (src->Width()/4) * (src->Height()/4)) == dst->TotalSizeInBytes());

    rawblock_type block;
    forrange(y, 0, src->Height()>>2) {
        const MemoryView<u8> scanline = dstPixels.SubRange(y * strideInBytes, strideInBytes);
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
template <BlockFormat _Format>
static void BlockCompress_(DXTImage* dst, const Image* src, DXTImage::Quality quality) {
    switch (dst->Space())
    {
    case ColorSpace::Linear:
        BlockSpaceCompress_<_Format, ColorSpace::Linear>(dst, src, quality);
        break;
    case ColorSpace::sRGB:
        BlockSpaceCompress_<_Format, ColorSpace::sRGB>(dst, src, quality);
        break;
    case ColorSpace::Float:
        BlockSpaceCompress_<_Format, ColorSpace::Float>(dst, src, quality);
        break;
    case ColorSpace::YCoCg:
        BlockSpaceCompress_<_Format, ColorSpace::YCoCg>(dst, src, quality);
        break;
    default:
        AssertNotImplemented();
        break;
    }
}
//----------------------------------------------------------------------------
static void Compress_(DXTImage* dst, const Image* src, DXTImage::Quality quality) {
    switch (dst->Format())
    {
    case BlockFormat::DXT1:
        BlockCompress_<BlockFormat::DXT1>(dst, src, quality);
        break;
    case BlockFormat::DXT5:
        BlockCompress_<BlockFormat::DXT5>(dst, src, quality);
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
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Pixmap, DXTImage, );
//----------------------------------------------------------------------------
DXTImage::DXTImage()
:   _width(0)
,   _height(0)
,   _format(BlockFormat::DXT1)
,   _space(ColorSpace::sRGB) {}
//----------------------------------------------------------------------------
DXTImage::DXTImage(size_t width, size_t height, BlockFormat format, ColorSpace space)
:   DXTImage() {
    Resize_DiscardData(width, height, format, space);
}
//----------------------------------------------------------------------------
DXTImage::~DXTImage() {}
//----------------------------------------------------------------------------
uint2 DXTImage::WidthHeight() const {
    return uint2(checked_cast<unsigned>(_width), checked_cast<unsigned>(_height));
}
//----------------------------------------------------------------------------
void DXTImage::CopyTo(DXTImage* dst) const {
    Assert(dst);

    dst->Resize_DiscardData(_width, _height, _format, _space);
    Assert(dst->_data.size() == _data.size());
    Assert(((BlockSizeInBytes()*_width*_height)>>4) == _data.size());

    ::memcpy(dst->_data.data(), _data.data(), _data.size());
}
//----------------------------------------------------------------------------
void DXTImage::Resize_DiscardData(const uint2& size, BlockFormat format, ColorSpace space) {
    Resize_DiscardData(size.x(), size.y(), format, space);
}
//----------------------------------------------------------------------------
void DXTImage::Resize_DiscardData(size_t width, size_t height, BlockFormat format, ColorSpace space) {
    AssertRelease( // DXT images must be aligned on 4 !
        (width % 4) == 0 &&
        (height % 4) == 0 );
    AssertRelease(
        (ColorSpace::sRGB == space) ||
        (ColorSpace::Linear == space) ||
        (ColorSpace::YCoCg == space) );
    Assert((ColorSpace::YCoCg != space) || (BlockFormat::DXT5 == format));

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
void Compress(DXTImage* dst, const Image* src, DXTImage::Quality quality/* = Quality::Default */) {
    Assert(dst);
    Assert(src);
    AssertRelease(src->Depth() == ColorDepth::_8bits);
    AssertRelease(src->Width() % 4 == 0);
    AssertRelease(src->Height() % 4 == 0);

    BlockFormat format;
    switch (src->Mask())
    {
    case Core::Pixmap::ColorMask::R:
    case Core::Pixmap::ColorMask::RGB:
        format = BlockFormat::DXT1;
        break;
    case Core::Pixmap::ColorMask::RG:
    case Core::Pixmap::ColorMask::RGBA:
        format = BlockFormat::DXT5;
        break;
    default:
        AssertNotImplemented();
        break;
    }

    LOG(Info, L"[Pixmap] Compress {0}_{1}_{2}:{3}x{4} image to {5}",
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
