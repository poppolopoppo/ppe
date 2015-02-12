#include "stdafx.h"

#include "Frustum.h"

#include "Maths/Transform/ScalarMatrix.h"
#include "Maths/Transform/ScalarMatrixHelpers.h"
#include "Ray.h"
#include "ScalarBoundingBox.h"
#include "ScalarBoundingBoxHelpers.h"
#include "ScalarVector.h"
#include "ScalarVectorHelpers.h"
#include "Sphere.h"

#include "Memory/MemoryView.h"

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

    Plane& ptop = _planes[size_t(FrustumPlane::Top)];
    Plane& pbottom = _planes[size_t(FrustumPlane::Bottom)];

    Plane& pleft = _planes[size_t(FrustumPlane::Left)];
    Plane& pright = _planes[size_t(FrustumPlane::Right)];

    Plane& pnear = _planes[size_t(FrustumPlane::Near)];
    Plane& pfar = _planes[size_t(FrustumPlane::Far)];

    // Left plane
    pleft.Normal().x()  = _matrix._14() + _matrix._11();
    pleft.Normal().y()  = _matrix._24() + _matrix._21();
    pleft.Normal().z()  = _matrix._34() + _matrix._31();
    pleft.D()           = _matrix._44() + _matrix._41();
    pleft = pleft.Normalize();

    // Right plane
    pright.Normal().x() = _matrix._14() - _matrix._11();
    pright.Normal().y() = _matrix._24() - _matrix._21();
    pright.Normal().z() = _matrix._34() - _matrix._31();
    pright.D()          = _matrix._44() - _matrix._41();
    pright = pright.Normalize();

    // Top plane
    ptop.Normal().x()   = _matrix._14() - _matrix._12();
    ptop.Normal().y()   = _matrix._24() - _matrix._22();
    ptop.Normal().z()   = _matrix._34() - _matrix._32();
    ptop.D()            = _matrix._44() - _matrix._42();
    ptop = ptop.Normalize();

    // Bottom plane
    pbottom.Normal().x()= _matrix._14() + _matrix._12();
    pbottom.Normal().y()= _matrix._24() + _matrix._22();
    pbottom.Normal().z()= _matrix._34() + _matrix._32();
    pbottom.D()         = _matrix._44() + _matrix._42();
    pbottom = pbottom.Normalize();

    // Near plane
    pnear.Normal().x()  = _matrix._14() + _matrix._13();
    pnear.Normal().y()  = _matrix._24() + _matrix._23();
    pnear.Normal().z()  = _matrix._34() + _matrix._33();
    pnear.D()           = _matrix._44() + _matrix._43();
    pnear = pnear.Normalize();

    // Far plane
    pfar.Normal().x()   = _matrix._14() - _matrix._13();
    pfar.Normal().y()   = _matrix._24() - _matrix._23();
    pfar.Normal().z()   = _matrix._34() - _matrix._33();
    pfar.D()            = _matrix._44() - _matrix._43();
    pfar = pfar.Normalize();
}
//----------------------------------------------------------------------------
void Frustum::GetCorners(const MemoryView<float3>& points) const {
    Assert(points.size() == 8);

    const Plane& ptop = _planes[size_t(FrustumPlane::Top)];
    const Plane& pbottom = _planes[size_t(FrustumPlane::Bottom)];

    const Plane& pleft = _planes[size_t(FrustumPlane::Left)];
    const Plane& pright = _planes[size_t(FrustumPlane::Right)];

    const Plane& pnear = _planes[size_t(FrustumPlane::Near)];
    const Plane& pfar = _planes[size_t(FrustumPlane::Far)];

    points[size_t(FrustumCorner::Near_LeftTop)]     = Plane::Get3PlanesInterPoint(pnear, pleft, ptop);
    points[size_t(FrustumCorner::Near_LeftBottom)]  = Plane::Get3PlanesInterPoint(pnear, pleft, pbottom);
    points[size_t(FrustumCorner::Near_RightBottom)] = Plane::Get3PlanesInterPoint(pnear, pright, pbottom);
    points[size_t(FrustumCorner::Near_RightTop)]    = Plane::Get3PlanesInterPoint(pnear, pright, ptop);

    points[size_t(FrustumCorner::Far_LeftTop)]      = Plane::Get3PlanesInterPoint(pfar, pleft, ptop);
    points[size_t(FrustumCorner::Far_LeftBottom)]   = Plane::Get3PlanesInterPoint(pfar, pleft, pbottom);
    points[size_t(FrustumCorner::Far_RightBottom)]  = Plane::Get3PlanesInterPoint(pfar, pright, pbottom);
    points[size_t(FrustumCorner::Far_RightTop)]     = Plane::Get3PlanesInterPoint(pfar, pright, ptop);
}
//----------------------------------------------------------------------------
void Frustum::GetCameraParams(FrustumCameraParams& params) const {
    float3 corners[8];
    GetCorners(MakeView(corners));

    const Plane& ptop = _planes[size_t(FrustumPlane::Top)];
    //const Plane& pbottom = _planes[size_t(FrustumPlane::Bottom)];

    const Plane& pleft = _planes[size_t(FrustumPlane::Left)];
    const Plane& pright = _planes[size_t(FrustumPlane::Right)];

    const Plane& pnear = _planes[size_t(FrustumPlane::Near)];
    const Plane& pfar = _planes[size_t(FrustumPlane::Far)];

    params.Position = Plane::Get3PlanesInterPoint(pright, ptop, pleft);
    params.LookAtDir = pnear.Normal();
    params.UpDir = Normalize3(Cross(pright.Normal(), pnear.Normal()));
    params.FOV = (F_HalfPi - std::acos(Dot3(pnear.Normal(), ptop.Normal()))) * 2;
    params.AspectRatio = Length3(corners[6] - corners[5]) / Length3(corners[4] - corners[5]);
    params.ZNear = fabsf(pnear.DistanceToPoint(params.Position));
    params.ZFar = fabsf(pfar.DistanceToPoint(params.Position));
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

    result._planes[size_t(FrustumPlane::Near)]      = Plane::FromTriangle(Near1, Near2, Near3).Normalize();
    result._planes[size_t(FrustumPlane::Far)]       = Plane::FromTriangle(Far3, Far2, Far1).Normalize();
    result._planes[size_t(FrustumPlane::Left)]      = Plane::FromTriangle(Near4, Near3, Far3).Normalize();
    result._planes[size_t(FrustumPlane::Right)]     = Plane::FromTriangle(Far1, Far2, Near2).Normalize();
    result._planes[size_t(FrustumPlane::Top)]       = Plane::FromTriangle(Near2, Far2, Far3).Normalize();
    result._planes[size_t(FrustumPlane::Bottom)]    = Plane::FromTriangle(Far4, Far1, Near1).Normalize();

    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
