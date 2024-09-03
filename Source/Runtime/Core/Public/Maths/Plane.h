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

    NODISCARD float3& Normal() { return _normal; }
    NODISCARD const float3& Normal() const { return _normal; }

    NODISCARD float& D() { return _d; }
    NODISCARD float D() const { return _d; }

    NODISCARD float4 Vec() const {
        return { _normal, _d };
    }

    NODISCARD float3 PointOnPlane() const;

    NODISCARD FPlane Normalize() const;

    NODISCARD float DistanceToPoint(const float3& point) const;
    NODISCARD EPlaneIntersectionType Intersects(const float3& point) const;

    NODISCARD bool Intersects(const FRay& ray) const;
    NODISCARD bool Intersects(const FRay& ray, float* distance) const;
    NODISCARD bool Intersects(const FRay& ray, float3* point) const;

    NODISCARD bool Intersects(const FPlane& plane) const;
    NODISCARD bool Intersects(const FPlane& plane, FRay* line) const;

    NODISCARD EPlaneIntersectionType Intersects(const float3& triangle1, const float3& triangle2, const float3& triangle3) const;
    NODISCARD EPlaneIntersectionType Intersects(const FBoundingBox& box) const;
    NODISCARD EPlaneIntersectionType Intersects(const FSphere& sphere) const;

    NODISCARD static FPlane Make(const float3& pos, const float3& normal);

    NODISCARD static FPlane FromTriangle(const float3& a, const float3& b, const float3& c);

    NODISCARD static FPlane Transform(const FPlane& plane, const FQuaternion& rotation);
    static void Transform(const TMemoryView<FPlane>& planes, const FQuaternion& rotation);

    NODISCARD static FPlane Transform(const FPlane& plane, const FMatrix& transformation);
    static void Transform(const TMemoryView<FPlane>& planes, const FMatrix& transformation);

    NODISCARD static float3 Get3PlanesInterPoint(const FPlane& p1, const FPlane& p2, const FPlane& p3);
    NODISCARD static EPlaneIntersectionType PointsIntersection(const FPlane& plane, const TMemoryView<const float3>& points);

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
