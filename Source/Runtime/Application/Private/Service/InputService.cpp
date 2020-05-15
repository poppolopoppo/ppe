#include "stdafx.h"

#include "Service/InputService.h"

#include "HAL/PlatformWindow.h"
#include "Input/KeyboardInputHandler.h"
#include "Input/GamepadInputHandler.h"
#include "Input/MouseInputHandler.h"
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
    FDefaultInputService_() {}

    virtual const FKeyboardState& Keyboard() const override final;
    virtual const FMouseState& Mouse() const override final;

    virtual const FGamepadState& Gamepad(size_t player) const override final;
    virtual const FGamepadState* FirstGamepadConnected() const override final;

    virtual void SetupWindow(FGenericWindow& window) override final;
    virtual void Poll() override final;
    virtual void Update(FTimespan dt) override final;
    virtual void Clear() override final;

private:
    FReadWriteLock _barrierRW;
    FKeyboardInputHandler _keyboard;
    FGamepadInputHandler _gamepad;
    FMouseInputHandler _mouse;
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
void FDefaultInputService_::Poll() {
    WRITESCOPELOCK(_barrierRW);

    _gamepad.Poll();
}
//----------------------------------------------------------------------------
void FDefaultInputService_::Update(FTimespan dt) {
    WRITESCOPELOCK(_barrierRW);

    _keyboard.Update(dt);
    _gamepad.Update(dt);
    _mouse.Update(dt);
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
void IInputService::MakeDefault(UInputService* input) {
    Assert(input);
    input->reset<FDefaultInputService_>();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
