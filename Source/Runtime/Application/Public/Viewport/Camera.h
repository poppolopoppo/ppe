#pragma once

#include "Application_fwd.h"

#include "Maths/Frustum.h"
#include "Maths/MathHelpers.h"
#include "Maths/ScalarMatrix.h"
#include "Maths/ScalarRectangle_fwd.h"
#include "Maths/ScalarVector.h"

#include "Memory/RefPtr.h"
#include "Thread/ThreadSafe.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------- -----------------------------------------------------
enum class ECameraProjection : u8 {
    Perspective = 0,
    Orthographic,
};
//----------------------------------------------------------------------------
class FCameraModel {
public:
    float3 Position{ float3::Zero };

    float3 Right{ float3::X };
    float3 Up{ float3::Y };
    float3 Forward{ float3::Z };

    float Fov{ PIOver3 };
    float ZNear{ 0.01f };
    float ZFar{ 10000.f };

    bool bCameraCut{ false }; // true when camera initialized/teleported
};
//----------------------------------------------------------------------------
class FCamera : public FRefCountable {
public:
    struct FState {
        FFrustum Frustum;

        float4x4 Projection{ float4x4::Identity() };
        float4x4 View{ float4x4::Identity() };

        float4x4 InvertProjection{ float4x4::Identity() };
        float4x4 InvertView{ float4x4::Identity() };

        float4x4 InvertViewProjection{ float4x4::Identity() };

        FCameraModel Model;
    };

    FCamera() = default;

    NODISCARD ECameraProjection Mode() const { return _data.LockShared()->Mode; }
    void SetMode(ECameraProjection value) { _data.LockExclusive()->Mode = value; }

    NODISCARD const FState& CurrentState() const { return _data.LockShared()->CurrentState; }
    NODISCARD const FState& PreviousState() const { return _data.LockShared()->PreviousState; }

    NODISCARD const FFrustum& Frustum() const { return _data.LockShared()->CurrentState.Frustum; }

    NODISCARD const float4x4& Projection() const { return _data.LockShared()->CurrentState.Projection; }
    NODISCARD const float4x4& View() const { return _data.LockShared()->CurrentState.View; }
    NODISCARD const float4x4& ViewProjection() const { return _data.LockShared()->CurrentState.Frustum.Matrix(); }

    NODISCARD const float4x4& InvertProjection() const { return _data.LockShared()->CurrentState.InvertProjection; }
    NODISCARD const float4x4& InvertView() const { return _data.LockShared()->CurrentState.InvertView; }
    NODISCARD const float4x4& InvertViewProjection() const { return _data.LockShared()->CurrentState.InvertViewProjection; }

    NODISCARD const float3& Up() const { return _data.LockShared()->CurrentState.Model.Up; }
    NODISCARD const float3& Forward() const { return _data.LockShared()->CurrentState.Model.Forward; }
    NODISCARD const float3& Right() const { return _data.LockShared()->CurrentState.Model.Right; }

    NODISCARD const float3& Position() const { return _data.LockShared()->CurrentState.Model.Position; }

    NODISCARD float ZFar() const { return _data.LockShared()->CurrentState.Model.ZFar; }
    NODISCARD float ZNear() const { return _data.LockShared()->CurrentState.Model.ZNear; }

    PPE_APPLICATION_API void UpdateModel(const FCameraModel& model, const FRectangle2f& viewport) NOEXCEPT;

private:
    struct FInternalData_ {
        FState CurrentState;
        FState PreviousState;

        ECameraProjection Mode{ ECameraProjection::Perspective };
    };

    TThreadSafe<FInternalData_, EThreadBarrier::RWDataRaceCheck> _data;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
