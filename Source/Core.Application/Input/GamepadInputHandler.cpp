#include "stdafx.h"

#include "GamepadInputHandler.h"

#include "Core.Graphics/Window/BasicWindow.h"
#include "Core.Graphics/Window/WindowMessage.h"

#include "Core/Diagnostic/Logger.h"
#include "Core/Maths/PackingHelpers.h"
#include "Core/Meta/Delegate.h"
#include "Core/Thread/ThreadContext.h"
#include "Core/Thread/ThreadPool.h"

//#define WITH_GAMEPADSTATE_VERBOSE //%_NOCOMMIT%

#ifdef PLATFORM_WINDOWS
#   include "XInputWrapper.h"
#else
#   error "no support"
#endif

namespace Core {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
#ifdef PLATFORM_WINDOWS
static void GamepadTestButton_(
    EGamepadButton btn,
    FGamepadButtonState& ups,
    FGamepadButtonState& presseds,
    FGamepadButtonState& downs,
    const ::XINPUT_STATE& stateXInput,
    const WORD btnXInput ) {
    if (stateXInput.Gamepad.wButtons & btnXInput) {
        if (presseds.Add_KeepExisting(btn))
            downs.Add_AssertUnique(btn);
    }
    else {
        if (presseds.Remove_ReturnIfExists(btn))
            ups.Add_AssertUnique(btn);
    }
}
#endif
//----------------------------------------------------------------------------
#ifdef PLATFORM_WINDOWS
// https://web.archive.org/web/20130418234531/http://www.gamasutra.com/blogs/JoshSutphin/20130416/190541/Doing_Thumbstick_Dead_Zones_Right.php
static void GamepadFilterStick_(FGamepadState::FSmoothAnalog* x, FGamepadState::FSmoothAnalog* y, i16 axisX, i16 axisY, i16 deadzone) {
    float fx = ShortM3276832767_to_FloatM11(axisX);
    float fy = ShortM3276832767_to_FloatM11(axisY);
    const float d = (deadzone / 32767.f);
    const float l = Sqrt(fx * fx + fy * fy);
    if (l <= d) {
        x->Set(0.f);
        y->Set(0.f);
    }
    else {
        const float filter = (l - d) / (l * (1 - d));
        x->Set(Clamp(fx * filter, -1.f, 1.f));
        y->Set(Clamp(fy * filter, -1.f, 1.f));
    }
}
#endif
//----------------------------------------------------------------------------
#ifdef PLATFORM_WINDOWS
static void GamepadFilterTrigger_(FGamepadState::FSmoothAnalog* x, u8 axis, int deadzone) {
    x->Set(axis >= deadzone ? Saturate(float(axis - deadzone)/(255 - deadzone)) : 0.f);
}
#endif
//----------------------------------------------------------------------------
#ifdef PLATFORM_WINDOWS
static void GamepadRumble_(const FGamepadState* pGamepad, ITaskContext& ctx) {
    Assert(pGamepad);
    UNUSED(ctx);

    ::XINPUT_VIBRATION vibration;
    vibration.wLeftMotorSpeed = Float01_to_UShort065535(pGamepad->LeftRumble());
    vibration.wRightMotorSpeed = Float01_to_UShort065535(pGamepad->RightRumble());

    const ::DWORD dwUserIndex = checked_cast<::DWORD>(pGamepad->Index());
    FXInputWrapper::Instance().Lock().XInputSetState()(dwUserIndex, &vibration);
}
#endif
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FGamepadInputHandler::FGamepadInputHandler() {
    forrange(i, 0, _state.Gamepads().size())
        _state.Gamepads()[i]._index = i;
}
//----------------------------------------------------------------------------
FGamepadInputHandler::~FGamepadInputHandler() {}
//----------------------------------------------------------------------------
const FMultiGamepadState& FGamepadInputHandler::State() const {
    AssertIsMainThread();
    return _state;
}
//----------------------------------------------------------------------------
void FGamepadInputHandler::RegisterMessageDelegates(Graphics::FBasicWindow *wnd) {
    UNUSED(wnd); // uses XInput instead
}
//----------------------------------------------------------------------------
void FGamepadInputHandler::UnregisterMessageDelegates(Graphics::FBasicWindow *wnd) {
    UNUSED(wnd); // uses XInput instead
}
//----------------------------------------------------------------------------
void FGamepadInputHandler::UpdateBeforeDispatch(Graphics::FBasicWindow *wnd) {
    AssertIsMainThread();
    UNUSED(wnd);

#ifdef PLATFORM_WINDOWS
    FXInputWrapper::FLocked& XInputWrapper = FXInputWrapper::Instance().Lock();
#endif

    int gamepadIndex = 0;
    for (FGamepadState& gamepad : _state.Gamepads()) {
        gamepad._buttonsDown.Clear();
        gamepad._buttonsUp.Clear();

        const bool wasConnected = gamepad.IsConnected();

#ifdef PLATFORM_WINDOWS
        ::XINPUT_STATE stateXInput;
        ::ZeroMemory(&stateXInput, sizeof(::XINPUT_STATE));

        if (XInputWrapper.Available() &&
            ERROR_SUCCESS == XInputWrapper.XInputGetState()(checked_cast<DWORD>(gamepadIndex), &stateXInput)) {
            gamepad._connected = true;

            GamepadFilterStick_(
                &gamepad._leftStickX, &gamepad._leftStickY,
                stateXInput.Gamepad.sThumbLX, stateXInput.Gamepad.sThumbLY,
                XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE );
            GamepadFilterStick_(
                &gamepad._rightStickX, &gamepad._rightStickY,
                stateXInput.Gamepad.sThumbRX, stateXInput.Gamepad.sThumbRY,
                XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE );

            GamepadFilterTrigger_(
                &gamepad._leftTrigger,
                stateXInput.Gamepad.bLeftTrigger,
                XINPUT_GAMEPAD_TRIGGER_THRESHOLD );
            GamepadFilterTrigger_(
                &gamepad._rightTrigger,
                stateXInput.Gamepad.bRightTrigger,
                XINPUT_GAMEPAD_TRIGGER_THRESHOLD );

#   define TEST_XINPUT_BTN(_GAMEPADBTN, _XINPUTBTN) \
            GamepadTestButton_(_GAMEPADBTN, \
                gamepad._buttonsUp, gamepad._buttonsPressed, gamepad._buttonsDown, \
                stateXInput, _XINPUTBTN )

            TEST_XINPUT_BTN(EGamepadButton::DPadUp,          XINPUT_GAMEPAD_DPAD_UP );
            TEST_XINPUT_BTN(EGamepadButton::DPadDown,        XINPUT_GAMEPAD_DPAD_DOWN );
            TEST_XINPUT_BTN(EGamepadButton::DPadLeft,        XINPUT_GAMEPAD_DPAD_LEFT );
            TEST_XINPUT_BTN(EGamepadButton::DPadRight,       XINPUT_GAMEPAD_DPAD_RIGHT );
            TEST_XINPUT_BTN(EGamepadButton::Start,           XINPUT_GAMEPAD_START );
            TEST_XINPUT_BTN(EGamepadButton::Back,            XINPUT_GAMEPAD_BACK );
            TEST_XINPUT_BTN(EGamepadButton::LeftThumb,       XINPUT_GAMEPAD_LEFT_THUMB );
            TEST_XINPUT_BTN(EGamepadButton::RightThumb,      XINPUT_GAMEPAD_RIGHT_THUMB );
            TEST_XINPUT_BTN(EGamepadButton::LeftShoulder,    XINPUT_GAMEPAD_LEFT_SHOULDER );
            TEST_XINPUT_BTN(EGamepadButton::RightShoulder,   XINPUT_GAMEPAD_RIGHT_SHOULDER );
            TEST_XINPUT_BTN(EGamepadButton::A,               XINPUT_GAMEPAD_A );
            TEST_XINPUT_BTN(EGamepadButton::B,               XINPUT_GAMEPAD_B );
            TEST_XINPUT_BTN(EGamepadButton::X,               XINPUT_GAMEPAD_X );
            TEST_XINPUT_BTN(EGamepadButton::Y,               XINPUT_GAMEPAD_Y );

#   undef TEST_XINPUT_BTN
        }
        else {
            gamepad._connected = false;
        }
#else
#   error "no support"
#endif

        if (gamepad._connected && not wasConnected) {
            LOG(Info, L"[Gamepad] Controller#{0} connected", gamepadIndex);
            gamepad._onConnect = true;
        }
        else if (not gamepad._connected && wasConnected) {
            LOG(Info, L"[Gamepad] Controller#{0} disconnected", gamepadIndex);
            gamepad._onDisconnect = true;
        }
        else {
            gamepad._onConnect = false;
            gamepad._onDisconnect = false;
        }

#ifdef WITH_GAMEPADSTATE_VERBOSE
        for (EGamepadButton btn : gamepad.ButtonsDown().MakeView())
            LOG(Info, L"[Gamepad] Controller#{0}: button <{1}> down", gamepadIndex, GamepadButtonToXBoxCStr(btn));
        for (EGamepadButton btn : gamepad.ButtonsUp().MakeView())
            LOG(Info, L"[Gamepad] Controller#{0}: button <{1}> up", gamepadIndex, GamepadButtonToXBoxCStr(btn));
#endif

        gamepadIndex++;
    }
}
//----------------------------------------------------------------------------
void FGamepadInputHandler::UpdateAfterDispatch(Graphics::FBasicWindow *wnd) {
    UNUSED(wnd);
}
//----------------------------------------------------------------------------
void FGamepadInputHandler::Rumble(size_t index, float left, float right) {
    if (FXInputWrapper::Instance().Available()) {
        FGamepadState& gamepad = _state.Gamepads()[index];
        Assert(gamepad.IsConnected());

        // snap to quantized values :
        left = UShort065535_to_Float01(Float01_to_UShort065535(left));
        right = UShort065535_to_Float01(Float01_to_UShort065535(right));

        if (gamepad._leftRumble != left || gamepad._rightRumble != right) {
            gamepad._leftRumble = left;
            gamepad._rightRumble = right;

            AsyncLowestPriority(TDelegate(&GamepadRumble_, const_cast<const FGamepadState*>(&gamepad)));
        }
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core
