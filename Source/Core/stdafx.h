// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WITH_CORE_CRTDBG_MAP_ALLOC 0

#if defined(_MSC_VER)
//  Include this asap to avoid M$ macro substitutions bullshit
#   include "Core/Misc/Platform_Windows.h"
#endif

#include "Core/Core.h"

#if USE_CORE_PRECOMPILEDHEADERS
#   include "Core/stdafx.generated.h"
#endif
