#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Camera/CameraController.h"

namespace Core {
namespace Engine {
class FKeyboardInputHandler;
class FMouseInputHandler;

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(KeyboardMouseCameraController);
class FKeyboardMouseCameraController : public Engine::FreeLookCameraController {
public:
    FKeyboardMouseCameraController(  const float3& position, float heading/* rad */, float pitch/* rad */,
                                    const FKeyboardInputHandler *keyboard,
                                    const FMouseInputHandler *mouse);
    virtual ~FKeyboardMouseCameraController();

    float Acceleration() const { return _acceleration; }
    float Inertia() const { return _inertia; }

    float SlowSpeed() const { return _slowSpeed; }
    float FastSpeed() const { return _fastSpeed; }

    float MouseSpeed() const { return _mouseSpeed; }
    float KeyboardSpeed() const { return _keyboardSpeed; }

    float HeadingSpeed() const { return _headingSpeed; }
    float PitchSpeed() const { return _pitchSpeed; }

    float ForwardSpeed() const { return _forwardSpeed; }
    float BackwardSpeed() const { return _backwardSpeed; }
    float StrafeSpeed() const { return _strafeSpeed; }

    void SetAcceleration(float v) { _acceleration = v; }
    void SetInertia(float v) { _inertia = v; }

    void SetSlowSpeed(float v) { _slowSpeed = v; }
    void SetFastSpeed(float v) { _fastSpeed = v; }

    void SetMouseSpeed(float v) { _mouseSpeed = v; }
    void SetKeyboardSpeed(float v) { _keyboardSpeed = v; }

    void SetHeadingSpeed(float v) { _headingSpeed = v; }
    void SetPitchSpeed(float v) { _pitchSpeed = v; }

    void SetForwardSpeed(float v) { _forwardSpeed = v; }
    void SetBackwardSpeed(float v) { _backwardSpeed = v; }
    void SetStrafeSpeed(float v) { _strafeSpeed = v; }

protected:
    virtual void UpdateImpl(float4x4 *view, const FTimeline& time) override;

private:
    const FKeyboardInputHandler *_keyboard;
    const FMouseInputHandler *_mouse;

    float _acceleration;
    float _inertia;

    float _slowSpeed;
    float _fastSpeed;

    float _mouseSpeed;
    float _keyboardSpeed;

    float _headingSpeed;
    float _pitchSpeed;
    float _forwardSpeed;
    float _backwardSpeed;
    float _strafeSpeed;

    float _deltaSpeed;
    float _deltaHeading;
    float _deltaPitch;
    float _deltaForwardBackward;
    float _deltaStrafe;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
