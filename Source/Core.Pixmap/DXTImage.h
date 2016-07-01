#pragma once

#include "Core.Pixmap/Pixmap.h"

#include "Core.Pixmap/Enums.h"
#include "Core.Pixmap/PixelStorage.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Maths/Geometry/ScalarVector_fwd.h"

namespace Core {
namespace Pixmap {
class Image;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// DXTImage : contains a DXT1 or a DXT5 compressed image
//----------------------------------------------------------------------------
FWD_REFPTR(DXTImage);
class DXTImage : public RefCountable {
public:
    typedef PIXELSTORAGE(DXTImage, u8) compressed_data_type;

    DXTImage();
    DXTImage(size_t width, size_t height, BlockFormat format, ColorSpace space);
    ~DXTImage();

    size_t Width() const { return _width; }
    size_t Height() const { return _height; }

    uint2 WidthHeight() const;

    BlockFormat Format() const { return _format; }
    ColorSpace Space() const { return _space; }

    size_t BlockCount() const { return (_width*_height)>>4; }
    size_t BlockSizeInBytes() const { return size_t(_format); }
    size_t TotalSizeInBytes() const { return _data.size(); }

    MemoryView<u8> MakeView() { return _data.MakeView(); }
    MemoryView<const u8> MakeConstView() const { return _data.MakeConstView(); }

    void CopyTo(DXTImage* dst) const;

    void Resize_DiscardData(const uint2& size, BlockFormat format, ColorSpace space);
    void Resize_DiscardData(size_t width, size_t height, BlockFormat format, ColorSpace space);

    void TrimData() { _data.TrimData(); }

    enum class Quality
    {
        Default = 0,
        Dithering, // Do not use with normals !
        HighQuality, // Better refinement, 30-40% slower
    };

    friend void Compress(DXTImage* dst, const Image* src, Quality quality);

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    size_t _width;
    size_t _height;

    BlockFormat _format;
    ColorSpace _space;

    compressed_data_type _data;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Pixmap
} //!namespace Core
