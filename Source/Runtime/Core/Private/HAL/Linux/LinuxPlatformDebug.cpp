#include "stdafx.h"

#include "HAL/Linux/LinuxPlatformDebug.h"

#if USE_PPE_PLATFORM_DEBUG && defined(PLATFORM_LINUX)

#   include "IO/StringBuilder.h"

#   include <inttypes.h>
#   include <iostream>
#   include <pthread.h>
#   include <sys/types.h>
#   include <sys/ptrace.h>
#   include <syslog.h>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static void SyslogTrace_(int priority,
    const wchar_t* category, i64 timestamp, const wchar_t* filename, size_t line,
    const wchar_t* text) {
    UNUSED(timestamp);

    FStringBuilder sb;
    sb  << '[' << MakeCStringView(category) << "] "
        << MakeCStringView(text) << " at: "
        << MakeCStringView(filename);

    ::syslog(priority, "%s (%d)", sb.c_str(), checked_cast<int>(line));
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FLinuxPlatformDebug::IsDebuggerPresent() {
    static int GIsDebuggerPresent = -1;
     if (-1 == GIsDebuggerPresent)
     {
        if (::ptrace(PTRACE_TRACEME, 0, 1, 0) < 0)
            GIsDebuggerPresent = 1;
        else ::ptrace(PTRACE_DETACH, 0, 1, 0);
            GIsDebuggerPresent = 0;
    }
    return (1 == GIsDebuggerPresent);
}
//----------------------------------------------------------------------------
void FLinuxPlatformDebug::OutputDebug(const char* text) {
    Assert(text);
    std::clog << text;
}
//----------------------------------------------------------------------------
void FLinuxPlatformDebug::OutputDebug(const wchar_t* wtext) {
    Assert(wtext);
    std::wclog << wtext;
}
//----------------------------------------------------------------------------
void FLinuxPlatformDebug::GuaranteeStackSizeForStackOverflowRecovery() {
    // #TODO: custom thread class ? it can only be done when calling pthread_create()

    // ::pthread_attr_t threadAttr;
    // Verify(0 == ::pthread_getattr_np(::pthread_self(), &threadAttr));
    // Verify(0 == ::pthread_attr_setguardsize(&threadAttr, 4096));
    // ::pthread_attr_destroy(&threadAttr);
}
//----------------------------------------------------------------------------
void FLinuxPlatformDebug::SetThreadDebugName(const char* name) {
    Assert(name);
    Verify(0 == ::pthread_setname_np(
        ::pthread_self(), name
    ));
}
//----------------------------------------------------------------------------
void FLinuxPlatformDebug::TraceVerbose(const wchar_t* category, i64 timestamp, const wchar_t* filename, size_t line, const wchar_t* text) {
    SyslogTrace_(LOG_DEBUG, category, timestamp, filename, line, text);
}
//----------------------------------------------------------------------------
void FLinuxPlatformDebug::TraceInformation(const wchar_t* category, i64 timestamp, const wchar_t* filename, size_t line, const wchar_t* text) {
    SyslogTrace_(LOG_INFO, category, timestamp, filename, line, text);
}
//----------------------------------------------------------------------------
void FLinuxPlatformDebug::TraceWarning(const wchar_t* category, i64 timestamp, const wchar_t* filename, size_t line, const wchar_t* text) {
    SyslogTrace_(LOG_WARNING, category, timestamp, filename, line, text);
}
//----------------------------------------------------------------------------
void FLinuxPlatformDebug::TraceError(const wchar_t* category, i64 timestamp, const wchar_t* filename, size_t line, const wchar_t* text) {
    SyslogTrace_(LOG_ERR, category, timestamp, filename, line, text);
}
//----------------------------------------------------------------------------
void FLinuxPlatformDebug::TraceFatal(const wchar_t* category, i64 timestamp, const wchar_t* filename, size_t line, const wchar_t* text) {
    SyslogTrace_(LOG_CRIT, category, timestamp, filename, line, text);
}
//----------------------------------------------------------------------------
void FLinuxPlatformDebug::BeginNamedEvent(u32 uid, const char* name) {
    UNUSED(uid);
    UNUSED(name);
    // #TODO?
}
//----------------------------------------------------------------------------
void FLinuxPlatformDebug::EndNamedEvent(u32 uid) {
    UNUSED(uid);
    // #TODO?
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!USE_PPE_PLATFORM_DEBUG && defined(PLATFORM_LINUX)