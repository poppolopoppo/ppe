#pragma once

#include "Core/Maths/Geometry/Ray.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline Ray::Ray(const float3& origin, const float3& direction)
:   _origin(origin)
,   _direction(direction) {}
//----------------------------------------------------------------------------
inline Ray::Ray(const Ray& other)
:   _origin(other._origin)
,   _direction(other._direction) {}
//----------------------------------------------------------------------------
inline Ray& Ray::operator =(const Ray& other) {
    _origin = other._origin;
    _direction = other._direction;
    return *this;
}
//----------------------------------------------------------------------------
inline bool Ray::Intersects(const float3& point) const {
    return Collision::RayIntersectsPoint(*this, point);
}
//----------------------------------------------------------------------------
inline bool Ray::Intersects(const Ray& ray) const {
    float3 point;
    return Collision::RayIntersectsRay(*this, ray, point);
}
//----------------------------------------------------------------------------
inline bool Ray::Intersects(const Ray& ray, float3& point) const {
    return Collision::RayIntersectsRay(*this, ray, point);
}
//----------------------------------------------------------------------------
inline bool Ray::Intersects(const Plane& plane) const {
    float distance;
    return Collision::RayIntersectsPlane(*this, plane, distance);
}
//----------------------------------------------------------------------------
inline bool Ray::Intersects(const Plane& plane, float& distance) const {
    return Collision::RayIntersectsPlane(*this, plane, distance);
}
//----------------------------------------------------------------------------
inline bool Ray::Intersects(const Plane& plane, float3& point) const {
    return Collision::RayIntersectsPlane(*this, plane, point);
}
//----------------------------------------------------------------------------
inline bool Ray::Intersects(const float3& triangle1, const float3& triangle2, const float3& triangle3) const {
    float distance;
    return Collision::RayIntersectsTriangle(*this, triangle1, triangle2, triangle3, distance);
}
//----------------------------------------------------------------------------
inline bool Ray::Intersects(const float3& triangle1, const float3& triangle2, const float3& triangle3, float& distance) const {
    return Collision::RayIntersectsTriangle(*this, triangle1, triangle2, triangle3, distance);
}
//----------------------------------------------------------------------------
inline bool Ray::Intersects(const float3& triangle1, const float3& triangle2, const float3& triangle3, float3& point) const {
    return Collision::RayIntersectsTriangle(*this, triangle1, triangle2, triangle3, point);
}
//----------------------------------------------------------------------------
inline bool Ray::Intersects(const BoundingBox& box) const {
    float distance;
    return Collision::RayIntersectsBox(*this, box, distance);
}
//----------------------------------------------------------------------------
inline bool Ray::Intersects(const BoundingBox& box, float& distance) const {
    return Collision::RayIntersectsBox(*this, box, distance);
}
//----------------------------------------------------------------------------
inline bool Ray::Intersects(const BoundingBox& box, float3& point) const {
    return Collision::RayIntersectsBox(*this, box, point);
}
//----------------------------------------------------------------------------
inline bool Ray::Intersects(const Sphere& sphere) const {
    float distance;
    return Collision::RayIntersectsSphere(*this, sphere, distance);
}
//----------------------------------------------------------------------------
inline bool Ray::Intersects(const Sphere& sphere, float& distance) const {
    return Collision::RayIntersectsSphere(*this, sphere, distance);
}
//----------------------------------------------------------------------------
inline bool Ray::Intersects(const Sphere& sphere, float3& point) const {
    return Collision::RayIntersectsSphere(*this, sphere, point);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
