#pragma once

#include "Application_fwd.h"

#include "Input/Device/KeyboardState.h"
#include "Input/InputDevice.h"

#include "HAL/PlatformWindow.h"
#include "Time/Timepoint.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FKeyboardDevice : public IInputDevice {
public:
    FKeyboardDevice();
    virtual ~FKeyboardDevice() override;

    FKeyboardDevice(const FKeyboardDevice& ) = delete;
    FKeyboardDevice& operator =(const FKeyboardDevice& ) = delete;

    const FKeyboardState& State() const { return _state; }

    void SetupWindow(FPlatformWindow& window);

public: // IInputDevice
    virtual FInputDeviceID InputDeviceID() const NOEXCEPT override { return _deviceId; }

    virtual void PostInputMessages(FTimespan dt, TAppendable<FInputMessage> messages) override;
    virtual void FlushInputKeys() override;

    virtual void SupportedInputKeys(TAppendable<FInputKey> keys) const NOEXCEPT override;

private:
    FKeyboardState _state;
    FInputDeviceID _deviceId;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
