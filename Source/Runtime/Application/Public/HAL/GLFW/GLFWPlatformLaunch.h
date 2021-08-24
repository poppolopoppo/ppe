#pragma once

#include "HAL/Generic/GenericPlatformLaunch.h"

#ifndef PLATFORM_GLFW
#   error "invalid include for current platform"
#endif

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FGLFWPlatformLaunch : public FGenericPlatformLaunch {
public:
    using FGenericPlatformLaunch::RunApplication;

    static PPE_APPLICATION_API void OnPlatformLaunch(const wchar_t* filename, size_t argc, const wchar_t* const* argv);
    static PPE_APPLICATION_API void OnPlatformShutdown();
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
