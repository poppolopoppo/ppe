#pragma once

#include "Core/Core.h"

#include "Core/Maths/Collision.h"
#include "Core/Maths/Plane.h"
#include "Core/Maths/ScalarVector_fwd.h"
#include "Core/Maths/ScalarMatrix.h"

namespace Core {
template <typename T>
class MemoryView;

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class FrustumPlane {
    Near = 0,
    Far,
    Left,
    Right,
    Top,
    Bottom
};
//----------------------------------------------------------------------------
enum class FrustumCorner {
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
struct FrustumCameraParams {
    float3  Position;
    float3  LookAtDir;
    float3  UpDir;
    float   FOV;
    float   ZNear;
    float   ZFar;
    float   AspectRatio;
};
//----------------------------------------------------------------------------
class Frustum {
public:
    Frustum();
    explicit Frustum(float4x4& viewProjection);

    Frustum(const Frustum& other);
    Frustum& operator =(const Frustum& other);

    void SetMatrix(const float4x4& viewProjection);
    const float4x4& Matrix() const { return _matrix; }

    const Plane& Near() const { return _planes[size_t(FrustumPlane::Near)]; }
    const Plane& Far() const { return _planes[size_t(FrustumPlane::Far)]; }

    const Plane& Left() const { return _planes[size_t(FrustumPlane::Left)]; }
    const Plane& Right() const { return _planes[size_t(FrustumPlane::Right)]; }

    const Plane& Top() const { return _planes[size_t(FrustumPlane::Top)]; }
    const Plane& Bottom() const { return _planes[size_t(FrustumPlane::Bottom)]; }

    bool IsOrthographic() const;

    void GetCorners(float3 (&points)[8]) const;
    void GetCorners(const MemoryView<float3>& points) const;
    void GetCameraParams(FrustumCameraParams& params) const;

    ContainmentType Contains(const float3& point) const;
    ContainmentType Contains(const MemoryView<const float3>& points) const;
    ContainmentType Contains(const BoundingBox& box) const;
    ContainmentType Contains(const Sphere& sphere) const;
    ContainmentType Contains(const Frustum& frustum) const;

    PlaneIntersectionType Intersects(const Plane& plane) const;
    bool Intersects(const BoundingBox& box) const;
    bool Intersects(const Sphere& sphere) const;
    bool Intersects(const Frustum& frustum) const;

    bool Intersects(const Ray& ray) const;
    bool Intersects(const Ray& ray, float& in, float& out) const;

    float GetWidthAtDepth(float depth) const;
    float GetHeightAtDepth(float depth) const;

    // Get the distance which when added to camera position along the lookat direction will do the effect of zoom to extents (zoom to fit) operation,
    // so all the passed points will fit in the current view.
    // if the returned value is poistive, the camera will move toward the lookat direction (ZoomIn).
    // if the returned value is negative, the camera will move in the revers direction of the lookat direction (ZoomOut).
    float GetZoomToExtentsShiftDistance(const MemoryView<const float3>& points) const;
    float GetZoomToExtentsShiftDistance(const BoundingBox& box);

    float3 GetZoomToExtentsShiftVector(const MemoryView<const float3>& points);
    float3 GetZoomToExtentsShiftVector(const BoundingBox& box);

    static Frustum FromCameraParams(const FrustumCameraParams& params);
    static Frustum FromCamera(const float3& cameraPos, const float3& lookDir, const float3& upDir, float fov, float znear, float zfar, float aspect);

private:
    float4x4 _matrix;
    Plane _planes[6];
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Maths/Frustum-inl.h"
