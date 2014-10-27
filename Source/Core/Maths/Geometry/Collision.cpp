#include "stdafx.h"

#include "Collision.h"

#include "Plane.h"
#include "Ray.h"
#include "Sphere.h"

#include "ScalarBoundingBox.h"
#include "ScalarBoundingBoxHelpers.h"

#include "ScalarVector.h"
#include "ScalarVectorHelpers.h"

namespace Core {
namespace Collision {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
float3 ClosestPointPointTriangle(const float3& point, const float3& vertex1, const float3& vertex2, const float3& vertex3) {
    //Source: Real-Time Collision Detection by Christer Ericson
    //Reference: Page 136

    float3 result;

    //Check if P in vertex region outside A
    float3 ab = vertex2 - vertex1;
    float3 ac = vertex3 - vertex1;
    float3 ap = point - vertex1;

    float d1 = Dot3(ab, ap);
    float d2 = Dot3(ac, ap);
    if (d1 <= 0.0f && d2 <= 0.0f)
        result = vertex1; //Barycentric coordinates (1,0,0)

    //Check if P in vertex region outside B
    float3 bp = point - vertex2;
    float d3 = Dot3(ab, bp);
    float d4 = Dot3(ac, bp);
    if (d3 >= 0.0f && d4 <= d3)
        result = vertex2; // barycentric coordinates (0,1,0)

    //Check if P in edge region of AB, if so return projection of P onto AB
    float vc = d1 * d4 - d3 * d2;
    if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f)
    {
        float v = d1 / (d1 - d3);
        result = vertex1 + v * ab; //Barycentric coordinates (1-v,v,0)
    }

    //Check if P in vertex region outside C
    float3 cp = point - vertex3;
    float d5 = Dot3(ab, cp);
    float d6 = Dot3(ac, cp);
    if (d6 >= 0.0f && d5 <= d6)
        result = vertex3; //Barycentric coordinates (0,0,1)

    //Check if P in edge region of AC, if so return projection of P onto AC
    float vb = d5 * d2 - d1 * d6;
    if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f)
    {
        float w = d2 / (d2 - d6);
        result = vertex1 + w * ac; //Barycentric coordinates (1-w,0,w)
    }

    //Check if P in edge region of BC, if so return projection of P onto BC
    float va = d3 * d6 - d5 * d4;
    if (va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f)
    {
        float w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
        result = vertex2 + w * (vertex3 - vertex2); //Barycentric coordinates (0,1-w,w)
    }

    //P inside face region. Compute Q through its barycentric coordinates (u,v,w)
    float denom = 1.0f / (va + vb + vc);
    float v2 = vb * denom;
    float w2 = vc * denom;
    result = vertex1 + ab * v2 + ac * w2; //= u*vertex1 + v*vertex2 + w*vertex3, u = va * denom = 1.0f - v - w

    return result;
}
//----------------------------------------------------------------------------
float3 ClosestPointPlanePoint(const Plane& plane, const float3& point) {
    //Source: Real-Time Collision Detection by Christer Ericson
    //Reference: Page 126

    float dot = Dot3(plane.Normal(), point);
    float t = dot - plane.D();

    return point - (t * plane.Normal());
}
//----------------------------------------------------------------------------
float3 ClosestPointBoxPoint(const AABB3f& box, const float3& point) {
    //Source: Real-Time Collision Detection by Christer Ericson
    //Reference: Page 130

    float3 temp = Max(point, box.Min());
    return Min(temp, box.Max());
}
//----------------------------------------------------------------------------
float3 ClosestPointSpherePoint(const Sphere& sphere, const float3& point) {
    //Source: Jorgy343
    //Reference: None

    //Get the unit direction from the sphere's center to the point.
    float3 result = Normalize3(point - sphere.Center());

    //Multiply the unit direction by the sphere's radius to get a vector
    //the length of the sphere.
    result *= sphere.Radius();

    //Add the sphere's center to the direction to get a point on the sphere.
    result += sphere.Center();

    return result;
}
//----------------------------------------------------------------------------
float3 ClosestPointSphereSphere(const Sphere& sphere1, const Sphere& sphere2) {
    //Source: Jorgy343
    //Reference: None

    //Get the unit direction from the first sphere's center to the second sphere's center.
    float3 result = Normalize3(sphere2.Center() - sphere1.Center());

    //Multiply the unit direction by the first sphere's radius to get a vector
    //the length of the first sphere.
    result *= sphere1.Radius();

    //Add the first sphere's center to the direction to get a point on the first sphere.
    result += sphere1.Center();

    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
float DistancePlanePoint(const Plane& plane, const float3& point) {
    //Source: Real-Time Collision Detection by Christer Ericson
    //Reference: Page 127

    float dot = Dot3(plane.Normal(), point);
    return dot - plane.D();
}
//----------------------------------------------------------------------------
float DistanceBoxPoint(const AABB3f& box, const float3& point) {
    //Source: Real-Time Collision Detection by Christer Ericson
    //Reference: Page 131

    float distance = 0.0f;

    if (point.x() < box.Min().x())
        distance += (box.Min().x() - point.x()) * (box.Min().x() - point.x());
    if (point.x() > box.Max().x())
        distance += (point.x() - box.Max().x()) * (point.x() - box.Max().x());

    if (point.y() < box.Min().y())
        distance += (box.Min().y() - point.y()) * (box.Min().y() - point.y());
    if (point.y() > box.Max().y())
        distance += (point.y() - box.Max().y()) * (point.y() - box.Max().y());

    if (point.z() < box.Min().z())
        distance += (box.Min().z() - point.z()) * (box.Min().z() - point.z());
    if (point.z() > box.Max().z())
        distance += (point.z() - box.Max().z()) * (point.z() - box.Max().z());

    return std::sqrt(distance);
}
//----------------------------------------------------------------------------
float DistanceBoxBox(const AABB3f& box1, const AABB3f& box2) {
    //Source:
    //Reference:

    float distance = 0.0f;

    //Distance for X.
    if (box1.Min().x() > box2.Max().x())
    {
        float delta = box2.Max().x() - box1.Min().x();
        distance += delta * delta;
    }
    else if (box2.Min().x() > box1.Max().x())
    {
        float delta = box1.Max().x() - box2.Min().x();
        distance += delta * delta;
    }

    //Distance for Y.
    if (box1.Min().y() > box2.Max().y())
    {
        float delta = box2.Max().y() - box1.Min().y();
        distance += delta * delta;
    }
    else if (box2.Min().y() > box1.Max().y())
    {
        float delta = box1.Max().y() - box2.Min().y();
        distance += delta * delta;
    }

    //Distance for Z.
    if (box1.Min().z() > box2.Max().z())
    {
        float delta = box2.Max().z() - box1.Min().z();
        distance += delta * delta;
    }
    else if (box2.Min().z() > box1.Max().z())
    {
        float delta = box1.Max().z() - box2.Min().z();
        distance += delta * delta;
    }

    return std::sqrt(distance);
}
//----------------------------------------------------------------------------
float DistanceSpherePoint(const Sphere& sphere, const float3& point) {
    //Source: Jorgy343
    //Reference: None

    float distance = Distance3(sphere.Center(), point);
    distance -= sphere.Radius();

    return Max(distance, 0.0f);
}
//----------------------------------------------------------------------------
float DistanceSphereSphere(const Sphere& sphere1, const Sphere& sphere2) {
    //Source: Jorgy343
    //Reference: None

    float distance = Distance3(sphere1.Center(), sphere2.Center());
    distance -= sphere1.Radius() + sphere2.Radius();

    return Max(distance, 0.0f);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool RayIntersectsPoint(const Ray& ray, const float3& point) {
    //Source: RayIntersectsSphere
    //Reference: None

    float3 m = ray.Origin() - point;

    //Same thing as RayIntersectsSphere except that the radius of the sphere (point)
    //is the epsilon for zero.
    float b = Dot3(m, ray.Direction());
    float c = Dot3(m, m) - F_Epsilon;

    if (c > 0.0f && b > 0.0f)
        return false;

    float discriminant = b * b - c;

    if (discriminant < 0.0f)
        return false;

    return true;
}
//----------------------------------------------------------------------------
bool RayIntersectsRay(const Ray& ray1, const Ray& ray2, float3& point) {
    //Source: Real-Time Rendering, Third Edition
    //Reference: Page 780

    float3 cross = Cross(ray1.Direction(), ray2.Direction());
    float denominator = Length3(cross);

    //Lines are parallel.
    if (std::abs(denominator) < F_Epsilon)
    {
        //Lines are parallel and on top of each other.
        if (std::abs(ray2.Origin().x() - ray1.Origin().x()) < F_Epsilon &&
            std::abs(ray2.Origin().y() - ray1.Origin().y()) < F_Epsilon &&
            std::abs(ray2.Origin().z() - ray1.Origin().z()) < F_Epsilon)
        {
            point = float3::Zero();
            return true;
        }
    }

    denominator = denominator * denominator;

    //3x3 matrix for the first ray.
    float m11 = ray2.Origin().x() - ray1.Origin().x();
    float m12 = ray2.Origin().y() - ray1.Origin().y();
    float m13 = ray2.Origin().z() - ray1.Origin().z();
    float m21 = ray2.Direction().x();
    float m22 = ray2.Direction().y();
    float m23 = ray2.Direction().z();
    float m31 = cross.x();
    float m32 = cross.y();
    float m33 = cross.z();

    //Determinant of first matrix.
    float dets =
        m11 * m22 * m33 +
        m12 * m23 * m31 +
        m13 * m21 * m32 -
        m11 * m23 * m32 -
        m12 * m21 * m33 -
        m13 * m22 * m31;

    //3x3 matrix for the second ray.
    m21 = ray1.Direction().x();
    m22 = ray1.Direction().y();
    m23 = ray1.Direction().z();

    //Determinant of the second matrix.
    float dett =
        m11 * m22 * m33 +
        m12 * m23 * m31 +
        m13 * m21 * m32 -
        m11 * m23 * m32 -
        m12 * m21 * m33 -
        m13 * m22 * m31;

    //t values of the point of intersection.
    float s = dets / denominator;
    float t = dett / denominator;

    //The points of intersection.
    float3 point1 = ray1.Origin() + (s * ray1.Direction());
    float3 point2 = ray2.Origin() + (t * ray2.Direction());

    //If the points are not equal, no intersection has occured.
    if (std::abs(point2.x() - point1.x()) > F_Epsilon ||
        std::abs(point2.y() - point1.y()) > F_Epsilon ||
        std::abs(point2.z() - point1.z()) > F_Epsilon)
    {
        point = float3::Zero();
        return false;
    }

    point = point1;
    return true;
}
//----------------------------------------------------------------------------
bool RayIntersectsPlane(const Ray& ray, const Plane& plane, float& distance) {
    //Source: Real-Time Collision Detection by Christer Ericson
    //Reference: Page 175

    float direction = Dot3(plane.Normal(), ray.Direction());

    if (std::abs(direction) < F_Epsilon)
    {
        distance = 0.0f;
        return false;
    }

    float position = Dot3(plane.Normal(), ray.Origin());
    distance = (plane.D() - position) / direction;

    if (distance < 0.0f)
    {
        if (distance < -F_Epsilon)
        {
            distance = 0;
            return false;
        }

        distance = 0.0f;
    }

    return true;
}
//----------------------------------------------------------------------------
bool RayIntersectsPlane(const Ray& ray, const Plane& plane, float3& point) {
    //Source: Real-Time Collision Detection by Christer Ericson
    //Reference: Page 175

    float distance;
    if (!RayIntersectsPlane(ray, plane, distance))
    {
        point = float3::Zero();
        return false;
    }

    point = ray.Origin() + (ray.Direction() * distance);
    return true;
}
//----------------------------------------------------------------------------
bool RayIntersectsTriangle(const Ray& ray, const float3& vertex1, const float3& vertex2, const float3& vertex3, float& distance) {
    //Source: Fast Minimum Storage Ray / Triangle Intersection
    //Reference: http://www.cs.virginia.edu/~gfx/Courses/2003/ImageSynthesis/papers/Acceleration/Fast%20MinimumStorage%20RayTriangle%20Intersection.pdf

    //Compute vectors along two edges of the triangle.
    float3 edge1, edge2;

    //Edge 1
    edge1.x() = vertex2.x() - vertex1.x();
    edge1.y() = vertex2.y() - vertex1.y();
    edge1.z() = vertex2.z() - vertex1.z();

    //Edge2
    edge2.x() = vertex3.x() - vertex1.x();
    edge2.y() = vertex3.y() - vertex1.y();
    edge2.z() = vertex3.z() - vertex1.z();

    //Cross product of ray direction and edge2 - first part of determinant.
    float3 directioncrossedge2;
    directioncrossedge2.x() = (ray.Direction().y() * edge2.z()) - (ray.Direction().z() * edge2.y());
    directioncrossedge2.y() = (ray.Direction().z() * edge2.x()) - (ray.Direction().x() * edge2.z());
    directioncrossedge2.z() = (ray.Direction().x() * edge2.y()) - (ray.Direction().y() * edge2.x());

    //Compute the determinant.
    float determinant;
    //Dot product of edge1 and the first part of determinant.
    determinant = (edge1.x() * directioncrossedge2.x()) + (edge1.y() * directioncrossedge2.y()) + (edge1.z() * directioncrossedge2.z());

    //If the ray is parallel to the triangle plane, there is no collision.
    //This also means that we are not culling, the ray may hit both the
    //back and the front of the triangle.
    if (determinant > -F_Epsilon && determinant < F_Epsilon)
    {
        distance = 0.0f;
        return false;
    }

    float inversedeterminant = 1.0f / determinant;

    //Calculate the U parameter of the intersection point.
    float3 distanceVector;
    distanceVector.x() = ray.Origin().x() - vertex1.x();
    distanceVector.y() = ray.Origin().y() - vertex1.y();
    distanceVector.z() = ray.Origin().z() - vertex1.z();

    float triangleU;
    triangleU = (distanceVector.x() * directioncrossedge2.x()) + (distanceVector.y() * directioncrossedge2.y()) + (distanceVector.z() * directioncrossedge2.z());
    triangleU *= inversedeterminant;

    //Make sure it is inside the triangle.
    if (triangleU < 0.0f || triangleU > 1.0f)
    {
        distance = 0.0f;
        return false;
    }

    //Calculate the V parameter of the intersection point.
    float3 distancecrossedge1;
    distancecrossedge1.x() = (distanceVector.y() * edge1.z()) - (distanceVector.z() * edge1.y());
    distancecrossedge1.y() = (distanceVector.z() * edge1.x()) - (distanceVector.x() * edge1.z());
    distancecrossedge1.z() = (distanceVector.x() * edge1.y()) - (distanceVector.y() * edge1.x());

    float triangleV;
    triangleV = ((ray.Direction().x() * distancecrossedge1.x()) + (ray.Direction().y() * distancecrossedge1.y())) + (ray.Direction().z() * distancecrossedge1.z());
    triangleV *= inversedeterminant;

    //Make sure it is inside the triangle.
    if (triangleV < 0.0f || triangleU + triangleV > 1.0f)
    {
        distance = 0.0f;
        return false;
    }

    //Compute the distance along the ray to the triangle.
    float raydistance;
    raydistance = (edge2.x() * distancecrossedge1.x()) + (edge2.y() * distancecrossedge1.y()) + (edge2.z() * distancecrossedge1.z());
    raydistance *= inversedeterminant;

    //Is the triangle behind the ray origin?
    if (raydistance < 0.0f)
    {
        distance = 0.0f;
        return false;
    }

    distance = raydistance;
    return true;
}
//----------------------------------------------------------------------------
bool RayIntersectsTriangle(const Ray& ray, const float3& vertex1, const float3& vertex2, const float3& vertex3, float3& point) {
    float distance;
    if (!RayIntersectsTriangle(ray, vertex1, vertex2, vertex3, distance))
    {
        point = float3::Zero();
        return false;
    }

    point = ray.Origin() + (ray.Direction() * distance);
    return true;
}
//----------------------------------------------------------------------------
bool RayIntersectsBox(const Ray& ray, const AABB3f& box, float& distance) {
    //Source: Real-Time Collision Detection by Christer Ericson
    //Reference: Page 179

    distance = 0.0f;
    float tmax = FLT_MAX;

    if (std::abs(ray.Direction().x()) < F_Epsilon)
    {
        if (ray.Origin().x() < box.Min().x() || ray.Origin().x() > box.Max().x())
        {
            distance = 0.0f;
            return false;
        }
    }
    else
    {
        float inverse = 1.0f / ray.Direction().x();
        float t1 = (box.Min().x() - ray.Origin().x()) * inverse;
        float t2 = (box.Max().x() - ray.Origin().x()) * inverse;

        if (t1 > t2)
        {
            float temp = t1;
            t1 = t2;
            t2 = temp;
        }

        distance = Max(t1, distance);
        tmax = Min(t2, tmax);

        if (distance > tmax)
        {
            distance = 0.0f;
            return false;
        }
    }

    if (std::abs(ray.Direction().y()) < F_Epsilon)
    {
        if (ray.Origin().y() < box.Min().y() || ray.Origin().y() > box.Max().y())
        {
            distance = 0.0f;
            return false;
        }
    }
    else
    {
        float inverse = 1.0f / ray.Direction().y();
        float t1 = (box.Min().y() - ray.Origin().y()) * inverse;
        float t2 = (box.Max().y() - ray.Origin().y()) * inverse;

        if (t1 > t2)
        {
            float temp = t1;
            t1 = t2;
            t2 = temp;
        }

        distance = Max(t1, distance);
        tmax = Min(t2, tmax);

        if (distance > tmax)
        {
            distance = 0.0f;
            return false;
        }
    }

    if (std::abs(ray.Direction().z()) < F_Epsilon)
    {
        if (ray.Origin().z() < box.Min().z() || ray.Origin().z() > box.Max().z())
        {
            distance = 0.0f;
            return false;
        }
    }
    else
    {
        float inverse = 1.0f / ray.Direction().z();
        float t1 = (box.Min().z() - ray.Origin().z()) * inverse;
        float t2 = (box.Max().z() - ray.Origin().z()) * inverse;

        if (t1 > t2)
        {
            float temp = t1;
            t1 = t2;
            t2 = temp;
        }

        distance = Max(t1, distance);
        tmax = Min(t2, tmax);

        if (distance > tmax)
        {
            distance = 0.0f;
            return false;
        }
    }

    return true;
}
//----------------------------------------------------------------------------
bool RayIntersectsBox(const Ray& ray, const AABB3f& box, float3& point) {
    float distance;
    if (!RayIntersectsBox(ray, box, distance))
    {
        point = float3::Zero();
        return false;
    }

    point = ray.Origin() + (ray.Direction() * distance);
    return true;
}
//----------------------------------------------------------------------------
bool RayIntersectsSphere(const Ray& ray, const Sphere& sphere, float& distance) {
    //Source: Real-Time Collision Detection by Christer Ericson
    //Reference: Page 177

    float3 m = ray.Origin() - sphere.Center();

    float b = Dot3(m, ray.Direction());
    float c = Dot3(m, m) - (sphere.Radius() * sphere.Radius());

    if (c > 0.0f && b > 0.0f)
    {
        distance = 0.0f;
        return false;
    }

    float discriminant = b * b - c;

    if (discriminant < 0.0f)
    {
        distance = 0.0f;
        return false;
    }

    distance = -b - std::sqrt(discriminant);

    if (distance < 0.0f)
        distance = 0.0f;

    return true;
}
//----------------------------------------------------------------------------
bool RayIntersectsSphere(const Ray& ray, const Sphere& sphere, float3& point) {
    float distance;
    if (!RayIntersectsSphere(ray, sphere, distance))
    {
        point = float3::Zero();
        return false;
    }

    point = ray.Origin() + (ray.Direction() * distance);
    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PlaneIntersectionType PlaneIntersectsPoint(const Plane& plane, const float3& point) {
    float distance = Dot3(plane.Normal(), point);
    distance += plane.D();

    if (distance > 0.0f)
        return PlaneIntersectionType::Front;

    if (distance < 0.0f)
        return PlaneIntersectionType::Back;

    return PlaneIntersectionType::Intersecting;
}
//----------------------------------------------------------------------------
bool PlaneIntersectsPlane(const Plane& plane1, const Plane& plane2) {
    float3 direction = Cross(plane1.Normal(), plane2.Normal());

    //If direction is the zero vector, the planes are parallel and possibly
    //coincident. It is not an intersection. The dot product will tell us.
    float denominator = Dot3(direction, direction);

    if (std::abs(denominator) < F_Epsilon)
        return false;

    return true;
}
//----------------------------------------------------------------------------
bool PlaneIntersectsPlane(const Plane& plane1, const Plane& plane2, Ray& line) {
    //Source: Real-Time Collision Detection by Christer Ericson
    //Reference: Page 207

    float3 direction = Cross(plane1.Normal(), plane2.Normal());

    //If direction is the zero vector, the planes are parallel and possibly
    //coincident. It is not an intersection. The dot product will tell us.
    float denominator = Dot3(direction, direction);

    //We assume the planes are normalized, therefore the denominator
    //only serves as a parallel and coincident check. Otherwise we need
    //to deivide the point by the denominator.
    if (std::abs(denominator) < F_Epsilon)
    {
        return false;
    }

    float3 temp = plane1.D() * plane2.Normal() - plane2.D() * plane1.Normal();
    float3 point = Cross(temp, direction);

    line = Ray(point, Normalize3(direction));

    return true;
}
//----------------------------------------------------------------------------
PlaneIntersectionType PlaneIntersectsTriangle(const Plane& plane, const float3& vertex1, const float3& vertex2, const float3& vertex3) {
    //Source: Real-Time Collision Detection by Christer Ericson
    //Reference: Page 207

    PlaneIntersectionType test1 = PlaneIntersectsPoint(plane, vertex1);
    PlaneIntersectionType test2 = PlaneIntersectsPoint(plane, vertex2);
    PlaneIntersectionType test3 = PlaneIntersectsPoint(plane, vertex3);

    if (test1 == PlaneIntersectionType::Front && test2 == PlaneIntersectionType::Front && test3 == PlaneIntersectionType::Front)
        return PlaneIntersectionType::Front;

    if (test1 == PlaneIntersectionType::Back && test2 == PlaneIntersectionType::Back && test3 == PlaneIntersectionType::Back)
        return PlaneIntersectionType::Back;

    return PlaneIntersectionType::Intersecting;
}
//----------------------------------------------------------------------------
PlaneIntersectionType PlaneIntersectsBox(const Plane& plane, const AABB3f& box) {
    //Source: Real-Time Collision Detection by Christer Ericson
    //Reference: Page 161

    float3 min;
    float3 max;

    max.x() = (plane.Normal().x() >= 0.0f) ? box.Min().x() : box.Max().x();
    max.y() = (plane.Normal().y() >= 0.0f) ? box.Min().y() : box.Max().y();
    max.z() = (plane.Normal().z() >= 0.0f) ? box.Min().z() : box.Max().z();
    min.x() = (plane.Normal().x() >= 0.0f) ? box.Max().x() : box.Min().x();
    min.y() = (plane.Normal().y() >= 0.0f) ? box.Max().y() : box.Min().y();
    min.z() = (plane.Normal().z() >= 0.0f) ? box.Max().z() : box.Min().z();

    float distance = Dot3(plane.Normal(), max);

    if (distance + plane.D() > 0.0f)
        return PlaneIntersectionType::Front;

    distance = Dot3(plane.Normal(), min);

    if (distance + plane.D() < 0.0f)
        return PlaneIntersectionType::Back;

    return PlaneIntersectionType::Intersecting;
}
//----------------------------------------------------------------------------
PlaneIntersectionType PlaneIntersectsSphere(const Plane& plane, const Sphere& sphere) {
    //Source: Real-Time Collision Detection by Christer Ericson
    //Reference: Page 160

    float distance = Dot3(plane.Normal(), sphere.Center());
    distance += plane.D();

    if (distance > sphere.Radius())
        return PlaneIntersectionType::Front;

    if (distance < -sphere.Radius())
        return PlaneIntersectionType::Back;

    return PlaneIntersectionType::Intersecting;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool BoxIntersectsTriangle(const AABB3f& box, const float3& vertex1, const float3& vertex2, const float3& vertex3) {
    if (BoxContainsPoint(box, vertex1) == ContainmentType::Contains)
        return true;

    if (BoxContainsPoint(box, vertex2) == ContainmentType::Contains)
        return true;

    if (BoxContainsPoint(box, vertex3) == ContainmentType::Contains)
        return true;

    return false;
}
//----------------------------------------------------------------------------
bool BoxIntersectsBox(const AABB3f& box1, const AABB3f& box2) {
    if (box1.Min().x() > box2.Max().x() || box2.Min().x() > box1.Max().x())
        return false;

    if (box1.Min().y() > box2.Max().y() || box2.Min().y() > box1.Max().y())
        return false;

    if (box1.Min().z() > box2.Max().z() || box2.Min().z() > box1.Max().z())
        return false;

    return true;
}
//----------------------------------------------------------------------------
bool BoxIntersectsSphere(const AABB3f& box, const Sphere& sphere) {
    //Source: Real-Time Collision Detection by Christer Ericson
    //Reference: Page 166

    float3 vector = Clamp(sphere.Center(), box.Min(), box.Max());
    float distance = DistanceSq3(sphere.Center(), vector);

    return distance <= sphere.Radius() * sphere.Radius();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool SphereIntersectsTriangle(const Sphere& sphere, const float3& vertex1, const float3& vertex2, const float3& vertex3) {
    //Source: Real-Time Collision Detection by Christer Ericson
    //Reference: Page 167

    float3 point = ClosestPointPointTriangle(sphere.Center(), vertex1, vertex2, vertex3);
    float3 v = point - sphere.Center();

    float dot = Dot3(v, v);

    return dot <= sphere.Radius() * sphere.Radius();
}
//----------------------------------------------------------------------------
bool SphereIntersectsSphere(const Sphere& sphere1, const Sphere& sphere2) {
    float radiisum = sphere1.Radius() + sphere2.Radius();
    return DistanceSq3(sphere1.Center(), sphere2.Center()) <= radiisum * radiisum;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ContainmentType BoxContainsPoint(const AABB3f& box, const float3& point) {
    if (box.Min().x() <= point.x() && box.Max().x() >= point.x() &&
        box.Min().y() <= point.y() && box.Max().y() >= point.y() &&
        box.Min().z() <= point.z() && box.Max().z() >= point.z())
    {
        return ContainmentType::Contains;
    }

    return ContainmentType::Disjoint;
}
//----------------------------------------------------------------------------
ContainmentType BoxContainsTriangle(const AABB3f& box, const float3& vertex1, const float3& vertex2, const float3& vertex3) {
    ContainmentType test1 = BoxContainsPoint(box, vertex1);
    ContainmentType test2 = BoxContainsPoint(box, vertex2);
    ContainmentType test3 = BoxContainsPoint(box, vertex3);

    if (test1 == ContainmentType::Contains && test2 == ContainmentType::Contains && test3 == ContainmentType::Contains)
        return ContainmentType::Contains;

    if (test1 == ContainmentType::Contains || test2 == ContainmentType::Contains || test3 == ContainmentType::Contains)
        return ContainmentType::Intersects;

    return ContainmentType::Disjoint;
}
//----------------------------------------------------------------------------
ContainmentType BoxContainsBox(const AABB3f& box1, const AABB3f& box2) {
    if (box1.Max().x() < box2.Min().x() || box1.Min().x() > box2.Max().x())
        return ContainmentType::Disjoint;

    if (box1.Max().y() < box2.Min().y() || box1.Min().y() > box2.Max().y())
        return ContainmentType::Disjoint;

    if (box1.Max().z() < box2.Min().z() || box1.Min().z() > box2.Max().z())
        return ContainmentType::Disjoint;

    if (box1.Min().x() <= box2.Min().x() && (box2.Max().x() <= box1.Max().x() &&
        box1.Min().y() <= box2.Min().y() && box2.Max().y() <= box1.Max().y()) &&
        box1.Min().z() <= box2.Min().z() && box2.Max().z() <= box1.Max().z())
    {
        return ContainmentType::Contains;
    }

    return ContainmentType::Intersects;
}
//----------------------------------------------------------------------------
ContainmentType BoxContainsSphere(const AABB3f& box, const Sphere& sphere) {
    float3 vector = Clamp(sphere.Center(), box.Min(), box.Max());
    float distance = DistanceSq3(sphere.Center(), vector);

    if (distance > sphere.Radius() * sphere.Radius())
        return ContainmentType::Disjoint;

    if ((((box.Min().x() + sphere.Radius() <= sphere.Center().x()) && (sphere.Center().x() <= box.Max().x() - sphere.Radius())) && ((box.Max().x() - box.Min().x() > sphere.Radius()) &&
        (box.Min().y() + sphere.Radius() <= sphere.Center().y()))) && (((sphere.Center().y() <= box.Max().y() - sphere.Radius()) && (box.Max().y() - box.Min().y() > sphere.Radius())) &&
        (((box.Min().z() + sphere.Radius() <= sphere.Center().z()) && (sphere.Center().z() <= box.Max().z() - sphere.Radius())) && (box.Max().x() - box.Min().x() > sphere.Radius()))))
    {
        return ContainmentType::Contains;
    }

    return ContainmentType::Intersects;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ContainmentType SphereContainsPoint(const Sphere& sphere, const float3& point) {
    if (DistanceSq3(point, sphere.Center()) <= sphere.Radius() * sphere.Radius())
        return ContainmentType::Contains;

    return ContainmentType::Disjoint;
}
//----------------------------------------------------------------------------
ContainmentType SphereContainsTriangle(const Sphere& sphere, const float3& vertex1, const float3& vertex2, const float3& vertex3) {
    //Source: Jorgy343
    //Reference: None

    ContainmentType test1 = SphereContainsPoint(sphere, vertex1);
    ContainmentType test2 = SphereContainsPoint(sphere, vertex2);
    ContainmentType test3 = SphereContainsPoint(sphere, vertex3);

    if (test1 == ContainmentType::Contains && test2 == ContainmentType::Contains && test3 == ContainmentType::Contains)
        return ContainmentType::Contains;

    if (SphereIntersectsTriangle(sphere, vertex1, vertex2, vertex3))
        return ContainmentType::Intersects;

    return ContainmentType::Disjoint;
}
//----------------------------------------------------------------------------
ContainmentType SphereContainsBox(const Sphere& sphere, const AABB3f& box) {
    float3 vector;

    if (!BoxIntersectsSphere(box, sphere))
        return ContainmentType::Disjoint;

    float radiussquared = sphere.Radius() * sphere.Radius();
    vector.x() = sphere.Center().x() - box.Min().x();
    vector.y() = sphere.Center().y() - box.Max().y();
    vector.z() = sphere.Center().z() - box.Max().z();

    if (LengthSq3(vector) > radiussquared)
        return ContainmentType::Intersects;

    vector.x() = sphere.Center().x() - box.Max().x();
    vector.y() = sphere.Center().y() - box.Max().y();
    vector.z() = sphere.Center().z() - box.Max().z();

    if (LengthSq3(vector) > radiussquared)
        return ContainmentType::Intersects;

    vector.x() = sphere.Center().x() - box.Max().x();
    vector.y() = sphere.Center().y() - box.Min().y();
    vector.z() = sphere.Center().z() - box.Max().z();

    if (LengthSq3(vector) > radiussquared)
        return ContainmentType::Intersects;

    vector.x() = sphere.Center().x() - box.Min().x();
    vector.y() = sphere.Center().y() - box.Min().y();
    vector.z() = sphere.Center().z() - box.Max().z();

    if (LengthSq3(vector) > radiussquared)
        return ContainmentType::Intersects;

    vector.x() = sphere.Center().x() - box.Min().x();
    vector.y() = sphere.Center().y() - box.Max().y();
    vector.z() = sphere.Center().z() - box.Min().z();

    if (LengthSq3(vector) > radiussquared)
        return ContainmentType::Intersects;

    vector.x() = sphere.Center().x() - box.Max().x();
    vector.y() = sphere.Center().y() - box.Max().y();
    vector.z() = sphere.Center().z() - box.Min().z();

    if (LengthSq3(vector) > radiussquared)
        return ContainmentType::Intersects;

    vector.x() = sphere.Center().x() - box.Max().x();
    vector.y() = sphere.Center().y() - box.Min().y();
    vector.z() = sphere.Center().z() - box.Min().z();

    if (LengthSq3(vector) > radiussquared)
        return ContainmentType::Intersects;

    vector.x() = sphere.Center().x() - box.Min().x();
    vector.y() = sphere.Center().y() - box.Min().y();
    vector.z() = sphere.Center().z() - box.Min().z();

    if (LengthSq3(vector) > radiussquared)
        return ContainmentType::Intersects;

    return ContainmentType::Contains;
}
//----------------------------------------------------------------------------
ContainmentType SphereContainsSphere(const Sphere& sphere1, const Sphere& sphere2) {
    float distance = Distance3(sphere1.Center(), sphere2.Center());

    if (sphere1.Radius() + sphere2.Radius() < distance)
        return ContainmentType::Disjoint;

    if (sphere1.Radius() - sphere2.Radius() < distance)
        return ContainmentType::Intersects;

    return ContainmentType::Contains;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Collision
} //!namespace Core
