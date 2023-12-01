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
    size_t rowPitch,
    size_t  slicePitch,
    EPixelFormat format,
    EImageAspect aspect) NOEXCEPT
    :   _parts(parts)
    , _dimensions(dimensions)
    , _rowPitch(rowPitch)
    , _slicePitch(slicePitch)
    , _format(format) {
    const auto encoding = EPixelFormat_Encoding(_format, aspect);
    _bitsPerPixel = encoding.BitsPerPixel;
    _loadRGBA32f = encoding.DecodeRGBA32f;
    _loadRGBA32i = encoding.DecodeRGBA32i;
    _loadRGBA32u = encoding.DecodeRGBA32u;
}
//----------------------------------------------------------------------------
void FImageView::Load(FRgba32f* pcol, const uint3& point) const NOEXCEPT {
    Assert(pcol);
    Assert(_loadRGBA32f);
    return _loadRGBA32f(pcol, Pixel(point));
}
//----------------------------------------------------------------------------
void FImageView::Load(FRgba32u* pcol, const uint3& point) const NOEXCEPT {
    Assert(pcol);
    Assert(_loadRGBA32u);
    return _loadRGBA32u(pcol, Pixel(point));
}
//----------------------------------------------------------------------------
void FImageView::Load(FRgba32i* pcol, const uint3& point) const NOEXCEPT {
    Assert(pcol);
    Assert(_loadRGBA32i);
    return _loadRGBA32i(pcol, Pixel(point));
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
        const uint3 point000 = FloorToUnsigned(pointf);
        const uint3 point111 = Min(point000 + 1_u32, _dimensions - 1_u32);
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
