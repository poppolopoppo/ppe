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
template <int _TraceLevel>
static void TraceLoggingImpl_(const wchar_t* category, i64 timestamp, const wchar_t* filename, size_t line, const wchar_t* text) {
    const FDateTime utc = FTimestamp{ timestamp }.ToDateTimeUTC();

    SYSTEMTIME syst;
    syst.wYear = checked_cast<::WORD>(utc.Year);
    syst.wMonth = checked_cast<::WORD>(utc.Month);
    syst.wDayOfWeek = checked_cast<::WORD>(utc.DayOfWeek);
    syst.wDay = checked_cast<::WORD>(utc.Day);
    syst.wHour = checked_cast<::WORD>(utc.Hours);
    syst.wMinute = checked_cast<::WORD>(utc.Minutes);
    syst.wSecond = checked_cast<::WORD>(utc.Seconds);
    syst.wMilliseconds = 0;

    TraceLoggingWrite(GWindowsTraceLogging_,
        "LogEvent",
        TraceLoggingLevel(_TraceLevel),
        TraceLoggingTid(::GetThreadId(NULL)),
        TraceLoggingWideString(category, "category"),
        TraceLoggingWideString(text, "message"),
        TraceLoggingSystemTimeUtc(syst, "date"),
        TraceLoggingWideString(filename, "filename"),
        TraceLoggingUInt32(u32(line), "line") );
}
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
void FWindowsTraceLogging::TraceVerbose(const wchar_t* category, i64 timestamp, const wchar_t* filename, size_t line, const wchar_t* text) {
    TraceLoggingImpl_<TRACE_LEVEL_VERBOSE>(category, timestamp, filename, line, text);
}
//----------------------------------------------------------------------------
void FWindowsTraceLogging::TraceInformation(const wchar_t* category, i64 timestamp, const wchar_t* filename, size_t line, const wchar_t* text) {
    TraceLoggingImpl_<TRACE_LEVEL_INFORMATION>(category, timestamp, filename, line, text);
}
//----------------------------------------------------------------------------
void FWindowsTraceLogging::TraceWarning(const wchar_t* category, i64 timestamp, const wchar_t* filename, size_t line, const wchar_t* text) {
    TraceLoggingImpl_<TRACE_LEVEL_WARNING>(category, timestamp, filename, line, text);
}
//----------------------------------------------------------------------------
void FWindowsTraceLogging::TraceError(const wchar_t* category, i64 timestamp, const wchar_t* filename, size_t line, const wchar_t* text) {
    TraceLoggingImpl_<TRACE_LEVEL_ERROR>(category, timestamp, filename, line, text);
}
//----------------------------------------------------------------------------
void FWindowsTraceLogging::TraceFatal(const wchar_t* category, i64 timestamp, const wchar_t* filename, size_t line, const wchar_t* text) {
    TraceLoggingImpl_<TRACE_LEVEL_CRITICAL/* TRACE_LEVEL_FATAL <- deprecated */>(category, timestamp, filename, line, text);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!USE_PPE_WINDOWS_TRACELOGGING
