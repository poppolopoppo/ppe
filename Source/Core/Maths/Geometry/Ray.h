#pragma once

#include "Core/Core.h"

#include "Core/Maths/Geometry/Collision.h"
#include "Core/Maths/Geometry/ScalarVector.h"
#include "Core/Maths/Geometry/ScalarVectorHelpers.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class Ray {
public:
    Ray(const float3& origin, const float3& direction);

    Ray(const Ray& other);
    Ray& operator =(const Ray& other);

    const float3& Origin() const { return _origin; }
    const float3& Direction() const { return _direction; }

    bool Intersects(const float3& point) const;

    bool Intersects(const Ray& ray) const;
    bool Intersects(const Ray& ray, float3& point) const;

    bool Intersects(const Plane& plane) const;
    bool Intersects(const Plane& plane, float& distance) const;
    bool Intersects(const Plane& plane, float3& point) const;

    bool Intersects(const float3& triangle1, const float3& triangle2, const float3& triangle3) const;
    bool Intersects(const float3& triangle1, const float3& triangle2, const float3& triangle3, float& distance) const;
    bool Intersects(const float3& triangle1, const float3& triangle2, const float3& triangle3, float3& point) const;

    bool Intersects(const BoundingBox& box) const;
    bool Intersects(const BoundingBox& box, float& distance) const;
    bool Intersects(const BoundingBox& box, float3& point) const;

    bool Intersects(const Sphere& sphere) const;
    bool Intersects(const Sphere& sphere, float& distance) const;
    bool Intersects(const Sphere& sphere, float3& point) const;

    static Ray FromSegment(const float3& a, const float3& b);

private:
    float3 _origin;
    float3 _direction;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Maths/Geometry/Ray-inl.h"
