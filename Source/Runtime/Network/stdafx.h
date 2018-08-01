// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

// Include all network related includes (priority matters)
#include "Core.Network/NetworkIncludes.h"

#include "Core/HAL/PlatformIncludes.h"

#include "Core.Network/Network.h"

#if USE_CORE_PRECOMPILEDHEADERS
#   include "Core.Network/stdafx.generated.h"
#endif
