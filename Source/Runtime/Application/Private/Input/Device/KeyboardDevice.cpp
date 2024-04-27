// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Input/Device/KeyboardDevice.h"

#include "HAL/PlatformKeyboard.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FKeyboardDevice::FKeyboardDevice()
:   _deviceId(static_cast<u32>(hash_ptr(this)))
{}
//----------------------------------------------------------------------------
FKeyboardDevice::~FKeyboardDevice() = default;
//----------------------------------------------------------------------------
void FKeyboardDevice::SetupWindow(FPlatformWindow& window) {
    FPlatformKeyboard::SetupMessageHandler(window, &_state).Forget();
}
//----------------------------------------------------------------------------
void FKeyboardDevice::PostInputMessages(FTimespan dt, TAppendable<FInputMessage> messages) {
    _state.Update(dt);

    const auto postKeyMessage = [&](EKeyboardKey key, EInputMessageEvent event) {
        if (const Meta::TOptionalReference<const FInputKey> inputKey = EInputKey::From(key))
            messages.push_back(FInputMessage{
                .Key = *inputKey,
                .Value = FInputDigital{ EInputMessageEvent::Released != event },
                .DeltaTime = dt,
                .DeviceId = _deviceId,
                .Event = event
            });
    };

    for (const EKeyboardKey key : _state.KeysDown())
        postKeyMessage(key, Pressed);
    for (const EKeyboardKey key : _state.KeysPressed())
        postKeyMessage(key, Repeat);
    for (const EKeyboardKey key : _state.KeysUp())
        postKeyMessage(key, Released);
}
//----------------------------------------------------------------------------
void FKeyboardDevice::FlushInputKeys() {
    _state.Clear();
}
//----------------------------------------------------------------------------
void FKeyboardDevice::SupportedInputKeys(TAppendable<FInputKey> keys) const NOEXCEPT {
    for (EKeyboardKey key : EachKeyboardKeys()) {
        if (const Meta::TOptionalReference<const FInputKey> inputKey = EInputKey::From(key))
            keys.push_back(*inputKey);
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
