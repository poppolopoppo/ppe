﻿// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#ifdef PLATFORM_WINDOWS

#include "HAL/Windows/WindowsPlatformKeyboard.h"

#include "Input/Device/KeyboardState.h"
#include "Input/KeyboardKey.h"

#include "HAL/Windows/WindowsPlatformMessageHandler.h"
#include "HAL/Windows/WindowsWindow.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
CONSTEXPR EKeyboardKey GInvalidKey_{0xFF};
//----------------------------------------------------------------------------
CONSTEXPR EKeyboardKey VirtualKeyToKeyboardKey_(const FWindowsPlatformMessageHandler::FMessage& msg) {
    const WPARAM dwVirtualKey = msg.WParam;
    UINT uScanCode = (msg.LParam & 0x00ff0000) >> 16;
    const int iExtended = (msg.LParam & 0x01000000) != 0;

    switch (dwVirtualKey) {
    case '0': return EKeyboardKey::_0;
    case '1': return EKeyboardKey::_1;
    case '2': return EKeyboardKey::_2;
    case '3': return EKeyboardKey::_3;
    case '4': return EKeyboardKey::_4;
    case '5': return EKeyboardKey::_5;
    case '6': return EKeyboardKey::_6;
    case '7': return EKeyboardKey::_7;
    case '8': return EKeyboardKey::_8;
    case '9': return EKeyboardKey::_9;
    case 'A': return EKeyboardKey::A;
    case 'B': return EKeyboardKey::B;
    case 'C': return EKeyboardKey::C;
    case 'D': return EKeyboardKey::D;
    case 'E': return EKeyboardKey::E;
    case 'F': return EKeyboardKey::F;
    case 'G': return EKeyboardKey::G;
    case 'H': return EKeyboardKey::H;
    case 'I': return EKeyboardKey::I;
    case 'J': return EKeyboardKey::J;
    case 'K': return EKeyboardKey::K;
    case 'L': return EKeyboardKey::L;
    case 'M': return EKeyboardKey::M;
    case 'N': return EKeyboardKey::N;
    case 'O': return EKeyboardKey::O;
    case 'P': return EKeyboardKey::P;
    case 'Q': return EKeyboardKey::Q;
    case 'R': return EKeyboardKey::R;
    case 'S': return EKeyboardKey::S;
    case 'T': return EKeyboardKey::_T;
    case 'U': return EKeyboardKey::U;
    case 'V': return EKeyboardKey::V;
    case 'W': return EKeyboardKey::W;
    case 'X': return EKeyboardKey::X;
    case 'Y': return EKeyboardKey::Y;
    case 'Z': return EKeyboardKey::Z;
    case VK_NUMPAD0: return EKeyboardKey::Numpad0;
    case VK_NUMPAD1: return EKeyboardKey::Numpad1;
    case VK_NUMPAD2: return EKeyboardKey::Numpad2;
    case VK_NUMPAD3: return EKeyboardKey::Numpad3;
    case VK_NUMPAD4: return EKeyboardKey::Numpad4;
    case VK_NUMPAD5: return EKeyboardKey::Numpad5;
    case VK_NUMPAD6: return EKeyboardKey::Numpad6;
    case VK_NUMPAD7: return EKeyboardKey::Numpad7;
    case VK_NUMPAD8: return EKeyboardKey::Numpad8;
    case VK_NUMPAD9: return EKeyboardKey::Numpad9;
    case VK_ADD: return EKeyboardKey::Plus;
    case VK_SUBTRACT: return EKeyboardKey::Minus;
    case VK_MULTIPLY: return EKeyboardKey::Asterix;
    case VK_DIVIDE: return EKeyboardKey::Slash;
    case VK_F1: return EKeyboardKey::F1;
    case VK_F2: return EKeyboardKey::F2;
    case VK_F3: return EKeyboardKey::F3;
    case VK_F4: return EKeyboardKey::F4;
    case VK_F5: return EKeyboardKey::F5;
    case VK_F6: return EKeyboardKey::F6;
    case VK_F7: return EKeyboardKey::F7;
    case VK_F8: return EKeyboardKey::F8;
    case VK_F9: return EKeyboardKey::F9;
    case VK_F10: return EKeyboardKey::F10;
    case VK_F11: return EKeyboardKey::F11;
    case VK_F12: return EKeyboardKey::F12;
    case VK_UP: return EKeyboardKey::UpArrow;
    case VK_DOWN: return EKeyboardKey::DownArrow;
    case VK_LEFT: return EKeyboardKey::LeftArrow;
    case VK_RIGHT: return EKeyboardKey::RightArrow;
    case VK_ESCAPE: return EKeyboardKey::Escape;
    case VK_SPACE: return EKeyboardKey::Space;
    case VK_PAUSE: return EKeyboardKey::Pause;
    case VK_PRINT: return EKeyboardKey::PrintScreen;
    case VK_SCROLL: return EKeyboardKey::ScrollLock;
    case VK_BACK: return EKeyboardKey::Backspace;
    case VK_RETURN: return EKeyboardKey::Enter;
    case VK_TAB: return EKeyboardKey::Tab;
    case VK_HOME: return EKeyboardKey::Home;
    case VK_END: return EKeyboardKey::End;
    case VK_INSERT: return EKeyboardKey::Insert;
    case VK_DELETE: return EKeyboardKey::Delete;
    case VK_PRIOR: return EKeyboardKey::PageUp;
    case VK_NEXT: return EKeyboardKey::PageDown;
    case VK_OEM_COMMA: return EKeyboardKey::Comma;
    case VK_OEM_PLUS: return EKeyboardKey::Plus;
    case VK_OEM_MINUS: return EKeyboardKey::Minus;
    case VK_OEM_PERIOD: return EKeyboardKey::Period;
    case VK_CAPITAL: return EKeyboardKey::CapsLock;
    case VK_NUMLOCK: return EKeyboardKey::NumLock;
    case VK_LMENU: return EKeyboardKey::LeftAlt;
    case VK_RMENU: return EKeyboardKey::RightAlt;
    case VK_LSHIFT: return EKeyboardKey::LeftShift;
    case VK_RSHIFT: return EKeyboardKey::RightShift;
    case VK_LWIN: return EKeyboardKey::LeftSuper;
    case VK_RWIN: return EKeyboardKey::RightSuper;
    case VK_LCONTROL: return EKeyboardKey::LeftControl;
    case VK_RCONTROL: return EKeyboardKey::RightControl;

    case VK_CONTROL:
        return (iExtended ? EKeyboardKey::RightControl : EKeyboardKey::LeftControl);
    case VK_MENU:
        return (iExtended ? EKeyboardKey::RightAlt : EKeyboardKey::LeftAlt);
    case VK_SHIFT:
        uScanCode = ::MapVirtualKey(uScanCode, MAPVK_VSC_TO_VK_EX);
        switch (uScanCode) {
        case VK_LSHIFT: return EKeyboardKey::LeftShift;
        case VK_RSHIFT: return EKeyboardKey::RightShift;
        default: AssertNotReached();
        }

    case VK_OEM_1: return EKeyboardKey::Semicolon;
    case VK_OEM_2: return EKeyboardKey::Slash;
    case VK_OEM_3: return EKeyboardKey::Apostrophe;
    case VK_OEM_4: return EKeyboardKey::LeftBracket;
    case VK_OEM_5: return EKeyboardKey::Backslash;
    case VK_OEM_6: return EKeyboardKey::RightBracket;
    case VK_OEM_7: return EKeyboardKey::Quote;

    default:
        return GInvalidKey_;
    }
}
//----------------------------------------------------------------------------
static bool KeyboarSetKeyDown_(FKeyboardState* keyboard, const FWindowsPlatformMessageHandler::FMessage& msg) {
    if (msg.WParam < 256) {
        const EKeyboardKey key = VirtualKeyToKeyboardKey_(msg);

        if (key != GInvalidKey_) {
            keyboard->SetKeyDown(key);
        }
    }
    return false;
}
//----------------------------------------------------------------------------
static bool KeyboarSetKeyUp_(FKeyboardState* keyboard, const FWindowsPlatformMessageHandler::FMessage& msg) {
    if (msg.WParam < 256) {
        const EKeyboardKey key = VirtualKeyToKeyboardKey_(msg);

        if (key != GInvalidKey_) {
            keyboard->SetKeyUp(key);
        }
    }
    return false;
}
//----------------------------------------------------------------------------
static bool KeyboarAddInputCharacter_(FKeyboardState* keyboard, const FWindowsPlatformMessageHandler::FMessage& msg) {
    if (msg.WParam > 0 && msg.WParam < 0x10000) {
        keyboard->AddCharacterInput(checked_cast<FKeyboardState::FCharacterInput>(msg.WParam));
    }
    return false;
}
//----------------------------------------------------------------------------
static bool KeyboardMessageHandler_(FKeyboardState* keyboard, const FWindowsPlatformMessageHandler::FMessage& msg) {
    Assert(keyboard);

    switch (msg.Type) {
    case EWindowsMessageType::KeyDown:
    case EWindowsMessageType::SysKeyDown:
        return KeyboarSetKeyDown_(keyboard, msg);

    case EWindowsMessageType::KeyUp:
    case EWindowsMessageType::SysKeyUp:
        return KeyboarSetKeyUp_(keyboard, msg);

    case EWindowsMessageType::Char:
        return KeyboarAddInputCharacter_(keyboard, msg);

    default:
        return false; // unhandled
    }
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FEventHandle FWindowsPlatformKeyboard::SetupMessageHandler(FWindowsWindow& window, FKeyboardState* keyboard) {
    Assert(keyboard);
    Assert(window.NativeHandle());

    return window.OnMessageWin32().Add([keyboard](const FWindowsWindow&, FWindowsMessage* msg) {
        msg->Handled |= KeyboardMessageHandler_(keyboard, *msg);
    });
}
//----------------------------------------------------------------------------
void FWindowsPlatformKeyboard::RemoveMessageHandler(FWindowsWindow& window, FEventHandle& handle) {
    Assert(window.NativeHandle());
    Assert(handle);

    window.OnMessageWin32().Remove(handle);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE

#endif //!PLATFORM_WINDOWS
