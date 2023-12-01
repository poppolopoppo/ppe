// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "HAL/Windows/TraceLogging.h"

#if USE_PPE_WINDOWS_TRACELOGGING

#include "Time/DateTime.h"
#include "Time/Timestamp.h"

#include "HAL/Windows/WindowsPlatformIncludes.h"

// https://docs.microsoft.com/en-us/windows/win32/tracelogging/tracelogging-native-quick-start
#include <TraceLoggingProvider.h> // Win10 SDK
#include <evntrace.h> // TRACE_LEVEL_*
#pragma comment(lib, "advapi32.lib")

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
// {F42256F3-2773-4C4F-9E81-9924DF0DCDB3}
TRACELOGGING_DEFINE_PROVIDER(
    GWindowsTraceLogging_,
    "PPE Logger",
    (0xf42256f3, 0x2773, 0x4c4f, 0x9e, 0x81, 0x99, 0x24, 0xdf, 0x0d, 0xcd, 0xb3) );
//----------------------------------------------------------------------------
static ::SYSTEMTIME MakeSystemTime_(i64 timestamp) {
    const FDateTime utc = FTimestamp{ timestamp }.ToDateTimeUTC();

    ::SYSTEMTIME syst;
    syst.wYear = checked_cast<::WORD>(utc.Year);
    syst.wMonth = checked_cast<::WORD>(utc.Month);
    syst.wDayOfWeek = checked_cast<::WORD>(utc.DayOfWeek);
    syst.wDay = checked_cast<::WORD>(utc.Day);
    syst.wHour = checked_cast<::WORD>(utc.Hours);
    syst.wMinute = checked_cast<::WORD>(utc.Minutes);
    syst.wSecond = checked_cast<::WORD>(utc.Seconds);
    syst.wMilliseconds = 0;

    return syst;
}
//----------------------------------------------------------------------------
#ifdef __clang__
#   pragma clang diagnostic push,
#   pragma clang diagnostic ignored "-Wbitwise-op-parentheses"
#endif
template <int _TraceLevel>
static void TraceLoggingImpl_(const std::thread::id& threadId, FConstChar category, i64 timestamp, FConstChar filename, size_t line, FConstChar text, const Opaq::value_view& data) {
#if 0 // don't have access to native thread id, use hash value?
    const u32 tid = static_cast<u32>(std::hash<std::thread::id>{}(threadId));
#else
    const u32 tid = bit_cast<u32, std::thread::id>(threadId);
#endif

    Unused(data); // #TODO: macros with va_args are annoying...

    TraceLoggingWrite(GWindowsTraceLogging_,
        "LogEvent",
        TraceLoggingLevel(_TraceLevel),
        TraceLoggingTid(tid),
        TraceLoggingString(category.c_str(), "category"),
        TraceLoggingString(text.c_str(), "message"),
        TraceLoggingSystemTimeUtc(MakeSystemTime_(timestamp), "date"),
        TraceLoggingString(filename.c_str(), "filename"),
        TraceLoggingUInt32(u32(line), "line") );
}
template <int _TraceLevel>
static void TraceLoggingImpl_(const std::thread::id& threadId, FConstWChar category, i64 timestamp, FConstWChar filename, size_t line, FConstWChar text, const Opaq::value_view& data) {

#if 0 // don't have access to native thread id, use hash value?
    const u32 tid = static_cast<u32>(std::hash<std::thread::id>{}(threadId));
#else
    const u32 tid = bit_cast<u32, std::thread::id>(threadId);
#endif

    Unused(data); // #TODO: macros with va_args are annoying...

    TraceLoggingWrite(GWindowsTraceLogging_,
        "LogEvent",
        TraceLoggingLevel(_TraceLevel),
        TraceLoggingTid(tid),
        TraceLoggingWideString(category.c_str(), "category"),
        TraceLoggingWideString(text.c_str(), "message"),
        TraceLoggingSystemTimeUtc(MakeSystemTime_(timestamp), "date"),
        TraceLoggingWideString(filename.c_str(), "filename"),
        TraceLoggingUInt32(u32(line), "line") );
}
#ifdef __clang__
#   pragma clang diagnostic pop
#endif
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FWindowsTraceLogging::Create() {
    ::TraceLoggingRegister(GWindowsTraceLogging_);
}
//----------------------------------------------------------------------------
void FWindowsTraceLogging::Destroy() {
    ::TraceLoggingUnregister(GWindowsTraceLogging_);
}
//----------------------------------------------------------------------------
void FWindowsTraceLogging::TraceVerbose(const std::thread::id& tid, FConstChar category, i64 timestamp, FConstChar filename, size_t line, FConstChar text, const Opaq::value_view& data) {
    return TraceLoggingImpl_<TRACE_LEVEL_VERBOSE>(tid, category, timestamp, filename, line, text, data);
}
void FWindowsTraceLogging::TraceInformation(const std::thread::id& tid, FConstChar category, i64 timestamp, FConstChar filename, size_t line, FConstChar text, const Opaq::value_view& data) {
    return TraceLoggingImpl_<TRACE_LEVEL_INFORMATION>(tid, category, timestamp, filename, line, text, data);
}
void FWindowsTraceLogging::TraceWarning(const std::thread::id& tid, FConstChar category, i64 timestamp, FConstChar filename, size_t line, FConstChar text, const Opaq::value_view& data) {
    return TraceLoggingImpl_<TRACE_LEVEL_WARNING>(tid, category, timestamp, filename, line, text, data);
}
void FWindowsTraceLogging::TraceError(const std::thread::id& tid, FConstChar category, i64 timestamp, FConstChar filename, size_t line, FConstChar text, const Opaq::value_view& data) {
    return TraceLoggingImpl_<TRACE_LEVEL_ERROR>(tid, category, timestamp, filename, line, text, data);
}
void FWindowsTraceLogging::TraceFatal(const std::thread::id& tid, FConstChar category, i64 timestamp, FConstChar filename, size_t line, FConstChar text, const Opaq::value_view& data) {
    return TraceLoggingImpl_<TRACE_LEVEL_CRITICAL/* TRACE_LEVEL_FATAL <- deprecated */>(tid, category, timestamp, filename, line, text, data);
}
//----------------------------------------------------------------------------
void FWindowsTraceLogging::TraceVerbose(const std::thread::id& tid, FConstWChar category, i64 timestamp, FConstWChar filename, size_t line, FConstWChar text, const Opaq::value_view& data) {
    return TraceLoggingImpl_<TRACE_LEVEL_VERBOSE>(tid, category, timestamp, filename, line, text, data);
}
void FWindowsTraceLogging::TraceInformation(const std::thread::id& tid, FConstWChar category, i64 timestamp, FConstWChar filename, size_t line, FConstWChar text, const Opaq::value_view& data) {
    return TraceLoggingImpl_<TRACE_LEVEL_INFORMATION>(tid, category, timestamp, filename, line, text, data);
}
void FWindowsTraceLogging::TraceWarning(const std::thread::id& tid, FConstWChar category, i64 timestamp, FConstWChar filename, size_t line, FConstWChar text, const Opaq::value_view& data) {
    return TraceLoggingImpl_<TRACE_LEVEL_WARNING>(tid, category, timestamp, filename, line, text, data);
}
void FWindowsTraceLogging::TraceError(const std::thread::id& tid, FConstWChar category, i64 timestamp, FConstWChar filename, size_t line, FConstWChar text, const Opaq::value_view& data) {
    return TraceLoggingImpl_<TRACE_LEVEL_ERROR>(tid, category, timestamp, filename, line, text, data);
}
void FWindowsTraceLogging::TraceFatal(const std::thread::id& tid, FConstWChar category, i64 timestamp, FConstWChar filename, size_t line, FConstWChar text, const Opaq::value_view& data) {
    return TraceLoggingImpl_<TRACE_LEVEL_CRITICAL/* TRACE_LEVEL_FATAL <- deprecated */>(tid, category, timestamp, filename, line, text, data);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!USE_PPE_WINDOWS_TRACELOGGING
