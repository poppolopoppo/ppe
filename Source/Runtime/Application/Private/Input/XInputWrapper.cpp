#include "stdafx.h"

#ifdef PLATFORM_WINDOWS

#include "XInputWrapper.h"

#include "Diagnostic/Logger.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FXInputWrapper::FXInputWrapper()
:   _XInputGetState(nullptr)
,   _XInputSetState(nullptr)
,   _XInputGetCapabilities(nullptr) {
    static const FStringView GXInputDllPossiblePaths[] = {
        // Should be L"xinput1_4.dll" for SDK 10, L"xinput9_1_0.dll" for SDK 8.1
        XINPUT_DLL_A,
        // Still fallback to older dll when compiled on SDK 10 but running on older system (API should be binary compatible)
        "xinput9_1_0.dll",
    };

    for (const FStringView& filename : GXInputDllPossiblePaths) {
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

        LOG(Info, L"[XInput] Successfully loaded : gamepad controller are available");
    }
    else {
        LOG(Warning, L"[XInput] Failed to load : gamepad controller won't be available !");
    }
}
//----------------------------------------------------------------------------
FXInputWrapper::~FXInputWrapper() {
    LOG(Info, L"[XInput] Destroying wrapper");
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE

#endif //!PLATFORM_WINDOWS
