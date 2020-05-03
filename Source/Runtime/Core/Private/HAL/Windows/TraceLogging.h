#pragma once

#include "Core.h"

#ifdef PLATFORM_WINDOWS

#include "HAL/Windows/WindowsPlatformDebug.h"

#define USE_PPE_WINDOWS_TRACELOGGING (USE_PPE_PLATFORM_DEBUG)

#if USE_PPE_WINDOWS_TRACELOGGING

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FWindowsTraceLogging {
public:
    static void Create();
    static void Destroy();

    static void TraceVerbose(const wchar_t* category, i64 timestamp, const wchar_t* filename, size_t line, const wchar_t* text);
    static void TraceInformation(const wchar_t* category, i64 timestamp, const wchar_t* filename, size_t line, const wchar_t* text);
    static void TraceWarning(const wchar_t* category, i64 timestamp, const wchar_t* filename, size_t line, const wchar_t* text);
    static void TraceError(const wchar_t* category, i64 timestamp, const wchar_t* filename, size_t line, const wchar_t* text);
    static void TraceFatal(const wchar_t* category, i64 timestamp, const wchar_t* filename, size_t line, const wchar_t* text);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!USE_PPE_WINDOWS_TRACELOGGING

#else
#   define USE_PPE_WINDOWS_TRACELOGGING (0)
#endif //!PLATFORM_WINDOWS
