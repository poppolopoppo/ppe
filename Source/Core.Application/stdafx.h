// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include "Core/HAL/PlatformIncludes.h"

#include "Core.Graphics/Graphics.h"

#if CORE_APPLICATION_CONSOLE && CORE_APPLICATION_GRAPHICS
#   error "can't compile Core.Application.Console and Core.Application.Graphics at the same time !"
#endif

#if !(CORE_APPLICATION_CONSOLE || CORE_APPLICATION_GRAPHICS)
#   error "must select either Core.Application.Console or Core.Application.Graphics !"
#endif

#if USE_CORE_PRECOMPILEDHEADERS
#   if      CORE_APPLICATION_CONSOLE
#       include "Core.Application/stdafx.Console.generated.h"
#   elif    CORE_APPLICATION_GRAPHICS
#       include "Core.Application/stdafx.Graphics.generated.h"
#   endif
#endif
