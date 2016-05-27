#pragma once

#include "Core.Pixmap/Pixmap.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Color/Color_fwd.h"
#include "Core/Container/RawStorage.h"
#include "Core/Maths/Geometry/ScalarVector_fwd.h"

namespace Core {
namespace Pixmap {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// FloatImage : always 4 float channels in linear space
//----------------------------------------------------------------------------
FWD_REFPTR(FloatImage);
class FloatImage : public RefCountable {
public:
    typedef ColorBGRAF color_type;
    typedef RAWSTORAGE(Image, color_type) floatdata_type;

    FloatImage();
    FloatImage(size_t width, size_t height);
    FloatImage(size_t width, size_t height, const color_type& value);
    ~FloatImage();

    size_t Width() const { return _width; }
    size_t Height() const { return _height; }

    uint2 WidthHeight() const;

    color_type& at(const uint2& xy);
    const color_type& at(const uint2& xy) const;

    color_type& at(size_t x, size_t y) {
        Assert((x < _width) && (y < _height));
        return _data[y * _width + x];
    }

    const color_type& at(size_t x, size_t y) const {
        Assert((x < _width) && (y < _height));
        return _data[y * _width + x];
    }

    color_type& operator [](const uint2& xy) { return at(xy); }
    const color_type& operator [](const uint2& xy) const { return at(xy); }

    MemoryView<color_type> MakeView() { return _data.MakeView(); }
    MemoryView<const color_type> MakeConstView() const { return _data.MakeConstView(); }

    MemoryView<color_type> Scanline(size_t row);
    MemoryView<const color_type> Scanline(size_t row) const;

    void Fill(const color_type& value);

    void Resize_DiscardData(const uint2& size);
    void Resize_DiscardData(const uint2& size, const color_type& value);

    void Resize_DiscardData(size_t width, size_t height);
    void Resize_DiscardData(size_t width, size_t height, const color_type& value);

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    size_t _width;
    size_t _height;

    floatdata_type _data;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Pixmap
} //!namespace Core
