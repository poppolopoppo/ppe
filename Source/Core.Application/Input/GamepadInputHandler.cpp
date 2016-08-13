#include "stdafx.h"

#include "GamepadInputHandler.h"

#include "Core.Graphics/Window/BasicWindow.h"
#include "Core.Graphics/Window/WindowMessage.h"

#include "Core/Maths/PackingHelpers.h"

//#define WITH_GAMEPADSTATE_VERBOSE //%_NOCOMMIT%

#ifdef WITH_GAMEPADSTATE_VERBOSE
#   include "Core/Diagnostic/Logger.h"
#endif

#ifdef OS_WINDOWS
#   include <Windows.h>
#   include <XInput.h>

#   pragma comment(lib, "XInput.lib")

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
#ifdef OS_WINDOWS
static void GamepadTestButton_(
    GamepadButton btn,
    GamepadButtonState& ups,
    GamepadButtonState& presseds,
    GamepadButtonState& downs,
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
#ifdef OS_WINDOWS
static float GamepadNormalizeAxis_(SHORT axis, SHORT deadzone) {
    if (axis <= -deadzone)
        return float(axis + deadzone)/(32768 - deadzone);
    else if (axis >= deadzone)
        return float(axis - deadzone)/(32767 - deadzone);
    else
        return 0.0f;
}
#endif
//----------------------------------------------------------------------------
#ifdef OS_WINDOWS
static float GamepadNormalizeTrigger_(BYTE axis, BYTE deadzone) {
    if (axis >= deadzone)
        return float(axis - deadzone)/(255 - deadzone);
    else
        return 0.0f;
}
#endif
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
GamepadInputHandler::GamepadInputHandler() {}
//----------------------------------------------------------------------------
GamepadInputHandler::~GamepadInputHandler() {}
//----------------------------------------------------------------------------
void GamepadInputHandler::RegisterMessageDelegates(Graphics::BasicWindow *wnd) {
    UNUSED(wnd); // uses XInput instead
}
//----------------------------------------------------------------------------
void GamepadInputHandler::UnregisterMessageDelegates(Graphics::BasicWindow *wnd) {
    UNUSED(wnd); // uses XInput instead
}
//----------------------------------------------------------------------------
void GamepadInputHandler::UpdateBeforeDispatch(Graphics::BasicWindow *wnd) {
    UNUSED(wnd);

    int gamepadIndex = 0;
    for (GamepadState& gamepad : _state.Gamepads()) {
        gamepad._buttonsDown.Clear();
        gamepad._buttonsUp.Clear();

        const bool wasConnected = gamepad.IsConnected();

#ifdef OS_WINDOWS
        ::XINPUT_STATE stateXInput;
        ::ZeroMemory(&stateXInput, sizeof(::XINPUT_STATE));

        if (ERROR_SUCCESS == ::XInputGetState(checked_cast<DWORD>(gamepadIndex), &stateXInput)) {
            gamepad._connected = true;

            gamepad._leftStickX = GamepadNormalizeAxis_(stateXInput.Gamepad.sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
            gamepad._leftStickY = GamepadNormalizeAxis_(stateXInput.Gamepad.sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);

            gamepad._rightStickX = GamepadNormalizeAxis_(stateXInput.Gamepad.sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
            gamepad._rightStickY = GamepadNormalizeAxis_(stateXInput.Gamepad.sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);

            gamepad._leftTrigger = GamepadNormalizeTrigger_(stateXInput.Gamepad.bLeftTrigger, XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
            gamepad._rightTrigger = GamepadNormalizeTrigger_(stateXInput.Gamepad.bRightTrigger, XINPUT_GAMEPAD_TRIGGER_THRESHOLD);

#   define TEST_XINPUT_BTN(_GAMEPADBTN, _XINPUTBTN) \
            GamepadTestButton_(_GAMEPADBTN, \
                gamepad._buttonsUp, gamepad._buttonsPressed, gamepad._buttonsDown, \
                stateXInput, _XINPUTBTN )

            TEST_XINPUT_BTN(GamepadButton::DPadUp,          XINPUT_GAMEPAD_DPAD_UP );
            TEST_XINPUT_BTN(GamepadButton::DPadDown,        XINPUT_GAMEPAD_DPAD_DOWN );
            TEST_XINPUT_BTN(GamepadButton::DPadLeft,        XINPUT_GAMEPAD_DPAD_LEFT );
            TEST_XINPUT_BTN(GamepadButton::DPadRight,       XINPUT_GAMEPAD_DPAD_RIGHT );
            TEST_XINPUT_BTN(GamepadButton::Start,           XINPUT_GAMEPAD_START );
            TEST_XINPUT_BTN(GamepadButton::Back,            XINPUT_GAMEPAD_BACK );
            TEST_XINPUT_BTN(GamepadButton::LeftThumb,       XINPUT_GAMEPAD_LEFT_THUMB );
            TEST_XINPUT_BTN(GamepadButton::RightThumb,      XINPUT_GAMEPAD_RIGHT_THUMB );
            TEST_XINPUT_BTN(GamepadButton::LeftShoulder,    XINPUT_GAMEPAD_LEFT_SHOULDER );
            TEST_XINPUT_BTN(GamepadButton::RightShoulder,   XINPUT_GAMEPAD_RIGHT_SHOULDER );
            TEST_XINPUT_BTN(GamepadButton::A,               XINPUT_GAMEPAD_A );
            TEST_XINPUT_BTN(GamepadButton::B,               XINPUT_GAMEPAD_B );
            TEST_XINPUT_BTN(GamepadButton::X,               XINPUT_GAMEPAD_X );
            TEST_XINPUT_BTN(GamepadButton::Y,               XINPUT_GAMEPAD_Y );

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
        for (GamepadButton btn : gamepad.ButtonsDown().MakeView())
            LOG(Info, L"[Gamepad] Controller#{0}: button <{1}> down", gamepadIndex, GamepadButtonToXBoxCStr(btn));
        for (GamepadButton btn : gamepad.ButtonsUp().MakeView())
            LOG(Info, L"[Gamepad] Controller#{0}: button <{1}> up", gamepadIndex, GamepadButtonToXBoxCStr(btn));
#endif

        gamepadIndex++;
    }
}
//----------------------------------------------------------------------------
void GamepadInputHandler::UpdateAfterDispatch(Graphics::BasicWindow *wnd) {
    UNUSED(wnd);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core
