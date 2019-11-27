#include "stdafx.h"

#ifdef PLATFORM_WINDOWS

#include "HAL/Windows/XInputWrapper.h"

#include "Diagnostic/Logger.h"

namespace PPE {
namespace Application {
LOG_CATEGORY(, XInput)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
// Mimic an unplugged controller when the dll is not available
//----------------------------------------------------------------------------
DWORD WINAPI XInputGetState_Dummy_(
    _In_  DWORD                 dwUserIndex,  // Index of the gamer associated with the device
    _Out_::XINPUT_STATE*        pState        // Receives the current state
) {
    UNUSED(dwUserIndex);
    pState = nullptr;
    return ERROR_DEVICE_NOT_CONNECTED;
}
//----------------------------------------------------------------------------
DWORD WINAPI XInputSetState_Dummy_(
    _In_ DWORD                  dwUserIndex,  // Index of the gamer associated with the device
    _In_::XINPUT_VIBRATION*     pVibration    // The vibration information to send to the controller
) {
    UNUSED(dwUserIndex);
    UNUSED(pVibration);
    return ERROR_DEVICE_NOT_CONNECTED;
}
//----------------------------------------------------------------------------
DWORD WINAPI XInputGetCapabilities_Dummy_(
    _In_  DWORD                 dwUserIndex,   // Index of the gamer associated with the device
    _In_  DWORD                 dwFlags,       // Input flags that identify the device type
    _Out_::XINPUT_CAPABILITIES* pCapabilities  // Receives the capabilities
) {
    UNUSED(dwUserIndex);
    UNUSED(dwFlags);
    pCapabilities = nullptr;
    return ERROR_DEVICE_NOT_CONNECTED;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FXInputWrapper::FXInputWrapper()
:   _XInputGetState(nullptr)
,   _XInputSetState(nullptr)
,   _XInputGetCapabilities(nullptr) {
    static const wchar_t* GXInputDllPossiblePaths[] = {
        // Should be L"xinput1_4.dll" for SDK 10, L"xinput9_1_0.dll" for SDK 8.1
        XINPUT_DLL_W,
        // Still fall back to older dll when compiled on SDK 10 but running on older system (API should be binary compatible)
        L"xinput9_1_0.dll",
    };

    for (const wchar_t* filename : GXInputDllPossiblePaths) {
        if (_XInputDLL.AttachOrLoad(filename))
            break;
    }

    if (_XInputDLL) {
        _XInputGetState = (FXInputGetState)_XInputDLL.FunctionAddr("XInputGetState");
        _XInputSetState = (FXInputSetState)_XInputDLL.FunctionAddr("XInputSetState");
        _XInputGetCapabilities = (FXInputGetCapabilities)_XInputDLL.FunctionAddr("XInputGetCapabilities");

        Assert(_XInputGetState);
        Assert(_XInputSetState);
        Assert(_XInputGetCapabilities);

        LOG(XInput, Info, L"successfully loaded : gamepad controller are available");
    }
    else {
        _XInputGetState = &XInputGetState_Dummy_;
        _XInputSetState = &XInputSetState_Dummy_;
        _XInputGetCapabilities = &XInputGetCapabilities_Dummy_;

        LOG(XInput, Warning, L"failed to load : fall back on dummy controller !");
    }
}
//----------------------------------------------------------------------------
FXInputWrapper::~FXInputWrapper() {
    if (_XInputDLL) {
        LOG(XInput, Info, L"destroying wrapper");

        // unload the DLL explicitly while holding the lock
        Meta::FLockGuard scopeLock(_barrier);
        _XInputDLL.Unload();
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE

#endif //!PLATFORM_WINDOWS
