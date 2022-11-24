// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Maths/Collision.h"

#include "Maths/Plane.h"
#include "Maths/Ray.h"
#include "Maths/Sphere.h"

#include "Maths/ScalarBoundingBox.h"
#include "Maths/ScalarBoundingBoxHelpers.h"

#include "Maths/ScalarVector.h"
#include "Maths/ScalarVectorHelpers.h"

#include "Maths/MathHelpers.h"

namespace PPE {
namespace Collision {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
float3 ClosestPointOnTriangleToPoint(const float3& p, const float3& a, const float3& b, const float3& c) {
    //Source: Real-Time Collision Detection by Christer Ericson
    //Reference: Page 136

    // Check if P in vertex region outside A
    float3 ab = b - a;
    float3 ac = c - a;
    float3 ap = p - a;
    float d1 = Dot(ab, ap);
    float d2 = Dot(ac, ap);
    if (d1 <= 0.0f && d2 <= 0.0f) return a; // barycentric coordinates (1,0,0)

    // Check if P in vertex region outside B
    float3 bp = p - b;
    float d3 = Dot(ab, bp);
    float d4 = Dot(ac, bp);
    if (d3 >= 0.0f && d4 <= d3) return b; // barycentric coordinates (0,1,0)

    // Check if P in edge region of AB, if so return projection of P onto AB
    float vc = d1*d4 - d3*d2;
    if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f) {
        float v = d1 / (d1 - d3);
        return a + v * ab; // barycentric coordinates (1-v,v,0)
    }

    // Check if P in vertex region outside C
    float3 cp = p - c;
    float d5 = Dot(ab, cp);
    float d6 = Dot(ac, cp);
    if (d6 >= 0.0f && d5 <= d6) return c; // barycentric coordinates (0,0,1)

    // Check if P in edge region of AC, if so return projection of P onto AC
    float vb = d5*d2 - d1*d6;
    if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f) {
        float w = d2 / (d2 - d6);
        return a + w * ac; // barycentric coordinates (1-w,0,w)
    }

    // Check if P in edge region of BC, if so return projection of P onto BC
    float va = d3*d6 - d5*d4;
    if (va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f) {
        float w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
        return b + w * (c - b); // barycentric coordinates (0,1-w,w)
    }

    // P inside face region. Compute Q through its barycentric coordinates (u,v,w)
    float denom = 1.0f / (va + vb + vc);
    float v = vb * denom;
    float w = vc * denom;
    return a + ab * v + ac * w; // = u*a + v*b + w*c, u = va * denom = 1.0f-v-w
}
//----------------------------------------------------------------------------
float3 ClosestPointOnPlaneToPoint(const FPlane& plane, const float3& point) {
    //Source: Real-Time Collision Detection by Christer Ericson
    //Reference: Page 126

    float dot = Dot(plane.Normal(), point);
    float t = dot - plane.D();

    return point - (t * plane.Normal());
}
//----------------------------------------------------------------------------
float3 ClosestPointOnBoxToPoint(const FBoundingBox& box, const float3& point) {
    //Source: Real-Time Collision Detection by Christer Ericson
    //Reference: Page 130

    float3 temp = Max(point, box.Min());
    return Min(temp, box.Max());
}
//----------------------------------------------------------------------------
float3 ClosestPointOnSphereToPoint(const FSphere& sphere, const float3& point) {
    //Source: Jorgy343
    //Reference: None

    //Get the unit direction from the sphere's center to the point.
    float3 result = Normalize(point - sphere.Center());

    //Multiply the unit direction by the sphere's radius to get a vector
    //the length of the sphere.
    result *= sphere.Radius();

    //Add the sphere's center to the direction to get a point on the sphere.
    result += sphere.Center();

    return result;
}
//----------------------------------------------------------------------------
float3 ClosestPointOnSphereToSphere(const FSphere& sphere1, const FSphere& sphere2) {
    //Source: Jorgy343
    //Reference: None

    //Get the unit direction from the first sphere's center to the second sphere's center.
    float3 result = Normalize(sphere2.Center() - sphere1.Center());

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
float DistancePlanePoint(const FPlane& plane, const float3& point) {
    //Source: Real-Time Collision Detection by Christer Ericson
    //Reference: Page 127

    float dot = Dot(plane.Normal(), point);
    return dot + plane.D();
}
//----------------------------------------------------------------------------
float DistanceBoxPoint(const FBoundingBox& box, const float3& point) {
    //Source: Real-Time Collision Detection by Christer Ericson
    //Reference: Page 131

    float distance = 0.0f;

    if (point.x < box.Min().x)
        distance += (box.Min().x - point.x) * (box.Min().x - point.x);
    if (point.x > box.Max().x)
        distance += (point.x - box.Max().x) * (point.x - box.Max().x);

    if (point.y < box.Min().y)
        distance += (box.Min().y - point.y) * (box.Min().y - point.y);
    if (point.y > box.Max().y)
        distance += (point.y - box.Max().y) * (point.y - box.Max().y);

    if (point.z < box.Min().z)
        distance += (box.Min().z - point.z) * (box.Min().z - point.z);
    if (point.z > box.Max().z)
        distance += (point.z - box.Max().z) * (point.z - box.Max().z);

    return Sqrt(distance);
}
//----------------------------------------------------------------------------
float DistanceBoxBox(const FBoundingBox& box1, const FBoundingBox& box2) {
    //Source:
    //Reference:

    float distance = 0.0f;

    //Distance for X.
    if (box1.Min().x > box2.Max().x)
        distance += Sqr(box2.Max().x - box1.Min().x);
    else if (box2.Min().x > box1.Max().x)
        distance += Sqr(box1.Max().x - box2.Min().x);

    //Distance for Y.
    if (box1.Min().y > box2.Max().y)
        distance += Sqr(box2.Max().y - box1.Min().y);
    else if (box2.Min().y > box1.Max().y)
        distance += Sqr(box1.Max().y - box2.Min().y);

    //Distance for Z.
    if (box1.Min().z > box2.Max().z)
        distance += Sqr(box2.Max().z - box1.Min().z);
    else if (box2.Min().z > box1.Max().z)
        distance += Sqr(box1.Max().z - box2.Min().z);

    return Sqrt(distance);
}
//----------------------------------------------------------------------------
float DistanceSpherePoint(const FSphere& sphere, const float3& point) {
    //Source: Jorgy343
    //Reference: None

    float distance = Distance(sphere.Center(), point);
    distance -= sphere.Radius();

    return Max(distance, 0.0f);
}
//----------------------------------------------------------------------------
float DistanceSphereSphere(const FSphere& sphere1, const FSphere& sphere2) {
    //Source: Jorgy343
    //Reference: None

    float distance = Distance(sphere1.Center(), sphere2.Center());
    distance -= sphere1.Radius() + sphere2.Radius();

    return Max(distance, 0.0f);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
float DistanceLine(const float2& a, const float2& b, const float2& point) {
    const float twiceSignedArea = Cross(a, b, point);
    const float base = Length(a - b);
    return Abs(twiceSignedArea / (base * 2));
}
//----------------------------------------------------------------------------
float2 LineBisector(const float2& a, const float2& b, const float2& c) {
    const float2 d0 = Normalize(b - a);
    const float2 d1 = Normalize(c - b);

    const float d = Dot(d0, LineOrtho(d1));
    return (Abs(d) >= EpsilonSQ
        ? (d0 - d1) / d
        : LineOrtho(d0) );
}
//----------------------------------------------------------------------------
bool LineIntersectsLine(const float2& a0, const float2& b0, const float2& a1, const float2& b1, float2* point) {
    const float2 slope0 = a0 - b0;
    const float2 slope1 = a1 - b1;

    const float d = Det(slope0, slope1);
    if (Abs(d) < EpsilonSQ)
        return false;

    *point = ( (slope1 * Det(a0, b0) - slope0 * Det(a1, b1)) / d );
    return true;
}
//----------------------------------------------------------------------------
float2 LineOrtho(const float2& dir) {
    return float2(-dir.y, dir.x);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// #TODO : http://www.iquilezles.org/www/articles/intersectors/intersectors.htm
//----------------------------------------------------------------------------
bool RayIntersectsPoint(const FRay& ray, const float3& point) {
    //Source: RayIntersectsSphere
    //Reference: None

    float3 m = ray.Origin() - point;

    //Same thing as RayIntersectsSphere except that the radius of the sphere (point)
    //is the epsilon for zero.
    float b = Dot(m, ray.Direction());
    float c = Dot(m, m) - Epsilon;

    if (c > 0.0f && b > 0.0f)
        return false;

    float discriminant = b * b - c;

    if (discriminant < 0.0f)
        return false;

    return true;
}
//----------------------------------------------------------------------------
bool RayIntersectsRay(const FRay& ray1, const FRay& ray2, float3* point) {
    //Source: Real-Time Rendering, Third Edition
    //Reference: Page 780

    float3 cross = Cross(ray1.Direction(), ray2.Direction());
    float denominator = Length(cross);

    //Lines are parallel.
    if (Abs(denominator) < Epsilon)
    {
        //Lines are parallel and on top of each other.
        if (Abs(ray2.Origin().x - ray1.Origin().x) < Epsilon &&
            Abs(ray2.Origin().y - ray1.Origin().y) < Epsilon &&
            Abs(ray2.Origin().z - ray1.Origin().z) < Epsilon)
        {
            *point = float3::Zero;
            return true;
        }
    }

    denominator = denominator * denominator;

    //3x3 matrix for the first ray.
    float m11 = ray2.Origin().x - ray1.Origin().x;
    float m12 = ray2.Origin().y - ray1.Origin().y;
    float m13 = ray2.Origin().z - ray1.Origin().z;
    float m21 = ray2.Direction().x;
    float m22 = ray2.Direction().y;
    float m23 = ray2.Direction().z;
    float m31 = cross.x;
    float m32 = cross.y;
    float m33 = cross.z;

    //Determinant of first matrix.
    float dets =
        m11 * m22 * m33 +
        m12 * m23 * m31 +
        m13 * m21 * m32 -
        m11 * m23 * m32 -
        m12 * m21 * m33 -
        m13 * m22 * m31;

    //3x3 matrix for the second ray.
    m21 = ray1.Direction().x;
    m22 = ray1.Direction().y;
    m23 = ray1.Direction().z;

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
    if (Abs(point2.x - point1.x) > Epsilon ||
        Abs(point2.y - point1.y) > Epsilon ||
        Abs(point2.z - point1.z) > Epsilon)
    {
        *point = float3::Zero;
        return false;
    }

    *point = point1;
    return true;
}
//----------------------------------------------------------------------------
bool RayIntersectsPlane(const FRay& ray, const FPlane& plane, float* distance) {
    //Source: Real-Time Collision Detection by Christer Ericson
    //Reference: Page 175

    float direction = Dot(plane.Normal(), ray.Direction());

    if (Abs(direction) < Epsilon)
    {
        *distance = 0.0f;
        return false;
    }

    float position = Dot(plane.Normal(), ray.Origin());
    float d = (plane.D() - position) / direction;

    if (d < 0.0f)
    {
        *distance = 0.0f;
        if (d < -Epsilon_v<float>)
            return false;
    }
    else
    {
        *distance = d;
    }

    return true;
}
//----------------------------------------------------------------------------
bool RayIntersectsPlane(const FRay& ray, const FPlane& plane, float3* point) {
    //Source: Real-Time Collision Detection by Christer Ericson
    //Reference: Page 175

    float distance;
    if (!RayIntersectsPlane(ray, plane, &distance))
    {
        *point = float3::Zero;
        return false;
    }

    *point = ray.Origin() + (ray.Direction() * distance);
    return true;
}
//----------------------------------------------------------------------------
bool RayIntersectsTriangle(const FRay& ray, const float3& vertex1, const float3& vertex2, const float3& vertex3, float* distance) {
    //Source: Fast Minimum Storage FRay / FTriangle Intersection
    //Reference: http://www.cs.virginia.edu/~gfx/Courses/2003/ImageSynthesis/papers/Acceleration/Fast%20MinimumStorage%20RayTriangle%20Intersection.pdf

    //Compute vectors along two edges of the triangle.
    float3 edge1, edge2;

    //Edge 1
    edge1.x = vertex2.x - vertex1.x;
    edge1.y = vertex2.y - vertex1.y;
    edge1.z = vertex2.z - vertex1.z;

    //Edge2
    edge2.x = vertex3.x - vertex1.x;
    edge2.y = vertex3.y - vertex1.y;
    edge2.z = vertex3.z - vertex1.z;

    //Cross product of ray direction and edge2 - first part of determinant.
    float3 directioncrossedge2;
    directioncrossedge2.x = (ray.Direction().y * edge2.z) - (ray.Direction().z * edge2.y);
    directioncrossedge2.y = (ray.Direction().z * edge2.x) - (ray.Direction().x * edge2.z);
    directioncrossedge2.z = (ray.Direction().x * edge2.y) - (ray.Direction().y * edge2.x);

    //Compute the determinant.
    float determinant;
    //Dot product of edge1 and the first part of determinant.
    determinant = (edge1.x * directioncrossedge2.x) + (edge1.y * directioncrossedge2.y) + (edge1.z * directioncrossedge2.z);

    //If the ray is parallel to the triangle plane, there is no collision.
    //This also means that we are not culling, the ray may hit both the
    //back and the front of the triangle.
    if (Abs(determinant) < Epsilon)
    {
        *distance = 0.0f;
        return false;
    }

    float inversedeterminant = 1.0f / determinant;

    //Calculate the U parameter of the intersection point.
    float3 distanceVector;
    distanceVector.x = ray.Origin().x - vertex1.x;
    distanceVector.y = ray.Origin().y - vertex1.y;
    distanceVector.z = ray.Origin().z - vertex1.z;

    float triangleU;
    triangleU = (distanceVector.x * directioncrossedge2.x) + (distanceVector.y * directioncrossedge2.y) + (distanceVector.z * directioncrossedge2.z);
    triangleU *= inversedeterminant;

    //Make sure it is inside the triangle.
    if (triangleU < 0.0f || triangleU > 1.0f)
    {
        *distance = 0.0f;
        return false;
    }

    //Calculate the V parameter of the intersection point.
    float3 distancecrossedge1;
    distancecrossedge1.x = (distanceVector.y * edge1.z) - (distanceVector.z * edge1.y);
    distancecrossedge1.y = (distanceVector.z * edge1.x) - (distanceVector.x * edge1.z);
    distancecrossedge1.z = (distanceVector.x * edge1.y) - (distanceVector.y * edge1.x);

    float triangleV;
    triangleV = ((ray.Direction().x * distancecrossedge1.x) + (ray.Direction().y * distancecrossedge1.y)) + (ray.Direction().z * distancecrossedge1.z);
    triangleV *= inversedeterminant;

    //Make sure it is inside the triangle.
    if (triangleV < 0.0f || triangleU + triangleV > 1.0f)
    {
        *distance = 0.0f;
        return false;
    }

    //Compute the distance along the ray to the triangle.
    float raydistance;
    raydistance = (edge2.x * distancecrossedge1.x) + (edge2.y * distancecrossedge1.y) + (edge2.z * distancecrossedge1.z);
    raydistance *= inversedeterminant;

    //Is the triangle behind the ray origin?
    if (raydistance < 0.0f)
    {
        *distance = 0.0f;
        return false;
    }

    *distance = raydistance;
    return true;
}
//----------------------------------------------------------------------------
bool RayIntersectsTriangle(const FRay& ray, const float3& vertex1, const float3& vertex2, const float3& vertex3, float3* point) {
    float distance;
    if (!RayIntersectsTriangle(ray, vertex1, vertex2, vertex3, &distance)) {
        *point = float3::Zero;
        return false;
    }

    *point = ray.Origin() + (ray.Direction() * distance);
    return true;
}
//----------------------------------------------------------------------------
bool RayIntersectsBox(const FRay& ray, const FBoundingBox& box, float* distance) {
    //Source: Real-Time Collision Detection by Christer Ericson
    //Reference: Page 179

    float d = 0.0f;
    float tmax = FLT_MAX;

    if (Abs(ray.Direction().x) < Epsilon)
    {
        if (ray.Origin().x < box.Min().x || ray.Origin().x > box.Max().x)
        {
            *distance = 0.0f;
            return false;
        }
    }
    else
    {
        float inverse = 1.0f / ray.Direction().x;
        float t1 = (box.Min().x - ray.Origin().x) * inverse;
        float t2 = (box.Max().x - ray.Origin().x) * inverse;

        if (t1 > t2)
            std::swap(t1, t2);

        d = Max(t1, d);
        tmax = Min(t2, tmax);

        if (d > tmax)
        {
            *distance = 0.0f;
            return false;
        }
    }

    if (Abs(ray.Direction().y) < Epsilon)
    {
        if (ray.Origin().y < box.Min().y || ray.Origin().y > box.Max().y)
        {
            *distance = 0.0f;
            return false;
        }
    }
    else
    {
        float inverse = 1.0f / ray.Direction().y;
        float t1 = (box.Min().y - ray.Origin().y) * inverse;
        float t2 = (box.Max().y - ray.Origin().y) * inverse;

        if (t1 > t2)
            std::swap(t1, t2);

        d = Max(t1, d);
        tmax = Min(t2, tmax);

        if (d > tmax)
        {
            *distance = 0.0f;
            return false;
        }
    }

    if (Abs(ray.Direction().z) < Epsilon)
    {
        if (ray.Origin().z < box.Min().z || ray.Origin().z > box.Max().z)
        {
            *distance = 0.0f;
            return false;
        }
    }
    else
    {
        float inverse = 1.0f / ray.Direction().z;
        float t1 = (box.Min().z - ray.Origin().z) * inverse;
        float t2 = (box.Max().z - ray.Origin().z) * inverse;

        if (t1 > t2)
            std::swap(t1, t2);

        d = Max(t1, d);
        tmax = Min(t2, tmax);

        if (d > tmax)
        {
            *distance = 0.0f;
            return false;
        }
    }

    *distance = d;
    return true;
}
//----------------------------------------------------------------------------
bool RayIntersectsBox(const FRay& ray, const FBoundingBox& box, float3* point) {
    float distance;
    if (!RayIntersectsBox(ray, box, &distance))
    {
        *point = float3::Zero;
        return false;
    }

    *point = ray.Origin() + (ray.Direction() * distance);
    return true;
}
//----------------------------------------------------------------------------
bool RayIntersectsSphere(const FRay& ray, const FSphere& sphere, float* distance) {
    //Source: Real-Time Collision Detection by Christer Ericson
    //Reference: Page 177

    float3 m = ray.Origin() - sphere.Center();

    float b = Dot(m, ray.Direction());
    float c = Dot(m, m) - (sphere.Radius() * sphere.Radius());

    if (c > 0.0f && b > 0.0f)
    {
        *distance = 0.0f;
        return false;
    }

    float discriminant = b * b - c;

    if (discriminant < 0.0f)
    {
        *distance = 0.0f;
        return false;
    }

    float d = -b - Sqrt(discriminant);

    if (d < 0.0f)
        d = 0.0f;

    *distance = d;
    return true;
}
//----------------------------------------------------------------------------
bool RayIntersectsSphere(const FRay& ray, const FSphere& sphere, float3* point) {
    float distance;
    if (!RayIntersectsSphere(ray, sphere, &distance))
    {
        *point = float3::Zero;
        return false;
    }

    *point = ray.Origin() + (ray.Direction() * distance);
    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
EPlaneIntersectionType PlaneIntersectsPoint(const FPlane& plane, const float3& point) {
    float distance = Dot(plane.Normal(), point);
    distance += plane.D();

    if (distance > 0.0f)
        return EPlaneIntersectionType::Front;

    if (distance < 0.0f)
        return EPlaneIntersectionType::Back;

    return EPlaneIntersectionType::Intersecting;
}
//----------------------------------------------------------------------------
bool PlaneIntersectsPlane(const FPlane& plane1, const FPlane& plane2) {
    float3 direction = Cross(plane1.Normal(), plane2.Normal());

    //If direction is the zero vector, the planes are parallel and possibly
    //coincident. It is not an intersection. The dot product will tell us.
    float denominator = Dot(direction, direction);

    if (Abs(denominator) < Epsilon)
        return false;

    return true;
}
//----------------------------------------------------------------------------
bool PlaneIntersectsPlane(const FPlane& plane1, const FPlane& plane2, FRay& line) {
    //Source: Real-Time Collision Detection by Christer Ericson
    //Reference: Page 207

    float3 direction = Cross(plane1.Normal(), plane2.Normal());

    //If direction is the zero vector, the planes are parallel and possibly
    //coincident. It is not an intersection. The dot product will tell us.
    float denominator = Dot(direction, direction);

    //We assume the planes are normalized, therefore the denominator
    //only serves as a parallel and coincident check. Otherwise we need
    //to deivide the point by the denominator.
    if (Abs(denominator) < Epsilon)
    {
        return false;
    }

    float3 temp = plane1.D() * plane2.Normal() - plane2.D() * plane1.Normal();
    float3 point = Cross(temp, direction);

    line = FRay(point, Normalize(direction));

    return true;
}
//----------------------------------------------------------------------------
EPlaneIntersectionType PlaneIntersectsTriangle(const FPlane& plane, const float3& vertex1, const float3& vertex2, const float3& vertex3) {
    //Source: Real-Time Collision Detection by Christer Ericson
    //Reference: Page 207

    EPlaneIntersectionType test1 = PlaneIntersectsPoint(plane, vertex1);
    EPlaneIntersectionType test2 = PlaneIntersectsPoint(plane, vertex2);
    EPlaneIntersectionType test3 = PlaneIntersectsPoint(plane, vertex3);

    if (test1 == EPlaneIntersectionType::Front && test2 == EPlaneIntersectionType::Front && test3 == EPlaneIntersectionType::Front)
        return EPlaneIntersectionType::Front;

    if (test1 == EPlaneIntersectionType::Back && test2 == EPlaneIntersectionType::Back && test3 == EPlaneIntersectionType::Back)
        return EPlaneIntersectionType::Back;

    return EPlaneIntersectionType::Intersecting;
}
//----------------------------------------------------------------------------
EPlaneIntersectionType PlaneIntersectsBox(const FPlane& plane, const FBoundingBox& box) {
    //Source: Real-Time Collision Detection by Christer Ericson
    //Reference: Page 161

    float3 min;
    float3 max;

    max.x = (plane.Normal().x >= 0.0f) ? box.Min().x : box.Max().x;
    max.y = (plane.Normal().y >= 0.0f) ? box.Min().y : box.Max().y;
    max.z = (plane.Normal().z >= 0.0f) ? box.Min().z : box.Max().z;
    min.x = (plane.Normal().x >= 0.0f) ? box.Max().x : box.Min().x;
    min.y = (plane.Normal().y >= 0.0f) ? box.Max().y : box.Min().y;
    min.z = (plane.Normal().z >= 0.0f) ? box.Max().z : box.Min().z;

    float distance = Dot(plane.Normal(), max);

    if (distance + plane.D() > 0.0f)
        return EPlaneIntersectionType::Front;

    distance = Dot(plane.Normal(), min);

    if (distance + plane.D() < 0.0f)
        return EPlaneIntersectionType::Back;

    return EPlaneIntersectionType::Intersecting;
}
//----------------------------------------------------------------------------
EPlaneIntersectionType PlaneIntersectsSphere(const FPlane& plane, const FSphere& sphere) {
    //Source: Real-Time Collision Detection by Christer Ericson
    //Reference: Page 160

    float distance = Dot(plane.Normal(), sphere.Center());
    distance += plane.D();

    if (distance > sphere.Radius())
        return EPlaneIntersectionType::Front;

    if (distance < -sphere.Radius())
        return EPlaneIntersectionType::Back;

    return EPlaneIntersectionType::Intersecting;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool BoxIntersectsTriangle(const FBoundingBox& box, const float3& vertex1, const float3& vertex2, const float3& vertex3) {
    //Source: Real-Time Collision Detection by Christer Ericson
    //Reference: Page 169

    const float3 boxcenter = box.Center();
    const float3 boxhalfsize = box.Max() - boxcenter;

    /*    use separating axis theorem to test overlap between triangle and box */
    /*    need to test for overlap in these directions: */
    /*    1) the {x,y,z}-directions (actually, since we use the AABB of the triangle */
    /*       we do not even need to test these) */
    /*    2) normal of the triangle */
    /*    3) crossproduct(edge from tri, {x,y,z}-directin) */
    /*       this gives 3x3=9 more tests */

    /* This is the fastest branch on Sun */
    /* move everything so that the boxcenter is in (0,0,0) */
    const float3 v0 = vertex1 - boxcenter;
    const float3 v1 = vertex2 - boxcenter;
    const float3 v2 = vertex3 - boxcenter;

    /* compute triangle edges */
    const float3 e0 = v1 - v0;      /* tri edge 0 */
    const float3 e1 = v2 - v1;      /* tri edge 1 */
    const float3 e2 = v0 - v2;      /* tri edge 2 */

    /*  test the 9 tests first (this was faster) */
    float rad;
    float min, max;
    float p0, p1, p2;
    float fex, fey, fez;

#define AXISTEST_X01(a, b, fa, fb)\
    p0 = a*v0.y - b*v0.z;\
    p2 = a*v2.y - b*v2.z;\
    if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;}\
    rad = fa * boxhalfsize.y + fb * boxhalfsize.z;\
    if(min>rad || max<-rad) return false;
#define AXISTEST_X2(a, b, fa, fb)\
    p0 = a*v0.y - b*v0.z;\
    p1 = a*v1.y - b*v1.z;\
    if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;}\
    rad = fa * boxhalfsize.y + fb * boxhalfsize.z;\
    if(min>rad || max<-rad) return false;

#define AXISTEST_Y02(a, b, fa, fb)\
    p0 = -a*v0.x + b*v0.z;\
    p2 = -a*v2.x + b*v2.z;\
    if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;}\
    rad = fa * boxhalfsize.x + fb * boxhalfsize.z;\
    if(min>rad || max<-rad) return false;
#define AXISTEST_Y1(a, b, fa, fb)\
    p0 = -a*v0.x + b*v0.z;\
    p1 = -a*v1.x + b*v1.z;\
    if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;}\
    rad = fa * boxhalfsize.x + fb * boxhalfsize.z;\
    if(min>rad || max<-rad) return false;

#define AXISTEST_Z12(a, b, fa, fb)\
    p1 = a*v1.x - b*v1.y;\
    p2 = a*v2.x - b*v2.y;\
    if(p2<p1) {min=p2; max=p1;} else {min=p1; max=p2;}\
    rad = fa * boxhalfsize.x + fb * boxhalfsize.y;\
    if(min>rad || max<-rad) return false;
#define AXISTEST_Z0(a, b, fa, fb)\
    p0 = a*v0.x - b*v0.y;\
    p1 = a*v1.x - b*v1.y;\
    if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;}\
    rad = fa * boxhalfsize.x + fb * boxhalfsize.y;\
    if(min>rad || max<-rad) return false;

    fex = Abs(e0.x);
    fey = Abs(e0.y);
    fez = Abs(e0.z);

    AXISTEST_X01(e0.z, e0.y, fez, fey);
    AXISTEST_Y02(e0.z, e0.x, fez, fex);
    AXISTEST_Z12(e0.y, e0.x, fey, fex);

    fex = Abs(e1.x);
    fey = Abs(e1.y);
    fez = Abs(e1.z);

    AXISTEST_X01(e1.z, e1.y, fez, fey);
    AXISTEST_Y02(e1.z, e1.x, fez, fex);
    AXISTEST_Z0(e1.y, e1.x, fey, fex);

    fex = Abs(e2.x);
    fey = Abs(e2.y);
    fez = Abs(e2.z);
    AXISTEST_X2(e2.z, e2.y, fez, fey);
    AXISTEST_Y1(e2.z, e2.x, fez, fex);
    AXISTEST_Z12(e2.y, e2.x, fey, fex);

#undef AXISTEST_X01
#undef AXISTEST_X2
#undef AXISTEST_Y02
#undef AXISTEST_Y1
#undef AXISTEST_Z12
#undef AXISTEST_Z0

    /*  first test overlap in the {x,y,z}-directions */
    /*  find min, max of the triangle each direction, and test for overlap in */
    /*  that direction -- this is equivalent to testing a minimal AABB around */
    /*  the triangle against the AABB */
    /* test in X-direction */
    min = Min3(v0.x, v1.x, v2.x);
    max = Max3(v0.x, v1.x, v2.x);
    if (min>boxhalfsize.x || max<-boxhalfsize.x) return false;

    /* test in Y-direction */
    min = Min3(v0.y, v1.y, v2.y);
    max = Max3(v0.y, v1.y, v2.y);
    if (min>boxhalfsize.y || max<-boxhalfsize.y) return false;

    /* test in Z-direction */
    min = Min3(v0.z, v1.z, v2.z);
    max = Max3(v0.z, v1.z, v2.z);
    if (min>boxhalfsize.z || max<-boxhalfsize.z) return false;

    /*  test if the box intersects the plane of the triangle */
    /*  compute plane equation of triangle: normal*x+d=0 */
    const float3 normal = Cross(e0, e1);

    const auto result = PlaneIntersectsBox(FPlane::Make(v0, normal), FBoundingBox(-boxhalfsize, boxhalfsize));
    return (result == EPlaneIntersectionType::Intersecting);
}
//----------------------------------------------------------------------------
bool BoxIntersectsBox(const FBoundingBox& box1, const FBoundingBox& box2) {
    if (box1.Min().x > box2.Max().x || box2.Min().x > box1.Max().x)
        return false;

    if (box1.Min().y > box2.Max().y || box2.Min().y > box1.Max().y)
        return false;

    if (box1.Min().z > box2.Max().z || box2.Min().z > box1.Max().z)
        return false;

    return true;
}
//----------------------------------------------------------------------------
bool BoxIntersectsSphere(const FBoundingBox& box, const FSphere& sphere) {
    //Source: Real-Time Collision Detection by Christer Ericson
    //Reference: Page 166

    float3 vector = Clamp(sphere.Center(), box.Min(), box.Max());
    float distance = DistanceSq(sphere.Center(), vector);

    return distance <= sphere.Radius() * sphere.Radius();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool SphereIntersectsTriangle(const FSphere& sphere, const float3& vertex1, const float3& vertex2, const float3& vertex3) {
    //Source: Real-Time Collision Detection by Christer Ericson
    //Reference: Page 167

    float3 point = ClosestPointOnTriangleToPoint(sphere.Center(), vertex1, vertex2, vertex3);
    float3 v = point - sphere.Center();

    float dot = Dot(v, v);

    return dot <= sphere.Radius() * sphere.Radius();
}
//----------------------------------------------------------------------------
bool SphereIntersectsSphere(const FSphere& sphere1, const FSphere& sphere2) {
    float radiisum = sphere1.Radius() + sphere2.Radius();
    return DistanceSq(sphere1.Center(), sphere2.Center()) <= radiisum * radiisum;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
EContainmentType BoxContainsPoint(const FBoundingBox& box, const float3& point) {
    if (box.Min().x <= point.x && box.Max().x >= point.x &&
        box.Min().y <= point.y && box.Max().y >= point.y &&
        box.Min().z <= point.z && box.Max().z >= point.z)
    {
        return EContainmentType::Contains;
    }

    return EContainmentType::Disjoint;
}
//----------------------------------------------------------------------------
EContainmentType BoxContainsTriangle(const FBoundingBox& box, const float3& vertex1, const float3& vertex2, const float3& vertex3) {
    return BoxContainsBox(box, { vertex1, vertex2, vertex3 });
}
//----------------------------------------------------------------------------
EContainmentType BoxContainsBox(const FBoundingBox& box1, const FBoundingBox& box2) {
    if (box1.Max().x < box2.Min().x || box1.Min().x > box2.Max().x)
        return EContainmentType::Disjoint;

    if (box1.Max().y < box2.Min().y || box1.Min().y > box2.Max().y)
        return EContainmentType::Disjoint;

    if (box1.Max().z < box2.Min().z || box1.Min().z > box2.Max().z)
        return EContainmentType::Disjoint;

    if (box1.Min().x <= box2.Min().x && (box2.Max().x <= box1.Max().x &&
        box1.Min().y <= box2.Min().y && box2.Max().y <= box1.Max().y) &&
        box1.Min().z <= box2.Min().z && box2.Max().z <= box1.Max().z)
    {
        return EContainmentType::Contains;
    }

    return EContainmentType::Intersects;
}
//----------------------------------------------------------------------------
EContainmentType BoxContainsSphere(const FBoundingBox& box, const FSphere& sphere) {
    float3 vector = Clamp(sphere.Center(), box.Min(), box.Max());
    float distance = DistanceSq(sphere.Center(), vector);

    if (distance > sphere.Radius() * sphere.Radius())
        return EContainmentType::Disjoint;

    if ((((box.Min().x + sphere.Radius() <= sphere.Center().x) && (sphere.Center().x <= box.Max().x - sphere.Radius())) && ((box.Max().x - box.Min().x > sphere.Radius()) &&
        (box.Min().y + sphere.Radius() <= sphere.Center().y))) && (((sphere.Center().y <= box.Max().y - sphere.Radius()) && (box.Max().y - box.Min().y > sphere.Radius())) &&
        (((box.Min().z + sphere.Radius() <= sphere.Center().z) && (sphere.Center().z <= box.Max().z - sphere.Radius())) && (box.Max().x - box.Min().x > sphere.Radius()))))
    {
        return EContainmentType::Contains;
    }

    return EContainmentType::Intersects;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
EContainmentType SphereContainsPoint(const FSphere& sphere, const float3& point) {
    if (DistanceSq(point, sphere.Center()) <= sphere.Radius() * sphere.Radius())
        return EContainmentType::Contains;

    return EContainmentType::Disjoint;
}
//----------------------------------------------------------------------------
EContainmentType SphereContainsTriangle(const FSphere& sphere, const float3& vertex1, const float3& vertex2, const float3& vertex3) {
    //Source: Jorgy343
    //Reference: None

    EContainmentType test1 = SphereContainsPoint(sphere, vertex1);
    EContainmentType test2 = SphereContainsPoint(sphere, vertex2);
    EContainmentType test3 = SphereContainsPoint(sphere, vertex3);

    if (test1 == EContainmentType::Contains && test2 == EContainmentType::Contains && test3 == EContainmentType::Contains)
        return EContainmentType::Contains;

    if (SphereIntersectsTriangle(sphere, vertex1, vertex2, vertex3))
        return EContainmentType::Intersects;

    return EContainmentType::Disjoint;
}
//----------------------------------------------------------------------------
EContainmentType SphereContainsBox(const FSphere& sphere, const FBoundingBox& box) {
    float3 vector;

    if (!BoxIntersectsSphere(box, sphere))
        return EContainmentType::Disjoint;

    float radiussquared = sphere.Radius() * sphere.Radius();
    vector.x = sphere.Center().x - box.Min().x;
    vector.y = sphere.Center().y - box.Max().y;
    vector.z = sphere.Center().z - box.Max().z;

    if (LengthSq(vector) > radiussquared)
        return EContainmentType::Intersects;

    vector.x = sphere.Center().x - box.Max().x;
    vector.y = sphere.Center().y - box.Max().y;
    vector.z = sphere.Center().z - box.Max().z;

    if (LengthSq(vector) > radiussquared)
        return EContainmentType::Intersects;

    vector.x = sphere.Center().x - box.Max().x;
    vector.y = sphere.Center().y - box.Min().y;
    vector.z = sphere.Center().z - box.Max().z;

    if (LengthSq(vector) > radiussquared)
        return EContainmentType::Intersects;

    vector.x = sphere.Center().x - box.Min().x;
    vector.y = sphere.Center().y - box.Min().y;
    vector.z = sphere.Center().z - box.Max().z;

    if (LengthSq(vector) > radiussquared)
        return EContainmentType::Intersects;

    vector.x = sphere.Center().x - box.Min().x;
    vector.y = sphere.Center().y - box.Max().y;
    vector.z = sphere.Center().z - box.Min().z;

    if (LengthSq(vector) > radiussquared)
        return EContainmentType::Intersects;

    vector.x = sphere.Center().x - box.Max().x;
    vector.y = sphere.Center().y - box.Max().y;
    vector.z = sphere.Center().z - box.Min().z;

    if (LengthSq(vector) > radiussquared)
        return EContainmentType::Intersects;

    vector.x = sphere.Center().x - box.Max().x;
    vector.y = sphere.Center().y - box.Min().y;
    vector.z = sphere.Center().z - box.Min().z;

    if (LengthSq(vector) > radiussquared)
        return EContainmentType::Intersects;

    vector.x = sphere.Center().x - box.Min().x;
    vector.y = sphere.Center().y - box.Min().y;
    vector.z = sphere.Center().z - box.Min().z;

    if (LengthSq(vector) > radiussquared)
        return EContainmentType::Intersects;

    return EContainmentType::Contains;
}
//----------------------------------------------------------------------------
EContainmentType SphereContainsSphere(const FSphere& sphere1, const FSphere& sphere2) {
    float distance = Distance(sphere1.Center(), sphere2.Center());

    if (sphere1.Radius() + sphere2.Radius() < distance)
        return EContainmentType::Disjoint;

    if (sphere1.Radius() - sphere2.Radius() < distance)
        return EContainmentType::Intersects;

    return EContainmentType::Contains;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Collision
} //!namespace PPE
