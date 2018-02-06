#include "stdafx.h"

#include "FloatImage.h"
#include "Pixmap_fwd.h"

#include "Core/Allocator/Allocation.h"
#include "Core/Allocator/PoolAllocator-impl.h"
#include "Core/Allocator/TrackingMalloc.h"
#include "Core/Color/Color.h"
#include "Core/Diagnostic/Logger.h"
#include "Core/Container/BitSet.h"
#include "Core/Maths/ScalarVector.h"
#include "Core/Maths/MathHelpers.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION

#define STBIR_MALLOC(size,c) Core::tracking_malloc_thread_local<MEMORY_DOMAIN_TAG(STBImage)>(size)
#define STBIR_FREE(ptr,c) Core::tracking_free_thread_local<MEMORY_DOMAIN_TAG(STBImage)>(ptr)
#define STBIR_ASSERT(x) \
    Assert(NOOP("stb_image_resize: "), (x))

#define STBIR_DEFAULT_FILTER_UPSAMPLE     STBIR_FILTER_CATMULLROM
#define STBIR_DEFAULT_FILTER_DOWNSAMPLE   STBIR_FILTER_CUBICBSPLINE // TODO: workaround invalid alpha with STBIR_FILTER_MITCHELL

#include "Core.External/stb/stb_image_resize.h"

namespace Core {
namespace Pixmap {
EXTERN_LOG_CATEGORY(CORE_PIXMAP_API, Pixmap);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Pixmap, FFloatImage, )
//----------------------------------------------------------------------------
FFloatImage::FFloatImage() : _width(0), _height(0) {}
//----------------------------------------------------------------------------
FFloatImage::FFloatImage(size_t width, size_t height)
:   FFloatImage() {
    Resize_DiscardData(width, height);
}
//----------------------------------------------------------------------------
FFloatImage::FFloatImage(size_t width, size_t height, const color_type& value)
:   FFloatImage() {
    Resize_DiscardData(width, height, value);
}
//----------------------------------------------------------------------------
FFloatImage::~FFloatImage() {}
//----------------------------------------------------------------------------
uint2 FFloatImage::WidthHeight() const {
    return uint2(checked_cast<unsigned>(_width), checked_cast<unsigned>(_height));
}
//----------------------------------------------------------------------------
float2 FFloatImage::DuDv() const {
    return float2(Du(), Dv());
}
//----------------------------------------------------------------------------
auto FFloatImage::at(const uint2& xy) -> color_type& {
    return at(xy.x(), xy.y());
}
//----------------------------------------------------------------------------
auto FFloatImage::at(const uint2& xy) const -> const color_type& {
    return at(xy.x(), xy.y());
}
//----------------------------------------------------------------------------
auto FFloatImage::SampleClamp(int x, int y) const -> const color_type& {
    x = Clamp(x, 0, int(_width)-1);
    y = Clamp(x, 0, int(_height)-1);
    return _data[x + y * _width];
}
//----------------------------------------------------------------------------
auto FFloatImage::SampleWrap(int x, int y) const -> const color_type& {
    x = (x + int(_width)) % int(_width);
    y = (y + int(_height)) % int(_height);
    return _data[x + y * _width];
}
//----------------------------------------------------------------------------
auto FFloatImage::Scanline(size_t row) -> TMemoryView<color_type> {
    return _data.MakeView().SubRange(row * _width, _width);
}
//----------------------------------------------------------------------------
auto FFloatImage::Scanline(size_t row) const -> TMemoryView<const color_type> {
    return _data.MakeConstView().SubRange(row * _width, _width);
}
//----------------------------------------------------------------------------
void FFloatImage::DiscardAlpha() {
    for (color_type& color : MakeView())
        color.A = 1.0f;
}
//----------------------------------------------------------------------------
bool FFloatImage::HasAlpha() const {
    for (const color_type& color : MakeConstView())
        if (color.A != 1.0f)
            return true;

    return false;
}
//----------------------------------------------------------------------------
bool FFloatImage::HasVisiblePixels(float cutoff/* = 1.0f/255 */) const {
    Assert(cutoff >= 0.f && cutoff < 1.f);

    for (const color_type& color : MakeConstView())
        if (color.A > cutoff)
            return true;

    return false;
}
//----------------------------------------------------------------------------
void FFloatImage::Fill(const color_type& value) {
    for (color_type& color : MakeView())
        color = value;
}
//----------------------------------------------------------------------------
void FFloatImage::CopyTo(FFloatImage* dst) const {
    Assert(dst);

    dst->Resize_DiscardData(_width, _height);
    const auto src = MakeConstView();
    ::memcpy(dst->_data.data(), src.data(), src.SizeInBytes());
}
//----------------------------------------------------------------------------
void FFloatImage::Resize_DiscardData(const uint2& size) {
    Resize_DiscardData(size.x(), size.y());
}
//----------------------------------------------------------------------------
void FFloatImage::Resize_DiscardData(const uint2& size, const color_type& value) {
    Resize_DiscardData(size.x(), size.y(), value);
}
//----------------------------------------------------------------------------
void FFloatImage::Resize_DiscardData(size_t width, size_t height) {
    Assert((width != 0) == (height != 0));

    if (_width == width &&
        _height == height)
        return;

    _width = width;
    _height = height;

    _data.Resize_DiscardData(width * height);
}
//----------------------------------------------------------------------------
void FFloatImage::Resize_DiscardData(size_t width, size_t height, const color_type& value) {
    Assert((width != 0) == (height != 0));

    Resize_DiscardData(width, height);

    for (color_type& color : MakeView())
        color = value;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Resize(FFloatImage* dst, const FFloatImage* src) {
    Assert(dst);
    return Resize(dst, src, dst->_width, dst->_height);
}
//----------------------------------------------------------------------------
bool Resize(FFloatImage* dst, const FFloatImage* src, size_t width, size_t height) {
    Assert(dst);
    Assert(src);
    Assert(width > 0);
    Assert(height > 0);

    LOG(Pixmap, Info, L"resizing a FFloatImage from {0}x{1} to {2}x{3}",
        src->Width(), src->Height(),
        width, height );

    dst->Resize_DiscardData(width, height);

    const float* srcPixels = reinterpret_cast<const float*>(src->_data.data());
    float* const dstPixels = reinterpret_cast<float*>(dst->_data.data());

    const int result = ::stbir_resize_float(
        srcPixels, checked_cast<int>(src->_width), checked_cast<int>(src->_height), 0,
        dstPixels, checked_cast<int>(width), checked_cast<int>(height), 0,
        4 );

    return (result == 1);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Pixmap
} //!namespace Core
