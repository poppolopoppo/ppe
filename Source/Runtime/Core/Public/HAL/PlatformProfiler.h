#pragma once

// simple wrapper for target platform

#include "HAL/TargetPlatform.h"
#include "HAL/Generic/GenericPlatformProfiler.h"
#if USE_PPE_PLATFORM_PROFILER
#   include PPE_HAL_MAKEINCLUDE(PlatformProfiler)
PPE_HAL_MAKEALIAS(PlatformProfiler)
#endif
