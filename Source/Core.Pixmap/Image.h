#pragma once

#include "Core.Pixmap/Pixmap.h"

#include "Core.Pixmap/Enums.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Container/RawStorage.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
class Filename;
namespace Pixmap {
class FloatImage;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(Image);
class Image : public RefCountable {
public:
    // raw data stores pixel data in bgra order by default :
    typedef RAWSTORAGE(Image, u8) rawdata_type;

    Image();
    Image(  size_t width, size_t height,
            ColorDepth depth = ColorDepth::_8bits,
            ColorMask mask = ColorMask::RGBA,
            ColorSpace space = ColorSpace::sRGB );
    Image(  rawdata_type&& rdata,
            size_t width, size_t height,
            ColorDepth depth = ColorDepth::_8bits,
            ColorMask mask = ColorMask::RGBA,
            ColorSpace space = ColorSpace::sRGB );
    ~Image();

    size_t Width() const { return _width; }
    size_t Height() const { return _height; }

    ColorDepth Depth() const { return _depth; }
    ColorMask Mask() const { return _mask; }
    ColorSpace Space() const { return _space; }

    size_t PixelSizeInBytes() const;
    size_t TotalSizeInBytes() const { return _data.SizeInBytes(); }

    MemoryView<u8> MakeView() { return _data.MakeView(); }
    MemoryView<const u8> MakeConstView() const { return _data.MakeConstView(); }

    MemoryView<u8> Scanline(size_t row);
    MemoryView<const u8> Scanline(size_t row) const;

    void Resize_DiscardData(size_t width, size_t height);
    void Resize_DiscardData(size_t width, size_t height, ColorDepth depth, ColorMask mask, ColorSpace space);

    void ConvertFrom(const FloatImage* src);
    void ConvertTo(FloatImage* dst) const;

    friend bool LoadImage(Image* dst, const Filename& filename);

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    size_t _width;
    size_t _height;

    ColorDepth _depth;
    ColorMask _mask;
    ColorSpace _space;

    rawdata_type _data;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Pixmap
} //!namespace Core
