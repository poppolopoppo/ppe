#pragma once

#include "Maths/Sphere.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline FSphere::FSphere(const float3& center, float radius)
:   _center(center)
,   _radius(radius) {}
//----------------------------------------------------------------------------
inline FSphere::FSphere(const FSphere& other)
:   _center(other._center)
,   _radius(other._radius) {}
//----------------------------------------------------------------------------
inline FSphere& FSphere::operator =(const FSphere& other) {
    _center = other._center;
    _radius = other._radius;
    return *this;
}
//----------------------------------------------------------------------------
inline bool FSphere::Intersects(const FRay& ray) const {
    float distance;
    return Collision::RayIntersectsSphere(ray, *this, &distance);
}
//----------------------------------------------------------------------------
inline bool FSphere::Intersects(const FRay& ray, float* distance) const {
    return Collision::RayIntersectsSphere(ray, *this, distance);
}
//----------------------------------------------------------------------------
inline bool FSphere::Intersects(const FRay& ray, float3* point) const {
    return Collision::RayIntersectsSphere(ray, *this, point);
}
//----------------------------------------------------------------------------
inline EPlaneIntersectionType FSphere::Intersects(const FPlane& plane) const {
    return Collision::PlaneIntersectsSphere(plane, *this);
}
//----------------------------------------------------------------------------
inline bool FSphere::Intersects(const float3& triangle1, const float3& triangle2, const float3& triangle3) const {
    return Collision::SphereIntersectsTriangle(*this, triangle1, triangle2, triangle3);
}
//----------------------------------------------------------------------------
inline bool FSphere::Intersects(const FSphere& sphere) const {
    return Collision::SphereIntersectsSphere(*this, sphere);
}
//----------------------------------------------------------------------------
inline EContainmentType FSphere::Contains(const float3& point) const {
    return Collision::SphereContainsPoint(*this, point);
}
//----------------------------------------------------------------------------
inline EContainmentType FSphere::Contains(const float3& triangle1, const float3& triangle2, const float3& triangle3) const {
    return Collision::SphereContainsTriangle(*this, triangle1, triangle2, triangle3);
}
//----------------------------------------------------------------------------
inline EContainmentType FSphere::Contains(const FBoundingBox& box) const {
    return Collision::SphereContainsBox(*this, box);
}
//----------------------------------------------------------------------------
inline EContainmentType FSphere::Contains(const FSphere& sphere) const {
    return Collision::SphereContainsSphere(*this, sphere);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
