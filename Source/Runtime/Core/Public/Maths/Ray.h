#pragma once

#include "Core.h"

#include "Maths/Collision.h"
#include "Maths/ScalarVector.h"
#include "Maths/ScalarVectorHelpers.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FRay {
public:
    FRay(const float3& origin, const float3& direction);

    FRay(const FRay& other);
    FRay& operator =(const FRay& other);

    const float3& Origin() const { return _origin; }
    const float3& Direction() const { return _direction; }

    float3 At(float distance) const { return (_origin + _direction * distance); }

    bool Intersects(const float3& point) const;

    bool Intersects(const FRay& ray) const;
    bool Intersects(const FRay& ray, float3* point) const;

    bool Intersects(const FPlane& plane) const;
    bool Intersects(const FPlane& plane, float* distance) const;
    bool Intersects(const FPlane& plane, float3* point) const;

    bool Intersects(const float3& triangle1, const float3& triangle2, const float3& triangle3) const;
    bool Intersects(const float3& triangle1, const float3& triangle2, const float3& triangle3, float* distance) const;
    bool Intersects(const float3& triangle1, const float3& triangle2, const float3& triangle3, float3* point) const;

    bool Intersects(const FBoundingBox& box) const;
    bool Intersects(const FBoundingBox& box, float* distance) const;
    bool Intersects(const FBoundingBox& box, float3* point) const;

    bool Intersects(const FSphere& sphere) const;
    bool Intersects(const FSphere& sphere, float* distance) const;
    bool Intersects(const FSphere& sphere, float3* point) const;

    static FRay FromSegment(const float3& a, const float3& b);
    static FRay Reflect(const FRay& ray, float distance, const float3& normal);
    static FRay Reflect(const FRay& ray, const float3& point, const float3& normal);

private:
    float3 _origin;
    float3 _direction;
};
//----------------------------------------------------------------------------
PPE_ASSUME_TYPE_AS_POD(FRay)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#include "Maths/Ray-inl.h"
