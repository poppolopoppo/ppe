#pragma once

#include "Core.h"

#include "Maths/Collision.h"
#include "Maths/Plane.h"
#include "Maths/ScalarBoundingBox.h"
#include "Maths/ScalarVector_fwd.h"
#include "Maths/ScalarMatrix.h"

namespace PPE {
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
    inline FFrustum();
    inline explicit FFrustum(float4x4& viewProjection); // world space frustum RH

    PPE_CORE_API FFrustum(const FFrustum& other);
    PPE_CORE_API FFrustum& operator =(const FFrustum& other);

    PPE_CORE_API void SetMatrixLH(const float4x4& viewProjection);
    PPE_CORE_API void SetMatrixRH(const float4x4& viewProjection);
    void SetMatrix(const float4x4& viewProjection) { SetMatrixRH(viewProjection); }

    NODISCARD const float4x4& Matrix() const { return _matrix; }

    NODISCARD const FPlane& Near() const { return _planes[static_cast<size_t>(EFrustumPlane::Near)]; }
    NODISCARD const FPlane& Far() const { return _planes[static_cast<size_t>(EFrustumPlane::Far)]; }

    NODISCARD const FPlane& Left() const { return _planes[static_cast<size_t>(EFrustumPlane::Left)]; }
    NODISCARD const FPlane& Right() const { return _planes[static_cast<size_t>(EFrustumPlane::Right)]; }

    NODISCARD const FPlane& Top() const { return _planes[static_cast<size_t>(EFrustumPlane::Top)]; }
    NODISCARD const FPlane& Bottom() const { return _planes[static_cast<size_t>(EFrustumPlane::Bottom)]; }

    NODISCARD TMemoryView<const float3> MakeCorners() const { return MakeView(_corners); }
    NODISCARD const FBoundingBox& GetBoundingBox() const { return _box; }

    PPE_CORE_API void GetCameraParams(FFrustumCameraParams& params) const;

    NODISCARD PPE_CORE_API EContainmentType Contains(const float3& point) const;
    NODISCARD PPE_CORE_API EContainmentType Contains(const TMemoryView<const float3>& points) const;
    NODISCARD PPE_CORE_API EContainmentType Contains(const FBoundingBox& box) const;
    NODISCARD PPE_CORE_API EContainmentType Contains(const FSphere& sphere) const;
    NODISCARD PPE_CORE_API EContainmentType Contains(const FFrustum& frustum) const;

    NODISCARD PPE_CORE_API EContainmentType ContainsConvexCube(const float3 (&points)[8]) const;

    NODISCARD PPE_CORE_API EPlaneIntersectionType Intersects(const FPlane& plane) const;
    NODISCARD inline bool Intersects(const FBoundingBox& box) const;
    NODISCARD inline bool Intersects(const FSphere& sphere) const;
    NODISCARD inline bool Intersects(const FFrustum& frustum) const;

    NODISCARD PPE_CORE_API bool Intersects(const FRay& ray) const;
    NODISCARD PPE_CORE_API bool Intersects(const FRay& ray, float& in, float& out) const;

    NODISCARD PPE_CORE_API float GetWidthAtDepth(float depth) const;
    NODISCARD PPE_CORE_API float GetHeightAtDepth(float depth) const;

    // Get the distance which when added to camera position along the lookat direction will do the effect of zoom to extents (zoom to fit) operation,
    // so all the passed points will fit in the current view.
    // if the returned value is poistive, the camera will move toward the lookat direction (ZoomIn).
    // if the returned value is negative, the camera will move in the revers direction of the lookat direction (ZoomOut).
    NODISCARD PPE_CORE_API float GetZoomToExtentsShiftDistance(const TMemoryView<const float3>& points) const;
    NODISCARD PPE_CORE_API float GetZoomToExtentsShiftDistance(const FBoundingBox& box);

    NODISCARD inline float3 GetZoomToExtentsShiftVector(const TMemoryView<const float3>& points);
    NODISCARD inline float3 GetZoomToExtentsShiftVector(const FBoundingBox& box);

    NODISCARD inline static FFrustum FromCameraParams(const FFrustumCameraParams& params);
    NODISCARD PPE_CORE_API static FFrustum FromCamera(const float3& cameraPos, const float3& lookDir, const float3& upDir, float fov, float znear, float zfar, float aspect);

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
} //!namespace PPE

#include "Maths/Frustum-inl.h"
