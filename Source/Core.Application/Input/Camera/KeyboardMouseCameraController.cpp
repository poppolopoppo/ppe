#include "stdafx.h"

#include "KeyboardMouseCameraController.h"

#include "Input/State/KeyboardInputHandler.h"
#include "Input/State/MouseInputHandler.h"

#include "Core/Maths/Geometry/ScalarVector.h"
#include "Core/Maths/Transform/ScalarMatrix.h"
#include "Core/Time/Timeline.h"

#include "Core.Graphics/Window/BasicWindow.h"
#include "Core.Graphics/Window/WindowMessageHandler.h"

namespace Core {
namespace Application {
class KeyboardInputHandler;
class MouseInputHandler;

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
KeyboardMouseCameraController::KeyboardMouseCameraController(
    const float3& position, float heading/* rad */, float pitch/* rad */,
    const KeyboardInputHandler *keyboard,
    const MouseInputHandler *mouse )
:   Engine::FreeLookCameraController(position, heading, pitch)
,   _keyboard(keyboard), _mouse(mouse)

,   _acceleration(1.1f)
,   _inertia(0.8f)

,   _slowSpeed(1.0f)
,   _fastSpeed(2.0f) // when shift is pressed

,   _mouseSpeed(1.0f)
,   _keyboardSpeed(2.5f)

,   _headingSpeed(0.0005f)
,   _pitchSpeed(0.0005f)

,   _forwardSpeed(0.005f)
,   _backwardSpeed(0.0035f)
,   _strafeSpeed(0.005f)

{
    Assert(keyboard);
    Assert(mouse);

    _deltaSpeed = _slowSpeed;
    _deltaHeading = _deltaPitch = 0;
    _deltaForwardBackward = _deltaStrafe = 0;
}
//----------------------------------------------------------------------------
KeyboardMouseCameraController::~KeyboardMouseCameraController() {}
//----------------------------------------------------------------------------
void KeyboardMouseCameraController::UpdateImpl(float4x4 *view, const Timeline& time) {
    const Graphics::BasicWindow *wnd = _mouse->Window();
    Assert(_keyboard->Window() == wnd);

    _deltaSpeed = (_keyboard->IsKeyPressed(KeyboardKey::Shift)) ? _fastSpeed : _slowSpeed;

    const MouseButton mouseLookButton = MouseButton::Button1;
    const MouseButton mouseTranslateButton = MouseButton::Button2;

    // heading/pitch on scroll button
    if (_mouse->IsButtonDown(mouseLookButton)) {
        wnd->SetCursorCapture(true);
        wnd->SetCursorPositionOnScreenCenter();
    }
    else if (_mouse->IsButtonUp(mouseLookButton)) {
        wnd->SetCursorCapture(false);
    }
    else if (_mouse->IsButtonPressed(mouseLookButton)) {
        const int deltaX = _mouse->ClientX() - (int)_mouse->Window()->Width()/2;
        const int deltaY = _mouse->ClientY() - (int)_mouse->Window()->Height()/2;

        wnd->SetCursorPositionOnScreenCenter();

        _deltaHeading += deltaX * _headingSpeed * _mouseSpeed;
        _deltaPitch += deltaY * _pitchSpeed * _mouseSpeed;

        _deltaHeading *= _acceleration;
        _deltaPitch *= _acceleration;
    }

    // 3D translation on mouse right button
    if (_mouse->IsButtonDown(mouseTranslateButton)) {
        wnd->SetCursorCapture(true);
        wnd->SetCursorPositionOnScreenCenter();
    }
    else if (_mouse->IsButtonUp(mouseTranslateButton)) {
        wnd->SetCursorCapture(false);
    }
    else if (_mouse->IsButtonPressed(mouseTranslateButton)) {
        const int deltaX = _mouse->ClientX() - (int)_mouse->Window()->Width()/2;
        const int deltaY = _mouse->ClientY() - (int)_mouse->Window()->Height()/2;

        wnd->SetCursorPositionOnScreenCenter();

        _deltaStrafe += deltaX * _strafeSpeed * _mouseSpeed;
        _deltaForwardBackward -= _mouseSpeed * (deltaY < 0)
            ? deltaY * _forwardSpeed
            : deltaY * _backwardSpeed;

        _deltaStrafe *= _acceleration;
        _deltaForwardBackward *= _acceleration;
    }

    // heading/pitch on keyboard arrow
    if (_keyboard->IsKeyPressed(KeyboardKey::Left)  ) {
        _deltaHeading -= _headingSpeed * _keyboardSpeed;
        _deltaHeading *= _acceleration;
    }
    if (_keyboard->IsKeyPressed(KeyboardKey::Right)  ) {
        _deltaHeading += _headingSpeed * _keyboardSpeed;
        _deltaHeading *= _acceleration;
    }
    if (_keyboard->IsKeyPressed(KeyboardKey::Up)  ) {
        _deltaPitch -= _pitchSpeed * _keyboardSpeed;
        _deltaPitch *= _acceleration;
    }
    if (_keyboard->IsKeyPressed(KeyboardKey::Down)) {
        _deltaPitch += _pitchSpeed * _keyboardSpeed;
        _deltaPitch *= _acceleration;
    }

    // 3D translation on keyboard zqsd
    if (_keyboard->IsKeyPressed(KeyboardKey::Q)  ) {
        _deltaStrafe -= _strafeSpeed * _keyboardSpeed;
        _deltaStrafe *= _acceleration;
    }
    if (_keyboard->IsKeyPressed(KeyboardKey::D)  ) {
        _deltaStrafe += _strafeSpeed * _keyboardSpeed;
        _deltaStrafe *= _acceleration;
    }
    if (_keyboard->IsKeyPressed(KeyboardKey::Z)  ) {
        _deltaForwardBackward += _forwardSpeed * _keyboardSpeed;
        _deltaForwardBackward *= _acceleration;
    }
    if (_keyboard->IsKeyPressed(KeyboardKey::S)) {
        _deltaForwardBackward -= _backwardSpeed * _keyboardSpeed;
        _deltaForwardBackward *= _acceleration;
    }

    // speed + inertia
    const float3 position = Position() +
        Forward() * (_deltaForwardBackward * _deltaSpeed) +
        Right() * (_deltaStrafe * _deltaSpeed);

    const float heading = Heading() + (_deltaHeading * _slowSpeed);
    const float pitch = Pitch() + (_deltaPitch * _slowSpeed);

    _deltaHeading *= _inertia;
    _deltaPitch *= _inertia;

    _deltaForwardBackward *= _inertia;
    _deltaStrafe *= _inertia;

    // finally update camera
    Engine::FreeLookCameraController::LookAt(position, heading, pitch);
    Engine::FreeLookCameraController::UpdateImpl(view, time);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core
