#pragma once

#include "Application.h"

#include "Application_fwd.h"
#include "HAL/Generic/GenericWindow.h"

#include "Memory/UniquePtr.h"
#include "Misc/Event.h"
#include "Time/Timepoint.h"

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

    virtual void Poll() = 0;
    virtual void Update(FTimespan dt) = 0;
    virtual void Clear() = 0;

public:
    static PPE_APPLICATION_API void MakeDefault(UInputService* input);

public:
    using FInputUpdateEvent = TFunction<void(IInputService&, FTimespan dt)>;

    THREADSAFE_EVENT(OnInputUpdate, FInputUpdateEvent);

    using FInputFocusEvent = TFunction<void(const IInputService&, const Application::FGenericWindow*)>;

    THREADSAFE_EVENT(OnWindowFocus, FInputFocusEvent);

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
