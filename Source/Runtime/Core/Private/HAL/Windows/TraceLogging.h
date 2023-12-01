#pragma once

#include "Core.h"

#ifdef PLATFORM_WINDOWS

#include "HAL/Windows/WindowsPlatformDebug.h"

#define USE_PPE_WINDOWS_TRACELOGGING (USE_PPE_PLATFORM_DEBUG)

#if USE_PPE_WINDOWS_TRACELOGGING

#include "IO/ConstChar.h"
#include "Misc/Opaque.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FWindowsTraceLogging {
public:
    static void Create();
    static void Destroy();

    static void TraceVerbose(const std::thread::id& tid, FConstChar category, i64 timestamp, FConstChar filename, size_t line, FConstChar text, const Opaq::value_view& data);
    static void TraceInformation(const std::thread::id& tid, FConstChar category, i64 timestamp, FConstChar filename, size_t line, FConstChar text, const Opaq::value_view& data);
    static void TraceWarning(const std::thread::id& tid, FConstChar category, i64 timestamp, FConstChar filename, size_t line, FConstChar text, const Opaq::value_view& data);
    static void TraceError(const std::thread::id& tid, FConstChar category, i64 timestamp, FConstChar filename, size_t line, FConstChar text, const Opaq::value_view& data);
    static void TraceFatal(const std::thread::id& tid, FConstChar category, i64 timestamp, FConstChar filename, size_t line, FConstChar text, const Opaq::value_view& data);

    static void TraceVerbose(const std::thread::id& tid, FConstWChar category, i64 timestamp, FConstWChar filename, size_t line, FConstWChar text, const Opaq::value_view& data);
    static void TraceInformation(const std::thread::id& tid, FConstWChar category, i64 timestamp, FConstWChar filename, size_t line, FConstWChar text, const Opaq::value_view& data);
    static void TraceWarning(const std::thread::id& tid, FConstWChar category, i64 timestamp, FConstWChar filename, size_t line, FConstWChar text, const Opaq::value_view& data);
    static void TraceError(const std::thread::id& tid, FConstWChar category, i64 timestamp, FConstWChar filename, size_t line, FConstWChar text, const Opaq::value_view& data);
    static void TraceFatal(const std::thread::id& tid, FConstWChar category, i64 timestamp, FConstWChar filename, size_t line, FConstWChar text, const Opaq::value_view& data);

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!USE_PPE_WINDOWS_TRACELOGGING

#else
#   define USE_PPE_WINDOWS_TRACELOGGING (0)
#endif //!PLATFORM_WINDOWS
