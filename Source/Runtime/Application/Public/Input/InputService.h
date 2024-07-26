#pragma once

#include "Application_fwd.h"
#include "InputDevice.h"

#include "HAL/Generic/GenericWindow.h"

#include "Container/Appendable.h"
#include "Memory/UniquePtr.h"
#include "Misc/Event.h"
#include "Time/Time_fwd.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_INTEFARCE_UNIQUEPTR(InputService);
class IInputService {
public:
    IInputService() = default;
    virtual ~IInputService() = default;

    IInputService(const IInputService&) = delete;
    IInputService& operator =(const IInputService&) = delete;

    virtual const Application::FKeyboardState& Keyboard() const = 0;
    virtual const Application::FMouseState& Mouse() const = 0;

    virtual const Application::FGamepadState& Gamepad(size_t player) const = 0;
    virtual const Application::FGamepadState* FirstGamepadConnected() const = 0;

    virtual void SetupWindow(Application::FGenericWindow& window) = 0;

    virtual Application::FGenericWindow* FocusedWindow() const = 0;
    virtual void SetWindowFocused(Application::FGenericWindow*) = 0;

    virtual Meta::TOptionalReference<const Application::IInputDevice> InputDevice(Application::FInputDeviceID deviceId) const NOEXCEPT = 0;

    virtual void PollInputEvents() = 0;
    virtual void PostInputMessages(const FTimeline& time) = 0;
    virtual void FlushInputKeys() = 0;
    virtual void SupportedInputKeys(TAppendable<Application::FInputKey> keys) const NOEXCEPT = 0;

    NODISCARD virtual bool HasInputListener(const Application::SCInputListener& listener) const NOEXCEPT = 0;
    virtual void PushInputListener(const Application::SInputListener& listener) = 0;
    virtual bool PopInputListener(const Application::SInputListener& listener) = 0;

    virtual Application::EInputListenerEvent ToggleFocus(const Application::SInputListener& listener, Application::EInputListenerEvent mode) = 0;

    NODISCARD virtual bool HasInputMapping(const Application::PCInputMapping& mapping) const NOEXCEPT = 0;
    virtual void AddInputMapping(const Application::PInputMapping& mapping, i32 priority) = 0;
    virtual bool RemoveInputMapping(const Application::PInputMapping& mapping) = 0;

public:
    using FUpdateInputEvent = TFunction<void(const IInputService&, FTimespan)>;

    THREADSAFE_EVENT(OnUpdateInput, FUpdateInputEvent);

    using FDeviceEvent = TFunction<void(const IInputService&, Application::FInputDeviceID)>;

    THREADSAFE_EVENT(OnDeviceConnected, FDeviceEvent);
    THREADSAFE_EVENT(OnDeviceDisconnected, FDeviceEvent);

    using FFocusEvent = TFunction<void(const IInputService&, const Application::FGenericWindow*)>;

    THREADSAFE_EVENT(OnWindowFocus, FFocusEvent);

    using FUnhandledKey = TFunction<void(const IInputService&, const Application::FInputMessage&)>;

    THREADSAFE_EVENT(OnUnhandledKey, FUnhandledKey);

public:
    static PPE_APPLICATION_API void MakeDefault(UInputService* input);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
