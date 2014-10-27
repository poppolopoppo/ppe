#pragma once

#include "Core/Maths/Geometry/Sphere.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline Sphere::Sphere(const float3& center, float radius)
:   _center(center)
,   _radius(radius) {}
//----------------------------------------------------------------------------
inline Sphere::Sphere(const Sphere& other)
:   _center(other._center)
,   _radius(other._radius) {}
//----------------------------------------------------------------------------
inline Sphere& Sphere::operator =(const Sphere& other) {
    _center = other._center;
    _radius = other._radius;
    return *this;
}
//----------------------------------------------------------------------------
inline bool Sphere::Intersects(const Ray& ray) const {
    float distance;
    return Collision::RayIntersectsSphere(ray, *this, distance);
}
//----------------------------------------------------------------------------
inline bool Sphere::Intersects(const Ray& ray, float& distance) const {
    return Collision::RayIntersectsSphere(ray, *this, distance);
}
//----------------------------------------------------------------------------
inline bool Sphere::Intersects(const Ray& ray, float3& point) const {
    return Collision::RayIntersectsSphere(ray, *this, point);
}
//----------------------------------------------------------------------------
inline PlaneIntersectionType Sphere::Intersects(const Plane& plane) const {
    return Collision::PlaneIntersectsSphere(plane, *this);
}
//----------------------------------------------------------------------------
inline bool Sphere::Intersects(const float3& triangle1, const float3& triangle2, const float3& triangle3) const {
    return Collision::SphereIntersectsTriangle(*this, triangle1, triangle2, triangle3);
}
//----------------------------------------------------------------------------
inline bool Sphere::Intersects(const Sphere& sphere) const {
    return Collision::SphereIntersectsSphere(*this, sphere);
}
//----------------------------------------------------------------------------
inline ContainmentType Sphere::Contains(const float3& point) const {
    return Collision::SphereContainsPoint(*this, point);
}
//----------------------------------------------------------------------------
inline ContainmentType Sphere::Contains(const float3& triangle1, const float3& triangle2, const float3& triangle3) const {
    return Collision::SphereContainsTriangle(*this, triangle1, triangle2, triangle3);
}
//----------------------------------------------------------------------------
inline ContainmentType Sphere::Contains(const BoundingBox& box) const {
    return Collision::SphereContainsBox(*this, box);
}
//----------------------------------------------------------------------------
inline ContainmentType Sphere::Contains(const Sphere& sphere) const {
    return Collision::SphereContainsSphere(*this, sphere);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
