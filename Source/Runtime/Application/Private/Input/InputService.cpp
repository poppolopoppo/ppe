#include "stdafx.h"

#include "Input/InputService.h"

#include "Input/KeyboardInputHandler.h"
#include "Input/GamepadInputHandler.h"
#include "Input/MouseInputHandler.h"

#include "HAL/PlatformWindow.h"
#include "Thread/ReadWriteLock.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
class FDefaultInputService_ : public IInputService {
public:
    FDefaultInputService_() = default;
    ~FDefaultInputService_() override = default;

    virtual const FKeyboardState& Keyboard() const override final;
    virtual const FMouseState& Mouse() const override final;

    virtual const FGamepadState& Gamepad(size_t player) const override final;
    virtual const FGamepadState* FirstGamepadConnected() const override final;

    virtual void SetupWindow(FGenericWindow& window) override final;
    virtual void Poll() override final;
    virtual void Update(FTimespan dt) override final;
    virtual void Clear() override final;

    virtual FGenericWindow* FocusedWindow() const override final { return _focusedWindow; }
    virtual void SetWindowFocused(FGenericWindow*) override final;

private:
    FReadWriteLock _barrierRW;
    FKeyboardInputHandler _keyboard;
    FGamepadInputHandler _gamepad;
    FMouseInputHandler _mouse;

    FPlatformWindow* _focusedWindow{ nullptr };
    FPlatformWindow* _nextFocusedWindow{ nullptr };
};
//----------------------------------------------------------------------------
const FKeyboardState& FDefaultInputService_::Keyboard() const {
    READSCOPELOCK(_barrierRW);
    return _keyboard.State();
}
//----------------------------------------------------------------------------
const FMouseState& FDefaultInputService_::Mouse() const {
    READSCOPELOCK(_barrierRW);
    return _mouse.State();
}
//----------------------------------------------------------------------------
const FGamepadState& FDefaultInputService_::Gamepad(size_t index) const {
    READSCOPELOCK(_barrierRW);
    return _gamepad.State(index);
}
//----------------------------------------------------------------------------
const FGamepadState* FDefaultInputService_::FirstGamepadConnected() const {
    READSCOPELOCK(_barrierRW);
    return _gamepad.FirstConnectedIFP();
}
//----------------------------------------------------------------------------
void FDefaultInputService_::SetupWindow(FGenericWindow& window) {
    WRITESCOPELOCK(_barrierRW);

    auto* const platformWindow = checked_cast<FPlatformWindow*>(&window);

    _keyboard.SetupWindow(*platformWindow);
    _mouse.SetupWindow(*platformWindow);
}
//----------------------------------------------------------------------------
void FDefaultInputService_::SetWindowFocused(FGenericWindow* window) {
    WRITESCOPELOCK(_barrierRW);

    if (window)
        _nextFocusedWindow = checked_cast<FPlatformWindow*>(window);
    else
        _nextFocusedWindow = nullptr;
}
//----------------------------------------------------------------------------
void FDefaultInputService_::Poll() {
    WRITESCOPELOCK(_barrierRW);

    _gamepad.Poll();
}
//----------------------------------------------------------------------------
void FDefaultInputService_::Update(FTimespan dt) {
    bool focusChanged = false;
    FGenericWindow* previousFocus = nullptr;
    {
        WRITESCOPELOCK(_barrierRW);

        _keyboard.Update(dt);
        _gamepad.Update(dt);
        _mouse.Update(dt);

        if (_nextFocusedWindow != _focusedWindow) {
            focusChanged = true;
            previousFocus = _focusedWindow;
            _focusedWindow = _nextFocusedWindow;
        }
    }

    if (focusChanged)
        _OnWindowFocus.Invoke(*this, previousFocus);

    _OnInputUpdate.Invoke(*this, dt);
}
//----------------------------------------------------------------------------
void FDefaultInputService_::Clear() {
    WRITESCOPELOCK(_barrierRW);

    _keyboard.Clear();
    _gamepad.Clear();
    _mouse.Clear();
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
//----------------------------------------------------------------------------
void IInputService::MakeDefault(UInputService* input) {
    Assert(input);
    input->create<Application::FDefaultInputService_>();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
