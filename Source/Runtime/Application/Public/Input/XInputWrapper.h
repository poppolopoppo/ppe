#pragma once

#include "Application.h"

#ifndef PLATFORM_WINDOWS
#   error "invalid include !"
#endif

#include "Meta/Singleton.h"
#include "Meta/ThreadResource.h"
#include "Misc/DLLWrapper.h"
#include "Misc/Platform_Windows.h"

#include <mutex>
#include <XInput.h>

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FXInputWrapper : Meta::TSingleton<FXInputWrapper> {
public:
    typedef DWORD (WINAPI* FXInputGetState)(
        _In_  DWORD           dwUserIndex,  // Index of the gamer associated with the device
        _Out_ ::XINPUT_STATE* pState        // Receives the current state
    );

    typedef DWORD (WINAPI* FXInputSetState)(
        _In_ DWORD               dwUserIndex,  // Index of the gamer associated with the device
        _In_ ::XINPUT_VIBRATION* pVibration    // The vibration information to send to the controller
    );

    typedef DWORD (WINAPI* FXInputGetCapabilities)(
        _In_  DWORD                  dwUserIndex,   // Index of the gamer associated with the device
        _In_  DWORD                  dwFlags,       // Input flags that identify the device type
        _Out_ ::XINPUT_CAPABILITIES* pCapabilities  // Receives the capabilities
    );

    class FLocked : Meta::FLockGuard {
    public:
        explicit FLocked(const FXInputWrapper& owner)
            : Meta::FLockGuard(owner._barrier)
            , _owner(&owner) {}

        bool Available() const { return _owner->Available(); }

        FXInputGetState XInputGetState() const { return _owner->_XInputGetState; }
        FXInputSetState XInputSetState() const { return _owner->_XInputSetState; }
        FXInputGetCapabilities XInputGetCapabilities() const { return _owner->_XInputGetCapabilities; }

    private:
        const FXInputWrapper* const _owner;
    };

    ~FXInputWrapper();

    bool Available() const { return (_XInputDLL.IsValid()); }

    FLocked Lock() const { return FLocked(*this); }

#ifdef WITH_PPE_ASSERT
    using Meta::TSingleton<FXInputWrapper>::HasInstance;
#endif
    using Meta::TSingleton<FXInputWrapper>::Get;

    static void Create() { Meta::TSingleton<FXInputWrapper>::Create(); }
    using Meta::TSingleton<FXInputWrapper>::Destroy;

private:
    friend Meta::TSingleton<FXInputWrapper>;

    FXInputWrapper();

    mutable std::mutex _barrier;

    FDLLWrapper _XInputDLL;

    FXInputGetState _XInputGetState;
    FXInputSetState _XInputSetState;
    FXInputGetCapabilities _XInputGetCapabilities;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE