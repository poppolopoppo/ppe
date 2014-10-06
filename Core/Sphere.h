#pragma once

#include "Core.h"

#include "Collision.h"
#include "ScalarVector.h"
#include "ScalarVectorHelpers.h"

namespace Core {
template <typename T>
class MemoryView;

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class Sphere {
public:
    Sphere(const float3& center, float radius);

    Sphere(const Sphere& other);
    Sphere& operator =(const Sphere& other);

    float3& Center() { return _center; }
    const float3& Center() const { return _center; }

    float& Radius() { return _radius; }
    float Radius() const { return _radius; }

    bool Intersects(const Ray& ray) const;
    bool Intersects(const Ray& ray, float& distance) const;
    bool Intersects(const Ray& ray, float3& point) const;
    PlaneIntersectionType Intersects(const Plane& plane) const;
    bool Intersects(const float3& triangle1, const float3& triangle2, const float3& triangle3) const;
    bool Intersects(const Sphere& sphere) const;

    ContainmentType Contains(const float3& point) const;
    ContainmentType Contains(const float3& triangle1, const float3& triangle2, const float3& triangle3) const;
    ContainmentType Contains(const BoundingBox& box) const;
    ContainmentType Contains(const Sphere& sphere) const;

    static Sphere FromSegment(const float3& a, const float3& b);
    static Sphere FromPoints(const MemoryView<const float3>& points);
    static Sphere FromBox(const BoundingBox& box);

    static Sphere Merge(const Sphere& lhs, const Sphere& rhs);

private:
    float3 _center;
    float _radius;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Sphere-inl.h"
