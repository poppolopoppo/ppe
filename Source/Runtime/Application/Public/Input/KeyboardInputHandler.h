#pragma once

#include "Application.h"

#include "Window/WindowMessageHandler.h"

#include "Input/KeyboardState.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FKeyboardInputHandler :
    public Graphics::IWindowMessageHandler
,   public IKeyboardService {
public:
    FKeyboardInputHandler();
    virtual ~FKeyboardInputHandler();

    FKeyboardInputHandler(const FKeyboardInputHandler& ) = delete;
    FKeyboardInputHandler& operator =(const FKeyboardInputHandler& ) = delete;

    virtual const FKeyboardState& State() const override final;

    virtual void RegisterMessageDelegates(Graphics::FBasicWindow *wnd) override final;
    virtual void UnregisterMessageDelegates(Graphics::FBasicWindow *wnd) override final;

    virtual void UpdateBeforeDispatch(Graphics::FBasicWindow *wnd) override final;
    virtual void UpdateAfterDispatch(Graphics::FBasicWindow *wnd) override final;

protected:
    static Graphics::MessageResult OnKeyboardKeyDown_(Graphics::IWindowMessageHandler *handler, Graphics::FBasicWindow *wnd, Graphics::EWindowMessage msg, Graphics::MessageLParam lparam, Graphics::MessageWParam wparam);
    static Graphics::MessageResult OnKeyboardKeyUp_(Graphics::IWindowMessageHandler *handler, Graphics::FBasicWindow *wnd, Graphics::EWindowMessage msg, Graphics::MessageLParam lparam, Graphics::MessageWParam wparam);
    static Graphics::MessageResult OnKeyboardSysKeyDown_(Graphics::IWindowMessageHandler *handler, Graphics::FBasicWindow *wnd, Graphics::EWindowMessage msg, Graphics::MessageLParam lparam, Graphics::MessageWParam wparam);
    static Graphics::MessageResult OnKeyboardSysKeyUp_(Graphics::IWindowMessageHandler *handler, Graphics::FBasicWindow *wnd, Graphics::EWindowMessage msg, Graphics::MessageLParam lparam, Graphics::MessageWParam wparam);

private:
    FKeyboardState _state;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
