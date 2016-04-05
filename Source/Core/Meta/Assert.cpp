#include "stdafx.h"

#include "Assert.h"

#if defined(WITH_CORE_ASSERT) || defined(WITH_CORE_ASSERT_RELEASE)

#ifdef OS_WINDOWS
#   include <windows.h>
#endif

#include "Diagnostic/CrtDebug.h"
#include "Diagnostic/DialogBox.h"
#include "Diagnostic/Exception.h"
#include "Diagnostic/Logger.h"

#include "IO/Stream.h"
#include "IO/StringSlice.h"

#include <atomic>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
static Dialog::Result AssertAbortRetryIgnore_(const WStringSlice& title, const char *msg, const wchar_t *file, unsigned line) {
    ThreadLocalWOStringStream oss;

    oss << title << L"\r\n"
        << L"----------------------------------------------------------------\r\n"
        << file << L'(' << line << L"): " << msg;

    return Dialog::AbortRetryIgnore(MakeStringSlice(oss.str()), title);
}
//----------------------------------------------------------------------------
static bool IsDebuggerAttached_() {
#ifdef OS_WINDOWS
    return ::IsDebuggerPresent() ? true : false;
#else
#   error "no support"
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#endif //!defined(WITH_CORE_ASSERT) || defined(WITH_CORE_ASSERT_RELEASE)

#ifdef WITH_CORE_ASSERT
namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static std::atomic<AssertionHandler> gAssertionHandler = { nullptr };
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
AssertException::AssertException(const char *msg, const wchar_t *file, unsigned line)
:   Exception(msg), _file(file), _line(line) {}
//----------------------------------------------------------------------------
AssertException::~AssertException() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void AssertionFailed(const char *msg, const wchar_t *file, unsigned line) {
    static THREAD_LOCAL bool gIsInAssertion = false;
    static THREAD_LOCAL bool gIgnoreAssertsInThisThread = false;
    static std::atomic<bool> gIgnoreAllAsserts = ATOMIC_VAR_INIT(false);
    if (gIgnoreAllAsserts || gIgnoreAssertsInThisThread)
        return;

    if (gIsInAssertion)
        throw AssertException("Assert reentrancy !", WIDESTRING(__FILE__), __LINE__);

    gIsInAssertion = true;

    bool failure = false;

    LOG(Assertion, L"error: '{0}' failed !\n\t{1}({2})", msg, file, line);

    AssertionHandler const handler = gAssertionHandler.load();

    if (handler) {
        failure = (*handler)(msg, file, line);
    }
    else if (IsDebuggerAttached_()) {
        BREAKPOINT();
    }
    else {
        switch (AssertAbortRetryIgnore_(MakeStringSlice(L"Assert debug failed !"), msg, file, line)) {
        case Dialog::Result::Abort:
            failure = true;
            break;
        case Dialog::Result::Retry:
            if (IsDebuggerAttached_())
                BREAKPOINT();
            break;
        case Dialog::Result::Ignore:
            failure = false;
            break;
        default:
            AssertNotReached();
        }
    }

    gIsInAssertion = false;

    if (failure)
        throw AssertException(msg, file, line);
}
//----------------------------------------------------------------------------
void SetAssertionHandler(AssertionHandler handler) {
    gAssertionHandler.store(handler);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
#endif //!WITH_CORE_ASSERT

#ifdef WITH_CORE_ASSERT_RELEASE
namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static std::atomic<AssertionReleaseHandler> gAssertionReleaseHandler = { nullptr };
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
AssertReleaseException::AssertReleaseException(const char *msg, const wchar_t *file, unsigned line)
:   Exception(msg), _file(file), _line(line) {}
//----------------------------------------------------------------------------
AssertReleaseException::~AssertReleaseException() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void AssertionReleaseFailed(const char *msg, const wchar_t *file, unsigned line) {
    static THREAD_LOCAL bool gIsInAssertion = false;
    static THREAD_LOCAL bool gIgnoreAssertsInThisThread = false;
    static std::atomic<bool> gIgnoreAllAsserts = ATOMIC_VAR_INIT(false);
    if (gIgnoreAllAsserts || gIgnoreAssertsInThisThread)
        return;

    if (gIsInAssertion)
        throw AssertReleaseException("Assert release reentrancy !", WIDESTRING(__FILE__), __LINE__);

    gIsInAssertion = true;

    bool failure = true; // AssertRelease() fails by default

    LOG(Assertion, L"error: '{0}' failed !\n\t{1}({2})", msg, file, line);

    AssertionReleaseHandler const handler = gAssertionReleaseHandler.load();

    if (handler) {
        failure = (*handler)(msg, file, line);
    }
    else if (IsDebuggerAttached_()) {
        BREAKPOINT();
    }
    else {
        switch (AssertAbortRetryIgnore_(MakeStringSlice(L"Assert release failed !"), msg, file, line)) {
        case Dialog::Result::Abort:
            failure = true;
            break;
        case Dialog::Result::Retry:
            if (IsDebuggerAttached_())
                BREAKPOINT();
            break;
        case Dialog::Result::Ignore:
            failure = false;
            break;
        default:
            AssertNotReached();
        }
    }

    gIsInAssertion = false;

    if (failure)
        throw AssertReleaseException(msg, file, line);
}
//----------------------------------------------------------------------------
void SetAssertionReleaseHandler(AssertionReleaseHandler handler) {
    gAssertionReleaseHandler.store(handler);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
#endif //!WITH_CORE_ASSERT_RELEASE
