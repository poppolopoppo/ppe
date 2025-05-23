﻿// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Maths/Ray.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FRay FRay::FromSegment(const float3& a, const float3& b) {
    Assert(LengthSq(a - b) > Epsilon);

    const float3& origin = a;
    const float3 direction = Normalize(b - origin);

    return FRay(origin, direction);
}
//----------------------------------------------------------------------------
FRay FRay::Reflect(const FRay& ray, float distance, const float3& normal) {
    return Reflect(ray, ray.At(distance), normal);
}
//----------------------------------------------------------------------------
FRay FRay::Reflect(const FRay& ray, const float3& point, const float3& normal) {
    const float3 reflected = ray.Direction() - normal * (2.f * Dot(ray.Direction(), normal));
    return FRay(point, Normalize(reflected));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
