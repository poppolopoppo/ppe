#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Graphics/Window/WindowMessageHandler.h"

#include "Core.Engine/Input/InputState.h"
#include "Core.Engine/Input/State/MouseButton.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class MouseState {
public:
    friend class MouseInputHandler;

    MouseState();
    ~MouseState();

    int X() const { return _x; }
    int Y() const { return _y; }

    const MouseButtonState& ButtonsDown() const { return _buttonsDown; }
    const MouseButtonState& ButtonsPressed() const { return _buttonsPressed; }
    const MouseButtonState& ButtonsUp() const { return _buttonsUp; }

    void Clear();

private:
    int _x, _y;

    MouseButtonState _buttonsDown;
    MouseButtonState _buttonsPressed;
    MouseButtonState _buttonsUp;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class MouseInputHandler : public Graphics::IWindowMessageHandler {
public:
    MouseInputHandler();
    virtual ~MouseInputHandler();

    MouseInputHandler(const MouseInputHandler& ) = delete;
    MouseInputHandler& operator =(const MouseInputHandler& ) = delete;

    const MouseState& State() const { return _state; }

    virtual void RegisterMessageDelegates(Graphics::BasicWindow *wnd) override;
    virtual void UnregisterMessageDelegates(Graphics::BasicWindow *wnd) override;

    virtual void UpdateBeforeDispatch(Graphics::BasicWindow *wnd) override;
    virtual void UpdateAfterDispatch(Graphics::BasicWindow *wnd) override;

    int ClientX() const { return _state.X(); }
    int ClientY() const { return _state.Y(); }

    bool IsButtonDown(MouseButton btn) const { return _state._buttonsDown.Contains(btn); }
    bool IsButtonPressed(MouseButton btn) const { return _state._buttonsPressed.Contains(btn); }
    bool IsButtonUp(MouseButton btn) const { return _state._buttonsUp.Contains(btn); }

    void ClearState() { _state.Clear(); }

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
} //!namespace Engine
} //!namespace Core
