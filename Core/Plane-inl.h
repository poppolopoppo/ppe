#pragma once

#include "Plane.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline Plane::Plane()
:   _normal(0, 0, 1)
,   _d(0) {}
//----------------------------------------------------------------------------
inline Plane::Plane(const float4& normalD)
:   _normal(normalD.x(), normalD.y(), normalD.z())
,   _d(normalD.w()) {}
//----------------------------------------------------------------------------
inline Plane::Plane(const float3& normal, float d)
:   _normal(normal)
,   _d(d) {}
//----------------------------------------------------------------------------
inline Plane::Plane(const float3& normal, const float3& point)
:   _normal(normal)
,   _d(-Dot3(normal, point)) {}
//----------------------------------------------------------------------------
inline Plane::Plane(const Plane& other)
:   _normal(other._normal)
,   _d(other._d) {}
//----------------------------------------------------------------------------
inline Plane& Plane::operator =(const Plane& other) {
    _normal = other._normal;
    _d = other._d;
    return *this;
}
//----------------------------------------------------------------------------
inline Plane Plane::Normalize() const {
    const float inv = 1.0f / Length3(_normal);
    return Plane(_normal * inv, _d * inv);
}
//----------------------------------------------------------------------------
inline PlaneIntersectionType Plane::Intersects(const float3& point) const {
    return Collision::PlaneIntersectsPoint(*this, point);
}
//----------------------------------------------------------------------------
inline bool Plane::Intersects(const Ray& ray) const {
    float distance;
    return Collision::RayIntersectsPlane(ray, *this, distance);
}
//----------------------------------------------------------------------------
inline bool Plane::Intersects(const Ray& ray, float& distance) const {
    return Collision::RayIntersectsPlane(ray, *this, distance);
}
//----------------------------------------------------------------------------
inline bool Plane::Intersects(const Ray& ray, float3& point) const {
    return Collision::RayIntersectsPlane(ray, *this, point);
}
//----------------------------------------------------------------------------
inline bool Plane::Intersects(const Plane& plane) const {
    return Collision::PlaneIntersectsPlane(*this, plane);
}
//----------------------------------------------------------------------------
inline bool Plane::Intersects(const Plane& plane, Ray& line) const {
    return Collision::PlaneIntersectsPlane(*this, plane, line);
}
//----------------------------------------------------------------------------
inline PlaneIntersectionType Plane::Intersects(const float3& triangle1, const float3& triangle2, const float3& triangle3) const {
    return Collision::PlaneIntersectsTriangle(*this, triangle1, triangle2, triangle3);
}
//----------------------------------------------------------------------------
inline PlaneIntersectionType Plane::Intersects(const BoundingBox& box) const {
    return Collision::PlaneIntersectsBox(*this, box);
}
//----------------------------------------------------------------------------
inline PlaneIntersectionType Plane::Intersects(const Sphere& sphere) const {
    return Collision::PlaneIntersectsSphere(*this, sphere);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
