// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Input/InputService.h"

#include "Input/Action/InputListener.h"
#include "Input/Device/KeyboardDevice.h"
#include "Input/Device/GamepadDevice.h"
#include "Input/Device/MouseDevice.h"

#include "HAL/PlatformGamepad.h"
#include "HAL/PlatformWindow.h"

#include "Container/Array.h"
#include "Container/Vector.h"
#include "Time/Timeline.h"
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

    virtual const FKeyboardState& Keyboard() const override final;
    virtual const FMouseState& Mouse() const override final;

    virtual const FGamepadState& Gamepad(size_t player) const override final;
    virtual const FGamepadState* FirstGamepadConnected() const override final;

    virtual void SetupWindow(FGenericWindow& window) override final;

    virtual FGenericWindow* FocusedWindow() const override final { return _focusedWindow; }
    virtual void SetWindowFocused(FGenericWindow*) override final;

    virtual Meta::TOptionalReference<const IInputDevice> InputDevice(FInputDeviceID deviceId) const NOEXCEPT override final;

    virtual void PollInputEvents() override final;
    virtual void PostInputMessages(const FTimeline& time) override final;
    virtual void FlushInputKeys() override final;
    virtual void SupportedInputKeys(TAppendable<FInputKey> keys) const NOEXCEPT override final;

    NODISCARD virtual bool HasInputListener(const SCInputListener& listener) const NOEXCEPT override final;
    virtual void PushInputListener(const SInputListener& listener) override final;
    virtual bool PopInputListener(const SInputListener& listener) override final;

    virtual EInputListenerEvent ToggleFocus(const SInputListener& listener, EInputListenerEvent mode) override final;

    NODISCARD virtual bool HasInputMapping(const PCInputMapping& mapping) const NOEXCEPT override final;
    virtual void AddInputMapping(const PInputMapping& mapping, i32 priority) override final;
    virtual bool RemoveInputMapping(const PInputMapping& mapping) override final;

private:
    FReadWriteLock _barrierRW;

    FInputListener _mainListener;

    FMouseDevice _mouse;
    FKeyboardDevice _keyboard;
    TStaticArray<FGamepadDevice, FPlatformGamepad::MaxNumGamepad> _gamepads =
        Meta::static_for<FPlatformGamepad::MaxNumGamepad>([](auto ...idx) {
            return MakeStaticArray<FGamepadDevice>(idx...);
        });

    VECTORINSITU(Input, SInputListener, 3) _listeners = { &_mainListener };

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
const FGamepadState& FDefaultInputService_::Gamepad(size_t player) const {
    READSCOPELOCK(_barrierRW);
    return _gamepads[player].State();
}
//----------------------------------------------------------------------------
const FGamepadState* FDefaultInputService_::FirstGamepadConnected() const {
    READSCOPELOCK(_barrierRW);
    for (const FGamepadDevice& gamepad : _gamepads) {
        if (gamepad.IsConnected())
            return (&gamepad.State());
    }
    return nullptr;
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
Meta::TOptionalReference<const IInputDevice> FDefaultInputService_::InputDevice(FInputDeviceID deviceId) const NOEXCEPT {
    READSCOPELOCK(_barrierRW);

    if (_keyboard.InputDeviceID() == deviceId)
        return &_keyboard;
    if (_mouse.InputDeviceID() == deviceId)
        return &_mouse;

    for (const FGamepadDevice& gamepad : _gamepads) {
        if (gamepad.InputDeviceID() == deviceId)
            return &gamepad;
    }

    return nullptr;
}
//----------------------------------------------------------------------------
void FDefaultInputService_::PollInputEvents() {
    WRITESCOPELOCK(_barrierRW);

    for (FGamepadDevice& gamepad : _gamepads)
        gamepad.PollGamepadEvents();
}
//----------------------------------------------------------------------------
void FDefaultInputService_::PostInputMessages(const FTimeline& time) {
    bool focusChanged = false;
    FGenericWindow* previousFocus = nullptr;
    VECTORINSITU(Input, FInputMessage, 8) frameMessages;

    const FTimespan dt = time.Elapsed();

    // harvest this frame input messages inside a write scope lock
    {
        WRITESCOPELOCK(_barrierRW);

        const TAppendable<FInputMessage> inputKey = MakeAppendable(frameMessages);

        _keyboard.PostInputMessages(dt, inputKey);
        _mouse.PostInputMessages(dt, inputKey);

        for (FGamepadDevice& gamepad : _gamepads) {
            gamepad.PostInputMessages(dt, inputKey);

            if (gamepad.State().OnConnect())
                _OnDeviceConnected(*this, gamepad.InputDeviceID());

            if (gamepad.State().OnDisconnect())
                _OnDeviceDisconnected(*this, gamepad.InputDeviceID());
        }

        if (_nextFocusedWindow != _focusedWindow) {
            focusChanged = true;
            previousFocus = _focusedWindow;
            _focusedWindow = _nextFocusedWindow;
        }
    }

    // trigger events outside of scope lock, to avoid dead-lock in delegates accessing *this

    for (const FInputMessage& message : frameMessages) {
        bool unhandled = true;
        for (const SInputListener& listener : _listeners) {
            const EInputListenerEvent result = listener->InputKey(message);
            if (result != EInputListenerEvent::Unhandled) {
                unhandled = false;
                if (result == EInputListenerEvent::Consumed)
                    break;
            }
        }

        if (unhandled)
            _OnUnhandledKey(*this, message);
    }

    if (focusChanged)
        _OnWindowFocus.Invoke(*this, previousFocus);

    // finally call update input callback for injected services

    _OnUpdateInput(*this, time.Elapsed());
}
//----------------------------------------------------------------------------
void FDefaultInputService_::FlushInputKeys() {
    WRITESCOPELOCK(_barrierRW);

    _keyboard.FlushInputKeys();
    _mouse.FlushInputKeys();

    for (FGamepadDevice& gamepad : _gamepads)
        gamepad.FlushInputKeys();
}
//----------------------------------------------------------------------------
void FDefaultInputService_::SupportedInputKeys(TAppendable<FInputKey> keys) const NOEXCEPT {
    READSCOPELOCK(_barrierRW);

    _keyboard.SupportedInputKeys(keys);
    _mouse.SupportedInputKeys(keys);

    for (const FGamepadDevice& gamepad : _gamepads)
        gamepad.SupportedInputKeys(keys);
}
//----------------------------------------------------------------------------
bool FDefaultInputService_::HasInputListener(const SCInputListener& listener) const NOEXCEPT {
    Assert(listener);
    READSCOPELOCK(_barrierRW);

    return Contains(_listeners, listener);
}
//----------------------------------------------------------------------------
void FDefaultInputService_::PushInputListener(const SInputListener& listener) {
    Assert(listener);
    WRITESCOPELOCK(_barrierRW);

    // check if listener already registered
    const auto it = std::find(_listeners.begin(), _listeners.end(), listener);
    if (_listeners.end() != it) {
        // early-out if listener is already on top
        if (_listeners.front() == listener)
            return;

        _listeners.erase(it);
    }

    // put new listener on top of input stack
    _listeners.insert(_listeners.begin(), listener);
}
//----------------------------------------------------------------------------
bool FDefaultInputService_::PopInputListener(const SInputListener& listener) {
    Assert(listener);
    WRITESCOPELOCK(_barrierRW);

    return Remove_ReturnIfExists(_listeners, listener);
}
//----------------------------------------------------------------------------
EInputListenerEvent FDefaultInputService_::ToggleFocus(const SInputListener& listener, EInputListenerEvent mode) {
    EInputListenerEvent previousMode = listener->Mode();
    if (not HasInputListener(listener))
        previousMode = EInputListenerEvent::Unhandled;

    if (mode != previousMode) {
        listener->SetMode(mode);

        switch (mode) {
        case EInputListenerEvent::Handled:
            FALLTHROUGH();
        case EInputListenerEvent::Consumed:
            PushInputListener(listener);
            break;
        case EInputListenerEvent::Unhandled:
            PopInputListener(listener);
            break;
        }
    }

    return previousMode;
}
//----------------------------------------------------------------------------
bool FDefaultInputService_::HasInputMapping(const PCInputMapping& mapping) const NOEXCEPT {
    Assert(mapping);
    READSCOPELOCK(_barrierRW);

    return _mainListener.HasMapping(mapping);
}
//----------------------------------------------------------------------------
void FDefaultInputService_::AddInputMapping(const PInputMapping& mapping, i32 priority) {
    WRITESCOPELOCK(_barrierRW);

    _mainListener.AddMapping(mapping, priority);
}
//----------------------------------------------------------------------------
bool FDefaultInputService_::RemoveInputMapping(const PInputMapping& mapping) {
    WRITESCOPELOCK(_barrierRW);

    return _mainListener.RemoveMapping(mapping);
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
