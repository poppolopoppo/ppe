// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Input/Device/MouseDevice.h"

#include "HAL/PlatformMouse.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FMouseDevice::FMouseDevice()
:   _deviceId(static_cast<u32>(hash_ptr(this)))
{}
//----------------------------------------------------------------------------
FMouseDevice::~FMouseDevice() = default;
//----------------------------------------------------------------------------
void FMouseDevice::SetupWindow(FPlatformWindow& window) {
    FPlatformMouse::SetupMessageHandler(window, &_state).Forget();
}
//----------------------------------------------------------------------------
void FMouseDevice::PostInputMessages(FTimespan dt, TAppendable<FInputMessage> messages) {
    _state.Update(dt);

    const auto postKeyMessage = [&](EMouseButton button, EInputMessageEvent event) {
        if (const Meta::TOptionalReference<const FInputKey> inputKey = EInputKey::From(button))
            messages.push_back(FInputMessage{
                .Key = *inputKey,
                .Value = FInputDigital{ EInputMessageEvent::Released != event },
                .DeltaTime = dt,
                .DeviceId = _deviceId,
                .Event = event,
            });
    };

    for (const EMouseButton button : _state.ButtonsDown())
        postKeyMessage(button, Pressed);
    for (const EMouseButton button : _state.ButtonsPressed())
        postKeyMessage(button, Repeat);
    for (const EMouseButton button : _state.ButtonsUp())
        postKeyMessage(button, Released);

    if (_state.HasMoved()) {
        messages.push_back(FInputMessage{
                .Key = EInputKey::Mouse2D,
                .Value = FInputAxis2D{
                    .Absolute = float2(_state.Client()),
                    .Relative = float2(_state.DeltaClient()),
                },
                .DeltaTime = dt,
                .DeviceId = _deviceId,
                .Event = Axis,
            });
    }
    if (const float delta = _state.WheelX().Delta(); Abs(delta) > SmallEpsilon) {
        messages.push_back(FInputMessage{
                .Key = EInputKey::MouseWheelAxisX,
                .Value = FInputAxis1D{
                    .Absolute = _state.Wheel().x,
                    .Relative = delta,
                },
                .DeltaTime = dt,
                .DeviceId = _deviceId,
                .Event = Axis,
            });
    }
    if (const float delta = _state.WheelY().Delta(); Abs(delta) > SmallEpsilon) {
        messages.push_back(FInputMessage{
                .Key = EInputKey::MouseWheelAxisY,
                .Value = FInputAxis1D{
                    .Absolute = _state.Wheel().y,
                    .Relative = delta,
                },
                .DeltaTime = dt,
                .DeviceId = _deviceId,
                .Event = Axis,
            });
    }
}
//----------------------------------------------------------------------------
void FMouseDevice::FlushInputKeys() {
    _state.Clear();
}
//----------------------------------------------------------------------------
void FMouseDevice::SupportedInputKeys(TAppendable<FInputKey> keys) const noexcept {
    for (EMouseAxis axis : EachMouseAxises()) {
        if (const Meta::TOptionalReference<const FInputKey> inputKey = EInputKey::From(axis))
            keys.push_back(*inputKey);
    }
    for (EMouseButton button : EachMouseButton()) {
        if (const Meta::TOptionalReference<const FInputKey> inputKey = EInputKey::From(button))
            keys.push_back(*inputKey);
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
