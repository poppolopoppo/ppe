#pragma once

#include "Meta/Aliases.h"

#define PPE_HAL_MAKEINCLUDE(_BASENAME) \
    STRINGIZE(HAL/TARGET_PLATFORM/CONCAT(TARGET_PLATFORM, _BASENAME).h)

#define PPE_HAL_TARGETALIAS(_BASENAME) \
    CONCAT3(F, TARGET_PLATFORM, _BASENAME)

#define PPE_HAL_MAKEALIAS(_BASENAME) \
    namespace PPE { \
        using CONCAT(F, _BASENAME) = PPE_HAL_TARGETALIAS(_BASENAME); \
    }
#define PPE_HAL_MAKEALIAS_NAMESPACE(_NAMESPACE, _BASENAME) \
    namespace PPE { namespace _NAMESPACE { \
        using CONCAT(F, _BASENAME) = PPE_HAL_TARGETALIAS(_BASENAME); \
    } }

#include PPE_HAL_MAKEINCLUDE(PlatformMacros)

// Note: this is the only HAL header where GenericPlatform is included after the current platform
// It allows GenericPlatformMacros.h to define optional macros if the current platform did not
#include "HAL/Generic/GenericPlatformMacros.h"
