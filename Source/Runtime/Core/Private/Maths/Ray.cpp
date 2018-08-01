#include "stdafx.h"

#include "Ray.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FRay FRay::FromSegment(const float3& a, const float3& b) {
    Assert(LengthSq3(a - b) > F_Epsilon);

    const float3& origin = a;
    const float3 direction = Normalize3(b - origin);

    return FRay(origin, direction);
}
//----------------------------------------------------------------------------
FRay FRay::Reflect(const FRay& ray, float distance, const float3& normal) {
    return Reflect(ray, ray.At(distance), normal);
}
//----------------------------------------------------------------------------
FRay FRay::Reflect(const FRay& ray, const float3& point, const float3& normal) {
    const float3 reflected = ray.Direction() - normal * (2.f * Dot3(ray.Direction(), normal));
    return FRay(point, Normalize3(reflected));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
