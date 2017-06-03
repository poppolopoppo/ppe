#pragma once

#include "Core.Application/Application.h"

#include "Core.Application/Input/GamepadState.h"

#include "Core.Graphics/Window/WindowMessageHandler.h"

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

    virtual const FMultiGamepadState& State() const override final;

    virtual void RegisterMessageDelegates(Graphics::FBasicWindow *wnd) override final;
    virtual void UnregisterMessageDelegates(Graphics::FBasicWindow *wnd) override final;

    virtual void UpdateBeforeDispatch(Graphics::FBasicWindow *wnd) override final;
    virtual void UpdateAfterDispatch(Graphics::FBasicWindow *wnd) override final;

private:
    FMultiGamepadState _state;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core
