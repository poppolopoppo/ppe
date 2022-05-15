#pragma once

// simple wrapper for target platform

#include "HAL/PlatformMacros.h"
#include PPE_HAL_MAKEINCLUDE(PlatformMouse)
PPE_HAL_MAKEALIAS_NAMESPACE(Application, PlatformMouse)

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
using ECursorType = PPE::Application::FPlatformMouse::ECursorType;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
