#pragma once

#include "Application_fwd.h"

#include "Input/Device/MouseState.h"
#include "Input/InputDevice.h"

#include "HAL/PlatformWindow.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FMouseDevice : public IInputDevice {
public:
    FMouseDevice();
    virtual ~FMouseDevice() override;

    FMouseDevice(const FMouseDevice& ) = delete;
    FMouseDevice& operator =(const FMouseDevice& ) = delete;

    const FMouseState& State() const { return _state; }

    void SetupWindow(FPlatformWindow& window);

public: // IInputDevice
    virtual FInputDeviceID InputDeviceID() const NOEXCEPT override { return _deviceId; }

    virtual void PostInputMessages(FTimespan dt, TAppendable<FInputMessage> messages) override;
    virtual void FlushInputKeys() override;

    virtual void SupportedInputKeys(TAppendable<FInputKey> keys) const NOEXCEPT override;

private:
    FMouseState _state;
    FInputDeviceID _deviceId;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
