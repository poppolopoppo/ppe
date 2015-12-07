#pragma once

#include "Core.Application/Application.h"

#include "Core.Graphics/Window/WindowMessageHandler.h"

#include "Core.Application/Input/MouseState.h"

namespace Core {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class MouseInputHandler :
    public Graphics::IWindowMessageHandler
,   public IInputStateProvider<MouseState> {
public:
    MouseInputHandler();
    virtual ~MouseInputHandler();

    MouseInputHandler(const MouseInputHandler& ) = delete;
    MouseInputHandler& operator =(const MouseInputHandler& ) = delete;

    virtual const MouseState& State() const override { return _state; }

    virtual void RegisterMessageDelegates(Graphics::BasicWindow *wnd) override;
    virtual void UnregisterMessageDelegates(Graphics::BasicWindow *wnd) override;

    virtual void UpdateBeforeDispatch(Graphics::BasicWindow *wnd) override;
    virtual void UpdateAfterDispatch(Graphics::BasicWindow *wnd) override;

protected:
    static Graphics::MessageResult OnMouseMove_(Graphics::IWindowMessageHandler *handler, Graphics::BasicWindow *wnd, Graphics::WindowMessage msg, Graphics::MessageLParam lparam, Graphics::MessageWParam wparam);
    static Graphics::MessageResult OnMouseLButtonDown_(Graphics::IWindowMessageHandler *handler, Graphics::BasicWindow *wnd, Graphics::WindowMessage msg, Graphics::MessageLParam lparam, Graphics::MessageWParam wparam);
    static Graphics::MessageResult OnMouseLButtonUp_(Graphics::IWindowMessageHandler *handler, Graphics::BasicWindow *wnd, Graphics::WindowMessage msg, Graphics::MessageLParam lparam, Graphics::MessageWParam wparam);
    static Graphics::MessageResult OnMouseRButtonDown_(Graphics::IWindowMessageHandler *handler, Graphics::BasicWindow *wnd, Graphics::WindowMessage msg, Graphics::MessageLParam lparam, Graphics::MessageWParam wparam);
    static Graphics::MessageResult OnMouseRButtonUp_(Graphics::IWindowMessageHandler *handler, Graphics::BasicWindow *wnd, Graphics::WindowMessage msg, Graphics::MessageLParam lparam, Graphics::MessageWParam wparam);
    static Graphics::MessageResult OnMouseMButtonDown_(Graphics::IWindowMessageHandler *handler, Graphics::BasicWindow *wnd, Graphics::WindowMessage msg, Graphics::MessageLParam lparam, Graphics::MessageWParam wparam);
    static Graphics::MessageResult OnMouseMButtonUp_(Graphics::IWindowMessageHandler *handler, Graphics::BasicWindow *wnd, Graphics::WindowMessage msg, Graphics::MessageLParam lparam, Graphics::MessageWParam wparam);

private:
    MouseState _state;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core
