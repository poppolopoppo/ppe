#include "stdafx.h"

#include "Frustum.h"

#include "Maths/ScalarMatrix.h"
#include "Maths/ScalarMatrixHelpers.h"
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
FFrustum::FFrustum(const FFrustum& other)
:   _matrix(other._matrix) {
    for (size_t i = 0; i < 6; ++i)
        _planes[i] = other._planes[i];
}
//----------------------------------------------------------------------------
FFrustum& FFrustum::operator =(const FFrustum& other) {
    _matrix = other._matrix;
    for (size_t i = 0; i < 6; ++i)
        _planes[i] = other._planes[i];

    return *this;
}
//----------------------------------------------------------------------------
void FFrustum::SetMatrix(const float4x4& viewProjection) {
    //http://www.chadvernon.com/blog/resources/directx9/frustum-culling/

    _matrix = viewProjection;

    FPlane& ptop = _planes[size_t(EFrustumPlane::Top)];
    FPlane& pbottom = _planes[size_t(EFrustumPlane::Bottom)];

    FPlane& pleft = _planes[size_t(EFrustumPlane::Left)];
    FPlane& pright = _planes[size_t(EFrustumPlane::Right)];

    FPlane& pnear = _planes[size_t(EFrustumPlane::Near)];
    FPlane& pfar = _planes[size_t(EFrustumPlane::Far)];

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
void FFrustum::GetCorners(const TMemoryView<float3>& points) const {
    Assert(points.size() == 8);

    const FPlane& ptop = _planes[size_t(EFrustumPlane::Top)];
    const FPlane& pbottom = _planes[size_t(EFrustumPlane::Bottom)];

    const FPlane& pleft = _planes[size_t(EFrustumPlane::Left)];
    const FPlane& pright = _planes[size_t(EFrustumPlane::Right)];

    const FPlane& pnear = _planes[size_t(EFrustumPlane::Near)];
    const FPlane& pfar = _planes[size_t(EFrustumPlane::Far)];

    points[size_t(EFrustumCorner::Near_LeftTop)]     = FPlane::Get3PlanesInterPoint(pnear, pleft, ptop);
    points[size_t(EFrustumCorner::Near_LeftBottom)]  = FPlane::Get3PlanesInterPoint(pnear, pleft, pbottom);
    points[size_t(EFrustumCorner::Near_RightBottom)] = FPlane::Get3PlanesInterPoint(pnear, pright, pbottom);
    points[size_t(EFrustumCorner::Near_RightTop)]    = FPlane::Get3PlanesInterPoint(pnear, pright, ptop);

    points[size_t(EFrustumCorner::Far_LeftTop)]      = FPlane::Get3PlanesInterPoint(pfar, pleft, ptop);
    points[size_t(EFrustumCorner::Far_LeftBottom)]   = FPlane::Get3PlanesInterPoint(pfar, pleft, pbottom);
    points[size_t(EFrustumCorner::Far_RightBottom)]  = FPlane::Get3PlanesInterPoint(pfar, pright, pbottom);
    points[size_t(EFrustumCorner::Far_RightTop)]     = FPlane::Get3PlanesInterPoint(pfar, pright, ptop);
}
//----------------------------------------------------------------------------
void FFrustum::GetCameraParams(FFrustumCameraParams& params) const {
    float3 corners[8];
    GetCorners(MakeView(corners));

    const FPlane& ptop = _planes[size_t(EFrustumPlane::Top)];
    //const FPlane& pbottom = _planes[size_t(EFrustumPlane::Bottom)];

    const FPlane& pleft = _planes[size_t(EFrustumPlane::Left)];
    const FPlane& pright = _planes[size_t(EFrustumPlane::Right)];

    const FPlane& pnear = _planes[size_t(EFrustumPlane::Near)];
    const FPlane& pfar = _planes[size_t(EFrustumPlane::Far)];

    params.Position = FPlane::Get3PlanesInterPoint(pright, ptop, pleft);
    params.LookAtDir = pnear.Normal();
    params.UpDir = Normalize3(Cross(pright.Normal(), pnear.Normal()));
    params.FOV = (F_HalfPi - std::acos(Dot3(pnear.Normal(), ptop.Normal()))) * 2;
    params.AspectRatio = Length3(corners[6] - corners[5]) / Length3(corners[4] - corners[5]);
    params.ZNear = fabsf(pnear.DistanceToPoint(params.Position));
    params.ZFar = fabsf(pfar.DistanceToPoint(params.Position));
}
//----------------------------------------------------------------------------
EContainmentType FFrustum::Contains(const float3& point) const {
    EPlaneIntersectionType result = EPlaneIntersectionType::Front;

    for (const FPlane& plane : _planes) {
        const EPlaneIntersectionType planeResult = plane.Intersects(point);

        if (EPlaneIntersectionType::Back == planeResult)
            return EContainmentType::Disjoint;
        else if (EPlaneIntersectionType::Intersecting == planeResult)
            result = EPlaneIntersectionType::Intersecting;
    }

    return (EPlaneIntersectionType::Intersecting == result)
        ? EContainmentType::Intersects
        : EContainmentType::Contains;
}
//----------------------------------------------------------------------------
EContainmentType FFrustum::Contains(const TMemoryView<const float3>& points) const {
    bool containsAll = true;
    bool containsAny = false;

    for (const float3& point : points) {
        const EContainmentType pointResult = Contains(point);
        if (EContainmentType::Intersects == pointResult ||
            EContainmentType::Contains == pointResult )
            containsAny = true;
        else
            containsAll = false;
    }

    if (containsAny)
        return containsAll ? EContainmentType::Contains : EContainmentType::Intersects;
    else
        return EContainmentType::Disjoint;
}
//----------------------------------------------------------------------------
EContainmentType FFrustum::Contains(const BoundingBox& box) const {
    // http://www.iquilezles.org/www/articles/frustumcorrect/frustumcorrect.htm
    size_t inside = 0;

    // check box outside/inside of frustum
    for (const FPlane& plane : _planes)
    {
        size_t out = 0;

        out += (plane.DistanceToPoint(float3(box.Min().x(), box.Min().y(), box.Min().z())) < 0.0f ? 1 : 0);
        out += (plane.DistanceToPoint(float3(box.Max().x(), box.Min().y(), box.Min().z())) < 0.0f ? 1 : 0);
        out += (plane.DistanceToPoint(float3(box.Min().x(), box.Max().y(), box.Min().z())) < 0.0f ? 1 : 0);
        out += (plane.DistanceToPoint(float3(box.Max().x(), box.Max().y(), box.Min().z())) < 0.0f ? 1 : 0);

        out += (plane.DistanceToPoint(float3(box.Min().x(), box.Min().y(), box.Max().z())) < 0.0f ? 1 : 0);
        out += (plane.DistanceToPoint(float3(box.Max().x(), box.Min().y(), box.Max().z())) < 0.0f ? 1 : 0);
        out += (plane.DistanceToPoint(float3(box.Min().x(), box.Max().y(), box.Max().z())) < 0.0f ? 1 : 0);
        out += (plane.DistanceToPoint(float3(box.Max().x(), box.Max().y(), box.Max().z())) < 0.0f ? 1 : 0);

        if (8 == out)
            return EContainmentType::Disjoint;
        else if (0 == out)
            inside++;
    }

    if (6 == inside)
        return EContainmentType::Contains;

    // TODO: create a class to speedup frustum collision
    float3 frustumCorners[8];
    GetCorners(frustumCorners);

    // check frustum outside/inside box
    size_t out;

    out=0; for (const float3& p : frustumCorners) out += ((p.x() > box.Max().x()) ? 1 : 0); if (8 == out) return EContainmentType::Disjoint;
    out=0; for (const float3& p : frustumCorners) out += ((p.x() < box.Min().x()) ? 1 : 0); if (8 == out) return EContainmentType::Disjoint;

    out=0; for (const float3& p : frustumCorners) out += ((p.y() > box.Max().y()) ? 1 : 0); if (8 == out) return EContainmentType::Disjoint;
    out=0; for (const float3& p : frustumCorners) out += ((p.y() < box.Min().y()) ? 1 : 0); if (8 == out) return EContainmentType::Disjoint;

    out=0; for (const float3& p : frustumCorners) out += ((p.z() > box.Max().z()) ? 1 : 0); if (8 == out) return EContainmentType::Disjoint;
    out=0; for (const float3& p : frustumCorners) out += ((p.z() < box.Min().z()) ? 1 : 0); if (8 == out) return EContainmentType::Disjoint;

    return EContainmentType::Intersects;
}
//----------------------------------------------------------------------------
EContainmentType FFrustum::Contains(const FSphere& sphere) const {
    EPlaneIntersectionType result = EPlaneIntersectionType::Front;

    for (const FPlane& plane : _planes) {
        const EPlaneIntersectionType planeResult = plane.Intersects(sphere);

        if (EPlaneIntersectionType::Back == planeResult)
            return EContainmentType::Disjoint;
        else if (EPlaneIntersectionType::Intersecting == planeResult)
            result = EPlaneIntersectionType::Intersecting;
    }

    return (EPlaneIntersectionType::Intersecting == result)
        ? EContainmentType::Intersects
        : EContainmentType::Contains;
}
//----------------------------------------------------------------------------
EContainmentType FFrustum::Contains(const FFrustum& frustum) const {
    float3 corners[8];
    frustum.GetCorners(corners);

    return Contains(MakeView<const float3>(corners));
}
//----------------------------------------------------------------------------
EPlaneIntersectionType FFrustum::Intersects(const FPlane& plane) const {
    float3 corners[8];
    GetCorners(corners);

    return FPlane::PointsIntersection(plane, MakeView<const float3>(corners));
}
//----------------------------------------------------------------------------
bool FFrustum::Intersects(const FRay& ray) const {
    for (const FPlane& plane : _planes)
        if (plane.Intersects(ray))
            return true;

    return false;
}
//----------------------------------------------------------------------------
bool FFrustum::Intersects(const FRay& ray, float& in, float& out) const {
    bool result = false;
    in = -1;
    out = -1;

    FFrustum ioFrsutrum(*this);
    ioFrsutrum._planes[size_t(EFrustumPlane::Top)].Normal() = -Top().Normal();
    ioFrsutrum._planes[size_t(EFrustumPlane::Bottom)].Normal() = -Bottom().Normal();

    ioFrsutrum._planes[size_t(EFrustumPlane::Left)].Normal() = -Left().Normal();
    ioFrsutrum._planes[size_t(EFrustumPlane::Right)].Normal() = -Right().Normal();

    ioFrsutrum._planes[size_t(EFrustumPlane::Near)].Normal() = -Near().Normal();
    ioFrsutrum._planes[size_t(EFrustumPlane::Far)].Normal() = -Far().Normal();

    for (const FPlane& plane : ioFrsutrum._planes) {
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
float FFrustum::GetWidthAtDepth(float depth) const {
    float hAngle = (F_HalfPi - std::acos(Dot3(Near().Normal(), Left().Normal())) );
    return std::tan(hAngle) * depth * 2;
}
//----------------------------------------------------------------------------
float FFrustum::GetHeightAtDepth(float depth) const {
    float vAngle = (F_HalfPi - std::acos(Dot3(Near().Normal(), Top().Normal())) );
    return std::tan(vAngle) * depth * 2;
}
//----------------------------------------------------------------------------
float FFrustum::GetZoomToExtentsShiftDistance(const TMemoryView<const float3>& points) const {
    float vAngle = (F_HalfPi - std::acos(Dot3(Near().Normal(), Top().Normal())) );
    float vSin = std::sin(vAngle);
    Assert(std::abs(vSin) > F_Epsilon);

    float hAngle = (F_HalfPi - std::acos(Dot3(Near().Normal(), Left().Normal())) );
    float hSin = std::sin(hAngle);

    float horizontalToVerticalMapping = vSin / hSin;

    FFrustum ioFrsutrum(*this);
    ioFrsutrum._planes[size_t(EFrustumPlane::Top)].Normal() = -Top().Normal();
    ioFrsutrum._planes[size_t(EFrustumPlane::Bottom)].Normal() = -Bottom().Normal();

    ioFrsutrum._planes[size_t(EFrustumPlane::Left)].Normal() = -Left().Normal();
    ioFrsutrum._planes[size_t(EFrustumPlane::Right)].Normal() = -Right().Normal();

    ioFrsutrum._planes[size_t(EFrustumPlane::Near)].Normal() = -Near().Normal();
    ioFrsutrum._planes[size_t(EFrustumPlane::Far)].Normal() = -Far().Normal();

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
float FFrustum::GetZoomToExtentsShiftDistance(const BoundingBox& box) {
    float3 corners[8];
    box.GetCorners(corners);

    return GetZoomToExtentsShiftDistance(MakeView<const float3>(corners));
}
//----------------------------------------------------------------------------
FFrustum FFrustum::FromCamera(const float3& cameraPos, const float3& lookDir, const float3& upDir, float fov, float znear, float zfar, float aspect) {
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

    FFrustum result;

    result._matrix =    MakeLookAtLHMatrix(cameraPos, cameraPos + lookDir * 10, upDir) *
                        MakePerspectiveFovLHMatrix(fov, aspect, znear, zfar);

    result._planes[size_t(EFrustumPlane::Near)]      = FPlane::FromTriangle(Near1, Near2, Near3).Normalize();
    result._planes[size_t(EFrustumPlane::Far)]       = FPlane::FromTriangle(Far3, Far2, Far1).Normalize();
    result._planes[size_t(EFrustumPlane::Left)]      = FPlane::FromTriangle(Near4, Near3, Far3).Normalize();
    result._planes[size_t(EFrustumPlane::Right)]     = FPlane::FromTriangle(Far1, Far2, Near2).Normalize();
    result._planes[size_t(EFrustumPlane::Top)]       = FPlane::FromTriangle(Near2, Far2, Far3).Normalize();
    result._planes[size_t(EFrustumPlane::Bottom)]    = FPlane::FromTriangle(Far4, Far1, Near1).Normalize();

    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
