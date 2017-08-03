#include "stdafx.h"

#ifdef PLATFORM_WINDOWS

#include "XInputWrapper.h"

#include "Core/Diagnostic/Logger.h"

namespace Core {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FXInputWrapper::FXInputWrapper()
:   _XInputGetState(nullptr)
,   _XInputSetState(nullptr)
,   _XInputGetCapabilities(nullptr) {
    static const wchar_t GXInputDll[] = XINPUT_DLL_W;

    _library = ::LoadLibraryW(GXInputDll);
    if (_library) {
        _XInputGetState = (FXInputGetState)::GetProcAddress(_library, "XInputGetState");
        _XInputSetState = (FXInputSetState)::GetProcAddress(_library, "XInputSetState");
        _XInputGetCapabilities = (FXInputGetCapabilities)::GetProcAddress(_library, "XInputGetCapabilities");

        Assert(_XInputGetState);
        Assert(_XInputSetState);
        Assert(_XInputGetCapabilities);

#ifdef USE_DEBUG_LOGGER
        wchar_t ModuleFilenameBuffer[1024];
        const wchar_t* ModuleFilename = GXInputDll;
        if (::GetModuleFileName(_library, ModuleFilenameBuffer, lengthof(ModuleFilenameBuffer)))
            ModuleFilename = ModuleFilenameBuffer;

        LOG(Info, L"[XInput] Successfully loaded '{0}' : gamepad controller are available", ModuleFilename);
#endif
    }
    else {
        LOG(Warning, L"[XInput] Failed to load '{0}' : gamepad controller won't be available !", GXInputDll);
    }
}
//----------------------------------------------------------------------------
FXInputWrapper::~FXInputWrapper() {
    if (_library) {
        ::FreeLibrary(_library);
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core

#endif //!PLATFORM_WINDOWS
