#pragma once

#include "Core.h"

#include "Collision.h"
#include "ScalarVector.h"
#include "ScalarVectorHelpers.h"
#include "ScalarMatrix_fwd.h"

namespace Core {
template <typename T>
class MemoryView;

class Quaternion;

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class Plane {
public:
    Plane();
    explicit Plane(const float4& normalD);
    Plane(const float3& normal, float d);
    Plane(const float3& normal, const float3& point);

    Plane(const Plane& other);
    Plane& operator =(const Plane& other);

    float3& Normal() { return _normal; }
    const float3& Normal() const { return _normal; }

    float& D() { return _d; }
    float D() const { return _d; }

    Plane Normalize() const;

    PlaneIntersectionType Intersects(const float3& point) const;

    bool Intersects(const Ray& ray) const;
    bool Intersects(const Ray& ray, float& distance) const;
    bool Intersects(const Ray& ray, float3& point) const;

    bool Intersects(const Plane& plane) const;
    bool Intersects(const Plane& plane, Ray& line) const;

    PlaneIntersectionType Intersects(const float3& triangle1, const float3& triangle2, const float3& triangle3) const;
    PlaneIntersectionType Intersects(const BoundingBox& box) const;
    PlaneIntersectionType Intersects(const Sphere& sphere) const;

    static Plane FromTriangle(const float3& a, const float3& b, const float3& c);

    static Plane Transform(const Plane& plane, const Quaternion& rotation);
    static void Transform(const MemoryView<Plane>& planes, const Quaternion& rotation);

    static Plane Transform(const Plane& plane, const Matrix& transformation);
    static void Transform(const MemoryView<Plane>& planes, const Matrix& transformation);

    static float3 Get3PlanesInterPoint(const Plane& p1, const Plane& p2, const Plane& p3);
    static PlaneIntersectionType PointsIntersection(const Plane& plane, const MemoryView<const float3>& points);

private:
    float3 _normal;
    float _d;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Plane-inl.h"
