#pragma once

#include "IODetouring.h"

#define USE_PPE_IODETOURING_DEBUG 0 // %_NOCOMMIT%

#if USE_PPE_IODETOURING_DEBUG
#   define IODETOURING_DEBUGPRINTF(_FMT, ...) IODetouringDebugPrintf_("[%08X] IODetouring: " _FMT, GetCurrentThreadId(), ## __VA_ARGS__)
#else
#   define IODETOURING_DEBUGPRINTF(_FMT, ...) NOOP()
#endif

#if USE_PPE_IODETOURING_DEBUG
#include "HAL/PlatformIncludes.h"

#include <strsafe.h>

PRAGMA_DISABLE_OPTIMIZATION // Note: voluntary spill into including sources

static void IODetouringDebugPrintf_(const char* pzFmt, ...) {
    char szTmp[8192];

    va_list args;
    va_start(args, pzFmt);
    StringCchVPrintfA(szTmp, ARRAYSIZE(szTmp), pzFmt, args);
    va_end(args);

    OutputDebugStringA(szTmp);
}
#endif //!USE_PPE_IODETOURING_DEBUG
