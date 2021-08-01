#pragma once

// simple wrapper for target platform

#include "HAL/Generic/GenericPlatformDebug.h"
#if USE_PPE_PLATFORM_DEBUG
#   include PPE_HAL_MAKEINCLUDE(PlatformDebug)
    PPE_HAL_MAKEALIAS(PlatformDebug)
#endif
