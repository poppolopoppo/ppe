#include "stdafx.h"

#include "Sphere.h"

#include "Memory/MemoryView.h"

#include "ScalarBoundingBox.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FSphere FSphere::FromSegment(const float3& a, const float3& b) {
    Assert(LengthSq3(a - b) > F_Epsilon);

    const float3& center = a;
    const float radius = Length3(b - center);

    return FSphere(center, radius);
}
//----------------------------------------------------------------------------
FSphere FSphere::FromPoints(const TMemoryView<const float3>& points) {
    //Find the center of all points.
    float3 center(0);
    for (const float3& p : points)
        center += p;

    //This is the center of our sphere.
    center /= (float)points.size();

    //Find the radius of the sphere
    float radius = 0.0f;
    for (size_t i = 0; i < points.size(); ++i) {
        //We are doing a relative distance comparasin to find the maximum distance
        //from the center of our sphere.
        float distance = DistanceSq3(center, points[i]);

        if (distance > radius)
            radius = distance;
    }

    //Find the real distance from the DistanceSquared.
    radius = std::sqrt(radius);

    //Construct the sphere.
    return FSphere(center, radius);
}
//----------------------------------------------------------------------------
FSphere FSphere::FromBox(const BoundingBox& box) {
    float3 center = box.Center();

    float x = box.Min().x() - box.Max().x();
    float y = box.Min().y() - box.Max().y();
    float z = box.Min().z() - box.Max().z();

    float distance = std::sqrt((x * x) + (y * y) + (z * z));

    return FSphere(center, distance * 0.5f);
}
//----------------------------------------------------------------------------
FSphere FSphere::Merge(const FSphere& lhs, const FSphere& rhs) {
    float3 difference = lhs.Center() - rhs.Center();

    float length = Length3(difference);
    float radius = lhs.Radius();
    float radius2 = rhs.Radius();

    if (radius + radius2 >= length) {
        if (radius - radius2 >= length)
            return lhs;

        if (radius2 - radius >= length)
            return rhs;
    }

    float3 vector = difference * (1.0f / length);
    float min = Min(-radius, length - radius2);
    float max = (Max(radius, length + radius2) - min) * 0.5f;

    return FSphere(lhs.Center() + vector * (max + min), max);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
