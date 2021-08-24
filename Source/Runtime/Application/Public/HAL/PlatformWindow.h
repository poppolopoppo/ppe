#pragma once

// simple wrapper for target platform

#include "HAL/PlatformMacros.h"
#include PPE_HAL_MAKEINCLUDE(Window)
namespace PPE { namespace Application {
using FPlatformWindow = CONCAT3(F, TARGET_PLATFORM, Window);
}}
