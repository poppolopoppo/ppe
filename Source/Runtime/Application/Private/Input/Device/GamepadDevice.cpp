// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Input/Device/GamepadDevice.h"

#include "HAL/PlatformGamepad.h"
#include "Maths/ScalarVectorHelpers.h"
#include "Thread/ThreadPool.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FGamepadDevice::FGamepadDevice(size_t controllerId)
:   _deviceId(static_cast<u32>(hash_ptr(this)))
,   _controllerId(checked_cast<u32>(controllerId)) {
    Assert_NoAssume(_controllerId < FPlatformGamepad::MaxNumGamepad);
}
//----------------------------------------------------------------------------
FGamepadDevice::~FGamepadDevice() = default;
//----------------------------------------------------------------------------
bool FGamepadDevice::PollGamepadEvents() {
    return FPlatformGamepad::Poll(_controllerId, &_state);
}
//----------------------------------------------------------------------------
void FGamepadDevice::AsyncRumble(float left, float right) const {
    if (not _state.IsConnected())
        return;

    // non blocking rumble:
    AsyncSyscall([controllerId(_controllerId), left, right](ITaskContext&) {
        FPlatformGamepad::Rumble(controllerId, left, right);
    },  ETaskPriority::High);
}
//----------------------------------------------------------------------------
void FGamepadDevice::PostInputMessages(FTimespan dt, TAppendable<FInputMessage> messages) {
    _state.Update(dt);

    if (not _state.IsConnected())
        return;

    const auto postKeyMessage = [&](EGamepadButton button, EInputMessageEvent event) {
        if (const Meta::TOptionalReference<const FInputKey> inputKey = EInputKey::From(button))
            messages.push_back(FInputMessage{
                .Key = *inputKey,
                .Value = FInputDigital{ EInputMessageEvent::Released != event },
                .DeltaTime = dt,
                .DeviceId = _deviceId,
                .Event = event
            });
    };

    for (const EGamepadButton button : _state.ButtonsDown())
        postKeyMessage(button, Pressed);
    for (const EGamepadButton button : _state.ButtonsPressed())
        postKeyMessage(button, Repeat);
    for (const EGamepadButton button : _state.ButtonsUp())
        postKeyMessage(button, Released);

    if (const float2 value = _state.LeftStick(); AnyGreater(Abs(value), float2::Zero)) {
        messages.push_back(FInputMessage{
                .Key = EInputKey::Gamepad_Left2D,
                .Value = FInputAxis2D{
                    .Absolute = value,
                    .Relative = _state.LeftStickDelta(),
                },
                .DeltaTime = dt,
                .DeviceId = _deviceId,
                .Event = Axis,
            });
    }
    if (const float2 value = _state.RightStick(); AnyGreater(Abs(value), float2::Zero)) {
        messages.push_back(FInputMessage{
                .Key = EInputKey::Gamepad_Right2D,
                .Value = FInputAxis2D{
                    .Absolute = value,
                    .Relative = _state.RightStickDelta(),
                },
                .DeltaTime = dt,
                .DeviceId = _deviceId,
                .Event = Axis,
            });
    }
    if (const float value = _state.LeftTrigger(); Abs(value) > 0) {
        messages.push_back(FInputMessage{
                .Key = EInputKey::Gamepad_LeftTriggerAxis,
                .Value = FInputAxis1D{
                    .Absolute = value,
                    .Relative = _state.TriggersDelta().x,
                },
                .DeltaTime = dt,
                .DeviceId = _deviceId,
                .Event = Axis,
            });
    }
    if (const float value = _state.RightTrigger(); Abs(value) > SmallEpsilon) {
        messages.push_back(FInputMessage{
                .Key = EInputKey::Gamepad_RightTriggerAxis,
                .Value = FInputAxis1D{
                    .Absolute = value,
                    .Relative = _state.TriggersDelta().y,
                },
                .DeltaTime = dt,
                .DeviceId = _deviceId,
                .Event = Axis,
            });
    }
}
//----------------------------------------------------------------------------
void FGamepadDevice::FlushInputKeys() {
    _state.Clear();
}
//----------------------------------------------------------------------------
void FGamepadDevice::SupportedInputKeys(TAppendable<FInputKey> keys) const noexcept {
    for (EGamepadAxis axis : EachGamepadAxises()) {
        if (const Meta::TOptionalReference<const FInputKey> inputKey = EInputKey::From(axis))
            keys.push_back(*inputKey);
    }
    for (EGamepadButton button : EachGamepadButtons()) {
        if (const Meta::TOptionalReference<const FInputKey> inputKey = EInputKey::From(button))
            keys.push_back(*inputKey);
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
