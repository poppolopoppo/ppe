#pragma once

#include "HAL/GLFW/GLFWPlatformLaunch.h"

#ifndef PLATFORM_LINUX
#   error "invalid include for current platform"
#endif

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FLinuxPlatformLaunch : public FGLFWPlatformLaunch {
public:
    using FGLFWPlatformLaunch::RunApplication;

    static PPE_APPLICATION_API void OnPlatformLaunch(const char* filename, size_t argc, const char** argv);
    static PPE_APPLICATION_API void OnPlatformShutdown();
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
