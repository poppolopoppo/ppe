#pragma once

// simple wrapper for target platform

#include "HAL/TargetPlatform.h"
#include PPE_HAL_MAKEINCLUDE(PlatformMessageHandler)
PPE_HAL_MAKEALIAS_NAMESPACE(Application, PlatformMessageHandler)