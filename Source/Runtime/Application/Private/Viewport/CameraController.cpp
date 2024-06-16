// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Viewport/CameraController.h"

#include "Input/Action/InputAction.h"
#include "Viewport/Camera.h"

#include "Maths/QuaternionHelpers.h"
#include "Maths/ScalarMatrixHelpers.h"
#include "Meta/Functor.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Free look camera
//----------------------------------------------------------------------------
FFreeLookCameraController::FFreeLookCameraController()
:   _fovInput(NEW_REF(Input, FInputAction, "CameraFov", EInputValueType::Axis1D))
,   _lookInput(NEW_REF(Input, FInputAction, "CameraLook", EInputValueType::Digital))
,   _moveInput(NEW_REF(Input, FInputAction, "CameraMove", EInputValueType::Axis2D))
,   _rotateInput(NEW_REF(Input, FInputAction, "CameraRotate", EInputValueType::Axis2D))
,   _speedInput(NEW_REF(Input, FInputAction, "CameraSpeed", EInputValueType::Axis1D)){
    // fov
    _fovInput->OnTriggered().Emplace([this](const FInputActionInstance& action, const FInputKey&) {
         _fov.AddClamp(action.Axis1D().Relative, _fovMinMax.x, _fovMinMax.y);
    });
    // translate
    _lookInput->OnTriggered().Emplace([this](const FInputActionInstance& action, const FInputKey&) {
         _bMouseLook = action.Digital();
    });
    // translate
    _moveInput->OnTriggered().Emplace([this](const FInputActionInstance& action, const FInputKey&) {
         Translate(action.Axis3D().Absolute);
    });
    // rotate
    _rotateInput->OnTriggered().Emplace([this](const FInputActionInstance& action, const FInputKey& key) {
        if (not key.IsMouseKey())
            Rotate(action.Axis2D().Absolute);
        else if (_bMouseLook)
            Rotate(action.Axis2D().Relative);
    });
    // speed
    _speedInput->OnTriggered().Emplace([this](const FInputActionInstance& action, const FInputKey& ) {
        _speedMultiplier.AddClamp(action.Axis1D().Relative, _speedMultiplierMinMax.x, _speedMultiplierMinMax.y);
    });
}
//----------------------------------------------------------------------------
void FFreeLookCameraController::SetFov(float value) NOEXCEPT {
    Assert(value >= _fovMinMax.x && value <= _fovMinMax.y);

    _fov.Reset(value);
}
//----------------------------------------------------------------------------
void FFreeLookCameraController::LookAt(const float3& position, float heading, float pitch, bool bTeleport) NOEXCEPT {
    Assert_NoAssume(not IsNANorINF(position));

    if (bTeleport) {
        _bTeleported = true;
        _position.Reset(position);
        _rotation.Reset(MakeYawPitchRollQuaternion(heading, pitch, 0.f));
    }
    else {
        _position.SetRaw(position);
        _rotation.SetRaw(MakeYawPitchRollQuaternion(heading, pitch, 0.f));
    }
}
//----------------------------------------------------------------------------
void FFreeLookCameraController::UpdateCamera(FTimespan dt, FCameraModel& model) {
    Unused(dt);

    _fov.Update(dt);
    _speedMultiplier.Update(dt);

    if (!_bTeleported && AnyGreater(Abs(_deltaRotation), float2::Zero)) {
        _rotation.SetRaw(MakeYawPitchRollQuaternion(_deltaRotation.x, _deltaRotation.y, 0.f) * _rotation.Raw());
    }

    _rotation.Update(dt);

    if (!_bTeleported && AnyGreater(Abs(_deltaPosition), float3::Zero)) {
        _position.Add(_rotation.Raw().Transform(_deltaPosition) * *_speedMultiplier);
    }

    _position.Update(dt);

    const FQuaternion rotation = *_rotation;

    model.Fov = *_fov;
    model.Position = *_position;
    model.Right = rotation.Transform(float3::X);
    model.Up = -rotation.Transform(float3::Y);
    model.Forward = rotation.Transform(float3::Z);
    model.bCameraCut = _bTeleported;

    _bMouseLook = false;
    _bTeleported = false;
    _deltaPosition = float3::Zero;
    _deltaRotation = float2::Zero;
}
//----------------------------------------------------------------------------
void FFreeLookCameraController::InputActionKeyMappings(TAppendable<FInputActionKeyMapping> keyMappings) const NOEXCEPT {

    const auto fovModifier = [this](float delta) NOEXCEPT -> FInputAction::FModifierEvent {
        return FInputAction::Modulate([this, delta](FTimespan dt) -> float {
            return static_cast<float>((double(delta) * double(_fovMinMax.y - _fovMinMax.x)) * *dt);
        });
    };
    const auto moveModifier = [this](const float3& delta) NOEXCEPT -> FInputAction::FModifierEvent {
        return FInputAction::Modulate([this, delta](FTimespan dt) -> float3 {
            return float3(double3{
                double(delta.x) * *(_strafeSpeed * dt),
                double(delta.y) * *(_upwardSpeed * dt),
                double(delta.z) * *(_forwardSpeed * dt)
            });
        });
    };
    const auto rotateModifier = [this](const float2& delta) NOEXCEPT -> FInputAction::FModifierEvent {
        return FInputAction::Modulate([this, delta](FTimespan dt) -> float2 {
            return float2(double2{
                double(delta.x) * *(_headingSpeed * dt),
                double(delta.y) * *(_pitchSpeed * dt),
            });
        });
    };
    const auto speedModifier = [this](float delta) NOEXCEPT -> FInputAction::FModifierEvent {
        return FInputAction::Modulate([this, delta](FTimespan dt) -> float {
            return static_cast<float>((double(delta) * double(_speedMultiplierMinMax.y - _speedMultiplierMinMax.x)) * *dt);
        });
    };

    keyMappings.emplace_back(EInputKey::W, _moveInput, moveModifier( float3::Z));
    keyMappings.emplace_back(EInputKey::S, _moveInput, moveModifier(-float3::Z));
    keyMappings.emplace_back(EInputKey::A, _moveInput, moveModifier(-float3::X));
    keyMappings.emplace_back(EInputKey::D, _moveInput, moveModifier( float3::X));

    keyMappings.emplace_back(EInputKey::UpArrow, _moveInput, moveModifier( float3::Z));
    keyMappings.emplace_back(EInputKey::DownArrow, _moveInput, moveModifier(-float3::Z));
    keyMappings.emplace_back(EInputKey::LeftArrow, _moveInput, moveModifier(-float3::X));
    keyMappings.emplace_back(EInputKey::RightArrow, _moveInput, moveModifier( float3::X));

    keyMappings.emplace_back(EInputKey::PageUp, _moveInput, moveModifier(float3::Y));
    keyMappings.emplace_back(EInputKey::PageDown, _moveInput, moveModifier(-float3::Y));

    keyMappings.emplace_back(EInputKey::LeftShift, _speedInput, speedModifier(0.001f));
    keyMappings.emplace_back(EInputKey::LeftControl, _speedInput, speedModifier(-0.001f));

    keyMappings.emplace_back(EInputKey::Q, _rotateInput, rotateModifier(-float2::X * _mouseSensitivity.x * 2));
    keyMappings.emplace_back(EInputKey::E, _rotateInput, rotateModifier( float2::X * _mouseSensitivity.x * 2));

    keyMappings.emplace_back(EInputKey::LeftMouseButton, _lookInput);
    keyMappings.emplace_back(EInputKey::Mouse2D, _rotateInput, rotateModifier(_mouseSensitivity));

    keyMappings.emplace_back(EInputKey::Gamepad_Left2D, _moveInput, moveModifier(float3(_gamepadSensitivity.x)));
    keyMappings.emplace_back(EInputKey::Gamepad_Right2D, _rotateInput, rotateModifier(float2(1,-1) * _gamepadSensitivity.y));

    keyMappings.emplace_back(EInputKey::Gamepad_DPad_Up, _moveInput, moveModifier(float3::Y));
    keyMappings.emplace_back(EInputKey::Gamepad_DPad_Down, _moveInput, moveModifier(-float3::Y));

    keyMappings.emplace_back(EInputKey::Gamepad_LeftShoulder, _speedInput, speedModifier(-0.01f));
    keyMappings.emplace_back(EInputKey::Gamepad_RightShoulder, _speedInput, speedModifier(0.01f));

    keyMappings.emplace_back(EInputKey::MouseWheelAxisY, _fovInput, fovModifier(0.01f));
    keyMappings.emplace_back(EInputKey::Add, _fovInput, fovModifier(0.001f));
    keyMappings.emplace_back(EInputKey::Subtract, _fovInput, fovModifier(-0.001f));
}
//----------------------------------------------------------------------------
// Orbit
//----------------------------------------------------------------------------
FOrbitCameraController::FOrbitCameraController()
:   _inputRotate(NEW_REF(Input, FInputAction, "CameraRotate", EInputValueType::Axis2D))
,   _inputZoom(NEW_REF(Input, FInputAction, "CameraZoom", EInputValueType::Axis2D))
{}
//----------------------------------------------------------------------------
void FOrbitCameraController::LookAt(const float3& position, const float3& target, bool bTeleport) NOEXCEPT {
    Assert_NoAssume(not IsNANorINF(position));
    Assert_NoAssume(not IsNANorINF(target));

    _eye = position;
    _target = target;
    _bTeleported = bTeleport;
}
//----------------------------------------------------------------------------
void FOrbitCameraController::UpdateCamera(FTimespan dt, FCameraModel& model) {
    Unused(dt);

    model.Position = _eye;
    model.Forward = SafeNormalize(_target - _eye);
    model.Right = SafeNormalize(Cross(model.Forward, float3(0,1,0)));
    model.Up = SafeNormalize(Cross(model.Forward, model.Right));
    model.bCameraCut = _bTeleported;

    _bTeleported = false;
}
//----------------------------------------------------------------------------
void FOrbitCameraController::InputActionKeyMappings(TAppendable<FInputActionKeyMapping> keyMappings) const NOEXCEPT {
    Unused(keyMappings);
    // #TODO: implement orbit camera controls
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
