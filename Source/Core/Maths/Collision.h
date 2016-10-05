#pragma once

#include "Core/Core.h"

#include "Core/Maths/ScalarBoundingBox_fwd.h"
#include "Core/Maths/ScalarVector_fwd.h"

namespace Core {
class FPlane;
class FRay;
class FSphere;

enum class EContainmentType {
    Contains = 0,
    Disjoint,
    Intersects
};

enum class EPlaneIntersectionType {
    Back = 0,
    Front,
    Intersecting
};

namespace Collision {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
float3 ClosestPointPointTriangle(const float3& point, const float3& vertex1, const float3& vertex2, const float3& vertex3);
//----------------------------------------------------------------------------
float3 ClosestPointPlanePoint(const FPlane& plane, const float3& point);
//----------------------------------------------------------------------------
float3 ClosestPointBoxPoint(const BoundingBox& box, const float3& point);
//----------------------------------------------------------------------------
float3 ClosestPointSpherePoint(const FSphere& sphere, const float3& point);
//----------------------------------------------------------------------------
float3 ClosestPointSphereSphere(const FSphere& sphere1, const FSphere& sphere2);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
float DistancePlanePoint(const FPlane& plane, const float3& point);
//----------------------------------------------------------------------------
float DistanceBoxPoint(const BoundingBox& box, const float3& point);
//----------------------------------------------------------------------------
float DistanceBoxBox(const BoundingBox& box1, const BoundingBox& box2);
//----------------------------------------------------------------------------
float DistanceSpherePoint(const FSphere& sphere, const float3& point);
//----------------------------------------------------------------------------
float DistanceSphereSphere(const FSphere& sphere1, const FSphere& sphere2);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
float DistanceLine(const float2& a, const float2& b, const float2& point);
//----------------------------------------------------------------------------
float2 LineBisector(const float2& a, const float2& b, const float2& c, float2& bisector);
//----------------------------------------------------------------------------
bool LineIntersectsLine(const float2& a0, const float2& b0, const float2& a1, const float2& b1, float2& point);
//----------------------------------------------------------------------------
float2 LineOrtho(const float2& dir);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool RayIntersectsPoint(const FRay& ray, const float3& point);
//----------------------------------------------------------------------------
bool RayIntersectsRay(const FRay& ray1, const FRay& ray2, float3& point);
//----------------------------------------------------------------------------
bool RayIntersectsPlane(const FRay& ray, const FPlane& plane, float& distance);
//----------------------------------------------------------------------------
bool RayIntersectsPlane(const FRay& ray, const FPlane& plane, float3& point);
//----------------------------------------------------------------------------
bool RayIntersectsTriangle(const FRay& ray, const float3& vertex1, const float3& vertex2, const float3& vertex3, float& distance);
//----------------------------------------------------------------------------
bool RayIntersectsTriangle(const FRay& ray, const float3& vertex1, const float3& vertex2, const float3& vertex3, float3& point);
//----------------------------------------------------------------------------
bool RayIntersectsBox(const FRay& ray, const BoundingBox& box, float& distance);
//----------------------------------------------------------------------------
bool RayIntersectsBox(const FRay& ray, const BoundingBox& box, float3& point);
//----------------------------------------------------------------------------
bool RayIntersectsSphere(const FRay& ray, const FSphere& sphere, float& distance);
//----------------------------------------------------------------------------
bool RayIntersectsSphere(const FRay& ray, const FSphere& sphere, float3& point);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
EPlaneIntersectionType PlaneIntersectsPoint(const FPlane& plane, const float3& point);
//----------------------------------------------------------------------------
bool PlaneIntersectsPlane(const FPlane& plane1, const FPlane& plane2);
//----------------------------------------------------------------------------
bool PlaneIntersectsPlane(const FPlane& plane1, const FPlane& plane2, FRay& line);
//----------------------------------------------------------------------------
EPlaneIntersectionType PlaneIntersectsTriangle(const FPlane& plane, const float3& vertex1, const float3& vertex2, const float3& vertex3);
//----------------------------------------------------------------------------
EPlaneIntersectionType PlaneIntersectsBox(const FPlane& plane, const BoundingBox& box);
//----------------------------------------------------------------------------
EPlaneIntersectionType PlaneIntersectsSphere(const FPlane& plane, const FSphere& sphere);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool BoxIntersectsTriangle(const BoundingBox& box, const float3& vertex1, const float3& vertex2, const float3& vertex3);
//----------------------------------------------------------------------------
bool BoxIntersectsBox(const BoundingBox& box1, const BoundingBox& box2);
//----------------------------------------------------------------------------
bool BoxIntersectsSphere(const BoundingBox& box, const FSphere& sphere);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool SphereIntersectsTriangle(const FSphere& sphere, const float3& vertex1, const float3& vertex2, const float3& vertex3);
//----------------------------------------------------------------------------
bool SphereIntersectsSphere(const FSphere& sphere1, const FSphere& sphere2);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
EContainmentType BoxContainsPoint(const BoundingBox& box, const float3& point);
//----------------------------------------------------------------------------
EContainmentType BoxContainsTriangle(const BoundingBox& box, const float3& vertex1, const float3& vertex2, const float3& vertex3);
//----------------------------------------------------------------------------
EContainmentType BoxContainsBox(const BoundingBox& box1, const BoundingBox& box2);
//----------------------------------------------------------------------------
EContainmentType BoxContainsSphere(const BoundingBox& box, const FSphere& sphere);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
EContainmentType SphereContainsPoint(const FSphere& sphere, const float3& point);
//----------------------------------------------------------------------------
EContainmentType SphereContainsTriangle(const FSphere& sphere, const float3& vertex1, const float3& vertex2, const float3& vertex3);
//----------------------------------------------------------------------------
EContainmentType SphereContainsBox(const FSphere& sphere, const BoundingBox& box);
//----------------------------------------------------------------------------
EContainmentType SphereContainsSphere(const FSphere& sphere1, const FSphere& sphere2);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Collision
} //!namespace Core
