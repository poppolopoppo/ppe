#pragma once

#include "Core.h"

#include "Maths/ScalarVector.h"
#include "Maths/ScalarVectorHelpers.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FPNTriangle {
public:
    FPNTriangle() = default;

    NODISCARD PPE_CORE_API float3 LerpPosition(float u, float v, float w) const NOEXCEPT;
    NODISCARD float3 LerpPosition(const float3& uvw) const { return LerpPosition(uvw.x, uvw.y, uvw.z); }

    NODISCARD PPE_CORE_API float3 LerpNormal(float u, float v, float w) const NOEXCEPT;
    NODISCARD float3 LerpNormal(const float3& uvw) const { return LerpNormal(uvw.x, uvw.y, uvw.z); }

    NODISCARD void Lerp(float3& position, float3& normal, float u, float v, float w) const { position = LerpPosition(u, v, w); normal = LerpNormal(u, v, w); }
    NODISCARD void Lerp(float3& position, float3& normal, const float3& uvw) const { Lerp(position, normal, uvw.x, uvw.y, uvw.z); }

    PPE_CORE_API static void FromTriangle(FPNTriangle& pn,
        const float3& p0, const float3& n0,
        const float3& p1, const float3& n1,
        const float3& p2, const float3& n2 ) NOEXCEPT;

private:
    float3 _p300, _p030, _p003;
    float3 _p210, _p021, _p201;
    float3 _p120, _p012, _p102;
    float3 _p111;

    float3 _n200, _n020, _n002;
    float3 _n110, _n011, _n101;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
