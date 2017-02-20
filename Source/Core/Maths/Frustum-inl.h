#pragma once

#include "Core/Maths/Frustum.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline FFrustum::FFrustum() {
    SetMatrix(float4x4::Identity());
}
//----------------------------------------------------------------------------
inline FFrustum::FFrustum(float4x4& viewProjection) {
    SetMatrix(viewProjection);
}
//----------------------------------------------------------------------------
inline void FFrustum::GetCorners(float3 (&points)[8]) const {
    return GetCorners(MakeView(points));
}
//----------------------------------------------------------------------------
inline bool FFrustum::Intersects(const FBoundingBox& box) const {
    return Contains(box) != EContainmentType::Disjoint;
}
//----------------------------------------------------------------------------
inline bool FFrustum::Intersects(const FSphere& sphere) const {
    return Contains(sphere) != EContainmentType::Disjoint;
}
//----------------------------------------------------------------------------
inline bool FFrustum::Intersects(const FFrustum& frustum) const {
    return Contains(frustum) != EContainmentType::Disjoint;
}
//----------------------------------------------------------------------------
inline float3 FFrustum::GetZoomToExtentsShiftVector(const TMemoryView<const float3>& points) {
    return Near().Normal() *  GetZoomToExtentsShiftDistance(points);
}
//----------------------------------------------------------------------------
inline float3 FFrustum::GetZoomToExtentsShiftVector(const FBoundingBox& box) {
    return Near().Normal() *  GetZoomToExtentsShiftDistance(box);
}
//----------------------------------------------------------------------------
inline FFrustum FFrustum::FromCameraParams(const FFrustumCameraParams& params) {
    return FromCamera(  params.Position,
                        params.LookAtDir,
                        params.UpDir,
                        params.FOV,
                        params.ZNear,
                        params.ZFar,
                        params.AspectRatio );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
