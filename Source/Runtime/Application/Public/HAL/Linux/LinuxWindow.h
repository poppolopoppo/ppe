#pragma once

#include "HAL/GLFW/GLFWWindow.h"

#ifndef PLATFORM_LINUX
#   error "invalid include for current platform"
#endif

#include "HAL/PlatformIncludes.h"

namespace PPE {
namespace Application {
FWD_REFPTR(LinuxWindow);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_APPLICATION_API FLinuxWindow : public FGLFWWindow {
public: // must be defined for every platform
    using FGLFWWindow::FNativeHandle;
    using FGLFWWindow::FWindowDefinition;
    using FGLFWWindow::EWindowType;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
