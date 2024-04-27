#pragma once

#include "ApplicationUI_fwd.h"

#include "Input/GamepadButton.h"
#include "Input/KeyboardKey.h"
#include "Input/MouseButton.h"
#include "Input/InputKey.h"

#include "Meta/Functor.h"

#include "External/imgui/imgui.git/imgui.h"
#include "Input/InputDevice.h"
#include "Maths/MathHelpers.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline void PostInputMessageToImGui(const FInputActionInstance& action, const FInputKey& key) NOEXCEPT {
#define POST_DIGITAL_KEY(_INPUTKEY, _IMGUIKEY) \
    case _INPUTKEY: \
    if (action.IsTriggerStarted()) \
        io.AddKeyEvent(_IMGUIKEY, true); \
    else if (action.IsTriggerCompleted()) \
        io.AddKeyEvent(_IMGUIKEY, false); \
    break;
#define POST_DIGITAL_KEYMOD(_INPUTKEY, _IMGUIKEY, _IMGUIKEYMOD) \
    case _INPUTKEY: \
    if (action.IsTriggerStarted()) { \
        io.AddKeyEvent(_IMGUIKEY, true); \
        io.AddKeyEvent(_IMGUIKEYMOD, true); \
    } else if (action.IsTriggerCompleted()) { \
        io.AddKeyEvent(_IMGUIKEYMOD, false); \
        io.AddKeyEvent(_IMGUIKEY, false); \
    } \
    break;

    ImGuiIO& io = ImGui::GetIO();

    Meta::Visit(key.Code,
        [](std::monostate) {
            // Any key
            AssertNotImplemented();
        },
        [&](EGamepadAxis value) {
            Assert_NoAssume(action.IsTriggerActive());
            switch (value) {
            case EGamepadAxis::LeftStick:
                if (const float left = Saturate(-action.Axis2D().Absolute.x); left > 0)
                    io.AddKeyAnalogEvent(ImGuiKey_GamepadLStickLeft, true, left);
                if (const float right = Saturate(action.Axis2D().Absolute.x); right > 0)
                    io.AddKeyAnalogEvent(ImGuiKey_GamepadLStickRight, true, right);
                if (const float down = Saturate(-action.Axis2D().Absolute.y); down > 0)
                    io.AddKeyAnalogEvent(ImGuiKey_GamepadLStickDown, true, down);
                if (const float up = Saturate(action.Axis2D().Absolute.y); up > 0)
                    io.AddKeyAnalogEvent(ImGuiKey_GamepadLStickUp, true, up);
                break;
            case EGamepadAxis::RightStick:
                if (const float left = Saturate(-action.Axis2D().Absolute.x); left > 0)
                    io.AddKeyAnalogEvent(ImGuiKey_GamepadRStickLeft, true, left);
                if (const float right = Saturate(action.Axis2D().Absolute.x); right > 0)
                    io.AddKeyAnalogEvent(ImGuiKey_GamepadRStickRight, true, right);
                if (const float down = Saturate(-action.Axis2D().Absolute.y); down > 0)
                    io.AddKeyAnalogEvent(ImGuiKey_GamepadRStickDown, true, down);
                if (const float up = Saturate(action.Axis2D().Absolute.y); up > 0)
                    io.AddKeyAnalogEvent(ImGuiKey_GamepadRStickUp, true, up);
                break;
            case EGamepadAxis::LeftTrigger:
                io.AddKeyAnalogEvent(ImGuiKey_GamepadL2, true, action.Axis1D().Absolute);
                break;
            case EGamepadAxis::RightTrigger:
                io.AddKeyAnalogEvent(ImGuiKey_GamepadR2, true, action.Axis1D().Absolute);
                break;
            // buttons ignored by ImGui
            case EGamepadAxis::Stick2:      FALLTHROUGH();
            case EGamepadAxis::Stick3:      FALLTHROUGH();
            case EGamepadAxis::Trigger2:    FALLTHROUGH();
            case EGamepadAxis::Trigger4:    break;
            }
        },
        [&](EGamepadButton value) {
            switch (value) {
            POST_DIGITAL_KEY(EGamepadButton::DPadUp,        ImGuiKey_GamepadDpadUp)
            POST_DIGITAL_KEY(EGamepadButton::DPadLeft,      ImGuiKey_GamepadDpadLeft)
            POST_DIGITAL_KEY(EGamepadButton::DPadRight,     ImGuiKey_GamepadDpadRight)
            POST_DIGITAL_KEY(EGamepadButton::DPadDown,      ImGuiKey_GamepadDpadDown)
            POST_DIGITAL_KEY(EGamepadButton::A,             ImGuiKey_GamepadFaceDown)
            POST_DIGITAL_KEY(EGamepadButton::B,             ImGuiKey_GamepadFaceRight)
            POST_DIGITAL_KEY(EGamepadButton::X,             ImGuiKey_GamepadFaceLeft)
            POST_DIGITAL_KEY(EGamepadButton::Y,             ImGuiKey_GamepadFaceUp)
            POST_DIGITAL_KEY(EGamepadButton::LeftThumb,     ImGuiKey_GamepadL3)
            POST_DIGITAL_KEY(EGamepadButton::RightThumb,    ImGuiKey_GamepadR3)
            POST_DIGITAL_KEY(EGamepadButton::Start,         ImGuiKey_GamepadStart)
            POST_DIGITAL_KEY(EGamepadButton::Back,          ImGuiKey_GamepadBack)
            POST_DIGITAL_KEY(EGamepadButton::LeftShoulder,  ImGuiKey_GamepadL1)
            POST_DIGITAL_KEY(EGamepadButton::RightShoulder, ImGuiKey_GamepadR1)
            }
        },
        [&](EMouseAxis value) {
            switch (value) {
            case EMouseAxis::Pointer:
            {
                const float2 clientPos = action.Axis2D().Absolute;
                io.AddMousePosEvent(clientPos.x, clientPos.y);
                break;
            }
            case EMouseAxis::ScrollWheelY:
                io.AddMouseWheelEvent(0, action.Axis1D().Absolute);
                break;
            case EMouseAxis::ScrollWheelX:
                io.AddMouseWheelEvent(action.Axis1D().Absolute, 0);
                break;
            }
        },
        [&](EMouseButton value) {
            switch (value) {
            case EMouseButton::Button0: FALLTHROUGH();
            case EMouseButton::Button1: FALLTHROUGH();
            case EMouseButton::Button2: FALLTHROUGH();
            case EMouseButton::Thumb0:  FALLTHROUGH();
            case EMouseButton::Thumb1:
                if (action.IsTriggerStarted())
                    io.AddMouseButtonEvent(static_cast<int>(value), true);
                else if (action.IsTriggerCompleted())
                    io.AddMouseButtonEvent(static_cast<int>(value), false);
                break;
            }
        },
        [&](EKeyboardKey value) {
            switch (value) {
            POST_DIGITAL_KEY(EKeyboardKey::_0,              ImGuiKey_0)
            POST_DIGITAL_KEY(EKeyboardKey::_1,              ImGuiKey_1)
            POST_DIGITAL_KEY(EKeyboardKey::_2,              ImGuiKey_2)
            POST_DIGITAL_KEY(EKeyboardKey::_3,              ImGuiKey_3)
            POST_DIGITAL_KEY(EKeyboardKey::_4,              ImGuiKey_4)
            POST_DIGITAL_KEY(EKeyboardKey::_5,              ImGuiKey_5)
            POST_DIGITAL_KEY(EKeyboardKey::_6,              ImGuiKey_6)
            POST_DIGITAL_KEY(EKeyboardKey::_7,              ImGuiKey_7)
            POST_DIGITAL_KEY(EKeyboardKey::_8,              ImGuiKey_8)
            POST_DIGITAL_KEY(EKeyboardKey::_9,              ImGuiKey_9)
            POST_DIGITAL_KEY(EKeyboardKey::A,               ImGuiKey_A)
            POST_DIGITAL_KEY(EKeyboardKey::B,               ImGuiKey_B)
            POST_DIGITAL_KEY(EKeyboardKey::C,               ImGuiKey_C)
            POST_DIGITAL_KEY(EKeyboardKey::D,               ImGuiKey_D)
            POST_DIGITAL_KEY(EKeyboardKey::E,               ImGuiKey_E)
            POST_DIGITAL_KEY(EKeyboardKey::F,               ImGuiKey_F)
            POST_DIGITAL_KEY(EKeyboardKey::G,               ImGuiKey_G)
            POST_DIGITAL_KEY(EKeyboardKey::H,               ImGuiKey_H)
            POST_DIGITAL_KEY(EKeyboardKey::I,               ImGuiKey_I)
            POST_DIGITAL_KEY(EKeyboardKey::J,               ImGuiKey_J)
            POST_DIGITAL_KEY(EKeyboardKey::K,               ImGuiKey_K)
            POST_DIGITAL_KEY(EKeyboardKey::L,               ImGuiKey_L)
            POST_DIGITAL_KEY(EKeyboardKey::M,               ImGuiKey_M)
            POST_DIGITAL_KEY(EKeyboardKey::N,               ImGuiKey_N)
            POST_DIGITAL_KEY(EKeyboardKey::O,               ImGuiKey_O)
            POST_DIGITAL_KEY(EKeyboardKey::P,               ImGuiKey_P)
            POST_DIGITAL_KEY(EKeyboardKey::Q,               ImGuiKey_Q)
            POST_DIGITAL_KEY(EKeyboardKey::R,               ImGuiKey_R)
            POST_DIGITAL_KEY(EKeyboardKey::S,               ImGuiKey_S)
            POST_DIGITAL_KEY(EKeyboardKey::_T,              ImGuiKey_T)
            POST_DIGITAL_KEY(EKeyboardKey::U,               ImGuiKey_U)
            POST_DIGITAL_KEY(EKeyboardKey::V,               ImGuiKey_V)
            POST_DIGITAL_KEY(EKeyboardKey::W,               ImGuiKey_W)
            POST_DIGITAL_KEY(EKeyboardKey::X,               ImGuiKey_X)
            POST_DIGITAL_KEY(EKeyboardKey::Y,               ImGuiKey_Y)
            POST_DIGITAL_KEY(EKeyboardKey::Z,               ImGuiKey_Z)
            POST_DIGITAL_KEY(EKeyboardKey::Numpad0,         ImGuiKey_Keypad0)
            POST_DIGITAL_KEY(EKeyboardKey::Numpad1,         ImGuiKey_Keypad1)
            POST_DIGITAL_KEY(EKeyboardKey::Numpad2,         ImGuiKey_Keypad2)
            POST_DIGITAL_KEY(EKeyboardKey::Numpad3,         ImGuiKey_Keypad3)
            POST_DIGITAL_KEY(EKeyboardKey::Numpad4,         ImGuiKey_Keypad4)
            POST_DIGITAL_KEY(EKeyboardKey::Numpad5,         ImGuiKey_Keypad5)
            POST_DIGITAL_KEY(EKeyboardKey::Numpad6,         ImGuiKey_Keypad6)
            POST_DIGITAL_KEY(EKeyboardKey::Numpad7,         ImGuiKey_Keypad7)
            POST_DIGITAL_KEY(EKeyboardKey::Numpad8,         ImGuiKey_Keypad8)
            POST_DIGITAL_KEY(EKeyboardKey::Numpad9,         ImGuiKey_Keypad9)
            POST_DIGITAL_KEY(EKeyboardKey::F1,              ImGuiKey_F1)
            POST_DIGITAL_KEY(EKeyboardKey::F2,              ImGuiKey_F2)
            POST_DIGITAL_KEY(EKeyboardKey::F3,              ImGuiKey_F3)
            POST_DIGITAL_KEY(EKeyboardKey::F4,              ImGuiKey_F4)
            POST_DIGITAL_KEY(EKeyboardKey::F5,              ImGuiKey_F5)
            POST_DIGITAL_KEY(EKeyboardKey::F6,              ImGuiKey_F6)
            POST_DIGITAL_KEY(EKeyboardKey::F7,              ImGuiKey_F7)
            POST_DIGITAL_KEY(EKeyboardKey::F8,              ImGuiKey_F8)
            POST_DIGITAL_KEY(EKeyboardKey::F9,              ImGuiKey_F9)
            POST_DIGITAL_KEY(EKeyboardKey::F10,             ImGuiKey_F10)
            POST_DIGITAL_KEY(EKeyboardKey::F11,             ImGuiKey_F11)
            POST_DIGITAL_KEY(EKeyboardKey::F12,             ImGuiKey_F12)
            POST_DIGITAL_KEY(EKeyboardKey::Escape,          ImGuiKey_Escape)
            POST_DIGITAL_KEY(EKeyboardKey::Space,           ImGuiKey_Space)
            POST_DIGITAL_KEY(EKeyboardKey::Pause,           ImGuiKey_Pause)
            POST_DIGITAL_KEY(EKeyboardKey::PrintScreen,     ImGuiKey_PrintScreen)
            POST_DIGITAL_KEY(EKeyboardKey::ScrollLock,      ImGuiKey_ScrollLock)
            POST_DIGITAL_KEY(EKeyboardKey::Backspace,       ImGuiKey_Backspace)
            POST_DIGITAL_KEY(EKeyboardKey::Enter,           ImGuiKey_Enter)
            POST_DIGITAL_KEY(EKeyboardKey::Tab,             ImGuiKey_Tab)
            POST_DIGITAL_KEY(EKeyboardKey::Home,            ImGuiKey_Home)
            POST_DIGITAL_KEY(EKeyboardKey::End,             ImGuiKey_End)
            POST_DIGITAL_KEY(EKeyboardKey::Insert,          ImGuiKey_Insert)
            POST_DIGITAL_KEY(EKeyboardKey::Delete,          ImGuiKey_Delete)
            POST_DIGITAL_KEY(EKeyboardKey::PageUp,          ImGuiKey_PageUp)
            POST_DIGITAL_KEY(EKeyboardKey::PageDown,        ImGuiKey_PageDown)
            POST_DIGITAL_KEY(EKeyboardKey::Comma,           ImGuiKey_Comma)
            POST_DIGITAL_KEY(EKeyboardKey::Equals,          ImGuiKey_Equal)
            // POST_DIGITAL_KEY(EKeyboardKey::Plus,            ImGuiKey_Plus)
            POST_DIGITAL_KEY(EKeyboardKey::Minus,           ImGuiKey_Minus)
            POST_DIGITAL_KEY(EKeyboardKey::Period,          ImGuiKey_Period)
            POST_DIGITAL_KEY(EKeyboardKey::Semicolon,       ImGuiKey_Semicolon)
            // POST_DIGITAL_KEY(EKeyboardKey::Underscore,      ImGuiKey_Underscore)
            // POST_DIGITAL_KEY(EKeyboardKey::Ampersand,       ImGuiKey_Ampersand)
            POST_DIGITAL_KEY(EKeyboardKey::Apostrophe,      ImGuiKey_Apostrophe)
            // POST_DIGITAL_KEY(EKeyboardKey::Asterix,         ImGuiKey_Asterix)
            // POST_DIGITAL_KEY(EKeyboardKey::Caret,           ImGuiKey_Caret)
            // POST_DIGITAL_KEY(EKeyboardKey::Colon,           ImGuiKey_Colon)
            // POST_DIGITAL_KEY(EKeyboardKey::Dollar,          ImGuiKey_Dollar)
            // POST_DIGITAL_KEY(EKeyboardKey::Exclamation,     ImGuiKey_Exclamation)
            // POST_DIGITAL_KEY(EKeyboardKey::Tilde,           ImGuiKey_Tilde)
            // POST_DIGITAL_KEY(EKeyboardKey::Quote,           ImGuiKey_Quote)
            POST_DIGITAL_KEY(EKeyboardKey::Slash,           ImGuiKey_Slash)
            POST_DIGITAL_KEY(EKeyboardKey::Backslash,       ImGuiKey_Backslash)
            POST_DIGITAL_KEY(EKeyboardKey::LeftBracket,     ImGuiKey_LeftBracket)
            POST_DIGITAL_KEY(EKeyboardKey::RightBracket,    ImGuiKey_RightBracket)
            // POST_DIGITAL_KEY(EKeyboardKey::LeftParantheses, ImGuiKey_LeftParantheses)
            // POST_DIGITAL_KEY(EKeyboardKey::RightParantheses,ImGuiKey_RightParantheses)
            POST_DIGITAL_KEY(EKeyboardKey::LeftArrow,       ImGuiKey_LeftArrow)
            POST_DIGITAL_KEY(EKeyboardKey::RightArrow,      ImGuiKey_RightArrow)
            POST_DIGITAL_KEY(EKeyboardKey::UpArrow,         ImGuiKey_UpArrow)
            POST_DIGITAL_KEY(EKeyboardKey::DownArrow,       ImGuiKey_DownArrow)
            POST_DIGITAL_KEY(EKeyboardKey::CapsLock,        ImGuiKey_CapsLock)
            POST_DIGITAL_KEY(EKeyboardKey::NumLock,         ImGuiKey_NumLock)

            POST_DIGITAL_KEYMOD(EKeyboardKey::LeftAlt,         ImGuiKey_LeftAlt, ImGuiMod_Alt)
            POST_DIGITAL_KEYMOD(EKeyboardKey::RightAlt,        ImGuiKey_RightAlt, ImGuiMod_Alt)
            POST_DIGITAL_KEYMOD(EKeyboardKey::LeftControl,     ImGuiKey_LeftCtrl, ImGuiMod_Ctrl)
            POST_DIGITAL_KEYMOD(EKeyboardKey::RightControl,    ImGuiKey_RightCtrl, ImGuiMod_Ctrl)
            POST_DIGITAL_KEYMOD(EKeyboardKey::LeftShift,       ImGuiKey_LeftShift, ImGuiMod_Shift)
            POST_DIGITAL_KEYMOD(EKeyboardKey::RightShift,      ImGuiKey_RightShift, ImGuiMod_Shift)
            POST_DIGITAL_KEYMOD(EKeyboardKey::LeftSuper,       ImGuiKey_LeftSuper, ImGuiMod_Super)
            POST_DIGITAL_KEYMOD(EKeyboardKey::RightSuper,      ImGuiKey_RightSuper, ImGuiMod_Super)

            // unhandled as explicit key by ImGui
            case EKeyboardKey::Plus: FALLTHROUGH();
            case EKeyboardKey::Underscore: FALLTHROUGH();
            case EKeyboardKey::Ampersand: FALLTHROUGH();
            case EKeyboardKey::Asterix: FALLTHROUGH();
            case EKeyboardKey::Caret: FALLTHROUGH();
            case EKeyboardKey::Colon: FALLTHROUGH();
            case EKeyboardKey::Dollar: FALLTHROUGH();
            case EKeyboardKey::Exclamation: FALLTHROUGH();
            case EKeyboardKey::Tilde: FALLTHROUGH();
            case EKeyboardKey::Quote: FALLTHROUGH();
            case EKeyboardKey::LeftParantheses: FALLTHROUGH();
            case EKeyboardKey::RightParantheses:
                break;
            }

        });
#undef POST_DIGITAL_KEYMOD
#undef POST_DIGITAL_KEY
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
