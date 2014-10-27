#pragma once

#include "Core/Core.h"

#include "Core/Maths/Geometry/ScalarBoundingBox_fwd.h"
#include "Core/Maths/Geometry/ScalarVector_fwd.h"

namespace Core {
class Plane;
class Ray;
class Sphere;

enum class ContainmentType {
    Contains = 0,
    Disjoint,
    Intersects
};

enum class PlaneIntersectionType {
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
float3 ClosestPointPlanePoint(const Plane& plane, const float3& point);
//----------------------------------------------------------------------------
float3 ClosestPointBoxPoint(const BoundingBox& box, const float3& point);
//----------------------------------------------------------------------------
float3 ClosestPointSpherePoint(const Sphere& sphere, const float3& point);
//----------------------------------------------------------------------------
float3 ClosestPointSphereSphere(const Sphere& sphere1, const Sphere& sphere2);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
float DistancePlanePoint(const Plane& plane, const float3& point);
//----------------------------------------------------------------------------
float DistanceBoxPoint(const BoundingBox& box, const float3& point);
//----------------------------------------------------------------------------
float DistanceBoxBox(const BoundingBox& box1, const BoundingBox& box2);
//----------------------------------------------------------------------------
float DistanceSpherePoint(const Sphere& sphere, const float3& point);
//----------------------------------------------------------------------------
float DistanceSphereSphere(const Sphere& sphere1, const Sphere& sphere2);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool RayIntersectsPoint(const Ray& ray, const float3& point);
//----------------------------------------------------------------------------
bool RayIntersectsRay(const Ray& ray1, const Ray& ray2, float3& point);
//----------------------------------------------------------------------------
bool RayIntersectsPlane(const Ray& ray, const Plane& plane, float& distance);
//----------------------------------------------------------------------------
bool RayIntersectsPlane(const Ray& ray, const Plane& plane, float3& point);
//----------------------------------------------------------------------------
bool RayIntersectsTriangle(const Ray& ray, const float3& vertex1, const float3& vertex2, const float3& vertex3, float& distance);
//----------------------------------------------------------------------------
bool RayIntersectsTriangle(const Ray& ray, const float3& vertex1, const float3& vertex2, const float3& vertex3, float3& point);
//----------------------------------------------------------------------------
bool RayIntersectsBox(const Ray& ray, const BoundingBox& box, float& distance);
//----------------------------------------------------------------------------
bool RayIntersectsBox(const Ray& ray, const BoundingBox& box, float3& point);
//----------------------------------------------------------------------------
bool RayIntersectsSphere(const Ray& ray, const Sphere& sphere, float& distance);
//----------------------------------------------------------------------------
bool RayIntersectsSphere(const Ray& ray, const Sphere& sphere, float3& point);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PlaneIntersectionType PlaneIntersectsPoint(const Plane& plane, const float3& point);
//----------------------------------------------------------------------------
bool PlaneIntersectsPlane(const Plane& plane1, const Plane& plane2);
//----------------------------------------------------------------------------
bool PlaneIntersectsPlane(const Plane& plane1, const Plane& plane2, Ray& line);
//----------------------------------------------------------------------------
PlaneIntersectionType PlaneIntersectsTriangle(const Plane& plane, const float3& vertex1, const float3& vertex2, const float3& vertex3);
//----------------------------------------------------------------------------
PlaneIntersectionType PlaneIntersectsBox(const Plane& plane, const BoundingBox& box);
//----------------------------------------------------------------------------
PlaneIntersectionType PlaneIntersectsSphere(const Plane& plane, const Sphere& sphere);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool BoxIntersectsTriangle(const BoundingBox& box, const float3& vertex1, const float3& vertex2, const float3& vertex3);
//----------------------------------------------------------------------------
bool BoxIntersectsBox(const BoundingBox& box1, const BoundingBox& box2);
//----------------------------------------------------------------------------
bool BoxIntersectsSphere(const BoundingBox& box, const Sphere& sphere);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool SphereIntersectsTriangle(const Sphere& sphere, const float3& vertex1, const float3& vertex2, const float3& vertex3);
//----------------------------------------------------------------------------
bool SphereIntersectsSphere(const Sphere& sphere1, const Sphere& sphere2);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ContainmentType BoxContainsPoint(const BoundingBox& box, const float3& point);
//----------------------------------------------------------------------------
ContainmentType BoxContainsTriangle(const BoundingBox& box, const float3& vertex1, const float3& vertex2, const float3& vertex3);
//----------------------------------------------------------------------------
ContainmentType BoxContainsBox(const BoundingBox& box1, const BoundingBox& box2);
//----------------------------------------------------------------------------
ContainmentType BoxContainsSphere(const BoundingBox& box, const Sphere& sphere);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ContainmentType SphereContainsPoint(const Sphere& sphere, const float3& point);
//----------------------------------------------------------------------------
ContainmentType SphereContainsTriangle(const Sphere& sphere, const float3& vertex1, const float3& vertex2, const float3& vertex3);
//----------------------------------------------------------------------------
ContainmentType SphereContainsBox(const Sphere& sphere, const BoundingBox& box);
//----------------------------------------------------------------------------
ContainmentType SphereContainsSphere(const Sphere& sphere1, const Sphere& sphere2);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Collision
} //!namespace Core
