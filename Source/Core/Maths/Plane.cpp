#include "stdafx.h"

#include "Plane.h"

#include "Maths/Quaternion.h"
#include "Maths/QuaternionHelpers.h"
#include "Maths/ScalarMatrix.h"
#include "Maths/ScalarMatrixHelpers.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FPlane FPlane::FromTriangle(const float3& a, const float3& b, const float3& c) {
    float x1 = b.x() - a.x();
    float y1 = b.y() - a.y();
    float z1 = b.z() - a.z();

    float x2 = c.x() - a.x();
    float y2 = c.y() - a.y();
    float z2 = c.z() - a.z();

    float yz = (y1 * z2) - (z1 * y2);
    float xz = (z1 * x2) - (x1 * z2);
    float xy = (x1 * y2) - (y1 * x2);

    float pyth = (yz * yz) + (xz * xz) + (xy * xy);
    Assert(std::abs(pyth) > F_EpsilonSQ);
    float invPyth = 1.0f / std::sqrt(pyth);

    float3 normal(yz * invPyth, xz * invPyth, xy * invPyth);
    float d = -((normal.x() * a.x()) + (normal.y() * a.y()) + (normal.z() * a.z()));

    return FPlane(normal, d);
}
//----------------------------------------------------------------------------
FPlane FPlane::Transform(const FPlane& plane, const FQuaternion& rotation) {
    float x2 = rotation.x() + rotation.x();
    float y2 = rotation.y() + rotation.y();
    float z2 = rotation.z() + rotation.z();

    float wx = rotation.w() * x2;
    float wy = rotation.w() * y2;
    float wz = rotation.w() * z2;

    float xx = rotation.x() * x2;
    float xy = rotation.x() * y2;
    float xz = rotation.x() * z2;

    float yy = rotation.y() * y2;
    float yz = rotation.y() * z2;
    float zz = rotation.z() * z2;

    float x = plane.Normal().x();
    float y = plane.Normal().y();
    float z = plane.Normal().z();

    const float3 normal(
        ((x * ((1.0f - yy) - zz)) + (y * (xy - wz))) + (z * (xz + wy)),
        ((x * (xy + wz)) + (y * ((1.0f - xx) - zz))) + (z * (yz - wx)),
        ((x * (xz - wy)) + (y * (yz + wx))) + (z * ((1.0f - xx) - yy)) );

    return FPlane(normal, plane.D());
}
//----------------------------------------------------------------------------
void FPlane::Transform(const TMemoryView<FPlane>& planes, const FQuaternion& rotation) {
    if (planes.empty())
        return;

    float x2 = rotation.x() + rotation.x();
    float y2 = rotation.y() + rotation.y();
    float z2 = rotation.z() + rotation.z();

    float wx = rotation.w() * x2;
    float wy = rotation.w() * y2;
    float wz = rotation.w() * z2;

    float xx = rotation.x() * x2;
    float xy = rotation.x() * y2;
    float xz = rotation.x() * z2;

    float yy = rotation.y() * y2;
    float yz = rotation.y() * z2;
    float zz = rotation.z() * z2;

    for (size_t i = 0; i < planes.size(); ++i) {
        float x = planes[i].Normal().x();
        float y = planes[i].Normal().y();
        float z = planes[i].Normal().z();

        /*
            * Note:
            * Factor common arithmetic out of loop.
        */
        planes[i].Normal().x() = ((x * ((1.0f - yy) - zz)) + (y * (xy - wz))) + (z * (xz + wy));
        planes[i].Normal().y() = ((x * (xy + wz)) + (y * ((1.0f - xx) - zz))) + (z * (yz - wx));
        planes[i].Normal().z() = ((x * (xz - wy)) + (y * (yz + wx))) + (z * ((1.0f - xx) - yy));
    }
}
//----------------------------------------------------------------------------
FPlane FPlane::Transform(const FPlane& plane, const Matrix& transformation) {
    Matrix inverse = Invert(transformation);

    float x = plane.Normal().x();
    float y = plane.Normal().y();
    float z = plane.Normal().z();
    float d = plane.D();

    const float3 normal(
        (((x * inverse._11()) + (y * inverse._12())) + (z * inverse._13())) + (d * inverse._14()),
        (((x * inverse._21()) + (y * inverse._22())) + (z * inverse._23())) + (d * inverse._24()),
        (((x * inverse._31()) + (y * inverse._32())) + (z * inverse._33())) + (d * inverse._34()) );

    return FPlane(
        normal,
        (((x * inverse._41()) + (y * inverse._42())) + (z * inverse._43())) + (d * inverse._44()) );
}
//----------------------------------------------------------------------------
void FPlane::Transform(const TMemoryView<FPlane>& planes, const Matrix& transformation) {
    if (planes.empty())
        return;

    Matrix inverse = Invert(transformation);

    for (FPlane& plane : planes) {
        float x = plane.Normal().x();
        float y = plane.Normal().y();
        float z = plane.Normal().z();
        float d = plane.D();

        plane.Normal().x() = (((x * inverse._11()) + (y * inverse._12())) + (z * inverse._13())) + (d * inverse._14());
        plane.Normal().y() = (((x * inverse._21()) + (y * inverse._22())) + (z * inverse._23())) + (d * inverse._24());
        plane.Normal().z() = (((x * inverse._31()) + (y * inverse._32())) + (z * inverse._33())) + (d * inverse._34());
        plane.D() = (((x * inverse._41()) + (y * inverse._42())) + (z * inverse._43())) + (d * inverse._44());
    }
}
//----------------------------------------------------------------------------
float3 FPlane::Get3PlanesInterPoint(const FPlane& p1, const FPlane& p2, const FPlane& p3) {
    //P = -d1 * N2xN3 / N1.N2xN3 - d2 * N3xN1 / N2.N3xN1 - d3 * N1xN2 / N3.N1xN2
    const float3 v =
        -p1.D() * Cross(p2.Normal(), p3.Normal()) / Dot3(p1.Normal(), Cross(p2.Normal(), p3.Normal()))
        -p2.D() * Cross(p3.Normal(), p1.Normal()) / Dot3(p2.Normal(), Cross(p3.Normal(), p1.Normal()))
        -p3.D() * Cross(p1.Normal(), p2.Normal()) / Dot3(p3.Normal(), Cross(p1.Normal(), p2.Normal()));

    return v;
}
//----------------------------------------------------------------------------
EPlaneIntersectionType FPlane::PointsIntersection(const FPlane& plane, const TMemoryView<const float3>& points) {
    EPlaneIntersectionType result = Collision::PlaneIntersectsPoint(plane, points[0]);

    for (size_t i = 1; i < points.size(); ++i)
        if (Collision::PlaneIntersectsPoint(plane, points[i]) != result)
            return EPlaneIntersectionType::Intersecting;

    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
