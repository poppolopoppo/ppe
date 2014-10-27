#pragma once

#include "Core/Maths/Geometry/Frustum.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline Frustum::Frustum() {
    SetMatrix(float4x4::Identity());
}
//----------------------------------------------------------------------------
inline Frustum::Frustum(float4x4& viewProjection) {
    SetMatrix(viewProjection);
}
//----------------------------------------------------------------------------
inline void Frustum::GetCorners(float3 (&points)[8]) const {
    return GetCorners(MakeView(points));
}
//----------------------------------------------------------------------------
inline bool Frustum::Intersects(const BoundingBox& box) const {
    return Contains(box) != ContainmentType::Disjoint;
}
//----------------------------------------------------------------------------
inline bool Frustum::Intersects(const Sphere& sphere) const {
    return Contains(sphere) != ContainmentType::Disjoint;
}
//----------------------------------------------------------------------------
inline bool Frustum::Intersects(const Frustum& frustum) const {
    return Contains(frustum) != ContainmentType::Disjoint;
}
//----------------------------------------------------------------------------
inline float3 Frustum::GetZoomToExtentsShiftVector(const MemoryView<const float3>& points) {
    return Near().Normal() *  GetZoomToExtentsShiftDistance(points);
}
//----------------------------------------------------------------------------
inline float3 Frustum::GetZoomToExtentsShiftVector(const BoundingBox& box) {
    return Near().Normal() *  GetZoomToExtentsShiftDistance(box);
}
//----------------------------------------------------------------------------
inline Frustum Frustum::FromCameraParams(const FrustumCameraParams& params) {
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
