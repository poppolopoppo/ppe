#pragma once

// simple wrapper for target platform

#include "HAL/PlatformMacros.h"
#include PPE_HAL_MAKEINCLUDE(PlatformGamepad)
PPE_HAL_MAKEALIAS_NAMESPACE(Application, PlatformGamepad)
