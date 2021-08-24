#pragma once

#include "HAL/GLFW/GLFWPlatformMessageHandler.h"

#ifndef PLATFORM_LINUX
#   error "invalid include for current platform"
#endif

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
using FLinuxMessageKeyboard = FGLFWMessageKeyboard;
using FLinuxMessageKeyboardEvent = FGLFWMessageKeyboardEvent;
//----------------------------------------------------------------------------
using FLinuxMessageMouse = FGLFWMessageMouse;
using FLinuxMessageMouseEvent = FGLFWMessageMouseEvent;
//----------------------------------------------------------------------------
using FLinuxPlatformMessageHandler = FGLFWPlatformMessageHandler;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
