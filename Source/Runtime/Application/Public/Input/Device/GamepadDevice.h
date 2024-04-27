#pragma once

#include "Application_fwd.h"

#include "Input/Device/GamepadState.h"
#include "Input/InputDevice.h"

#include "Meta/Utility.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FGamepadDevice : public IInputDevice {
public:
    explicit FGamepadDevice(size_t controllerIndex);
    virtual ~FGamepadDevice() override;

    FGamepadDevice(const FGamepadDevice& ) = delete;
    FGamepadDevice& operator =(const FGamepadDevice& ) = delete;

    const FGamepadState& State() const { return _state; }

    bool IsConnected() const { return _state.IsConnected(); }

    bool UseFilteredInputs() const { return _state.UseFilteredInputs(); }
    void SetUseFilteredInputs(bool value) { _state.SetUseFilteredInputs(value); }

    bool PollGamepadEvents();
    void AsyncRumble(float left, float right) const;

public: // IInputDevice
    virtual FInputDeviceID InputDeviceID() const NOEXCEPT override { return _deviceId; }

    virtual void PostInputMessages(FTimespan dt, TAppendable<FInputMessage> messages) override;
    virtual void FlushInputKeys() override;

    virtual void SupportedInputKeys(TAppendable<FInputKey> keys) const NOEXCEPT override;

private:
    FGamepadState _state;
    FInputDeviceID _deviceId;
    u32 _controllerId;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
