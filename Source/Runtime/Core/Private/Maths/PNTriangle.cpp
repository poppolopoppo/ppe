#include "stdafx.h"

#include "Maths/PNTriangle.h"

// http://www.gris.informatik.tu-darmstadt.de/lehre/courses/gdvI/_ws0607_/seminar/development_pn_triangles_dasbach.pdf
// http://ogldev.atspace.co.uk/www/tutorial31/tutorial31.html (mistake in the first link corrected here -> p0 != p300)

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
FORCE_INLINE float3 ProjectToPlane_(const float3& p, const float3& plane, const float3& normal) {
    return (p - normal * Dot3(p - plane, normal));
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FPNTriangle::FPNTriangle() {}
//----------------------------------------------------------------------------
FPNTriangle::~FPNTriangle() {}
//----------------------------------------------------------------------------
float3 FPNTriangle::LerpPosition(float u, float v, float w) const {
    return _p300 * (w * w * w) +
           _p030 * (u * u * u) +
           _p003 * (v * v * v) +
           _p210 * (3.0f * w * w * u) +
           _p120 * (3.0f * w * u * u) +
           _p201 * (3.0f * w * w * v) +
           _p021 * (3.0f * u * u * v) +
           _p102 * (3.0f * w * v * v) +
           _p012 * (3.0f * u * v * v) +
           _p111 * (6.0f * w * u * v);
}
//----------------------------------------------------------------------------
float3 FPNTriangle::LerpNormal(float u, float v, float w) const {
    return Normalize3(
           _n200 * (w * w) +
           _n020 * (u * u) +
           _n002 * (v * v) +
           _n110 * (w * u) +
           _n011 * (u * v) +
           _n101 * (w * v) );
}
//----------------------------------------------------------------------------
void FPNTriangle::FromTriangle(
    FPNTriangle& pn,
    const float3& p0, const float3& n0,
    const float3& p1, const float3& n1,
    const float3& p2, const float3& n2 ) {
    // The original vertices stay the same
    pn._p030 = p0; pn._n020 = n0;
    pn._p003 = p1; pn._n002 = n1;
    pn._p300 = p2; pn._n200 = n2;

    // Edges are names according to the opposing vertex
    const float3 e300 = pn._p003 - pn._p030;
    const float3 e030 = pn._p300 - pn._p003;
    const float3 e003 = pn._p030 - pn._p300;

    // Generate two midpoints on each edge
    pn._p021 = pn._p030 + e300 / 3.0f;
    pn._p012 = pn._p030 + e300 * (2.0f / 3.0f);
    pn._p102 = pn._p003 + e030 / 3.0f;
    pn._p201 = pn._p003 + e030 * (2.0f / 3.0f);
    pn._p210 = pn._p300 + e003 / 3.0f;
    pn._p120 = pn._p300 + e003 * (2.0f / 3.0f);

    // Project each midpoint on the plane defined by the nearest vertex and its normal
    pn._p021 = ProjectToPlane_(pn._p021, pn._p030, pn._n020);
    pn._p012 = ProjectToPlane_(pn._p012, pn._p003, pn._n002);
    pn._p102 = ProjectToPlane_(pn._p102, pn._p003, pn._n002);
    pn._p201 = ProjectToPlane_(pn._p201, pn._p300, pn._n200);
    pn._p210 = ProjectToPlane_(pn._p210, pn._p300, pn._n200);
    pn._p120 = ProjectToPlane_(pn._p120, pn._p030, pn._n020);

    // Handle the center
    const float3 c = (pn._p003 + pn._p030 + pn._p300) / 3.0f;
    pn._p111 = (pn._p021 + pn._p012 + pn._p102 + pn._p201 + pn._p210 + pn._p120) / 6.0f;
    pn._p111 += (pn._p111 - c) / 2.0f;

    // Normal control points
    const float3 h200 = pn._n002 + pn._n020;
    const float3 h020 = pn._n200 + pn._n002;
    const float3 h002 = pn._n020 + pn._n200;

    pn._n110 = Normalize3(h200 - e300 * ( 2.0f * Dot3(e300, h200) / Dot3(e300, e300) ));
    pn._n011 = Normalize3(h020 - e030 * ( 2.0f * Dot3(e030, h020) / Dot3(e030, e030) ));
    pn._n101 = Normalize3(h002 - e003 * ( 2.0f * Dot3(e003, h002) / Dot3(e003, e003) ));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
