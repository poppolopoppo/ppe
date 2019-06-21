#pragma once

#include "Application.h"

#include "Application_fwd.h"
#include "Memory/UniquePtr.h"
#include "Time/Timepoint.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_INTEFARCE_UNIQUEPTR(InputService);
class IInputService {
public:
    IInputService() {}
    virtual ~IInputService() = default;

    IInputService(const IInputService&) = delete;
    IInputService& operator =(const IInputService&) = delete;

    virtual const FKeyboardState& Keyboard() const = 0;
    virtual const FMouseState& Mouse() const = 0;

    virtual const FGamepadState& Gamepad(size_t player) const = 0;
    virtual const FGamepadState* FirstGamepadConnected() const = 0;

    virtual void SetupWindow(FGenericWindow& window) = 0;
    virtual void Poll() = 0;
    virtual void Update(FTimespan dt) = 0;
    virtual void Clear() = 0;

public:
    static PPE_APPLICATION_API void MakeDefault(UInputService* input);

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
