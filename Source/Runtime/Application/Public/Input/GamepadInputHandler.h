#pragma once

#include "Application.h"

#include "Input/GamepadState.h"

#include "Window/WindowMessageHandler.h"

namespace PPE {
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

    void Rumble(size_t index, float left, float right);

private:
    FMultiGamepadState _state;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
