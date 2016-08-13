#pragma once

#include "Core/Core.h"

#include "Core/Maths/ScalarVector.h"
#include "Core/Maths/ScalarVectorHelpers.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PNTriangle {
public:
    PNTriangle();
    ~PNTriangle();

    float3 LerpPosition(float u, float v, float w) const;
    float3 LerpPosition(const float3& uvw) const { return LerpPosition(uvw.x(), uvw.y(), uvw.z()); }

    float3 LerpNormal(float u, float v, float w) const;
    float3 LerpNormal(const float3& uvw) const { return LerpNormal(uvw.x(), uvw.y(), uvw.z()); }

    void Lerp(float3& position, float3& normal, float u, float v, float w) const { position = LerpPosition(u, v, w); normal = LerpNormal(u, v, w); }
    void Lerp(float3& position, float3& normal, const float3& uvw) const { Lerp(position, normal, uvw.x(), uvw.y(), uvw.z()); }

    static void FromTriangle(   PNTriangle& pn,
                                const float3& p0, const float3& n0,
                                const float3& p1, const float3& n1,
                                const float3& p2, const float3& n2 );

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
} //!namespace Core
