#pragma once

#include "Application_fwd.h"

#include "Viewport/Viewport.h"

#include "Maths/ScalarRectangle.h"
#include "Meta/Optional.h"
#include "Misc/Event.h"
#include "Thread/ThreadSafe.h"
#include "Time/Time_fwd.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class EMPTY_BASES FViewportClient : public FRefCountable {
public:
    FViewportClient() = default;
    PPE_APPLICATION_API virtual ~FViewportClient();

    PPE_APPLICATION_API FViewportClient(
        const PCamera& camera,
        const PMainWindow& window,
        Meta::TOptional<FRectangle2i> clientRect = std::nullopt,
        EViewportFlags viewportFlags = Default) NOEXCEPT;

    NODISCARD const PCamera& Camera() const { return _camera; }
    NODISCARD const FViewport& Viewport() const { return _viewport; }

    NODISCARD PPE_APPLICATION_API const PMainWindow& Window() const NOEXCEPT;
    NODISCARD PPE_APPLICATION_API const FRectangle2i& ClientRect() const NOEXCEPT;
    NODISCARD PPE_APPLICATION_API EViewportFlags ViewportFlags() const NOEXCEPT;
    NODISCARD PPE_APPLICATION_API const FFrustum& Frustum() const NOEXCEPT;

    NODISCARD PPE_APPLICATION_API const float4x4& Projection() const NOEXCEPT;
    NODISCARD PPE_APPLICATION_API const float4x4& View() const NOEXCEPT;
    NODISCARD PPE_APPLICATION_API const float4x4& ViewProjection() const NOEXCEPT;

    NODISCARD PPE_APPLICATION_API const float4x4& InvertProjection() const NOEXCEPT;
    NODISCARD PPE_APPLICATION_API const float4x4& InvertView() const NOEXCEPT;
    NODISCARD PPE_APPLICATION_API const float4x4& InvertViewProjection() const NOEXCEPT;

    NODISCARD PPE_APPLICATION_API const float3& Up() const NOEXCEPT;
    NODISCARD PPE_APPLICATION_API const float3& Forward() const NOEXCEPT;
    NODISCARD PPE_APPLICATION_API const float3& Right() const NOEXCEPT;

    NODISCARD PPE_APPLICATION_API const float3& Position() const NOEXCEPT;

    NODISCARD PPE_APPLICATION_API float ZFar() const NOEXCEPT;
    NODISCARD PPE_APPLICATION_API float ZNear() const NOEXCEPT;

    NODISCARD PPE_APPLICATION_API float3 WorldToClip(const float3& worldPos) const NOEXCEPT;
    NODISCARD PPE_APPLICATION_API int2 WorldToClient(const float3& worldPos) const NOEXCEPT;
    NODISCARD int2 WorldToScreen(const float3& worldPos) const NOEXCEPT { return ClientToScreen(WorldToClient(worldPos)); }

    NODISCARD PPE_APPLICATION_API FRay ClipToWorld(const float3& clipPos) const NOEXCEPT;
    NODISCARD PPE_APPLICATION_API FRay ClientToWorld(const int2& clientPos) const NOEXCEPT;
    NODISCARD PPE_APPLICATION_API FRay ScreenToWorld(const int2& screenPos) const NOEXCEPT;

    NODISCARD PPE_APPLICATION_API int2 ScreenToClient(const int2& screenPos) const NOEXCEPT;
    NODISCARD PPE_APPLICATION_API int2 ClientToScreen(const int2& clientPos) const NOEXCEPT;

    NODISCARD PPE_APPLICATION_API float2 ClientToTexCoord(const int2& clientPos) const NOEXCEPT;
    NODISCARD PPE_APPLICATION_API int2 TexCoordToClient(const float2& texCoord) const NOEXCEPT;

    PPE_APPLICATION_API virtual void Update(FTimespan dt, ICameraController& controller, Meta::TOptional<FRectangle2i> clientRect = std::nullopt);

private:
    PCamera _camera;
    FViewport _viewport;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
