#include "stdafx.h"

#ifdef PLATFORM_WINDOWS

#include "HAL/Windows/WindowsPlatformLaunch.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FWindowsPlatformLaunch::OnPlatformLaunch(void* appHandle, int nShowCmd, const wchar_t* filename, size_t argc, const wchar_t* const* argv) {
    FGenericPlatformLaunch::OnPlatformLaunch(appHandle, nShowCmd, filename, argc, argv);
}
//----------------------------------------------------------------------------
void FWindowsPlatformLaunch::OnPlatformShutdown() {
    FGenericPlatformLaunch::OnPlatformShutdown();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE

#endif //!PLATFORM_WINDOWS
