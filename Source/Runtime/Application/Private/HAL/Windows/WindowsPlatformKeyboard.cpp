#include "stdafx.h"

#ifdef PLATFORM_WINDOWS

#include "HAL/Windows/WindowsPlatformKeyboard.h"

#include "Input/KeyboardKey.h"
#include "Input/KeyboardState.h"
#include "HAL/Windows/WindowsPlatformMessageHandler.h"
#include "HAL/Windows/WindowsWindow.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static const u8 GVirtualKey_to_KeyboardKey[0xFF] = {
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    u8(EKeyboardKey::Backspace),
    u8(EKeyboardKey::Tab),
    0xFF,
    0xFF,
    0xFF,
    u8(EKeyboardKey::Enter),
    0xFF,
    0xFF,
    u8(EKeyboardKey::Shift),
    u8(EKeyboardKey::Control),
    u8(EKeyboardKey::Alt),
    u8(EKeyboardKey::Pause),
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    u8(EKeyboardKey::Escape),
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    u8(EKeyboardKey::Space),
    u8(EKeyboardKey::PageUp),
    u8(EKeyboardKey::PageDown),
    u8(EKeyboardKey::End),
    u8(EKeyboardKey::Home),
    u8(EKeyboardKey::Left),
    u8(EKeyboardKey::Up),
    u8(EKeyboardKey::Right),
    u8(EKeyboardKey::Down),
    0xFF,
    u8(EKeyboardKey::PrintScreen),
    0xFF,
    0xFF,
    u8(EKeyboardKey::Insert),
    u8(EKeyboardKey::Delete),
    0xFF,
    u8(EKeyboardKey::_0),
    u8(EKeyboardKey::_1),
    u8(EKeyboardKey::_2),
    u8(EKeyboardKey::_3),
    u8(EKeyboardKey::_4),
    u8(EKeyboardKey::_5),
    u8(EKeyboardKey::_6),
    u8(EKeyboardKey::_7),
    u8(EKeyboardKey::_8),
    u8(EKeyboardKey::_9),
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    u8(EKeyboardKey::A),
    u8(EKeyboardKey::B),
    u8(EKeyboardKey::C),
    u8(EKeyboardKey::D),
    u8(EKeyboardKey::E),
    u8(EKeyboardKey::F),
    u8(EKeyboardKey::G),
    u8(EKeyboardKey::H),
    u8(EKeyboardKey::I),
    u8(EKeyboardKey::J),
    u8(EKeyboardKey::K),
    u8(EKeyboardKey::L),
    u8(EKeyboardKey::M),
    u8(EKeyboardKey::N),
    u8(EKeyboardKey::O),
    u8(EKeyboardKey::P),
    u8(EKeyboardKey::Q),
    u8(EKeyboardKey::R),
    u8(EKeyboardKey::S),
    u8(EKeyboardKey::T),
    u8(EKeyboardKey::U),
    u8(EKeyboardKey::V),
    u8(EKeyboardKey::W),
    u8(EKeyboardKey::X),
    u8(EKeyboardKey::Y),
    u8(EKeyboardKey::Z),
    0xFF,
    0xFF,
    u8(EKeyboardKey::Menu),
    0xFF,
    0xFF,
    u8(EKeyboardKey::Numpad0),
    u8(EKeyboardKey::Numpad1),
    u8(EKeyboardKey::Numpad2),
    u8(EKeyboardKey::Numpad3),
    u8(EKeyboardKey::Numpad4),
    u8(EKeyboardKey::Numpad5),
    u8(EKeyboardKey::Numpad6),
    u8(EKeyboardKey::Numpad7),
    u8(EKeyboardKey::Numpad8),
    u8(EKeyboardKey::Numpad9),
    u8(EKeyboardKey::Multiply),
    u8(EKeyboardKey::Add),
    0xFF,
    u8(EKeyboardKey::Subtract),
    0xFF,
    u8(EKeyboardKey::Divide),
    u8(EKeyboardKey::F1),
    u8(EKeyboardKey::F2),
    u8(EKeyboardKey::F3),
    u8(EKeyboardKey::F4),
    u8(EKeyboardKey::F5),
    u8(EKeyboardKey::F6),
    u8(EKeyboardKey::F7),
    u8(EKeyboardKey::F8),
    u8(EKeyboardKey::F9),
    u8(EKeyboardKey::F10),
    u8(EKeyboardKey::F11),
    u8(EKeyboardKey::F12),
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    u8(EKeyboardKey::ScrollLock),
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
};
//----------------------------------------------------------------------------
static bool KeyboarSetKeyDown_(FKeyboardState* keyboard, ::WPARAM vkey) {
    const u8 key = GVirtualKey_to_KeyboardKey[checked_cast<u8>(vkey)];

    if (key != 0xFF) {
        keyboard->SetKeyDown(EKeyboardKey(key));
        return true;
    }
    else {
        return false;
    }
}
//----------------------------------------------------------------------------
static bool KeyboarSetKeyUp_(FKeyboardState* keyboard, ::WPARAM vkey) {
    const u8 key = GVirtualKey_to_KeyboardKey[checked_cast<u8>(vkey)];

    if (key != 0xFF) {
        keyboard->SetKeyUp(EKeyboardKey(key));
        return true;
    }
    else {
        return false;
    }
}
//----------------------------------------------------------------------------
static bool KeyboardMessageHandler_(FKeyboardState* keyboard, const FWindowsPlatformMessageHandler::FMessage& msg) {
    Assert(keyboard);

    switch (msg.Type) {
    case EWindowsMessageType::KeyDown:
    case EWindowsMessageType::SysKeyDown:
        return KeyboarSetKeyDown_(keyboard, msg.WParam);

    case EWindowsMessageType::KeyUp:
    case EWindowsMessageType::SysKeyUp:
        return KeyboarSetKeyUp_(keyboard, msg.WParam);

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
