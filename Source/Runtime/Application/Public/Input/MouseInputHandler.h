#pragma once

#include "Application.h"

#include "Window/WindowMessageHandler.h"

#include "Input/MouseState.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FMouseInputHandler :
    public Graphics::IWindowMessageHandler
,   public IInputStateProvider<FMouseState> {
public:
    FMouseInputHandler();
    virtual ~FMouseInputHandler();

    FMouseInputHandler(const FMouseInputHandler& ) = delete;
    FMouseInputHandler& operator =(const FMouseInputHandler& ) = delete;

    virtual const FMouseState& State() const override final;

    virtual void RegisterMessageDelegates(Graphics::FBasicWindow *wnd) override final;
    virtual void UnregisterMessageDelegates(Graphics::FBasicWindow *wnd) override final;

    virtual void UpdateBeforeDispatch(Graphics::FBasicWindow *wnd) override final;
    virtual void UpdateAfterDispatch(Graphics::FBasicWindow *wnd) override final;

protected:
    static Graphics::MessageResult OnMouseMove_(Graphics::IWindowMessageHandler *handler, Graphics::FBasicWindow *wnd, Graphics::EWindowMessage msg, Graphics::MessageLParam lparam, Graphics::MessageWParam wparam);
    static Graphics::MessageResult OnMouseLButtonDown_(Graphics::IWindowMessageHandler *handler, Graphics::FBasicWindow *wnd, Graphics::EWindowMessage msg, Graphics::MessageLParam lparam, Graphics::MessageWParam wparam);
    static Graphics::MessageResult OnMouseLButtonUp_(Graphics::IWindowMessageHandler *handler, Graphics::FBasicWindow *wnd, Graphics::EWindowMessage msg, Graphics::MessageLParam lparam, Graphics::MessageWParam wparam);
    static Graphics::MessageResult OnMouseRButtonDown_(Graphics::IWindowMessageHandler *handler, Graphics::FBasicWindow *wnd, Graphics::EWindowMessage msg, Graphics::MessageLParam lparam, Graphics::MessageWParam wparam);
    static Graphics::MessageResult OnMouseRButtonUp_(Graphics::IWindowMessageHandler *handler, Graphics::FBasicWindow *wnd, Graphics::EWindowMessage msg, Graphics::MessageLParam lparam, Graphics::MessageWParam wparam);
    static Graphics::MessageResult OnMouseMButtonDown_(Graphics::IWindowMessageHandler *handler, Graphics::FBasicWindow *wnd, Graphics::EWindowMessage msg, Graphics::MessageLParam lparam, Graphics::MessageWParam wparam);
    static Graphics::MessageResult OnMouseMButtonUp_(Graphics::IWindowMessageHandler *handler, Graphics::FBasicWindow *wnd, Graphics::EWindowMessage msg, Graphics::MessageLParam lparam, Graphics::MessageWParam wparam);

private:
    FMouseState _state;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE