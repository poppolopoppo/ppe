#include "stdafx.h"

#ifdef PLATFORM_LINUX

#include "HAL/Linux/LinuxPlatformLaunch.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FLinuxPlatformLaunch::OnPlatformLaunch(const wchar_t* filename, size_t argc, const wchar_t* const* argv) {
    FGenericPlatformLaunch::OnPlatformLaunch(nullptr, 0, filename, argc, argv);
}
//----------------------------------------------------------------------------
void FLinuxPlatformLaunch::OnPlatformShutdown() {
    FGenericPlatformLaunch::OnPlatformShutdown();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE

#endif //!PLATFORM_LINUX
