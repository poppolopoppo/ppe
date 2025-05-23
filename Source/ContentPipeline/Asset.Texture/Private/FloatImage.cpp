﻿// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "FloatImage.h"
#include "Pixmap_fwd.h"

#include "Allocator/Allocation.h"
#include "Allocator/PoolAllocator-impl.h"
#include "Allocator/TrackingMalloc.h"
#include "Color/Color.h"
#include "Container/BitSet.h"
#include "Diagnostic/Logger.h"
#include "HAL/PlatformMemory.h"
#include "Maths/ScalarVector.h"
#include "Maths/MathHelpers.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION

#define STBIR_MALLOC(size,c) PPE::tracking_malloc<MEMORYDOMAIN_TAG(STBImage)>(size)
#define STBIR_FREE(ptr,c) PPE::tracking_free(ptr)
#define STBIR_ASSERT(x) \
    Assert(NOOP("stb_image_resize: "), (x))

#define STBIR_DEFAULT_FILTER_UPSAMPLE     STBIR_FILTER_CATMULLROM
#define STBIR_DEFAULT_FILTER_DOWNSAMPLE   STBIR_FILTER_CUBICBSPLINE // TODO: workaround invalid alpha with STBIR_FILTER_MITCHELL

PRAGMA_MSVC_WARNING_PUSH()
PRAGMA_MSVC_WARNING_DISABLE(4100) // 'XXX': unreferenced formal parameter

#include "Core.External/stb/stb_image_resize.h"

PRAGMA_MSVC_WARNING_POP()

namespace PPE {
namespace Pixmap {
EXTERN_LOG_CATEGORY(PPE_PIXMAP_API, Pixmap)
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
    FPlatformMemory::MemcpyLarge(dst->_data.data(), src.data(), src.SizeInBytes());
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

    PPE_LOG(Pixmap, Info, "resizing a FFloatImage from {0}x{1} to {2}x{3}",
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
} //!namespace PPE
