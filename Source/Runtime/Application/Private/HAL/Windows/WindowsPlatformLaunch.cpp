// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

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

    // sets the process-default DPI awareness to system-DPI awareness
    ::SetProcessDPIAware();

#if !USE_PPE_SANITIZER
    // disable virtual memory hooks when building with sanitizer enabled
    FVirtualAllocDetour::StartHooks();
#endif

    FGenericPlatformLaunch::OnPlatformLaunch(appHandle, nShowCmd, filename, argc, argv);
}
//----------------------------------------------------------------------------
void FWindowsPlatformLaunch::OnPlatformShutdown() {
    FGenericPlatformLaunch::OnPlatformShutdown();

#if !USE_PPE_SANITIZER
    // disable virtual memory hooks when building with sanitizer enabled
    FVirtualAllocDetour::ShutdownHooks();
#endif

    ::CoUninitialize(); // ALAP
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE

#endif //!PLATFORM_WINDOWS
