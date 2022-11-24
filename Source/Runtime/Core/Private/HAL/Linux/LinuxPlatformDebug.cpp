// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "HAL/Linux/LinuxPlatformDebug.h"

#if USE_PPE_PLATFORM_DEBUG && defined(PLATFORM_LINUX)

#   include "HAL/TargetPlatform.h"

#   include "Diagnostic/Logger.h"
#   include "IO/StringBuilder.h"

#   include <inttypes.h>
#   include <iostream>
#   include <pthread.h>
#   include <sys/types.h>
#   include <sys/ptrace.h>
#   include <sys/wait.h>
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
    Unused(timestamp);

    FStringBuilder sb;
    sb  << '[' << MakeCStringView(category) << "] "
        << MakeCStringView(text) << " at: "
        << MakeCStringView(filename);

    ::syslog(priority, "%s (%d)", sb.c_str(), checked_cast<int>(line));
}
//----------------------------------------------------------------------------
static int CheckIfGdbIsPresent_() {
    int pid = ::fork();
    int status;
    int res;

    if (pid == -1) {
        ::perror("fork");
        return -1;
    }

    if (pid == 0) {
        int ppid = ::getppid();

        /* Child */
        if (::ptrace(PTRACE_ATTACH, ppid, NULL, NULL) == 0) {
            /* Wait for the parent to stop and continue it */
            ::waitpid(ppid, NULL, 0);
            ::ptrace(PTRACE_CONT, ppid, NULL, NULL);

            /* Detach */
            ::ptrace(PTRACE_DETACH, ppid, NULL, NULL);

            /* We were the tracers, so gdb is not present */
            res = 0;
        }
        else {
            /* Trace failed so gdb is present */
            res = 1;
        }

        _Exit(res);
    }
    else {
        ::waitpid(pid, &status, 0);
        res = WEXITSTATUS(status);
    }
    return res;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FLinuxPlatformDebug::IsDebuggerPresent() {
    static int GIsDebuggerPresent = -1;
    if (Unlikely(-1 == GIsDebuggerPresent))
        GIsDebuggerPresent = CheckIfGdbIsPresent_();
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
    const int error = ::pthread_setname_np(
        ::pthread_self(), name
    );
    CLOG(0 != error, HAL, Error, L"failed to set thread debug name: '{1}' ({0})", error, MakeCStringView(name));
    Unused(error);
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
    Unused(uid);
    Unused(name);
    // #TODO?
}
//----------------------------------------------------------------------------
void FLinuxPlatformDebug::EndNamedEvent(u32 uid) {
    Unused(uid);
    // #TODO?
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!USE_PPE_PLATFORM_DEBUG && defined(PLATFORM_LINUX)
