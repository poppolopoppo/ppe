#pragma once

#include "Application_fwd.h"

#include "Input/Action/InputMapping.h"
#include "Input/Device/FilteredAnalog.h"

#include "Maths/Quaternion.h"
#include "Maths/ScalarVector.h"
#include "Time/Time_fwd.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_APPLICATION_API ICameraController : public IInputActionKeyMappingProvider {
public:
    virtual ~ICameraController() = default;

    virtual void UpdateCamera(FTimespan dt, FCameraModel& model) = 0;
};
//----------------------------------------------------------------------------
class FDummyCameraController final : public ICameraController {
public:
    virtual void UpdateCamera(FTimespan , FCameraModel& ) override {}
    virtual void InputActionKeyMappings(TAppendable<FInputActionKeyMapping> ) const NOEXCEPT override {}
};
//----------------------------------------------------------------------------
class FFreeLookCameraController : public ICameraController {
public:
    PPE_APPLICATION_API FFreeLookCameraController();

    float Fov() const { return *_fov; }
    float3 Position() const { return *_position; }
    FQuaternion Rotation() const { return *_rotation; }
    bool IsTeleported() const { return _bTeleported; }

    const PInputAction& FovInput() const { return _fovInput; }
    const PInputAction& LookInput() const { return _lookInput; }
    const PInputAction& MoveInput() const { return _moveInput; }
    const PInputAction& RotateInput() const { return _rotateInput; }
    const PInputAction& SpeedInput() const { return _speedInput; }

    float2 FovMinMax() const { return _fovMinMax; }
    void SetFovMinMax(const float2& value) { _fovMinMax = value; }

    const FMeterSeconds& ForwardSpeed() const { return _forwardSpeed; }
    void SetForwardSpeed(const FMeterSeconds& value) { _forwardSpeed = value; }

    const FMeterSeconds& StrafeSpeed() const { return _strafeSpeed; }
    void SetStrafeSpeed(const FMeterSeconds& value) { _strafeSpeed = value; }

    const FMeterSeconds& UpwardSpeed() const { return _upwardSpeed; };
    void SetUpwardSpeed(const FMeterSeconds& value) { _upwardSpeed = value; };

    const FRadianSeconds& HeadingSpeed() const { return _headingSpeed; }
    void SetHeadingSpeed(const FRadianSeconds& value) { _headingSpeed = value; }

    const FRadianSeconds& PitchSpeed() const { return _pitchSpeed; }
    void SetPitchSpeed(const FRadianSeconds& value) { _pitchSpeed = value; }

    const float2& GamepadSensitivity() const { return _gamepadSensitivity; }
    void SetGamepadSensitivity(const float2& value) { _gamepadSensitivity = value; }

    const float2& MouseSensitivity() const { return _mouseSensitivity; }
    void SetMouseSensitivity(const float2& value) { _mouseSensitivity = value; }

    float PositionInertia() const { return _position.Sensitivity(); }
    void SetPositionInternia(float value) { _position.SetSensitivity(value); }

    float RotationInertia() const { return _rotation.Sensitivity(); }
    void SetRotationInertia(float value) { _rotation.SetSensitivity(value); }

    float SpeedMultiplier() const { return *_speedMultiplier; }
    void SetSpeedMultiplier(float value) { _speedMultiplier.Reset(value); }

    float2 SpeedMultiplierMinMax() const { return _speedMultiplierMinMax; }
    void SetSpeedMultiplierMinMax(const float2& value) { _speedMultiplierMinMax = value; }

    PPE_APPLICATION_API void SetFov(float value) NOEXCEPT;
    PPE_APPLICATION_API void LookAt(const float3& position, float heading, float pitch, bool bTeleport = false) NOEXCEPT;

    void Translate(const float3& translate) { _deltaPosition += translate; }
    void Rotate(const float2& angle) { _deltaRotation += angle; }

    // ICameraController
    PPE_APPLICATION_API virtual void UpdateCamera(FTimespan dt, FCameraModel& model) override;

    // IInputActionKeyMappingProvider
    PPE_APPLICATION_API virtual void InputActionKeyMappings(TAppendable<FInputActionKeyMapping> keyMappings) const NOEXCEPT override;

private:
    PInputAction _fovInput;
    PInputAction _lookInput;
    PInputAction _moveInput;
    PInputAction _rotateInput;
    PInputAction _speedInput;

    FMeterSeconds _forwardSpeed{ 1 };
    FMeterSeconds _strafeSpeed{ 1 };
    FMeterSeconds _upwardSpeed{ 1 };

    FRadianSeconds _headingSpeed{ 10 };
    FRadianSeconds _pitchSpeed{ 10 };

    float2 _gamepadSensitivity{ .3f, .1f };
    float2 _mouseSensitivity{ 0.05f };

    float2 _fovMinMax{ PI_v<float> / 15.f, (5.f * PI_v<float>) / 7.f };
    float2 _speedMultiplierMinMax{ .1f, 50.f };

    FFilteredAnalog3 _position;
    TBasicFilteredAnalog<FQuaternion> _rotation;
    FFilteredAnalog _fov{ PIOver3 };
    FFilteredAnalog _speedMultiplier{ 1.f, .8f };

    float3 _deltaPosition{ float3::Zero };
    float2 _deltaRotation{ float2::Zero };

    bool _bMouseLook{ false };
    bool _bTeleported{ false };
};
//----------------------------------------------------------------------------
class FOrbitCameraController : public ICameraController {
public:
    PPE_APPLICATION_API FOrbitCameraController();

    const PInputAction& InputRotate() const { return _inputRotate; }
    const PInputAction& InputZoom() const { return _inputZoom; }

    PPE_APPLICATION_API void LookAt(const float3& position, const float3& target, bool bTeleport = false) NOEXCEPT;

    // ICameraController
    PPE_APPLICATION_API virtual void UpdateCamera(FTimespan dt, FCameraModel& model) override;

    // IInputActionKeyMappingProvider
    PPE_APPLICATION_API virtual void InputActionKeyMappings(TAppendable<FInputActionKeyMapping> keyMappings) const NOEXCEPT override;

private:
    float3 _eye;
    float3 _target;

    PInputAction _inputRotate;
    PInputAction _inputZoom;

    bool _bTeleported{ false };
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
