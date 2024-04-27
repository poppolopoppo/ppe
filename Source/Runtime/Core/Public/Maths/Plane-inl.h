#pragma once

#include "Maths/Plane.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline FPlane::FPlane()
:   _normal(0, 0, 1)
,   _d(0) {}
//----------------------------------------------------------------------------
inline FPlane::FPlane(const float4& normalD)
:   _normal(normalD.x, normalD.y, normalD.z)
,   _d(normalD.w) {}
//----------------------------------------------------------------------------
inline FPlane::FPlane(const float3& normal, float d)
:   _normal(normal)
,   _d(d) {}
//----------------------------------------------------------------------------
inline FPlane::FPlane(const float3& normal, const float3& point)
:   _normal(normal)
,   _d(-Dot(normal, point)) {}
//----------------------------------------------------------------------------
inline FPlane::FPlane(const FPlane& other)
:   _normal(other._normal)
,   _d(other._d) {}
//----------------------------------------------------------------------------
inline FPlane& FPlane::operator =(const FPlane& other) {
    _normal = other._normal;
    _d = other._d;
    return *this;
}
//----------------------------------------------------------------------------
inline float3 FPlane::PointOnPlane() const {
    return _normal * _d;
}
//----------------------------------------------------------------------------
inline FPlane FPlane::Normalize() const {
    const float norm = Length(_normal);
    Assert(Abs(norm) > SmallEpsilon);
    return FPlane(_normal / norm, _d / norm);
}
//----------------------------------------------------------------------------
inline float FPlane::DistanceToPoint(const float3& point) const {
    return Collision::DistancePlanePoint(*this, point);
}
//----------------------------------------------------------------------------
inline EPlaneIntersectionType FPlane::Intersects(const float3& point) const {
    return Collision::PlaneIntersectsPoint(*this, point);
}
//----------------------------------------------------------------------------
inline bool FPlane::Intersects(const FRay& ray) const {
    float distance;
    return Collision::RayIntersectsPlane(ray, *this, &distance);
}
//----------------------------------------------------------------------------
inline bool FPlane::Intersects(const FRay& ray, float* distance) const {
    return Collision::RayIntersectsPlane(ray, *this, distance);
}
//----------------------------------------------------------------------------
inline bool FPlane::Intersects(const FRay& ray, float3* point) const {
    return Collision::RayIntersectsPlane(ray, *this, point);
}
//----------------------------------------------------------------------------
inline bool FPlane::Intersects(const FPlane& plane) const {
    return Collision::PlaneIntersectsPlane(*this, plane);
}
//----------------------------------------------------------------------------
inline bool FPlane::Intersects(const FPlane& plane, FRay* line) const {
    return Collision::PlaneIntersectsPlane(*this, plane, line);
}
//----------------------------------------------------------------------------
inline EPlaneIntersectionType FPlane::Intersects(const float3& triangle1, const float3& triangle2, const float3& triangle3) const {
    return Collision::PlaneIntersectsTriangle(*this, triangle1, triangle2, triangle3);
}
//----------------------------------------------------------------------------
inline EPlaneIntersectionType FPlane::Intersects(const FBoundingBox& box) const {
    return Collision::PlaneIntersectsBox(*this, box);
}
//----------------------------------------------------------------------------
inline EPlaneIntersectionType FPlane::Intersects(const FSphere& sphere) const {
    return Collision::PlaneIntersectsSphere(*this, sphere);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
