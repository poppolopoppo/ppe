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

namespace Core {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FMouseInputHandler::FMouseInputHandler() {}
//----------------------------------------------------------------------------
FMouseInputHandler::~FMouseInputHandler() {}
//----------------------------------------------------------------------------
const FMouseState& FMouseInputHandler::State() const {
	AssertIsMainThread();
	return _state;
}
//----------------------------------------------------------------------------
void FMouseInputHandler::RegisterMessageDelegates(Graphics::FBasicWindow *wnd) {
	AssertIsMainThread();
    Assert(wnd);

    RegisterMessageDelegate(wnd, Graphics::EWindowMessage::MouseMove, &FMouseInputHandler::OnMouseMove_);
    RegisterMessageDelegate(wnd, Graphics::EWindowMessage::LButtonDown, &FMouseInputHandler::OnMouseLButtonDown_);
    RegisterMessageDelegate(wnd, Graphics::EWindowMessage::LButtonUp, &FMouseInputHandler::OnMouseLButtonUp_);
    RegisterMessageDelegate(wnd, Graphics::EWindowMessage::RButtonDown, &FMouseInputHandler::OnMouseRButtonDown_);
    RegisterMessageDelegate(wnd, Graphics::EWindowMessage::RButtonUp, &FMouseInputHandler::OnMouseRButtonUp_);
    RegisterMessageDelegate(wnd, Graphics::EWindowMessage::MButtonDown, &FMouseInputHandler::OnMouseMButtonDown_);
    RegisterMessageDelegate(wnd, Graphics::EWindowMessage::MButtonUp, &FMouseInputHandler::OnMouseMButtonUp_);
}
//----------------------------------------------------------------------------
void FMouseInputHandler::UnregisterMessageDelegates(Graphics::FBasicWindow *wnd) {
	AssertIsMainThread();
    Assert(wnd);

    UnregisterMessageDelegate(wnd, Graphics::EWindowMessage::MouseMove, &FMouseInputHandler::OnMouseMove_);
    UnregisterMessageDelegate(wnd, Graphics::EWindowMessage::LButtonDown, &FMouseInputHandler::OnMouseLButtonDown_);
    UnregisterMessageDelegate(wnd, Graphics::EWindowMessage::LButtonUp, &FMouseInputHandler::OnMouseLButtonUp_);
    UnregisterMessageDelegate(wnd, Graphics::EWindowMessage::RButtonDown, &FMouseInputHandler::OnMouseRButtonDown_);
    UnregisterMessageDelegate(wnd, Graphics::EWindowMessage::RButtonUp, &FMouseInputHandler::OnMouseRButtonUp_);
    UnregisterMessageDelegate(wnd, Graphics::EWindowMessage::MButtonDown, &FMouseInputHandler::OnMouseMButtonDown_);
    UnregisterMessageDelegate(wnd, Graphics::EWindowMessage::MButtonUp, &FMouseInputHandler::OnMouseMButtonUp_);
}
//----------------------------------------------------------------------------
void FMouseInputHandler::UpdateBeforeDispatch(Graphics::FBasicWindow *wnd) {
	AssertIsMainThread();
    UNUSED(wnd);

    _state._buttonsDown.Clear();
    _state._buttonsUp.Clear();
}
//----------------------------------------------------------------------------
void FMouseInputHandler::UpdateAfterDispatch(Graphics::FBasicWindow *wnd) {
	AssertIsMainThread();
    UNUSED(wnd);

    for (EMouseButton btn : _state._buttonsUp.MakeView()) {
#ifdef WITH_MOUSESTATE_VERBOSE
        LOG(Info, L"[Mouse] button {0} up", MouseButtonToCStr(btn));
#endif
        _state._buttonsPressed.Remove_ReturnIfExists/*Remove_AssertExists echec cuisant... emulation? */(btn);
        _state._buttonsDown.Remove_ReturnIfExists(btn);
    }

    for (EMouseButton btn : _state._buttonsDown.MakeView()) {
#ifdef WITH_MOUSESTATE_VERBOSE
        LOG(Info, L"[Mouse] button {0} down", MouseButtonToCStr(btn));
#endif
        _state._buttonsPressed.Add_KeepExisting(btn);
    }
}
//----------------------------------------------------------------------------
Graphics::MessageResult FMouseInputHandler::OnMouseMove_(Graphics::IWindowMessageHandler *handler, Graphics::FBasicWindow *wnd, Graphics::EWindowMessage msg, Graphics::MessageLParam lparam, Graphics::MessageWParam wparam) {
	AssertIsMainThread();
    UNUSED(msg);
    UNUSED(wparam);

    FMouseInputHandler *const mouse = checked_cast<FMouseInputHandler *>(handler);
    mouse->_state._clientX = checked_cast<int>(LOWORD(lparam));
    mouse->_state._clientY = checked_cast<int>(HIWORD(lparam));

    mouse->_state._relativeX = float(mouse->_state._clientX)/wnd->Width();
    mouse->_state._relativeY = float(mouse->_state._clientY)/wnd->Height();

    return 0;
}
//----------------------------------------------------------------------------
Graphics::MessageResult FMouseInputHandler::OnMouseLButtonDown_(Graphics::IWindowMessageHandler *handler, Graphics::FBasicWindow *wnd, Graphics::EWindowMessage msg, Graphics::MessageLParam lparam, Graphics::MessageWParam wparam) {
	AssertIsMainThread();
    UNUSED(wnd);
    UNUSED(msg);
    UNUSED(lparam);
    UNUSED(wparam);

    FMouseInputHandler *const mouse = checked_cast<FMouseInputHandler *>(handler);
    mouse->_state._buttonsDown.Add_AssertUnique(EMouseButton::Button0);
    return 0;
}
//----------------------------------------------------------------------------
Graphics::MessageResult FMouseInputHandler::OnMouseLButtonUp_(Graphics::IWindowMessageHandler *handler, Graphics::FBasicWindow *wnd, Graphics::EWindowMessage msg, Graphics::MessageLParam lparam, Graphics::MessageWParam wparam) {
	AssertIsMainThread();
    UNUSED(wnd);
    UNUSED(msg);
    UNUSED(lparam);
    UNUSED(wparam);

    FMouseInputHandler *const mouse = checked_cast<FMouseInputHandler *>(handler);
    mouse->_state._buttonsUp.Add_AssertUnique(EMouseButton::Button0);
    return 0;
}
//----------------------------------------------------------------------------
Graphics::MessageResult FMouseInputHandler::OnMouseRButtonDown_(Graphics::IWindowMessageHandler *handler, Graphics::FBasicWindow *wnd, Graphics::EWindowMessage msg, Graphics::MessageLParam lparam, Graphics::MessageWParam wparam) {
	AssertIsMainThread();
    UNUSED(wnd);
    UNUSED(msg);
    UNUSED(lparam);
    UNUSED(wparam);

    FMouseInputHandler *const mouse = checked_cast<FMouseInputHandler *>(handler);
    mouse->_state._buttonsDown.Add_AssertUnique(EMouseButton::Button1);
    return 0;
}
//----------------------------------------------------------------------------
Graphics::MessageResult FMouseInputHandler::OnMouseRButtonUp_(Graphics::IWindowMessageHandler *handler, Graphics::FBasicWindow *wnd, Graphics::EWindowMessage msg, Graphics::MessageLParam lparam, Graphics::MessageWParam wparam) {
	AssertIsMainThread();
    UNUSED(wnd);
    UNUSED(msg);
    UNUSED(lparam);
    UNUSED(wparam);

    FMouseInputHandler *const mouse = checked_cast<FMouseInputHandler *>(handler);
    mouse->_state._buttonsUp.Add_AssertUnique(EMouseButton::Button1);
    return 0;
}
//----------------------------------------------------------------------------
Graphics::MessageResult FMouseInputHandler::OnMouseMButtonDown_(Graphics::IWindowMessageHandler *handler, Graphics::FBasicWindow *wnd, Graphics::EWindowMessage msg, Graphics::MessageLParam lparam, Graphics::MessageWParam wparam) {
	AssertIsMainThread();
    UNUSED(wnd);
    UNUSED(msg);
    UNUSED(lparam);
    UNUSED(wparam);

    FMouseInputHandler *const mouse = checked_cast<FMouseInputHandler *>(handler);
    mouse->_state._buttonsDown.Add_AssertUnique(EMouseButton::Button2);
    return 0;
}
//----------------------------------------------------------------------------
Graphics::MessageResult FMouseInputHandler::OnMouseMButtonUp_(Graphics::IWindowMessageHandler *handler, Graphics::FBasicWindow *wnd, Graphics::EWindowMessage msg, Graphics::MessageLParam lparam, Graphics::MessageWParam wparam) {
	AssertIsMainThread();
    UNUSED(wnd);
    UNUSED(msg);
    UNUSED(lparam);
    UNUSED(wparam);

    FMouseInputHandler *const mouse = checked_cast<FMouseInputHandler *>(handler);
    mouse->_state._buttonsUp.Add_AssertUnique(EMouseButton::Button2);
    return 0;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core
