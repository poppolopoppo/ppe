// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "RHI/ImageView.h"

#include "RHI/PixelFormatHelpers.h"
#include "RHI/SamplerEnums.h"

#include "Maths/ScalarVectorHelpers.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FImageView::FImageView(
    const TMemoryView<const subpart_type>& parts,
    const uint3& dimensions,
    EPixelFormat format,
    EImageAspect aspect,
    bool bTilable ) NOEXCEPT
:   FImageView(parts, dimensions, format, aspect,
        bTilable ? RHI::EAddressMode::Repeat : RHI::EAddressMode::ClampToEdge)
{}
//----------------------------------------------------------------------------
FImageView::FImageView(
    const TMemoryView<const subpart_type>& parts,
    const uint3& dimensions,
    EPixelFormat format,
    EImageAspect aspect,
    EAddressMode addressMode ) NOEXCEPT
    :   _parts(parts)
    ,   _dimensions(dimensions)
    ,   _format(format)
    ,   _addressMode(addressMode) {
    const auto info = EPixelFormat_Infos(format);
    _slicePitch = info.SlicePitch(aspect, dimensions, 1);
    _rowPitch = info.RowPitch(aspect, dimensions);

    const auto encoding = EPixelFormat_Encoding(_format, aspect);
    _bitsPerPixel = encoding.BitsPerPixel;

    _loadRGBA32f = encoding.DecodeRGBA32f;
    _loadRGBA32i = encoding.DecodeRGBA32i;
    _loadRGBA32u = encoding.DecodeRGBA32u;

    _storeRGBA32f = encoding.EncodeRGBA32f;
    _storeRGBA32i = encoding.EncodeRGBA32i;
    _storeRGBA32u = encoding.EncodeRGBA32u;
}
//----------------------------------------------------------------------------
FImageView::FImageView(
    const TMemoryView<const subpart_type>& parts,
    const uint3& dimensions,
    size_t rowPitch,
    size_t  slicePitch,
    EPixelFormat format,
    EImageAspect aspect) NOEXCEPT
    :   _parts(parts)
    ,   _rowPitch(rowPitch)
    ,   _slicePitch(slicePitch)
    ,   _dimensions(dimensions)
    ,   _format(format)
    ,   _addressMode(EAddressMode::ClampToBorder) {
    const auto encoding = EPixelFormat_Encoding(_format, aspect);
    _bitsPerPixel = encoding.BitsPerPixel;

    _loadRGBA32f = encoding.DecodeRGBA32f;
    _loadRGBA32i = encoding.DecodeRGBA32i;
    _loadRGBA32u = encoding.DecodeRGBA32u;

    _storeRGBA32f = encoding.EncodeRGBA32f;
    _storeRGBA32i = encoding.EncodeRGBA32i;
    _storeRGBA32u = encoding.EncodeRGBA32u;
}
//----------------------------------------------------------------------------
void FImageView::Load(FRgba32f* pcol, const uint3& point) const NOEXCEPT {
    Assert(pcol);
    Assert(_loadRGBA32f);
    return _loadRGBA32f(pcol, At(point));
}
//----------------------------------------------------------------------------
void FImageView::Load(FRgba32u* pcol, const uint3& point) const NOEXCEPT {
    Assert(pcol);
    Assert(_loadRGBA32u);
    return _loadRGBA32u(pcol, At(point));
}
//----------------------------------------------------------------------------
void FImageView::Load(FRgba32i* pcol, const uint3& point) const NOEXCEPT {
    Assert(pcol);
    Assert(_loadRGBA32i);
    return _loadRGBA32i(pcol, At(point));
}
//----------------------------------------------------------------------------
void FImageView::Load(FRgba32f* pcol, const int3& point) const NOEXCEPT {
    Assert(pcol);
    Assert(_loadRGBA32f);
    return _loadRGBA32f(pcol, Pixel(point));
}
//----------------------------------------------------------------------------
void FImageView::Load(FRgba32u* pcol, const int3& point) const NOEXCEPT {
    Assert(pcol);
    Assert(_loadRGBA32u);
    return _loadRGBA32u(pcol, Pixel(point));
}
//----------------------------------------------------------------------------
void FImageView::Load(FRgba32i* pcol, const int3& point) const NOEXCEPT {
    Assert(pcol);
    Assert(_loadRGBA32i);
    return _loadRGBA32i(pcol, Pixel(point));
}
//----------------------------------------------------------------------------
void FImageView::Store(const uint3& safePoint, const FRgba32f& col) const NOEXCEPT {
    Assert(_storeRGBA32f);
    return _storeRGBA32f(RemoveConstView(At(safePoint)), col);
}
//----------------------------------------------------------------------------
void FImageView::Store(const uint3& safePoint, const FRgba32i& col) const NOEXCEPT {
    Assert(_storeRGBA32i);
    return _storeRGBA32i(RemoveConstView(At(safePoint)), col);
}
//----------------------------------------------------------------------------
void FImageView::Store(const uint3& safePoint, const FRgba32u& col) const NOEXCEPT {
    Assert(_storeRGBA32u);
    return _storeRGBA32u(RemoveConstView(At(safePoint)), col);
}
//----------------------------------------------------------------------------
void FImageView::Store(const int3& point, const FRgba32f& col) const NOEXCEPT {
    Assert(_storeRGBA32f);
    return _storeRGBA32f(RemoveConstView(Pixel(point)), col);
}
//----------------------------------------------------------------------------
void FImageView::Store(const int3& point, const FRgba32i& col) const NOEXCEPT {
    Assert(_storeRGBA32i);
    return _storeRGBA32i(RemoveConstView(Pixel(point)), col);
}
//----------------------------------------------------------------------------
void FImageView::Store(const int3& point, const FRgba32u& col) const NOEXCEPT {
    Assert(_storeRGBA32u);
    return _storeRGBA32u(RemoveConstView(Pixel(point)), col);
}
//----------------------------------------------------------------------------
void FImageView::Load(FRgba32f* pcol, const float3& uvw) const NOEXCEPT {
    Assert(pcol);
    Assert(_loadRGBA32f);
    return _loadRGBA32f(pcol, Texel(uvw));
}
//----------------------------------------------------------------------------
void FImageView::Load(FRgba32u* pcol, const float3& uvw) const NOEXCEPT {
    Assert(pcol);
    Assert(_loadRGBA32u);
    return _loadRGBA32u(pcol, Texel(uvw));
}
//----------------------------------------------------------------------------
void FImageView::Load(FRgba32i* pcol, const float3& uvw) const NOEXCEPT {
    Assert(pcol);
    Assert(_loadRGBA32i);
    return _loadRGBA32i(pcol, Texel(uvw));
}
//----------------------------------------------------------------------------
void FImageView::Load(FRgba32f* pcol, const float3& uvw, ETextureFilter filter) const NOEXCEPT {
    switch (filter) {
    case ETextureFilter::Nearest:
    {
        Load(pcol, uvw);
        return;
    }
    case ETextureFilter::Linear:
    {
        float3 pointf{ (uvw + 1.0f) * 0.5f * float3(Dimensions()) + (0.5f - Epsilon) };
        const int3 point000 = FloorToInt(pointf);
        const int3 point111 = Min(point000 + 1_i32, checked_cast<i32>(_dimensions - 1_u32));
        pointf = SStep(pointf - float3(point000));

        FRgba32f col00{ Meta::NoInit };
        Load(&col00, point000);

        if (Likely(point111 != point000)) {

            FRgba32f col10{ Meta::NoInit };
            FRgba32f col01{ Meta::NoInit };
            FRgba32f col11{ Meta::NoInit };
            Load(&col10, Blend(point111, point000, bool3(1, 0, 0)));
            Load(&col01, Blend(point111, point000, bool3(0, 1, 0)));
            Load(&col11, Blend(point111, point000, bool3(1, 1, 0)));

            // 2d
            *pcol = BilateralLerp(col00, col10, col11, col01, pointf.xy);

            if (point111.z != point000.z) {
                // load next slice
                Load(&col00, Blend(point111, point000, bool3(0, 0, 1)));
                Load(&col10, Blend(point111, point000, bool3(1, 0, 1)));
                Load(&col01, Blend(point111, point000, bool3(0, 1, 1)));
                Load(&col11, Blend(point111, point000, bool3(1, 1, 1)));

                // 3d
                *pcol = Lerp(*pcol, BilateralLerp(col00, col10, col11, col01, pointf.xy), pointf.z);
            }
            else {
                Assert_NoAssume(point111.z == point000.z);
            }
        }
        else {
            *pcol = col00;
        }
        return;
    }
    case ETextureFilter::Cubic:
    {
        AssertNotImplemented(); // #TODO: add support for cubic filtering? fake with IQ smooth bilinear?
        return;
    }
    default:
    {
        AssertNotReached();
        return;
    }
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
} //!namespace RHI
