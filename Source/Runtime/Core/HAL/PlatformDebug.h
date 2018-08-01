#pragma once

// simple wrapper for target platform

#include "Core/HAL/TargetPlatform.h"
#include "Core/HAL/Generic/GenericPlatformDebug.h"
#if USE_CORE_PLATFORM_DEBUG
#   include CORE_HAL_MAKEINCLUDE(PlatformDebug)
    CORE_HAL_MAKEALIAS(PlatformDebug)
#endif
