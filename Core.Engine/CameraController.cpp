#include "stdafx.h"

#include "CameraController.h"

#include "Core/MathHelpers.h"
#include "Core/Quaternion.h"
#include "Core/QuaternionHelpers.h"
#include "Core/ScalarMatrixHelpers.h"
#include "Core/Timeline.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ICameraController::ICameraController()
:   _view(float4x4::Identity()) {}
//----------------------------------------------------------------------------
ICameraController::~ICameraController() {}
//----------------------------------------------------------------------------
void ICameraController::CurrentView(float4x4 *view) const {
    Assert(view);
    *view = _view;
}
//----------------------------------------------------------------------------
void ICameraController::Update(const Timeline& time) {
    UpdateImpl(&_view, time);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FreeLookCameraController::FreeLookCameraController(const float3& position, float heading, float pitch) {
    LookAt(position, heading, pitch);
}
//----------------------------------------------------------------------------
FreeLookCameraController::~FreeLookCameraController() {}
//----------------------------------------------------------------------------
void FreeLookCameraController::LookAt(const float3& position, float heading, float pitch) {
    _position = position;
    _heading = std::fmod(heading, F_2PI);
    _pitch = std::fmod(pitch, F_2PI);
}
//----------------------------------------------------------------------------
void FreeLookCameraController::UpdateImpl(float4x4 *view, const Timeline& time) {
    Assert(view);

    const Quaternion rotation = MakeYawPitchRollQuaternion(_heading, _pitch, 0.0f);

    const float3 dU(0.0f, 1.0f, 0.0f);
    const float3 dV(0.0f, 0.0f, 1.0f);

    _up = Normalize3(rotation.Transform(dU));
    _forward = Normalize3(rotation.Transform(dV));
    _right = Normalize3(Cross(_up, _forward));

    *view = MakeLookAtLHMatrix(_position, _position + _forward, _up);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
