#include "stdafx.h"

#ifdef PLATFORM_WINDOWS

#include "HAL/Windows/WindowsPlatformLaunch.h"
#include "HAL/Windows/VirtualAllocDetour.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FWindowsPlatformLaunch::OnPlatformLaunch(void* appHandle, int nShowCmd, const wchar_t* filename, size_t argc, const wchar_t* const* argv) {
    VerifyRelease(SUCCEEDED(::CoInitialize(NULL))); // ASAP

    FVirtualAllocDetour::StartHooks();

    FGenericPlatformLaunch::OnPlatformLaunch(appHandle, nShowCmd, filename, argc, argv);
}
//----------------------------------------------------------------------------
void FWindowsPlatformLaunch::OnPlatformShutdown() {
    FGenericPlatformLaunch::OnPlatformShutdown();

    FVirtualAllocDetour::ShutdownHooks();

    ::CoUninitialize(); // ALAP
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE

#endif //!PLATFORM_WINDOWS
