#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Graphics/Window/WindowMessageHandler.h"

#include "Core.Engine/Input/InputState.h"
#include "Core.Engine/Input/State/KeyboardKey.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FKeyboardState {
public:
    friend class FKeyboardInputHandler;

    FKeyboardState();
    ~FKeyboardState();

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
class FKeyboardInputHandler : public Graphics::IWindowMessageHandler {
public:
    FKeyboardInputHandler();
    virtual ~FKeyboardInputHandler();

    FKeyboardInputHandler(const FKeyboardInputHandler& ) = delete;
    FKeyboardInputHandler& operator =(const FKeyboardInputHandler& ) = delete;

    const FKeyboardState& State() const { return _state; }

    virtual void RegisterMessageDelegates(Graphics::FBasicWindow *wnd) override;
    virtual void UnregisterMessageDelegates(Graphics::FBasicWindow *wnd) override;

    virtual void UpdateBeforeDispatch(Graphics::FBasicWindow *wnd) override;
    virtual void UpdateAfterDispatch(Graphics::FBasicWindow *wnd) override;

    bool IsKeyDown(EKeyboardKey key) const { return _state._keysDown.Contains(key); }
    bool IsKeyPressed(EKeyboardKey key) const { return _state._keysPressed.Contains(key); }
    bool IsKeyUp(EKeyboardKey key) const { return _state._keysUp.Contains(key); }

    void ClearState() { _state.Clear(); }

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
} //!namespace Engine
} //!namespace Core
