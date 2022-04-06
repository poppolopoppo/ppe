#include "stdafx.h"

#include "Maths/Frustum.h"

#include "Maths/ScalarMatrix.h"
#include "Maths/ScalarMatrixHelpers.h"
#include "Maths/Ray.h"
#include "Maths/ScalarBoundingBox.h"
#include "Maths/ScalarBoundingBoxHelpers.h"
#include "Maths/ScalarVector.h"
#include "Maths/ScalarVectorHelpers.h"
#include "Maths/Sphere.h"

#include "Memory/MemoryView.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static void ComputeFrustumCorners_(const FPlane (&planes)[6], float3 (&points)[8]) {
    const FPlane& ptop = planes[size_t(EFrustumPlane::Top)];
    const FPlane& pbottom = planes[size_t(EFrustumPlane::Bottom)];

    const FPlane& pleft = planes[size_t(EFrustumPlane::Left)];
    const FPlane& pright = planes[size_t(EFrustumPlane::Right)];

    const FPlane& pnear = planes[size_t(EFrustumPlane::Near)];
    const FPlane& pfar = planes[size_t(EFrustumPlane::Far)];

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
static EContainmentType FrustumContainsConvexVolume_(
    const FPlane (&frustumPlanes)[6], const float3 (&frustumCorners)[8],
    const FBoundingBox& convexBounds, const float3 (&convexCorners)[8]
    ) {
    // http://www.iquilezles.org/www/articles/frustumcorrect/frustumcorrect.htm
    size_t inside = 0;

    // check box outside/inside of frustum
    for (const FPlane& plane : frustumPlanes)
    {
        size_t out = 0;

        for (const float3& p : convexCorners)
          out += (plane.DistanceToPoint(p) < 0.0f ? 1 : 0);

        if (8 == out)
            return EContainmentType::Disjoint;
        else if (0 == out)
            inside++;
    }

    if (6 == inside)
        return EContainmentType::Contains;

    // check frustum outside/inside box
    size_t out;

    out=0; for (const float3& p : frustumCorners) out += ((p.x > convexBounds.Max().x) ? 1 : 0); if (8 == out) return EContainmentType::Disjoint;
    out=0; for (const float3& p : frustumCorners) out += ((p.x < convexBounds.Min().x) ? 1 : 0); if (8 == out) return EContainmentType::Disjoint;

    out=0; for (const float3& p : frustumCorners) out += ((p.y > convexBounds.Max().y) ? 1 : 0); if (8 == out) return EContainmentType::Disjoint;
    out=0; for (const float3& p : frustumCorners) out += ((p.y < convexBounds.Min().y) ? 1 : 0); if (8 == out) return EContainmentType::Disjoint;

    out=0; for (const float3& p : frustumCorners) out += ((p.z > convexBounds.Max().z) ? 1 : 0); if (8 == out) return EContainmentType::Disjoint;
    out=0; for (const float3& p : frustumCorners) out += ((p.z < convexBounds.Min().z) ? 1 : 0); if (8 == out) return EContainmentType::Disjoint;

    return EContainmentType::Intersects;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FFrustum::FFrustum(const FFrustum& other)
:   _matrix(other._matrix)
,   _box(other._box) {
    for (size_t i = 0; i < 6; ++i)
        _planes[i] = other._planes[i];
    for (size_t i = 0; i < 8; ++i)
        _corners[i] = other._corners[i];
}
//----------------------------------------------------------------------------
FFrustum& FFrustum::operator =(const FFrustum& other) {
    _matrix = other._matrix;
    _box = other._box;
    for (size_t i = 0; i < 6; ++i)
        _planes[i] = other._planes[i];
    for (size_t i = 0; i < 8; ++i)
        _corners[i] = other._corners[i];

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
    pleft.Normal().x  = _matrix._14() + _matrix._11();
    pleft.Normal().y  = _matrix._24() + _matrix._21();
    pleft.Normal().z  = _matrix._34() + _matrix._31();
    pleft.D()           = _matrix._44() + _matrix._41();
    pleft = pleft.Normalize();

    // Right plane
    pright.Normal().x = _matrix._14() - _matrix._11();
    pright.Normal().y = _matrix._24() - _matrix._21();
    pright.Normal().z = _matrix._34() - _matrix._31();
    pright.D()          = _matrix._44() - _matrix._41();
    pright = pright.Normalize();

    // Top plane
    ptop.Normal().x   = _matrix._14() - _matrix._12();
    ptop.Normal().y   = _matrix._24() - _matrix._22();
    ptop.Normal().z   = _matrix._34() - _matrix._32();
    ptop.D()            = _matrix._44() - _matrix._42();
    ptop = ptop.Normalize();

    // Bottom plane
    pbottom.Normal().x= _matrix._14() + _matrix._12();
    pbottom.Normal().y= _matrix._24() + _matrix._22();
    pbottom.Normal().z= _matrix._34() + _matrix._32();
    pbottom.D()         = _matrix._44() + _matrix._42();
    pbottom = pbottom.Normalize();

    // Near plane
    pnear.Normal().x  = _matrix._14() + _matrix._13();
    pnear.Normal().y  = _matrix._24() + _matrix._23();
    pnear.Normal().z  = _matrix._34() + _matrix._33();
    pnear.D()           = _matrix._44() + _matrix._43();
    pnear = pnear.Normalize();

    // Far plane
    pfar.Normal().x   = _matrix._14() - _matrix._13();
    pfar.Normal().y   = _matrix._24() - _matrix._23();
    pfar.Normal().z   = _matrix._34() - _matrix._33();
    pfar.D()            = _matrix._44() - _matrix._43();
    pfar = pfar.Normalize();

    InitProperties_();
}
//----------------------------------------------------------------------------
void FFrustum::GetCameraParams(FFrustumCameraParams& params) const {
    const FPlane& ptop = _planes[size_t(EFrustumPlane::Top)];
    //const FPlane& pbottom = _planes[size_t(EFrustumPlane::Bottom)];

    const FPlane& pleft = _planes[size_t(EFrustumPlane::Left)];
    const FPlane& pright = _planes[size_t(EFrustumPlane::Right)];

    const FPlane& pnear = _planes[size_t(EFrustumPlane::Near)];
    const FPlane& pfar = _planes[size_t(EFrustumPlane::Far)];

    params.Position = FPlane::Get3PlanesInterPoint(pright, ptop, pleft);
    params.LookAtDir = pnear.Normal();
    params.UpDir = Normalize(Cross(pright.Normal(), pnear.Normal()));
    params.FOV = (F_HalfPi - std::acos(Dot(pnear.Normal(), ptop.Normal()))) * 2;
    params.AspectRatio = Length(_corners[6] - _corners[5]) / Length(_corners[4] - _corners[5]);
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
EContainmentType FFrustum::Contains(const FBoundingBox& box) const {
    float3 boxCorners[8];
    box.MakeCorners(boxCorners);
    return FrustumContainsConvexVolume_(_planes, _corners, box, boxCorners);
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
    return FrustumContainsConvexVolume_(_planes, _corners, frustum._box, frustum._corners);
}
//----------------------------------------------------------------------------
EContainmentType FFrustum::ContainsConvexCube(const float3 (&points)[8]) const {
    FBoundingBox box = MakeBoundingBox(MakeView(points));
    return FrustumContainsConvexVolume_(_planes, _corners, box, points);
}
//----------------------------------------------------------------------------
EPlaneIntersectionType FFrustum::Intersects(const FPlane& plane) const {
    return FPlane::PointsIntersection(plane, _corners);
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
        const bool planeResult = plane.Intersects(ray, &planeDistance);

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
    float hAngle = (F_HalfPi - std::acos(Dot(Near().Normal(), Left().Normal())) );
    return std::tan(hAngle) * depth * 2;
}
//----------------------------------------------------------------------------
float FFrustum::GetHeightAtDepth(float depth) const {
    float vAngle = (F_HalfPi - std::acos(Dot(Near().Normal(), Top().Normal())) );
    return std::tan(vAngle) * depth * 2;
}
//----------------------------------------------------------------------------
float FFrustum::GetZoomToExtentsShiftDistance(const TMemoryView<const float3>& points) const {
    float vAngle = (F_HalfPi - std::acos(Dot(Near().Normal(), Top().Normal())) );
    float vSin = std::sin(vAngle);
    Assert(std::abs(vSin) > F_Epsilon);

    float hAngle = (F_HalfPi - std::acos(Dot(Near().Normal(), Left().Normal())) );
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
float FFrustum::GetZoomToExtentsShiftDistance(const FBoundingBox& box) {
    float3 boxCorners[8];
    box.MakeCorners(boxCorners);
    return GetZoomToExtentsShiftDistance(boxCorners);
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

    float3 rightDir = Normalize(Cross(upDir, lookDir));
    float3 Near1 = nearCenter - nearHalfHeight * upDir + nearHalfWidth * rightDir;
    float3 Near2 = nearCenter + nearHalfHeight * upDir + nearHalfWidth * rightDir;
    float3 Near3 = nearCenter + nearHalfHeight * upDir - nearHalfWidth * rightDir;
    float3 Near4 = nearCenter - nearHalfHeight * upDir - nearHalfWidth * rightDir;
    float3 Far1 = farCenter - farHalfHeight * upDir + farHalfWidth * rightDir;
    float3 Far2 = farCenter + farHalfHeight * upDir + farHalfWidth * rightDir;
    float3 Far3 = farCenter + farHalfHeight * upDir - farHalfWidth * rightDir;
    float3 Far4 = farCenter - farHalfHeight * upDir - farHalfWidth * rightDir;

    FFrustum result;

    result._matrix = (
        MakeLookAtLHMatrix(cameraPos, float3(cameraPos + lookDir * 10.f), upDir) *
        MakePerspectiveFovLHMatrix(fov, aspect, znear, zfar));

    result._planes[size_t(EFrustumPlane::Near)]      = FPlane::FromTriangle(Near1, Near2, Near3).Normalize();
    result._planes[size_t(EFrustumPlane::Far)]       = FPlane::FromTriangle(Far3, Far2, Far1).Normalize();
    result._planes[size_t(EFrustumPlane::Left)]      = FPlane::FromTriangle(Near4, Near3, Far3).Normalize();
    result._planes[size_t(EFrustumPlane::Right)]     = FPlane::FromTriangle(Far1, Far2, Near2).Normalize();
    result._planes[size_t(EFrustumPlane::Top)]       = FPlane::FromTriangle(Near2, Far2, Far3).Normalize();
    result._planes[size_t(EFrustumPlane::Bottom)]    = FPlane::FromTriangle(Far4, Far1, Near1).Normalize();

    result.InitProperties_();

    return result;
}
//----------------------------------------------------------------------------
void FFrustum::InitProperties_() {
    ComputeFrustumCorners_(_planes, _corners);
    _box = MakeBoundingBox(MakeConstView(_corners));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
