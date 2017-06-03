#include "stdafx.h"

#include "KeyboardInputHandler.h"

#include "Core.Graphics/Window/BasicWindow.h"
#include "Core.Graphics/Window/WindowMessage.h"

//#define WITH_KEYBOARDSTATE_VERBOSE //%_NOCOMMIT%

#ifdef WITH_KEYBOARDSTATE_VERBOSE
#   include "Core/Diagnostic/Logger.h"
#endif

#ifdef PLATFORM_WINDOWS
#   include "Core/Misc/Platform_Windows.h"
#else
#   error "no support"
#endif

#pragma warning( push )
#pragma warning( disable : 4100 ) // C4100 'XXX'�: param�tre formel non r�f�renc�

namespace Core {
namespace Engine {
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
static void AddVirtualKey_IfMapped_(KeyboardKeyState& state, Graphics::MessageWParam virtualKey) {
    const u8 keyboardKey = GVirtualKey_to_KeyboardKey[checked_cast<u8>(virtualKey)];

    if (keyboardKey != 0xFF)
        state.Add_KeepExisting(EKeyboardKey(keyboardKey));
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FKeyboardState::FKeyboardState() {}
//----------------------------------------------------------------------------
FKeyboardState::~FKeyboardState() {}
//----------------------------------------------------------------------------
void FKeyboardState::Clear() {
    _keysDown.Clear();
    _keysPressed.Clear();
    _keysUp.Clear();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FKeyboardInputHandler::FKeyboardInputHandler() {}
//----------------------------------------------------------------------------
FKeyboardInputHandler::~FKeyboardInputHandler() {}
//----------------------------------------------------------------------------
void FKeyboardInputHandler::RegisterMessageDelegates(Graphics::FBasicWindow *wnd) {
    Assert(wnd);

    RegisterMessageDelegate(wnd, Graphics::EWindowMessage::KeyDown, &FKeyboardInputHandler::OnKeyboardKeyDown_);
    RegisterMessageDelegate(wnd, Graphics::EWindowMessage::KeyUp, &FKeyboardInputHandler::OnKeyboardKeyUp_);
    RegisterMessageDelegate(wnd, Graphics::EWindowMessage::SysKeyDown, &FKeyboardInputHandler::OnKeyboardSysKeyDown_);
    RegisterMessageDelegate(wnd, Graphics::EWindowMessage::SysKeyUp, &FKeyboardInputHandler::OnKeyboardSysKeyUp_);
}
//----------------------------------------------------------------------------
void FKeyboardInputHandler::UnregisterMessageDelegates(Graphics::FBasicWindow *wnd) {
    Assert(wnd);

    UnregisterMessageDelegate(wnd, Graphics::EWindowMessage::KeyDown, &FKeyboardInputHandler::OnKeyboardKeyDown_);
    UnregisterMessageDelegate(wnd, Graphics::EWindowMessage::KeyUp, &FKeyboardInputHandler::OnKeyboardKeyUp_);
    UnregisterMessageDelegate(wnd, Graphics::EWindowMessage::SysKeyDown, &FKeyboardInputHandler::OnKeyboardSysKeyDown_);
    UnregisterMessageDelegate(wnd, Graphics::EWindowMessage::SysKeyUp, &FKeyboardInputHandler::OnKeyboardSysKeyUp_);
}
//----------------------------------------------------------------------------
void FKeyboardInputHandler::UpdateBeforeDispatch(Graphics::FBasicWindow *wnd) {
    _state._keysDown.Clear();
    _state._keysUp.Clear();

    if (!wnd->HasFocus())
        _state._keysPressed.Clear();
}
//----------------------------------------------------------------------------
void FKeyboardInputHandler::UpdateAfterDispatch(Graphics::FBasicWindow *wnd) {
    for (EKeyboardKey key : _state._keysUp.MakeView()) {
#ifdef WITH_KEYBOARDSTATE_VERBOSE
        LOG(Info, L"[Keyboard] key {0} up", KeyboardKeyToCStr(key));
#endif
        _state._keysPressed.Remove_ReturnIfExists/*Remove_AssertExists echec cuisant... emulation? */(key);
        _state._keysDown.Remove_ReturnIfExists(key);
    }

    for (EKeyboardKey key : _state._keysDown.MakeView()) {
#ifdef WITH_KEYBOARDSTATE_VERBOSE
        LOG(Info, L"[Keyboard] key {0} down", KeyboardKeyToCStr(key));
#endif
        _state._keysPressed.Add_KeepExisting(std::move(key));
    }
}
//----------------------------------------------------------------------------
Graphics::MessageResult FKeyboardInputHandler::OnKeyboardKeyDown_(Graphics::IWindowMessageHandler *handler, Graphics::FBasicWindow *wnd, Graphics::EWindowMessage msg, Graphics::MessageLParam lparam, Graphics::MessageWParam wparam) {
    FKeyboardInputHandler *const keyboard = checked_cast<FKeyboardInputHandler *>(handler);
    AddVirtualKey_IfMapped_(keyboard->_state._keysDown, wparam);
    return 0;
}
//----------------------------------------------------------------------------
Graphics::MessageResult FKeyboardInputHandler::OnKeyboardKeyUp_(Graphics::IWindowMessageHandler *handler, Graphics::FBasicWindow *wnd, Graphics::EWindowMessage msg, Graphics::MessageLParam lparam, Graphics::MessageWParam wparam) {
    FKeyboardInputHandler *const keyboard = checked_cast<FKeyboardInputHandler *>(handler);
    AddVirtualKey_IfMapped_(keyboard->_state._keysUp, wparam);
    return 0;
}
//----------------------------------------------------------------------------
Graphics::MessageResult FKeyboardInputHandler::OnKeyboardSysKeyDown_(Graphics::IWindowMessageHandler *handler, Graphics::FBasicWindow *wnd, Graphics::EWindowMessage msg, Graphics::MessageLParam lparam, Graphics::MessageWParam wparam) {
    FKeyboardInputHandler *const keyboard = checked_cast<FKeyboardInputHandler *>(handler);
    AddVirtualKey_IfMapped_(keyboard->_state._keysDown, wparam);
    return 0;
}
//----------------------------------------------------------------------------
Graphics::MessageResult FKeyboardInputHandler::OnKeyboardSysKeyUp_(Graphics::IWindowMessageHandler *handler, Graphics::FBasicWindow *wnd, Graphics::EWindowMessage msg, Graphics::MessageLParam lparam, Graphics::MessageWParam wparam) {
    FKeyboardInputHandler *const keyboard = checked_cast<FKeyboardInputHandler *>(handler);
    AddVirtualKey_IfMapped_(keyboard->_state._keysUp, wparam);
    return 0;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core

#pragma warning( pop )
