#pragma once

#include "Application.h"

#include "Core.Graphics/WindowMessageHandler.h"

#include "KeyboardKey.h"
#include "InputState.h"

namespace Core {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class KeyboardState {
public:
    friend class KeyboardInputHandler;

    KeyboardState();
    ~KeyboardState();

    const KeyboardKeyState& KeysDown() const { return _keysDown; }
    const KeyboardKeyState& KeysPressed() const { return _keysPressed; }
    const KeyboardKeyState& KeysUp() const { return _keysUp; }

    void Clear();

private:
    KeyboardKeyState _keysDown;
    KeyboardKeyState _keysPressed;
    KeyboardKeyState _keysUp;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class KeyboardInputHandler : public Graphics::IWindowMessageHandler {
public:
    KeyboardInputHandler();
    virtual ~KeyboardInputHandler();

    KeyboardInputHandler(const KeyboardInputHandler& ) = delete;
    KeyboardInputHandler& operator =(const KeyboardInputHandler& ) = delete;

    const KeyboardState& State() const { return _state; }

    virtual void RegisterMessageDelegates(Graphics::BasicWindow *wnd) override;
    virtual void UnregisterMessageDelegates(Graphics::BasicWindow *wnd) override;

    virtual void UpdateBeforeDispatch(Graphics::BasicWindow *wnd) override;
    virtual void UpdateAfterDispatch(Graphics::BasicWindow *wnd) override;

    bool IsKeyDown(KeyboardKey key) const { return _state._keysDown.Contains(key); }
    bool IsKeyPressed(KeyboardKey key) const { return _state._keysPressed.Contains(key); }
    bool IsKeyUp(KeyboardKey key) const { return _state._keysUp.Contains(key); }

    void ClearState() { _state.Clear(); }

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
