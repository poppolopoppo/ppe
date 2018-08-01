#pragma once

#include "Core.h"

#include "Maths/Collision.h"
#include "Maths/ScalarVector.h"
#include "Maths/ScalarVectorHelpers.h"
#include "Maths/ScalarMatrix_fwd.h"

namespace PPE {
template <typename T>
class TMemoryView;

class FQuaternion;

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FPlane {
public:
    FPlane();
    explicit FPlane(const float4& normalD);
    FPlane(const float3& normal, float d);
    FPlane(const float3& normal, const float3& point);

    FPlane(const FPlane& other);
    FPlane& operator =(const FPlane& other);

    float3& Normal() { return _normal; }
    const float3& Normal() const { return _normal; }

    float& D() { return _d; }
    float D() const { return _d; }

    float3 PointOnPlane() const;

    FPlane Normalize() const;

    float DistanceToPoint(const float3& point) const;
    EPlaneIntersectionType Intersects(const float3& point) const;

    bool Intersects(const FRay& ray) const;
    bool Intersects(const FRay& ray, float* distance) const;
    bool Intersects(const FRay& ray, float3* point) const;

    bool Intersects(const FPlane& plane) const;
    bool Intersects(const FPlane& plane, FRay* line) const;

    EPlaneIntersectionType Intersects(const float3& triangle1, const float3& triangle2, const float3& triangle3) const;
    EPlaneIntersectionType Intersects(const FBoundingBox& box) const;
    EPlaneIntersectionType Intersects(const FSphere& sphere) const;

    static FPlane Make(const float3& pos, const float3& normal);

    static FPlane FromTriangle(const float3& a, const float3& b, const float3& c);

    static FPlane Transform(const FPlane& plane, const FQuaternion& rotation);
    static void Transform(const TMemoryView<FPlane>& planes, const FQuaternion& rotation);

    static FPlane Transform(const FPlane& plane, const Matrix& transformation);
    static void Transform(const TMemoryView<FPlane>& planes, const Matrix& transformation);

    static float3 Get3PlanesInterPoint(const FPlane& p1, const FPlane& p2, const FPlane& p3);
    static EPlaneIntersectionType PointsIntersection(const FPlane& plane, const TMemoryView<const float3>& points);

private:
    float3 _normal;
    float _d;
};
//----------------------------------------------------------------------------
PPE_ASSUME_TYPE_AS_POD(FPlane)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#include "Maths/Plane-inl.h"
