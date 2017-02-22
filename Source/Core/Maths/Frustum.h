#pragma once

#include "Core/Core.h"

#include "Core/Maths/Collision.h"
#include "Core/Maths/Plane.h"
#include "Core/Maths/ScalarBoundingBox.h"
#include "Core/Maths/ScalarVector_fwd.h"
#include "Core/Maths/ScalarMatrix.h"

namespace Core {
template <typename T>
class TMemoryView;

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EFrustumPlane {
    Near = 0,
    Far,
    Left,
    Right,
    Top,
    Bottom
};
//----------------------------------------------------------------------------
enum class EFrustumCorner {
    Near_LeftTop = 0,
    Near_LeftBottom,
    Near_RightBottom,
    Near_RightTop,

    Far_LeftTop,
    Far_LeftBottom,
    Far_RightBottom,
    Far_RightTop,
};
//----------------------------------------------------------------------------
struct FFrustumCameraParams {
    float3  Position;
    float3  LookAtDir;
    float3  UpDir;
    float   FOV;
    float   ZNear;
    float   ZFar;
    float   AspectRatio;
};
//----------------------------------------------------------------------------
class FFrustum {
public:
    FFrustum();
    explicit FFrustum(float4x4& viewProjection);

    FFrustum(const FFrustum& other);
    FFrustum& operator =(const FFrustum& other);

    void SetMatrix(const float4x4& viewProjection);
    const float4x4& Matrix() const { return _matrix; }

    const FPlane& Near() const { return _planes[size_t(EFrustumPlane::Near)]; }
    const FPlane& Far() const { return _planes[size_t(EFrustumPlane::Far)]; }

    const FPlane& Left() const { return _planes[size_t(EFrustumPlane::Left)]; }
    const FPlane& Right() const { return _planes[size_t(EFrustumPlane::Right)]; }

    const FPlane& Top() const { return _planes[size_t(EFrustumPlane::Top)]; }
    const FPlane& Bottom() const { return _planes[size_t(EFrustumPlane::Bottom)]; }

    bool IsOrthographic() const;

    TMemoryView<const float3> GetCorners() const { return MakeView(_corners); }
    const FBoundingBox& GetBoundingBox() const { return _box; }

    void GetCameraParams(FFrustumCameraParams& params) const;

    EContainmentType Contains(const float3& point) const;
    EContainmentType Contains(const TMemoryView<const float3>& points) const;
    EContainmentType Contains(const FBoundingBox& box) const;
    EContainmentType Contains(const FSphere& sphere) const;
    EContainmentType Contains(const FFrustum& frustum) const;

    EContainmentType ContainsConvexCube(const float3 (&points)[8]) const;

    EPlaneIntersectionType Intersects(const FPlane& plane) const;
    bool Intersects(const FBoundingBox& box) const;
    bool Intersects(const FSphere& sphere) const;
    bool Intersects(const FFrustum& frustum) const;

    bool Intersects(const FRay& ray) const;
    bool Intersects(const FRay& ray, float& in, float& out) const;

    float GetWidthAtDepth(float depth) const;
    float GetHeightAtDepth(float depth) const;

    // Get the distance which when added to camera position along the lookat direction will do the effect of zoom to extents (zoom to fit) operation,
    // so all the passed points will fit in the current view.
    // if the returned value is poistive, the camera will move toward the lookat direction (ZoomIn).
    // if the returned value is negative, the camera will move in the revers direction of the lookat direction (ZoomOut).
    float GetZoomToExtentsShiftDistance(const TMemoryView<const float3>& points) const;
    float GetZoomToExtentsShiftDistance(const FBoundingBox& box);

    float3 GetZoomToExtentsShiftVector(const TMemoryView<const float3>& points);
    float3 GetZoomToExtentsShiftVector(const FBoundingBox& box);

    static FFrustum FromCameraParams(const FFrustumCameraParams& params);
    static FFrustum FromCamera(const float3& cameraPos, const float3& lookDir, const float3& upDir, float fov, float znear, float zfar, float aspect);

private:
    float4x4 _matrix;
    FPlane _planes[6];
    float3 _corners[8];
    FBoundingBox _box;

    void InitProperties_();
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Maths/Frustum-inl.h"
