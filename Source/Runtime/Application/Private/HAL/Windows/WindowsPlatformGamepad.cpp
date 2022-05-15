#include "stdafx.h"

#ifdef PLATFORM_WINDOWS

#include "HAL/Windows/WindowsPlatformGamepad.h"

#include "HAL/Windows/XInputWrapper.h"
#include "Input/GamepadState.h"
#include "Maths/PackingHelpers.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static bool GamepadIsButtonPressed_(const ::XINPUT_STATE& stateXInput, ::WORD btnXInput) {
    return (!!(stateXInput.Gamepad.wButtons & btnXInput));
}
//----------------------------------------------------------------------------
// https://web.archive.org/web/20130418234531/http://www.gamasutra.com/blogs/JoshSutphin/20130416/190541/Doing_Thumbstick_Dead_Zones_Right.php
static void GamepadFilterStick_(float* x, float* y, i16 axisX, i16 axisY, i16 deadzone) {
    float fx = ShortM3276832767_to_FloatM11(axisX);
    float fy = ShortM3276832767_to_FloatM11(axisY);
    const float d = ShortM3276832767_to_FloatM11(deadzone);
    const float l = Sqrt(fx * fx + fy * fy);
    if (l <= d) {
        *x = 0.f;
        *y = 0.f;
    }
    else {
        const float filter = (l - d) / (l * (1 - d));
        *x = Clamp(fx * filter, -1.f, 1.f);
        *y = Clamp(fy * filter, -1.f, 1.f);
    }
}
//----------------------------------------------------------------------------
static float GamepadFilterTrigger_(u8 axis, int deadzone) {
    return (axis >= deadzone
        ? Saturate(float(axis - deadzone) / (255 - deadzone))
        : 0.f );
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FWindowsPlatformGamepad::Poll(FControllerStates* gamepads) {
    bool hasConnectedGamepad = false;

    forrange(controllerId, 0, ::DWORD(MaxNumGamepad)) {
        FGamepadState& gamepad = (*gamepads)[controllerId];
        hasConnectedGamepad |= Poll(controllerId, &gamepad);
    }

    return hasConnectedGamepad;
}
//----------------------------------------------------------------------------
bool FWindowsPlatformGamepad::Poll(FControllerId index, FGamepadState* gamepad) {
    Assert(index < MaxNumGamepad);
    Assert(gamepad);

    ::XINPUT_STATE stateXInput;
    ::SecureZeroMemory(&stateXInput, sizeof(::XINPUT_STATE));
    {
        const FXInputWrapper::FLocked XInput(FXInputWrapper::Get());
        if (XInput.GetState()(checked_cast<::DWORD>(index), &stateXInput) != ERROR_SUCCESS) {
            gamepad->SetStatus(index, false);
            return false;
        }
    }

    gamepad->SetStatus(index, true);

    float x, y;

    GamepadFilterStick_(
        &x, &y,
        stateXInput.Gamepad.sThumbLX,
        stateXInput.Gamepad.sThumbLY,
        XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
    gamepad->SetLeftStick(x, y);

    GamepadFilterStick_(
        &x, &y,
        stateXInput.Gamepad.sThumbRX,
        stateXInput.Gamepad.sThumbRY,
        XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
    gamepad->SetRightStick(x, y);

    x = GamepadFilterTrigger_(
        stateXInput.Gamepad.bLeftTrigger,
        XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
    y = GamepadFilterTrigger_(
        stateXInput.Gamepad.bRightTrigger,
        XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
    gamepad->SetTriggers(x, y);

#   define TEST_XINPUT_BTN(_GAMEPADBTN, _XINPUTBTN) \
        if (GamepadIsButtonPressed_(stateXInput, _XINPUTBTN)) \
            gamepad->SetButtonPressed(_GAMEPADBTN)

    TEST_XINPUT_BTN(EGamepadButton::DPadUp, XINPUT_GAMEPAD_DPAD_UP);
    TEST_XINPUT_BTN(EGamepadButton::DPadDown, XINPUT_GAMEPAD_DPAD_DOWN);
    TEST_XINPUT_BTN(EGamepadButton::DPadLeft, XINPUT_GAMEPAD_DPAD_LEFT);
    TEST_XINPUT_BTN(EGamepadButton::DPadRight, XINPUT_GAMEPAD_DPAD_RIGHT);
    TEST_XINPUT_BTN(EGamepadButton::Start, XINPUT_GAMEPAD_START);
    TEST_XINPUT_BTN(EGamepadButton::Back, XINPUT_GAMEPAD_BACK);
    TEST_XINPUT_BTN(EGamepadButton::LeftThumb, XINPUT_GAMEPAD_LEFT_THUMB);
    TEST_XINPUT_BTN(EGamepadButton::RightThumb, XINPUT_GAMEPAD_RIGHT_THUMB);
    TEST_XINPUT_BTN(EGamepadButton::LeftShoulder, XINPUT_GAMEPAD_LEFT_SHOULDER);
    TEST_XINPUT_BTN(EGamepadButton::RightShoulder, XINPUT_GAMEPAD_RIGHT_SHOULDER);
    TEST_XINPUT_BTN(EGamepadButton::A, XINPUT_GAMEPAD_A);
    TEST_XINPUT_BTN(EGamepadButton::B, XINPUT_GAMEPAD_B);
    TEST_XINPUT_BTN(EGamepadButton::X, XINPUT_GAMEPAD_X);
    TEST_XINPUT_BTN(EGamepadButton::Y, XINPUT_GAMEPAD_Y);

#   undef TEST_XINPUT_BTN

    return true;
}
//----------------------------------------------------------------------------
bool FWindowsPlatformGamepad::Rumble(FControllerId index, float left, float right) {
    Assert(left >= 0.f && left <= 1.f);
    Assert(right >= 0.f && right <= 1.f);

    ::XINPUT_VIBRATION vibration;
    vibration.wLeftMotorSpeed = Float01_to_UShort065535(left);
    vibration.wRightMotorSpeed = Float01_to_UShort065535(right);

    const ::DWORD dwUserIndex = checked_cast<::DWORD>(index);

    const FXInputWrapper::FLocked XInput(FXInputWrapper::Get());
    const ::DWORD dwError = XInput.SetState()(dwUserIndex, &vibration);

    return (ERROR_SUCCESS == dwError);
}
//----------------------------------------------------------------------------
void FWindowsPlatformGamepad::Start() {
    FXInputWrapper::Create();
}
//----------------------------------------------------------------------------
void FWindowsPlatformGamepad::Shutdown() {
    FXInputWrapper::Destroy();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE

#endif //!PLATFORM_WINDOWS
