#pragma once

#include "Application.h"

#include "Core.Engine/CameraController.h"

namespace Core {
namespace Application {
class KeyboardInputHandler;
class MouseInputHandler;

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(KeyboardMouseCameraController);
class KeyboardMouseCameraController : public Engine::FreeLookCameraController {
public:
    KeyboardMouseCameraController(  const float3& position, float heading/* rad */, float pitch/* rad */,
                                    const KeyboardInputHandler *keyboard,
                                    const MouseInputHandler *mouse);
    virtual ~KeyboardMouseCameraController();

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
    virtual void UpdateImpl(float4x4 *view, const Timeline& time) override;

private:
    const KeyboardInputHandler *_keyboard;
    const MouseInputHandler *_mouse;

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
} //!namespace Application
} //!namespace Core
