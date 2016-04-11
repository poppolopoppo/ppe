#pragma once

#include "Core.Application/Application.h"

#include "Core.Graphics/Window/WindowMessageHandler.h"

#include "Core.Application/Input/GamepadState.h"

namespace Core {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class GamepadInputHandler :
    public Graphics::IWindowMessageHandler
,   public IInputStateProvider<MultiGamepadState> {
public:
    GamepadInputHandler();
    virtual ~GamepadInputHandler();

    GamepadInputHandler(const GamepadInputHandler& ) = delete;
    GamepadInputHandler& operator =(const GamepadInputHandler& ) = delete;

    virtual const MultiGamepadState& State() const override { return _state; }

    virtual void RegisterMessageDelegates(Graphics::BasicWindow *wnd) override;
    virtual void UnregisterMessageDelegates(Graphics::BasicWindow *wnd) override;

    virtual void UpdateBeforeDispatch(Graphics::BasicWindow *wnd) override;
    virtual void UpdateAfterDispatch(Graphics::BasicWindow *wnd) override;

private:
    MultiGamepadState _state;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core
