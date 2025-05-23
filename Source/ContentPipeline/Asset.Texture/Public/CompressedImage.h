#pragma once

#include "Pixmap.h"

#include "Enums.h"
#include "PixelStorage.h"

#include "Allocator/PoolAllocator.h"
#include "Maths/ScalarVector_fwd.h"

namespace PPE {
namespace Pixmap {
FWD_REFPTR(Image);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// FCompressedImage : contains a DXT1 or a DXT5 compressed image
//----------------------------------------------------------------------------
FWD_REFPTR(CompressedImage);
class FCompressedImage : public FRefCountable {
public:
    typedef PIXELSTORAGE(CompressedImage, u8) compressed_data_type;

    FCompressedImage();
    FCompressedImage(size_t width, size_t height, EBlockFormat format, EColorSpace space);
    ~FCompressedImage();

    size_t Width() const { return _width; }
    size_t Height() const { return _height; }

    uint2 WidthHeight() const;

    EBlockFormat Format() const { return _format; }
    EColorSpace Space() const { return _space; }

    size_t BlockCount() const { return (_width*_height)>>4; }
    size_t BlockSizeInBytes() const { return size_t(_format); }
    size_t TotalSizeInBytes() const { return _data.size(); }

    TMemoryView<u8> MakeView() { return _data.MakeView(); }
    TMemoryView<const u8> MakeConstView() const { return _data.MakeConstView(); }

    void CopyTo(FCompressedImage* dst) const;

    void Resize_DiscardData(const uint2& size, EBlockFormat format, EColorSpace space);
    void Resize_DiscardData(size_t width, size_t height, EBlockFormat format, EColorSpace space);

    void TrimData() { _data.TrimData(); }

    enum class EQuality
    {
        Default = 0,
        Dithering, // Do not use with normals !
        HighQuality, // Better refinement, 30-40% slower
    };

    friend void Compress(FCompressedImage* dst, const FImage* src, EQuality quality);

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    size_t _width;
    size_t _height;

    EBlockFormat _format;
    EColorSpace _space;

    compressed_data_type _data;
};
//----------------------------------------------------------------------------
void Compress(FCompressedImage* dst, const FImage* src, FCompressedImage::EQuality quality);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Pixmap
} //!namespace PPE
