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
class FMouseState {
public:
    friend class FMouseInputHandler;

    FMouseState();
    ~FMouseState();

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
class FMouseInputHandler : public Graphics::IWindowMessageHandler {
public:
    FMouseInputHandler();
    virtual ~FMouseInputHandler();

    FMouseInputHandler(const FMouseInputHandler& ) = delete;
    FMouseInputHandler& operator =(const FMouseInputHandler& ) = delete;

    const FMouseState& State() const { return _state; }

    virtual void RegisterMessageDelegates(Graphics::FBasicWindow *wnd) override;
    virtual void UnregisterMessageDelegates(Graphics::FBasicWindow *wnd) override;

    virtual void UpdateBeforeDispatch(Graphics::FBasicWindow *wnd) override;
    virtual void UpdateAfterDispatch(Graphics::FBasicWindow *wnd) override;

    int ClientX() const { return _state.X(); }
    int ClientY() const { return _state.Y(); }

    float RelativeX() const { return _relativeX; }
    float RelativeY() const { return _relativeY; }

    bool IsButtonDown(EMouseButton btn) const { return _state._buttonsDown.Contains(btn); }
    bool IsButtonPressed(EMouseButton btn) const { return _state._buttonsPressed.Contains(btn); }
    bool IsButtonUp(EMouseButton btn) const { return _state._buttonsUp.Contains(btn); }

    void ClearState() { _state.Clear(); }

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
    float _relativeX;
    float _relativeY;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
