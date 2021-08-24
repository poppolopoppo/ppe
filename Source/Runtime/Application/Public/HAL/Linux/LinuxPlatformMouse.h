#pragma once

#include "HAL/GLFW/GLFWPlatformMouse.h"

#ifndef PLATFORM_LINUX
#   error "invalid include for current platform"
#endif

#include "HAL/PlatformIncludes.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
using ELinuxCursorType = EGLFWCursorType;
//----------------------------------------------------------------------------
using FLinuxPlatformMouse = FGLFWPlatformMouse;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
