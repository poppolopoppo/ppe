#pragma once

#include "Core.Pixmap/Pixmap.h"

#include "Core.Pixmap/Enums.h"
#include "Core.Pixmap/PixelStorage.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
class Filename;
class IStreamWriter;
namespace Pixmap {
class FloatImage;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(Image);
class Image : public RefCountable {
public:
    // raw data stores pixel data in rgba order by default :
    typedef PIXELSTORAGE(Image, u8) raw_data_type;

    Image();
    Image(  size_t width, size_t height,
            ColorDepth depth = ColorDepth::_8bits,
            ColorMask mask = ColorMask::RGBA,
            ColorSpace space = ColorSpace::sRGB );
    Image(  raw_data_type&& rdata,
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
    size_t TotalSizeInBytes() const { return _data.size(); }

    MemoryView<u8> MakeView() { return _data.MakeView(); }
    MemoryView<const u8> MakeConstView() const { return _data.MakeConstView(); }

    MemoryView<u8> Scanline(size_t row);
    MemoryView<const u8> Scanline(size_t row) const;

    template <typename T>
    void Fill(T value);

    void Resize_DiscardData(size_t width, size_t height);
    void Resize_DiscardData(size_t width, size_t height, ColorDepth depth, ColorMask mask, ColorSpace space);

    void ConvertFrom(const FloatImage* src);
    void ConvertTo(FloatImage* dst) const;

    friend bool Load(Image* dst, const Filename& filename);
    friend bool Load(Image* dst, const Filename& filename, const MemoryView<const u8>& content);

    friend bool Save(const Image* src, const Filename& filename);
    friend bool Save(const Image* src, const Filename& filename, IStreamWriter* writer);

    SINGLETON_POOL_ALLOCATED_DECL();

    static void Start();
    static void Shutdown();

private:
    size_t _width;
    size_t _height;

    ColorDepth _depth;
    ColorMask _mask;
    ColorSpace _space;

    raw_data_type _data;
};
//----------------------------------------------------------------------------
template <typename T>
void Image::Fill(T value) {
    Assert(sizeof(T) == PixelSizeInBytes());
    u8* const pend = pdata + _data.size();
    for (u8* pdata = _data.data(); pdata < pend; pdata += sizeof(T))
        *reinterpret_cast<T*>(pdata) = value;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Pixmap
} //!namespace Core
