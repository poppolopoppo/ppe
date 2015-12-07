#include "stdafx.h"

#include "MouseInputHandler.h"

#include "Core.Graphics/Window/BasicWindow.h"
#include "Core.Graphics/Window/WindowMessage.h"

//#define WITH_MOUSESTATE_VERBOSE //%_NOCOMMIT%

#ifdef WITH_MOUSESTATE_VERBOSE
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
MouseInputHandler::MouseInputHandler() {}
//----------------------------------------------------------------------------
MouseInputHandler::~MouseInputHandler() {}
//----------------------------------------------------------------------------
void MouseInputHandler::RegisterMessageDelegates(Graphics::BasicWindow *wnd) {
    Assert(wnd);

    RegisterMessageDelegate(wnd, Graphics::WindowMessage::MouseMove, &MouseInputHandler::OnMouseMove_);
    RegisterMessageDelegate(wnd, Graphics::WindowMessage::LButtonDown, &MouseInputHandler::OnMouseLButtonDown_);
    RegisterMessageDelegate(wnd, Graphics::WindowMessage::LButtonUp, &MouseInputHandler::OnMouseLButtonUp_);
    RegisterMessageDelegate(wnd, Graphics::WindowMessage::RButtonDown, &MouseInputHandler::OnMouseRButtonDown_);
    RegisterMessageDelegate(wnd, Graphics::WindowMessage::RButtonUp, &MouseInputHandler::OnMouseRButtonUp_);
    RegisterMessageDelegate(wnd, Graphics::WindowMessage::MButtonDown, &MouseInputHandler::OnMouseMButtonDown_);
    RegisterMessageDelegate(wnd, Graphics::WindowMessage::MButtonUp, &MouseInputHandler::OnMouseMButtonUp_);
}
//----------------------------------------------------------------------------
void MouseInputHandler::UnregisterMessageDelegates(Graphics::BasicWindow *wnd) {
    Assert(wnd);

    UnregisterMessageDelegate(wnd, Graphics::WindowMessage::MouseMove, &MouseInputHandler::OnMouseMove_);
    UnregisterMessageDelegate(wnd, Graphics::WindowMessage::LButtonDown, &MouseInputHandler::OnMouseLButtonDown_);
    UnregisterMessageDelegate(wnd, Graphics::WindowMessage::LButtonUp, &MouseInputHandler::OnMouseLButtonUp_);
    UnregisterMessageDelegate(wnd, Graphics::WindowMessage::RButtonDown, &MouseInputHandler::OnMouseRButtonDown_);
    UnregisterMessageDelegate(wnd, Graphics::WindowMessage::RButtonUp, &MouseInputHandler::OnMouseRButtonUp_);
    UnregisterMessageDelegate(wnd, Graphics::WindowMessage::MButtonDown, &MouseInputHandler::OnMouseMButtonDown_);
    UnregisterMessageDelegate(wnd, Graphics::WindowMessage::MButtonUp, &MouseInputHandler::OnMouseMButtonUp_);
}
//----------------------------------------------------------------------------
void MouseInputHandler::UpdateBeforeDispatch(Graphics::BasicWindow *wnd) {
    _state._buttonsDown.Clear();
    _state._buttonsUp.Clear();
}
//----------------------------------------------------------------------------
void MouseInputHandler::UpdateAfterDispatch(Graphics::BasicWindow *wnd) {
    for (MouseButton btn : _state._buttonsUp.MakeView()) {
#ifdef WITH_MOUSESTATE_VERBOSE
        LOG(Information, L"[Mouse] button {0} up", u32(btn));
#endif
        _state._buttonsPressed.Remove_ReturnIfExists/*Remove_AssertExists echec cuisant... emulation? */(btn);
        _state._buttonsDown.Remove_ReturnIfExists(btn);
    }

    for (MouseButton btn : _state._buttonsDown.MakeView()) {
#ifdef WITH_MOUSESTATE_VERBOSE
        LOG(Information, L"[Mouse] button {0} down", u32(btn));
#endif
        _state._buttonsPressed.Add_KeepExisting(std::move(btn));
    }
}
//----------------------------------------------------------------------------
Graphics::MessageResult MouseInputHandler::OnMouseMove_(Graphics::IWindowMessageHandler *handler, Graphics::BasicWindow *wnd, Graphics::WindowMessage msg, Graphics::MessageLParam lparam, Graphics::MessageWParam wparam) {
    MouseInputHandler *const mouse = checked_cast<MouseInputHandler *>(handler);
    mouse->_state._clientX = checked_cast<int>(LOWORD(lparam));
    mouse->_state._clientY = checked_cast<int>(HIWORD(lparam));

    mouse->_state._relativeX = float(mouse->_state._clientX)/wnd->Width();
    mouse->_state._relativeY = float(mouse->_state._clientY)/wnd->Height();

    return 0;
}
//----------------------------------------------------------------------------
Graphics::MessageResult MouseInputHandler::OnMouseLButtonDown_(Graphics::IWindowMessageHandler *handler, Graphics::BasicWindow *wnd, Graphics::WindowMessage msg, Graphics::MessageLParam lparam, Graphics::MessageWParam wparam) {
    MouseInputHandler *const mouse = checked_cast<MouseInputHandler *>(handler);
    mouse->_state._buttonsDown.Add_AssertUnique(MouseButton::Button0);
    return 0;
}
//----------------------------------------------------------------------------
Graphics::MessageResult MouseInputHandler::OnMouseLButtonUp_(Graphics::IWindowMessageHandler *handler, Graphics::BasicWindow *wnd, Graphics::WindowMessage msg, Graphics::MessageLParam lparam, Graphics::MessageWParam wparam) {
    MouseInputHandler *const mouse = checked_cast<MouseInputHandler *>(handler);
    mouse->_state._buttonsUp.Add_AssertUnique(MouseButton::Button0);
    return 0;
}
//----------------------------------------------------------------------------
Graphics::MessageResult MouseInputHandler::OnMouseRButtonDown_(Graphics::IWindowMessageHandler *handler, Graphics::BasicWindow *wnd, Graphics::WindowMessage msg, Graphics::MessageLParam lparam, Graphics::MessageWParam wparam) {
    MouseInputHandler *const mouse = checked_cast<MouseInputHandler *>(handler);
    mouse->_state._buttonsDown.Add_AssertUnique(MouseButton::Button1);
    return 0;
}
//----------------------------------------------------------------------------
Graphics::MessageResult MouseInputHandler::OnMouseRButtonUp_(Graphics::IWindowMessageHandler *handler, Graphics::BasicWindow *wnd, Graphics::WindowMessage msg, Graphics::MessageLParam lparam, Graphics::MessageWParam wparam) {
    MouseInputHandler *const mouse = checked_cast<MouseInputHandler *>(handler);
    mouse->_state._buttonsUp.Add_AssertUnique(MouseButton::Button1);
    return 0;
}
//----------------------------------------------------------------------------
Graphics::MessageResult MouseInputHandler::OnMouseMButtonDown_(Graphics::IWindowMessageHandler *handler, Graphics::BasicWindow *wnd, Graphics::WindowMessage msg, Graphics::MessageLParam lparam, Graphics::MessageWParam wparam) {
    MouseInputHandler *const mouse = checked_cast<MouseInputHandler *>(handler);
    mouse->_state._buttonsDown.Add_AssertUnique(MouseButton::Button2);
    return 0;
}
//----------------------------------------------------------------------------
Graphics::MessageResult MouseInputHandler::OnMouseMButtonUp_(Graphics::IWindowMessageHandler *handler, Graphics::BasicWindow *wnd, Graphics::WindowMessage msg, Graphics::MessageLParam lparam, Graphics::MessageWParam wparam) {
    MouseInputHandler *const mouse = checked_cast<MouseInputHandler *>(handler);
    mouse->_state._buttonsUp.Add_AssertUnique(MouseButton::Button2);
    return 0;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core

#pragma warning( pop )
