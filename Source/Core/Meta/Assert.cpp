#include "stdafx.h"

#include "Assert.h"

#if defined(WITH_CORE_ASSERT) || defined(WITH_CORE_ASSERT_RELEASE)

#ifdef OS_WINDOWS
#   include <windows.h>
#endif

#include "Diagnostic/Callstack.h"
#include "Diagnostic/CrtDebug.h"
#include "Diagnostic/DecodedCallstack.h"
#include "Diagnostic/DialogBox.h"
#include "Diagnostic/Logger.h"

#include <atomic>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
static DialogBox::Result AssertAbortRetryIgnore_(const char *msg, const wchar_t *file, unsigned line) {
    Callstack callstack;
    Callstack::Capture(&callstack, 3, Callstack::MaxDeph);

    DecodedCallstack decodedCallstack;
    callstack.Decode(&decodedCallstack);

    wchar_t text[4096];
    Format(
        text,
        L"{0}\n"
        L"  at: {1}({2})\n"
        L"\n"
        L"Callstack :\n{3}",
        msg,
        file, line,
        decodedCallstack);

    return DialogBox::AbortRetryIgnore(text, L"Assert failed !", DialogBox::Icon::Exclamation);
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
static volatile AssertionHandler *gAssertionHandler = nullptr;
static THREAD_LOCAL bool gIsInAssertion = false;
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
AssertException::AssertException(const char *msg, const wchar_t *file, unsigned line)
:   std::exception(msg), _file(file), _line(line) {}
//----------------------------------------------------------------------------
AssertException::~AssertException() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void AssertionFailed(const char *msg, const wchar_t *file, unsigned line) {
    static bool gDotNotBugMeAgain = false;
    if (gDotNotBugMeAgain)
        return;

    if (gIsInAssertion)
        throw AssertException("Assert reentrancy !", WIDESTRING(__FILE__), __LINE__);

    gIsInAssertion = true;

    bool failure = false;

    LOG(Assertion, L"error: '{0}' failed !\n\t{1}({2})", msg, file, line);

    AssertionHandler const* handler = (AssertionHandler const*)gAssertionHandler;
    std::atomic_thread_fence(std::memory_order_acq_rel);

    if (handler) {
        failure = (*handler)(msg, file, line);
    }
    else if (IsDebuggerAttached_()) {
        BREAKPOINT();
    }
    else {
        switch (AssertAbortRetryIgnore_(msg, file, line)) {
        case DialogBox::Result::Abort:
            failure = true;
            break;
        case DialogBox::Result::Retry:
            if (IsDebuggerAttached_())
                BREAKPOINT();
            break;
        case DialogBox::Result::Ignore:
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
void SetAssertionHandler(AssertionHandler *handler) {
    gAssertionHandler = handler;
    std::atomic_thread_fence(std::memory_order_acq_rel);
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
static volatile AssertionReleaseHandler *gAssertionReleaseHandler = nullptr;
static THREAD_LOCAL bool gIsInAssertionRelease = false;
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
AssertReleaseException::AssertReleaseException(const char *msg, const wchar_t *file, unsigned line)
:   std::exception(msg), _file(file), _line(line) {}
//----------------------------------------------------------------------------
AssertReleaseException::~AssertReleaseException() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void AssertionReleaseFailed(const char *msg, const wchar_t *file, unsigned line) {
    static bool gDotNotBugMeAgain = false;
    if (gDotNotBugMeAgain)
        return;

    if (gIsInAssertionRelease)
        throw AssertReleaseException("Assert release reentrancy !", WIDESTRING(__FILE__), __LINE__);

    gIsInAssertionRelease = true;

    bool failure = false;

    LOG(Assertion, L"error: '{0}' failed !\n\t{1}({2})", msg, file, line);

    AssertionReleaseHandler const* handler = (AssertionReleaseHandler const*)gAssertionReleaseHandler;
    std::atomic_thread_fence(std::memory_order_acq_rel);

    if (handler) {
        failure = (*handler)(msg, file, line);
    }
    else if (IsDebuggerAttached_()) {
        BREAKPOINT();
    }
    else {
        switch (AssertAbortRetryIgnore_(msg, file, line)) {
        case DialogBox::Result::Abort:
            failure = true;
            break;
        case DialogBox::Result::Retry:
            if (IsDebuggerAttached_())
                BREAKPOINT();
            break;
        case DialogBox::Result::Ignore:
            failure = false;
            break;
        default:
            AssertNotReached();
        }
    }

    gIsInAssertionRelease = false;

    if (failure)
        throw AssertReleaseException(msg, file, line);
}
//----------------------------------------------------------------------------
void SetAssertionReleaseHandler(AssertionReleaseHandler *handler) {
    gAssertionReleaseHandler = handler;
    std::atomic_thread_fence(std::memory_order_acq_rel);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
#endif //!WITH_CORE_ASSERT_RELEASE
