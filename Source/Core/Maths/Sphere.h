#pragma once

#include "Core/Core.h"

#include "Core/Maths/Collision.h"
#include "Core/Maths/ScalarVector.h"
#include "Core/Maths/ScalarVectorHelpers.h"

namespace Core {
template <typename T>
class TMemoryView;

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FSphere {
public:
    FSphere(const float3& center, float radius);

    FSphere(const FSphere& other);
    FSphere& operator =(const FSphere& other);

    float3& Center() { return _center; }
    const float3& Center() const { return _center; }

    float& Radius() { return _radius; }
    float Radius() const { return _radius; }

    bool Intersects(const FRay& ray) const;
    bool Intersects(const FRay& ray, float* distance) const;
    bool Intersects(const FRay& ray, float3* point) const;
    EPlaneIntersectionType Intersects(const FPlane& plane) const;
    bool Intersects(const float3& triangle1, const float3& triangle2, const float3& triangle3) const;
    bool Intersects(const FSphere& sphere) const;

    EContainmentType Contains(const float3& point) const;
    EContainmentType Contains(const float3& triangle1, const float3& triangle2, const float3& triangle3) const;
    EContainmentType Contains(const FBoundingBox& box) const;
    EContainmentType Contains(const FSphere& sphere) const;

    FBoundingBox ToBox() const;

    static FSphere FromSegment(const float3& a, const float3& b);
    static FSphere FromPoints(const TMemoryView<const float3>& points);
    static FSphere FromBox(const FBoundingBox& box);

    static FSphere Merge(const FSphere& lhs, const FSphere& rhs);

private:
    float3 _center;
    float _radius;
};
//----------------------------------------------------------------------------
CORE_ASSUME_TYPE_AS_POD(FSphere)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Maths/Sphere-inl.h"
