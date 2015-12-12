#include "stdafx.h"

#include "KeyboardInputHandler.h"

#include "Core.Graphics/Window/BasicWindow.h"
#include "Core.Graphics/Window/WindowMessage.h"

//#define WITH_KEYBOARDSTATE_VERBOSE //%_NOCOMMIT%

#ifdef WITH_KEYBOARDSTATE_VERBOSE
#   include "Core/Diagnostic/Logger.h"
#endif

#ifdef OS_WINDOWS
#   include <Windows.h>
#else
#   error "no support"
#endif

#pragma warning( push )
#pragma warning( disable : 4100 ) // C4100 'XXX' : paramètre formel non référencé

namespace Core {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static const u8 gVirtualKey_to_KeyboardKey[0xFF] = {
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    u8(KeyboardKey::Backspace),
    u8(KeyboardKey::Tab),
    0xFF,
    0xFF,
    0xFF,
    u8(KeyboardKey::Enter),
    0xFF,
    0xFF,
    u8(KeyboardKey::Shift),
    u8(KeyboardKey::Control),
    u8(KeyboardKey::Alt),
    u8(KeyboardKey::Pause),
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    u8(KeyboardKey::Escape),
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    u8(KeyboardKey::Space),
    u8(KeyboardKey::PageUp),
    u8(KeyboardKey::PageDown),
    u8(KeyboardKey::End),
    u8(KeyboardKey::Home),
    u8(KeyboardKey::Left),
    u8(KeyboardKey::Up),
    u8(KeyboardKey::Right),
    u8(KeyboardKey::Down),
    0xFF,
    u8(KeyboardKey::PrintScreen),
    0xFF,
    0xFF,
    u8(KeyboardKey::Insert),
    u8(KeyboardKey::Delete),
    0xFF,
    u8(KeyboardKey::_0),
    u8(KeyboardKey::_1),
    u8(KeyboardKey::_2),
    u8(KeyboardKey::_3),
    u8(KeyboardKey::_4),
    u8(KeyboardKey::_5),
    u8(KeyboardKey::_6),
    u8(KeyboardKey::_7),
    u8(KeyboardKey::_8),
    u8(KeyboardKey::_9),
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    u8(KeyboardKey::A),
    u8(KeyboardKey::B),
    u8(KeyboardKey::C),
    u8(KeyboardKey::D),
    u8(KeyboardKey::E),
    u8(KeyboardKey::F),
    u8(KeyboardKey::G),
    u8(KeyboardKey::H),
    u8(KeyboardKey::I),
    u8(KeyboardKey::J),
    u8(KeyboardKey::K),
    u8(KeyboardKey::L),
    u8(KeyboardKey::M),
    u8(KeyboardKey::N),
    u8(KeyboardKey::O),
    u8(KeyboardKey::P),
    u8(KeyboardKey::Q),
    u8(KeyboardKey::R),
    u8(KeyboardKey::S),
    u8(KeyboardKey::T),
    u8(KeyboardKey::U),
    u8(KeyboardKey::V),
    u8(KeyboardKey::W),
    u8(KeyboardKey::X),
    u8(KeyboardKey::Y),
    u8(KeyboardKey::Z),
    0xFF,
    0xFF,
    u8(KeyboardKey::Menu),
    0xFF,
    0xFF,
    u8(KeyboardKey::Numpad0),
    u8(KeyboardKey::Numpad1),
    u8(KeyboardKey::Numpad2),
    u8(KeyboardKey::Numpad3),
    u8(KeyboardKey::Numpad4),
    u8(KeyboardKey::Numpad5),
    u8(KeyboardKey::Numpad6),
    u8(KeyboardKey::Numpad7),
    u8(KeyboardKey::Numpad8),
    u8(KeyboardKey::Numpad9),
    u8(KeyboardKey::Multiply),
    u8(KeyboardKey::Add),
    0xFF,
    u8(KeyboardKey::Substract),
    0xFF,
    u8(KeyboardKey::Divide),
    u8(KeyboardKey::F1),
    u8(KeyboardKey::F2),
    u8(KeyboardKey::F3),
    u8(KeyboardKey::F4),
    u8(KeyboardKey::F5),
    u8(KeyboardKey::F6),
    u8(KeyboardKey::F7),
    u8(KeyboardKey::F8),
    u8(KeyboardKey::F9),
    u8(KeyboardKey::F10),
    u8(KeyboardKey::F11),
    u8(KeyboardKey::F12),
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
    u8(KeyboardKey::ScrollLock),
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
static void AddVirtualKey_IfMapped_(KeyboardKeyState& state, Graphics::MessageWParam virtualKey) {
    const u8 keyboardKey = gVirtualKey_to_KeyboardKey[checked_cast<u8>(virtualKey)];

    if (keyboardKey != 0xFF)
        state.Add_KeepExisting(KeyboardKey(keyboardKey));
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
KeyboardInputHandler::KeyboardInputHandler() {}
//----------------------------------------------------------------------------
KeyboardInputHandler::~KeyboardInputHandler() {}
//----------------------------------------------------------------------------
void KeyboardInputHandler::RegisterMessageDelegates(Graphics::BasicWindow *wnd) {
    Assert(wnd);

    RegisterMessageDelegate(wnd, Graphics::WindowMessage::KeyDown, &KeyboardInputHandler::OnKeyboardKeyDown_);
    RegisterMessageDelegate(wnd, Graphics::WindowMessage::KeyUp, &KeyboardInputHandler::OnKeyboardKeyUp_);
    RegisterMessageDelegate(wnd, Graphics::WindowMessage::SysKeyDown, &KeyboardInputHandler::OnKeyboardSysKeyDown_);
    RegisterMessageDelegate(wnd, Graphics::WindowMessage::SysKeyUp, &KeyboardInputHandler::OnKeyboardSysKeyUp_);
}
//----------------------------------------------------------------------------
void KeyboardInputHandler::UnregisterMessageDelegates(Graphics::BasicWindow *wnd) {
    Assert(wnd);

    UnregisterMessageDelegate(wnd, Graphics::WindowMessage::KeyDown, &KeyboardInputHandler::OnKeyboardKeyDown_);
    UnregisterMessageDelegate(wnd, Graphics::WindowMessage::KeyUp, &KeyboardInputHandler::OnKeyboardKeyUp_);
    UnregisterMessageDelegate(wnd, Graphics::WindowMessage::SysKeyDown, &KeyboardInputHandler::OnKeyboardSysKeyDown_);
    UnregisterMessageDelegate(wnd, Graphics::WindowMessage::SysKeyUp, &KeyboardInputHandler::OnKeyboardSysKeyUp_);
}
//----------------------------------------------------------------------------
void KeyboardInputHandler::UpdateBeforeDispatch(Graphics::BasicWindow *wnd) {
    _state._keysDown.Clear();
    _state._keysUp.Clear();

    if (!wnd->HasFocus())
        _state._keysPressed.Clear();
}
//----------------------------------------------------------------------------
void KeyboardInputHandler::UpdateAfterDispatch(Graphics::BasicWindow *wnd) {
    for (KeyboardKey key : _state._keysUp.MakeView()) {
#ifdef WITH_KEYBOARDSTATE_VERBOSE
        LOG(Info, L"[Keyboard] key {0} up", KeyboardKeyToCStr(key));
#endif
        _state._keysPressed.Remove_ReturnIfExists/*Remove_AssertExists echec cuisant... emulation? */(key);
        _state._keysDown.Remove_ReturnIfExists(key);
    }

    for (KeyboardKey key : _state._keysDown.MakeView()) {
#ifdef WITH_KEYBOARDSTATE_VERBOSE
        LOG(Info, L"[Keyboard] key {0} down", KeyboardKeyToCStr(key));
#endif
        _state._keysPressed.Add_KeepExisting(std::move(key));
    }
}
//----------------------------------------------------------------------------
Graphics::MessageResult KeyboardInputHandler::OnKeyboardKeyDown_(Graphics::IWindowMessageHandler *handler, Graphics::BasicWindow *wnd, Graphics::WindowMessage msg, Graphics::MessageLParam lparam, Graphics::MessageWParam wparam) {
    KeyboardInputHandler *const keyboard = checked_cast<KeyboardInputHandler *>(handler);
    AddVirtualKey_IfMapped_(keyboard->_state._keysDown, wparam);
    return 0;
}
//----------------------------------------------------------------------------
Graphics::MessageResult KeyboardInputHandler::OnKeyboardKeyUp_(Graphics::IWindowMessageHandler *handler, Graphics::BasicWindow *wnd, Graphics::WindowMessage msg, Graphics::MessageLParam lparam, Graphics::MessageWParam wparam) {
    KeyboardInputHandler *const keyboard = checked_cast<KeyboardInputHandler *>(handler);
    AddVirtualKey_IfMapped_(keyboard->_state._keysUp, wparam);
    return 0;
}
//----------------------------------------------------------------------------
Graphics::MessageResult KeyboardInputHandler::OnKeyboardSysKeyDown_(Graphics::IWindowMessageHandler *handler, Graphics::BasicWindow *wnd, Graphics::WindowMessage msg, Graphics::MessageLParam lparam, Graphics::MessageWParam wparam) {
    KeyboardInputHandler *const keyboard = checked_cast<KeyboardInputHandler *>(handler);
    AddVirtualKey_IfMapped_(keyboard->_state._keysDown, wparam);
    return 0;
}
//----------------------------------------------------------------------------
Graphics::MessageResult KeyboardInputHandler::OnKeyboardSysKeyUp_(Graphics::IWindowMessageHandler *handler, Graphics::BasicWindow *wnd, Graphics::WindowMessage msg, Graphics::MessageLParam lparam, Graphics::MessageWParam wparam) {
    KeyboardInputHandler *const keyboard = checked_cast<KeyboardInputHandler *>(handler);
    AddVirtualKey_IfMapped_(keyboard->_state._keysUp, wparam);
    return 0;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core

#pragma warning( pop )
