#pragma once

#include "Core.Application/Application.h"

#include "Core.Graphics/Window/WindowMessageHandler.h"

#include "Core.Application/Input/GamepadState.h"

namespace Core {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FGamepadInputHandler :
    public Graphics::IWindowMessageHandler
,   public IInputStateProvider<FMultiGamepadState> {
public:
    FGamepadInputHandler();
    virtual ~FGamepadInputHandler();

    FGamepadInputHandler(const FGamepadInputHandler& ) = delete;
    FGamepadInputHandler& operator =(const FGamepadInputHandler& ) = delete;

    virtual const FMultiGamepadState& State() const override { return _state; }

    virtual void RegisterMessageDelegates(Graphics::FBasicWindow *wnd) override;
    virtual void UnregisterMessageDelegates(Graphics::FBasicWindow *wnd) override;

    virtual void UpdateBeforeDispatch(Graphics::FBasicWindow *wnd) override;
    virtual void UpdateAfterDispatch(Graphics::FBasicWindow *wnd) override;

private:
    FMultiGamepadState _state;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core
