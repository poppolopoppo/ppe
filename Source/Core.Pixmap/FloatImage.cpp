#include "stdafx.h"

#include "FloatImage.h"
#include "Pixmap_fwd.h"

#include "Core/Allocator/PoolAllocator-impl.h"
#include "Core/Color/Color.h"
#include "Core/Maths/Geometry/ScalarVector.h"

namespace Core {
namespace Pixmap {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Pixmap, FloatImage, )
//----------------------------------------------------------------------------
FloatImage::FloatImage() : _width(0), _height(0) {}
//----------------------------------------------------------------------------
FloatImage::FloatImage(size_t width, size_t height)
:   FloatImage() {
    Resize_DiscardData(width, height);
}
//----------------------------------------------------------------------------
FloatImage::FloatImage(size_t width, size_t height, const color_type& value)
:   FloatImage() {
    Resize_DiscardData(width, height, value);
}
//----------------------------------------------------------------------------
FloatImage::~FloatImage() {}
//----------------------------------------------------------------------------
uint2 FloatImage::WidthHeight() const {
    return uint2(checked_cast<unsigned>(_width), checked_cast<unsigned>(_height));
}
//----------------------------------------------------------------------------
auto FloatImage::at(const uint2& xy) -> color_type& {
    return at(xy.x(), xy.y());
}
//----------------------------------------------------------------------------
auto FloatImage::at(const uint2& xy) const -> const color_type& {
    return at(xy.x(), xy.y());
}
//----------------------------------------------------------------------------
auto FloatImage::Scanline(size_t row) -> MemoryView<color_type> {
    return _data.MakeView().SubRange(row * _width, _width);
}
//----------------------------------------------------------------------------
auto FloatImage::Scanline(size_t row) const -> MemoryView<const color_type> {
    return _data.MakeConstView().SubRange(row * _width, _width);
}
//----------------------------------------------------------------------------
void FloatImage::Fill(const color_type& value) {
    for (color_type& color : _data)
        color = value;
}
//----------------------------------------------------------------------------
void FloatImage::Resize_DiscardData(const uint2& size) {
    Resize_DiscardData(size.x(), size.y());
}
//----------------------------------------------------------------------------
void FloatImage::Resize_DiscardData(const uint2& size, const color_type& value) {
    Resize_DiscardData(size.x(), size.y(), value);
}
//----------------------------------------------------------------------------
void FloatImage::Resize_DiscardData(size_t width, size_t height) {
    Resize_DiscardData(width, height, FloatImage::color_type(0.0f));
}
//----------------------------------------------------------------------------
void FloatImage::Resize_DiscardData(size_t width, size_t height, const color_type& value) {
    Assert((width != 0) == (height != 0));

    _width = width;
    _height = height;

    _data.Resize_DiscardData(width * height);

    for (color_type& color : _data)
        color = value;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Pixmap
} //!namespace Core
