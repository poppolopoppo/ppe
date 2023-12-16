#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Maths/Frustum.h"
#include "Core/Maths/ScalarVector.h"
#include "Core/Maths/ScalarMatrix.h"

namespace Core {
namespace Engine {
class ICamera;
class ICameraController;

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class ECameraRay {
    LeftTop = 0,
    LeftBottom,
    RightBottom,
    RightTop
};
//----------------------------------------------------------------------------
class FCameraModel {
public:
    FCameraModel();
    ~FCameraModel();

    FCameraModel(const FCameraModel& ) = delete;
    FCameraModel& operator =(const FCameraModel& ) = delete;

    const float4x4& View() const { return _view; }
    const float4x4& Projection() const { return _projection; }
    const float4x4& ViewProjection() const { return _viewProjection; }

    const float4x4& InvertView() const { return _invertView; }
    const float4x4& InvertProjection() const { return _invertView; }
    const float4x4& InvertViewProjection() const { return _invertViewProjection; }

    const PPE::FFrustum& FFrustum() const { return _frustum; }
    TMemoryView<const float3> FrustumCorners() const { return MakeView(_frustumCorners); }

    const FFrustumCameraParams& Parameters() const { return _parameters; }

    void GetFrustumRays(float3 (&rays)[4]) const { return GetFrustumRays(MakeView(rays)); }
    void GetFrustumRays(const TMemoryView<float3>& rays) const;

    void Update(const float4x4& view, const float4x4& projection);
    void CopyTo(FCameraModel *dst) const;
    bool Equals(const FCameraModel& other) const;

private:
    float4x4 _view;
    float4x4 _projection;
    float4x4 _viewProjection;

    float4x4 _invertView;
    float4x4 _invertProjection;
    float4x4 _invertViewProjection;

    PPE::FFrustum _frustum;
    float3 _frustumCorners[8];

    FFrustumCameraParams _parameters;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
