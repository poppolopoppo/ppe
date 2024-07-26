#pragma once

#include "Application_fwd.h"

#include "Input/InputKey.h"

#include "Container/Appendable.h"
#include "Time/Time_fwd.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EInputMessageEvent : u8 {
    Pressed             = 0,
    Released,
    Repeat,
    DoubleClick,
    Axis,
};
//----------------------------------------------------------------------------
struct FInputMessage {
    FInputKey Key;
    FInputValue Value;

    FTimespan DeltaTime;
    FInputDeviceID DeviceId;
    EInputMessageEvent Event;

    NODISCARD bool IsPressed() const { return (Event == EInputMessageEvent::Pressed); }
    NODISCARD bool IsReleased() const { return (Event == EInputMessageEvent::Released); }
    NODISCARD bool IsRepeat() const { return (Event == EInputMessageEvent::Repeat); }
    NODISCARD bool IsDoubleClick() const { return (Event == EInputMessageEvent::DoubleClick); }
    NODISCARD bool IsAxis() const { return (Event == EInputMessageEvent::Axis); }

    NODISCARD FInputDigital Digital() const { return std::get<FInputDigital>(Value); }
    NODISCARD FInputAxis1D Axis1D() const { return std::get<FInputAxis1D>(Value); }
    NODISCARD const FInputAxis2D& Axis2D() const { return std::get<FInputAxis2D>(Value); }
    NODISCARD const FInputAxis3D& Axis3D() const { return std::get<FInputAxis3D>(Value); }
};
//----------------------------------------------------------------------------
class IInputDevice {
public:
    virtual ~IInputDevice() = default;

    virtual FInputDeviceID InputDeviceID() const NOEXCEPT = 0;

    virtual void PostInputMessages(FTimespan dt, TAppendable<FInputMessage> messages) = 0;
    virtual void FlushInputKeys() = 0;

    virtual void SupportedInputKeys(TAppendable<FInputKey> keys) const NOEXCEPT = 0;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
