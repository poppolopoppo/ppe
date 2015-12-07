#pragma once

#include "Core.Application/Application.h"

#include "Core.Graphics/Window/WindowMessageHandler.h"

#include "Core.Application/Input/KeyboardState.h"

namespace Core {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class KeyboardInputHandler :
    public Graphics::IWindowMessageHandler
,   public IKeyboardService {
public:
    KeyboardInputHandler();
    virtual ~KeyboardInputHandler();

    KeyboardInputHandler(const KeyboardInputHandler& ) = delete;
    KeyboardInputHandler& operator =(const KeyboardInputHandler& ) = delete;

    virtual const KeyboardState& State() const override { return _state; }

    virtual void RegisterMessageDelegates(Graphics::BasicWindow *wnd) override;
    virtual void UnregisterMessageDelegates(Graphics::BasicWindow *wnd) override;

    virtual void UpdateBeforeDispatch(Graphics::BasicWindow *wnd) override;
    virtual void UpdateAfterDispatch(Graphics::BasicWindow *wnd) override;

protected:
    static Graphics::MessageResult OnKeyboardKeyDown_(Graphics::IWindowMessageHandler *handler, Graphics::BasicWindow *wnd, Graphics::WindowMessage msg, Graphics::MessageLParam lparam, Graphics::MessageWParam wparam);
    static Graphics::MessageResult OnKeyboardKeyUp_(Graphics::IWindowMessageHandler *handler, Graphics::BasicWindow *wnd, Graphics::WindowMessage msg, Graphics::MessageLParam lparam, Graphics::MessageWParam wparam);
    static Graphics::MessageResult OnKeyboardSysKeyDown_(Graphics::IWindowMessageHandler *handler, Graphics::BasicWindow *wnd, Graphics::WindowMessage msg, Graphics::MessageLParam lparam, Graphics::MessageWParam wparam);
    static Graphics::MessageResult OnKeyboardSysKeyUp_(Graphics::IWindowMessageHandler *handler, Graphics::BasicWindow *wnd, Graphics::WindowMessage msg, Graphics::MessageLParam lparam, Graphics::MessageWParam wparam);

private:
    KeyboardState _state;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core
