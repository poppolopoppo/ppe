#include "stdafx.h"

#include "Frustum.h"

#include "Ray.h"
#include "Sphere.h"

#include "ScalarBoundingBox.h"
#include "ScalarBoundingBoxHelpers.h"

#include "ScalarMatrix.h"
#include "ScalarMatrixHelpers.h"

#include "ScalarVector.h"
#include "ScalarVectorHelpers.h"

#include "MemoryView.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
Frustum::Frustum(const Frustum& other)
:   _matrix(other._matrix) {
    for (size_t i = 0; i < 6; ++i)
        _planes[i] = other._planes[i];
}
//----------------------------------------------------------------------------
Frustum& Frustum::operator =(const Frustum& other) {
    _matrix = other._matrix;
    for (size_t i = 0; i < 6; ++i)
        _planes[i] = other._planes[i];

    return *this;
}
//----------------------------------------------------------------------------
void Frustum::SetMatrix(const float4x4& viewProjection) {
    //http://www.chadvernon.com/blog/resources/directx9/frustum-culling/

    _matrix = viewProjection;

    Plane& top = _planes[size_t(FrustumPlane::Top)];
    Plane& bottom = _planes[size_t(FrustumPlane::Bottom)];

    Plane& left = _planes[size_t(FrustumPlane::Left)];
    Plane& right = _planes[size_t(FrustumPlane::Right)];

    Plane& near = _planes[size_t(FrustumPlane::Near)];
    Plane& far = _planes[size_t(FrustumPlane::Far)];

    // Left plane
    left.Normal().x() = _matrix._14() + _matrix._11();
    left.Normal().y() = _matrix._24() + _matrix._21();
    left.Normal().z() = _matrix._34() + _matrix._31();
    left.D() = _matrix._44() + _matrix._41();
    left = left.Normalize();

    // Right plane
    right.Normal().x() = _matrix._14() - _matrix._11();
    right.Normal().y() = _matrix._24() - _matrix._21();
    right.Normal().z() = _matrix._34() - _matrix._31();
    right.D() = _matrix._44() - _matrix._41();
    right = right.Normalize();

    // Top plane
    top.Normal().x() = _matrix._14() - _matrix._12();
    top.Normal().y() = _matrix._24() - _matrix._22();
    top.Normal().z() = _matrix._34() - _matrix._32();
    top.D() = _matrix._44() - _matrix._42();
    top = top.Normalize();

    // Bottom plane
    bottom.Normal().x() = _matrix._14() + _matrix._12();
    bottom.Normal().y() = _matrix._24() + _matrix._22();
    bottom.Normal().z() = _matrix._34() + _matrix._32();
    bottom.D() = _matrix._44() + _matrix._42();
    bottom = bottom.Normalize();

    // Near plane
    near.Normal().x() = _matrix._13();
    near.Normal().y() = _matrix._23();
    near.Normal().z() = _matrix._33();
    near.D() = _matrix._43();
    near = near.Normalize();

    // Far plane
    far.Normal().x() = _matrix._14() - _matrix._13();
    far.Normal().y() = _matrix._24() - _matrix._23();
    far.Normal().z() = _matrix._34() - _matrix._33();
    far.D() = _matrix._44() - _matrix._43();
    far = far.Normalize();
}
//----------------------------------------------------------------------------
void Frustum::GetCorners(const MemoryView<float3>& points) const {
    Assert(points.size() == 8);

    const Plane& top = _planes[size_t(FrustumPlane::Top)];
    const Plane& bottom = _planes[size_t(FrustumPlane::Bottom)];

    const Plane& left = _planes[size_t(FrustumPlane::Left)];
    const Plane& right = _planes[size_t(FrustumPlane::Right)];

    const Plane& near = _planes[size_t(FrustumPlane::Near)];
    const Plane& far = _planes[size_t(FrustumPlane::Far)];

    points[0] = Plane::Get3PlanesInterPoint(near,  bottom,  right);   //Near1
    points[1] = Plane::Get3PlanesInterPoint(near,  top,  right);      //Near2
    points[2] = Plane::Get3PlanesInterPoint(near,  top,  left);       //Near3
    points[3] = Plane::Get3PlanesInterPoint(near,  bottom,  left);    //Near3

    points[4] = Plane::Get3PlanesInterPoint(far,  bottom,  right);    //Far1
    points[5] = Plane::Get3PlanesInterPoint(far,  top,  right);       //Far2
    points[6] = Plane::Get3PlanesInterPoint(far,  top,  left);        //Far3
    points[7] = Plane::Get3PlanesInterPoint(far,  bottom,  left);     //Far3
}
//----------------------------------------------------------------------------
void Frustum::GetCameraParams(FrustumCameraParams& params) const {
    float3 corners[8];
    GetCorners(MakeView(corners));

    const Plane& top = _planes[size_t(FrustumPlane::Top)];
    //const Plane& bottom = _planes[size_t(FrustumPlane::Bottom)];

    const Plane& left = _planes[size_t(FrustumPlane::Left)];
    const Plane& right = _planes[size_t(FrustumPlane::Right)];

    const Plane& near = _planes[size_t(FrustumPlane::Near)];
    const Plane& far = _planes[size_t(FrustumPlane::Far)];

    params.Position = Plane::Get3PlanesInterPoint(right, top, left);
    params.LookAtDir = near.Normal();
    params.UpDir = Normalize3(Cross(right.Normal(), near.Normal()));
    params.FOV = (F_HalfPi - std::acos(Dot3(near.Normal(), top.Normal()))) * 2;
    params.AspectRatio = Length3(corners[6] - corners[5]) / Length3(corners[4] - corners[5]);
    params.ZNear = Length3(params.Position + (near.Normal() * near.D()));
    params.ZFar = Length3(params.Position + (far.Normal() * far.D()));
}
//----------------------------------------------------------------------------
ContainmentType Frustum::Contains(const float3& point) const {
    PlaneIntersectionType result = PlaneIntersectionType::Front;

    for (const Plane& plane : _planes) {
        const PlaneIntersectionType planeResult = plane.Intersects(point);

        if (PlaneIntersectionType::Back == planeResult)
            return ContainmentType::Disjoint;
        else if (PlaneIntersectionType::Intersecting == planeResult)
            result = PlaneIntersectionType::Intersecting;
    }

    return (PlaneIntersectionType::Intersecting == result)
        ? ContainmentType::Intersects
        : ContainmentType::Contains;
}
//----------------------------------------------------------------------------
ContainmentType Frustum::Contains(const MemoryView<const float3>& points) const {
    bool containsAll = true;
    bool containsAny = false;

    for (const float3& point : points) {
        const ContainmentType pointResult = Contains(point);
        if (ContainmentType::Intersects == pointResult ||
            ContainmentType::Contains == pointResult )
            containsAny = true;
        else
            containsAll = false;
    }

    if (containsAny)
        return containsAll ? ContainmentType::Contains : ContainmentType::Intersects;
    else
        return ContainmentType::Disjoint;
}
//----------------------------------------------------------------------------
ContainmentType Frustum::Contains(const BoundingBox& box) const {
    float3 corners[8];
    box.GetCorners(corners);

    return Contains(MakeView<const float3>(corners));
}
//----------------------------------------------------------------------------
ContainmentType Frustum::Contains(const Sphere& sphere) const {
    PlaneIntersectionType result = PlaneIntersectionType::Front;

    for (const Plane& plane : _planes) {
        const PlaneIntersectionType planeResult = plane.Intersects(sphere);

        if (PlaneIntersectionType::Back == planeResult)
            return ContainmentType::Disjoint;
        else if (PlaneIntersectionType::Intersecting == planeResult)
            result = PlaneIntersectionType::Intersecting;
    }

    return (PlaneIntersectionType::Intersecting == result)
        ? ContainmentType::Intersects
        : ContainmentType::Contains;
}
//----------------------------------------------------------------------------
ContainmentType Frustum::Contains(const Frustum& frustum) const {
    float3 corners[8];
    frustum.GetCorners(corners);

    return Contains(MakeView<const float3>(corners));
}
//----------------------------------------------------------------------------
PlaneIntersectionType Frustum::Intersects(const Plane& plane) const {
    float3 corners[8];
    GetCorners(corners);

    return Plane::PointsIntersection(plane, MakeView<const float3>(corners));
}
//----------------------------------------------------------------------------
bool Frustum::Intersects(const Ray& ray) const {
    for (const Plane& plane : _planes)
        if (plane.Intersects(ray))
            return true;

    return false;
}
//----------------------------------------------------------------------------
bool Frustum::Intersects(const Ray& ray, float& in, float& out) const {
    bool result = false;
    in = -1;
    out = -1;

    Frustum ioFrsutrum(*this);
    ioFrsutrum._planes[size_t(FrustumPlane::Top)].Normal() = -Top().Normal();
    ioFrsutrum._planes[size_t(FrustumPlane::Bottom)].Normal() = -Bottom().Normal();

    ioFrsutrum._planes[size_t(FrustumPlane::Left)].Normal() = -Left().Normal();
    ioFrsutrum._planes[size_t(FrustumPlane::Right)].Normal() = -Right().Normal();

    ioFrsutrum._planes[size_t(FrustumPlane::Near)].Normal() = -Near().Normal();
    ioFrsutrum._planes[size_t(FrustumPlane::Far)].Normal() = -Far().Normal();

    for (const Plane& plane : ioFrsutrum._planes) {
        float planeDistance;
        const bool planeResult = plane.Intersects(ray, planeDistance);

        if (!planeResult)
            continue;

        result = true;
        if (in < 0) {
            in = planeDistance;
        }
        else {
            if (out < 0) {
                if (planeDistance < in) {
                    out = in;
                    in = planeDistance;
                }
                else {
                    out = planeDistance;
                }
            }
        }
    }

    //if the intersection happed at one point, then the ray starts from inside the frustum
    //and intersects it while going out.
    if (result && in > 0 && out < 0)
    {
        out = in;
        in = -1;
    }

    return result;
}
//----------------------------------------------------------------------------
float Frustum::GetWidthAtDepth(float depth) const {
    float hAngle = (F_HalfPi - std::acos(Dot3(Near().Normal(), Left().Normal())) );
    return std::tan(hAngle) * depth * 2;
}
//----------------------------------------------------------------------------
float Frustum::GetHeightAtDepth(float depth) const {
    float vAngle = (F_HalfPi - std::acos(Dot3(Near().Normal(), Top().Normal())) );
    return std::tan(vAngle) * depth * 2;
}
//----------------------------------------------------------------------------
float Frustum::GetZoomToExtentsShiftDistance(const MemoryView<const float3>& points) const {
    float vAngle = (F_HalfPi - std::acos(Dot3(Near().Normal(), Top().Normal())) );
    float vSin = std::sin(vAngle);
    Assert(std::abs(vSin) > F_Epsilon);

    float hAngle = (F_HalfPi - std::acos(Dot3(Near().Normal(), Left().Normal())) );
    float hSin = std::sin(hAngle);

    float horizontalToVerticalMapping = vSin / hSin;

    Frustum ioFrsutrum(*this);
    ioFrsutrum._planes[size_t(FrustumPlane::Top)].Normal() = -Top().Normal();
    ioFrsutrum._planes[size_t(FrustumPlane::Bottom)].Normal() = -Bottom().Normal();

    ioFrsutrum._planes[size_t(FrustumPlane::Left)].Normal() = -Left().Normal();
    ioFrsutrum._planes[size_t(FrustumPlane::Right)].Normal() = -Right().Normal();

    ioFrsutrum._planes[size_t(FrustumPlane::Near)].Normal() = -Near().Normal();
    ioFrsutrum._planes[size_t(FrustumPlane::Far)].Normal() = -Far().Normal();

    float maxPointDist = FLT_MIN;
    for (const float3& point : points)
    {
        float pointDist = Collision::DistancePlanePoint(ioFrsutrum.Top(), point);
        pointDist = Max(pointDist, Collision::DistancePlanePoint(ioFrsutrum.Bottom(), point));
        pointDist = Max(pointDist, Collision::DistancePlanePoint(ioFrsutrum.Left(), point) * horizontalToVerticalMapping);
        pointDist = Max(pointDist, Collision::DistancePlanePoint(ioFrsutrum.Right(), point) * horizontalToVerticalMapping);

        maxPointDist = Max(maxPointDist, pointDist);
    }

    return -maxPointDist / vSin;
}
//----------------------------------------------------------------------------
float Frustum::GetZoomToExtentsShiftDistance(const BoundingBox& box) {
    float3 corners[8];
    box.GetCorners(corners);

    return GetZoomToExtentsShiftDistance(MakeView<const float3>(corners));
}
//----------------------------------------------------------------------------
Frustum Frustum::FromCamera(const float3& cameraPos, const float3& lookDir, const float3& upDir, float fov, float znear, float zfar, float aspect) {
    //http://knol.google.com/k/view-frustum

    Assert(IsNormalized(lookDir));
    Assert(IsNormalized(upDir));

    float3 nearCenter = cameraPos + lookDir * znear;
    float3 farCenter = cameraPos + lookDir * zfar;
    float nearHalfHeight = (float)(znear * std::tan(fov / 2.0f));
    float farHalfHeight = (float)(zfar * std::tan(fov / 2.0f));
    float nearHalfWidth = nearHalfHeight * aspect;
    float farHalfWidth = farHalfHeight * aspect;

    float3 rightDir = Normalize3(Cross(upDir, lookDir));
    float3 Near1 = nearCenter - nearHalfHeight * upDir + nearHalfWidth * rightDir;
    float3 Near2 = nearCenter + nearHalfHeight * upDir + nearHalfWidth * rightDir;
    float3 Near3 = nearCenter + nearHalfHeight * upDir - nearHalfWidth * rightDir;
    float3 Near4 = nearCenter - nearHalfHeight * upDir - nearHalfWidth * rightDir;
    float3 Far1 = farCenter - farHalfHeight * upDir + farHalfWidth * rightDir;
    float3 Far2 = farCenter + farHalfHeight * upDir + farHalfWidth * rightDir;
    float3 Far3 = farCenter + farHalfHeight * upDir - farHalfWidth * rightDir;
    float3 Far4 = farCenter - farHalfHeight * upDir - farHalfWidth * rightDir;

    Frustum result;

    result._matrix =    MakeLookAtLHMatrix(cameraPos, cameraPos + lookDir * 10, upDir) *
                        MakePerspectiveFovLHMatrix(fov, aspect, znear, zfar);

    Plane& top = result._planes[size_t(FrustumPlane::Top)];
    Plane& bottom = result._planes[size_t(FrustumPlane::Bottom)];

    Plane& left = result._planes[size_t(FrustumPlane::Left)];
    Plane& right = result._planes[size_t(FrustumPlane::Right)];

    Plane& near = result._planes[size_t(FrustumPlane::Near)];
    Plane& far = result._planes[size_t(FrustumPlane::Far)];

    near = Plane::FromTriangle(Near1, Near2, Near3).Normalize();
    far = Plane::FromTriangle(Far3, Far2, Far1).Normalize();
    left = Plane::FromTriangle(Near4, Near3, Far3).Normalize();
    right = Plane::FromTriangle(Far1, Far2, Near2).Normalize();
    top = Plane::FromTriangle(Near2, Far2, Far3).Normalize();
    bottom = Plane::FromTriangle(Far4, Far1, Near1).Normalize();

    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
