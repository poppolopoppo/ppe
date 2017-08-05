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
    static const wchar_t* GXInputDllPossiblePaths[] = {
		// Should be L"xinput1_4.dll" for SDK 10, L"xinput9_1_0.dll" for SDK 8.1
        XINPUT_DLL_W,
		// Still fallback to older dll when compiled on SDK 10 but running on older system (API should be binary compatible)
		L"xinput9_1_0.dll", 
    };

	const DWORD libraryFlags = (LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
	const wchar_t* ModuleFilename = nullptr;
	for (const wchar_t* filename : GXInputDllPossiblePaths) {
		ModuleFilename = filename;
		if (_library = ::LoadLibraryExW(filename, nullptr, libraryFlags))
			break;
	}

    if (_library) {
        _XInputGetState = (FXInputGetState)::GetProcAddress(_library, "XInputGetState");
        _XInputSetState = (FXInputSetState)::GetProcAddress(_library, "XInputSetState");
        _XInputGetCapabilities = (FXInputGetCapabilities)::GetProcAddress(_library, "XInputGetCapabilities");

        Assert(_XInputGetState);
        Assert(_XInputSetState);
        Assert(_XInputGetCapabilities);

#ifdef USE_DEBUG_LOGGER
        wchar_t ModuleFilenameBuffer[1024];
        if (::GetModuleFileName(_library, ModuleFilenameBuffer, lengthof(ModuleFilenameBuffer)))
            ModuleFilename = ModuleFilenameBuffer;

        LOG(Info, L"[XInput] Successfully loaded '{0}' : gamepad controller are available", ModuleFilename);
#endif
    }
    else {
        LOG(Warning, L"[XInput] Failed to load '{0}' : gamepad controller won't be available !", ModuleFilename);
    }
}
//----------------------------------------------------------------------------
FXInputWrapper::~FXInputWrapper() {
    if (_library)
        ::FreeLibrary(_library);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core

#endif //!PLATFORM_WINDOWS
