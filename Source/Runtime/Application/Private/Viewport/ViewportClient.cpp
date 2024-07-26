// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Viewport/ViewportClient.h"

#include "Viewport/Camera.h"
#include "Viewport/CameraController.h"
#include "Window/MainWindow.h"

#include "Maths/Ray.h"
#include "Maths/ScalarMatrixHelpers.h"
#include "Maths/ScalarRectangle.h"
#include "Maths/ScalarVectorHelpers.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FViewportClient::FViewportClient(
    const PCamera& camera,
    const PMainWindow& window,
    Meta::TOptional<FRectangle2i> clientRect,
    EViewportFlags viewportFlags) NOEXCEPT
:   _camera(camera)
,   _viewport(window, clientRect.value_or(FRectangle2i{int2(window->Dimensions())}), viewportFlags) {
    _viewport.Window()->AddListener(this);
}
//----------------------------------------------------------------------------
FViewportClient::~FViewportClient() {
    _viewport.Window()->RemoveListener(this);
}
//----------------------------------------------------------------------------
float3 FViewportClient::WorldToClip(const float3& worldPos) const NOEXCEPT {
    return TransformPosition3(_camera->ViewProjection(), worldPos);
}
//----------------------------------------------------------------------------
int2 FViewportClient::WorldToClient(const float3& worldPos) const NOEXCEPT {
    const float3 clipPos = WorldToClip(worldPos);

    float2 texCoord = (clipPos.xy + 1.f) * 0.5f;
    texCoord.y = 1.f - texCoord.y;

    return TruncToInt(Saturate(texCoord) * float2(_viewport.ClientRect().Extents()));
}
//----------------------------------------------------------------------------
FRay FViewportClient::ClipToWorld(const float3& clipPos) const NOEXCEPT {
    Assert_NoAssume(AllGreaterEqual(clipPos, -float3::One));
    Assert_NoAssume(AllLessEqual(clipPos, float3::One));

    const float2 texCoord = Saturate((clipPos.xy + 1.f) * 0.5f);

    const TMemoryView<const float3> frustumCorners = Frustum().MakeCorners();
    const float3 nearPos = BilateralLerp(
        frustumCorners[static_cast<size_t>(EFrustumCorner::Near_LeftBottom)],
        frustumCorners[static_cast<size_t>(EFrustumCorner::Near_RightBottom)],
        frustumCorners[static_cast<size_t>(EFrustumCorner::Near_RightTop)],
        frustumCorners[static_cast<size_t>(EFrustumCorner::Near_LeftTop)],
        texCoord);
    const float3 farPos = BilateralLerp(
        frustumCorners[static_cast<size_t>(EFrustumCorner::Far_LeftBottom)],
        frustumCorners[static_cast<size_t>(EFrustumCorner::Far_RightBottom)],
        frustumCorners[static_cast<size_t>(EFrustumCorner::Far_RightTop)],
        frustumCorners[static_cast<size_t>(EFrustumCorner::Far_LeftTop)],
        texCoord);

    const float3 worldPos = Lerp(nearPos, farPos, Saturate(clipPos.z));
    return FRay{ worldPos, SafeNormalize(farPos - nearPos) };
}
//----------------------------------------------------------------------------
FRay FViewportClient::ClientToWorld(const int2& clientPos) const NOEXCEPT {
    const float2 texCoord = ClientToTexCoord(clientPos);

    const TMemoryView<const float3> frustumCorners = Frustum().MakeCorners();
    const float3 nearPos = BilateralLerp(
        frustumCorners[static_cast<size_t>(EFrustumCorner::Near_LeftBottom)],
        frustumCorners[static_cast<size_t>(EFrustumCorner::Near_RightBottom)],
        frustumCorners[static_cast<size_t>(EFrustumCorner::Near_RightTop)],
        frustumCorners[static_cast<size_t>(EFrustumCorner::Near_LeftTop)],
        texCoord);
    const float3 farPos = BilateralLerp(
        frustumCorners[static_cast<size_t>(EFrustumCorner::Far_LeftBottom)],
        frustumCorners[static_cast<size_t>(EFrustumCorner::Far_RightBottom)],
        frustumCorners[static_cast<size_t>(EFrustumCorner::Far_RightTop)],
        frustumCorners[static_cast<size_t>(EFrustumCorner::Far_LeftTop)],
        texCoord);

    // use farPos instead of cameraPosition to handle perspective and orthographic projection
    //return FRay{nearPos, SafeNormalize(nearPos - Position()) };
    return FRay{ nearPos, SafeNormalize(farPos - nearPos)} ;
}
//----------------------------------------------------------------------------
FRay FViewportClient::ScreenToWorld(const int2& screenPos) const NOEXCEPT {
    return ClientToWorld(ScreenToClient(screenPos));
}
//----------------------------------------------------------------------------
void FViewportClient::Update(FTimespan dt, ICameraController& controller, Meta::TOptional<FRectangle2i> clientRect) {
    // update client rect if window was resized
    if (_windowResized.has_value()) {
        _viewport.SetClientRect(FRectangle2i(checked_cast<int>(*_windowResized)));
        _windowResized.reset();
    }

    // update client rect if client specified one
    if (clientRect.has_value()) {
        _viewport.SetClientRect(*clientRect);
    }

    FCameraModel model = _camera->CurrentState().Model;
    controller.UpdateCamera(dt, model);

    _camera->UpdateModel(model, FRectangle2f{ float2(_viewport.ClientRect().Extents()) });
}
//----------------------------------------------------------------------------
void FViewportClient::OnWindowResize(const uint2& size) NOEXCEPT {
    _windowResized = size;
}
//----------------------------------------------------------------------------
const PMainWindow& FViewportClient::Window() const NOEXCEPT {
    return _viewport.Window();
}
//----------------------------------------------------------------------------
const FRectangle2i& FViewportClient::ClientRect() const NOEXCEPT {
    return _viewport.ClientRect();
}
//----------------------------------------------------------------------------
EViewportFlags FViewportClient::ViewportFlags() const NOEXCEPT {
    return _viewport.Flags();
}
//----------------------------------------------------------------------------
const FFrustum& FViewportClient::Frustum() const NOEXCEPT {
    return _camera->Frustum();
}
//----------------------------------------------------------------------------
const float4x4& FViewportClient::Projection() const NOEXCEPT {
    return _camera->Projection();
}
//----------------------------------------------------------------------------
const float4x4& FViewportClient::View() const NOEXCEPT {
    return _camera->View();
}
//----------------------------------------------------------------------------
const float4x4& FViewportClient::ViewProjection() const NOEXCEPT {
    return _camera->ViewProjection();
}
//----------------------------------------------------------------------------
const float4x4& FViewportClient::InvertProjection() const NOEXCEPT {
    return _camera->InvertProjection();
}
//----------------------------------------------------------------------------
const float4x4& FViewportClient::InvertView() const NOEXCEPT {
    return _camera->InvertView();
}
//----------------------------------------------------------------------------
const float4x4& FViewportClient::InvertViewProjection() const NOEXCEPT {
    return _camera->InvertViewProjection();
}
//----------------------------------------------------------------------------
const float3& FViewportClient::Up() const NOEXCEPT {
    return _camera->Up();
}
//----------------------------------------------------------------------------
const float3& FViewportClient::Forward() const NOEXCEPT {
    return _camera->Forward();
}
//----------------------------------------------------------------------------
const float3& FViewportClient::Right() const NOEXCEPT {
    return _camera->Right();
}
//----------------------------------------------------------------------------
const float3& FViewportClient::Position() const NOEXCEPT {
    return _camera->Position();
}
//----------------------------------------------------------------------------
float FViewportClient::ZFar() const NOEXCEPT {
    return _camera->ZFar();
}
//----------------------------------------------------------------------------
float FViewportClient::ZNear() const NOEXCEPT {
    return _camera->ZNear();
}
//----------------------------------------------------------------------------
int2 FViewportClient::ScreenToClient(const int2& screenPos) const NOEXCEPT {
    return _viewport.ScreenToClient(screenPos);
}
//----------------------------------------------------------------------------
int2 FViewportClient::ClientToScreen(const int2& clientPos) const NOEXCEPT {
    return _viewport.ClientToScreen(clientPos);
}
//----------------------------------------------------------------------------
float2 FViewportClient::ClientToTexCoord(const int2& clientPos) const NOEXCEPT {
    return _viewport.ClientToTexCoord(clientPos);
}
//----------------------------------------------------------------------------
int2 FViewportClient::TexCoordToClient(const float2& texCoord) const NOEXCEPT {
    return _viewport.TexCoordToClient(texCoord);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
