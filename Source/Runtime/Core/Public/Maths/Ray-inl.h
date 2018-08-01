#pragma once

#include "Maths/Ray.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline FRay::FRay(const float3& origin, const float3& direction)
:   _origin(origin)
,   _direction(direction) {}
//----------------------------------------------------------------------------
inline FRay::FRay(const FRay& other)
:   _origin(other._origin)
,   _direction(other._direction) {}
//----------------------------------------------------------------------------
inline FRay& FRay::operator =(const FRay& other) {
    _origin = other._origin;
    _direction = other._direction;
    return *this;
}
//----------------------------------------------------------------------------
inline bool FRay::Intersects(const float3& point) const {
    return Collision::RayIntersectsPoint(*this, point);
}
//----------------------------------------------------------------------------
inline bool FRay::Intersects(const FRay& ray) const {
    float3 point;
    return Collision::RayIntersectsRay(*this, ray, &point);
}
//----------------------------------------------------------------------------
inline bool FRay::Intersects(const FRay& ray, float3* point) const {
    return Collision::RayIntersectsRay(*this, ray, point);
}
//----------------------------------------------------------------------------
inline bool FRay::Intersects(const FPlane& plane) const {
    float distance;
    return Collision::RayIntersectsPlane(*this, plane, &distance);
}
//----------------------------------------------------------------------------
inline bool FRay::Intersects(const FPlane& plane, float* distance) const {
    return Collision::RayIntersectsPlane(*this, plane, distance);
}
//----------------------------------------------------------------------------
inline bool FRay::Intersects(const FPlane& plane, float3* point) const {
    return Collision::RayIntersectsPlane(*this, plane, point);
}
//----------------------------------------------------------------------------
inline bool FRay::Intersects(const float3& triangle1, const float3& triangle2, const float3& triangle3) const {
    float distance;
    return Collision::RayIntersectsTriangle(*this, triangle1, triangle2, triangle3, &distance);
}
//----------------------------------------------------------------------------
inline bool FRay::Intersects(const float3& triangle1, const float3& triangle2, const float3& triangle3, float* distance) const {
    return Collision::RayIntersectsTriangle(*this, triangle1, triangle2, triangle3, distance);
}
//----------------------------------------------------------------------------
inline bool FRay::Intersects(const float3& triangle1, const float3& triangle2, const float3& triangle3, float3* point) const {
    return Collision::RayIntersectsTriangle(*this, triangle1, triangle2, triangle3, point);
}
//----------------------------------------------------------------------------
inline bool FRay::Intersects(const FBoundingBox& box) const {
    float distance;
    return Collision::RayIntersectsBox(*this, box, &distance);
}
//----------------------------------------------------------------------------
inline bool FRay::Intersects(const FBoundingBox& box, float* distance) const {
    return Collision::RayIntersectsBox(*this, box, distance);
}
//----------------------------------------------------------------------------
inline bool FRay::Intersects(const FBoundingBox& box, float3* point) const {
    return Collision::RayIntersectsBox(*this, box, point);
}
//----------------------------------------------------------------------------
inline bool FRay::Intersects(const FSphere& sphere) const {
    float distance;
    return Collision::RayIntersectsSphere(*this, sphere, &distance);
}
//----------------------------------------------------------------------------
inline bool FRay::Intersects(const FSphere& sphere, float* distance) const {
    return Collision::RayIntersectsSphere(*this, sphere, distance);
}
//----------------------------------------------------------------------------
inline bool FRay::Intersects(const FSphere& sphere, float3* point) const {
    return Collision::RayIntersectsSphere(*this, sphere, point);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
