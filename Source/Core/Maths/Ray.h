#pragma once

#include "Core/Core.h"

#include "Core/Maths/Collision.h"
#include "Core/Maths/ScalarVector.h"
#include "Core/Maths/ScalarVectorHelpers.h"

namespace Core {
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

    bool Intersects(const float3& point) const;

    bool Intersects(const FRay& ray) const;
    bool Intersects(const FRay& ray, float3& point) const;

    bool Intersects(const FPlane& plane) const;
    bool Intersects(const FPlane& plane, float& distance) const;
    bool Intersects(const FPlane& plane, float3& point) const;

    bool Intersects(const float3& triangle1, const float3& triangle2, const float3& triangle3) const;
    bool Intersects(const float3& triangle1, const float3& triangle2, const float3& triangle3, float& distance) const;
    bool Intersects(const float3& triangle1, const float3& triangle2, const float3& triangle3, float3& point) const;

    bool Intersects(const FBoundingBox& box) const;
    bool Intersects(const FBoundingBox& box, float& distance) const;
    bool Intersects(const FBoundingBox& box, float3& point) const;

    bool Intersects(const FSphere& sphere) const;
    bool Intersects(const FSphere& sphere, float& distance) const;
    bool Intersects(const FSphere& sphere, float3& point) const;

    static FRay FromSegment(const float3& a, const float3& b);

private:
    float3 _origin;
    float3 _direction;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Maths/Ray-inl.h"
