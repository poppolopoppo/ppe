#include "stdafx.h"

#include "Assert.h"

#if defined(WITH_CORE_ASSERT) || defined(WITH_CORE_ASSERT_RELEASE)

#include "Diagnostic/CrtDebug.h"
#include "Diagnostic/DialogBox.h"
#include "Diagnostic/Exception.h"
#include "Diagnostic/Logger.h"

#include "IO/StringBuilder.h"
#include "IO/StringView.h"

#include "Misc/TargetPlatform.h"

#include <atomic>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
static Dialog::EResult AssertAbortRetryIgnore_(const FWStringView& title, const char *msg, const wchar_t *file, unsigned line) {
    FWStringBuilder oss;

    oss << title << Crlf
        << L"----------------------------------------------------------------" << Crlf
        << file << L'(' << line << L"): " << msg;

    return Dialog::AbortRetryIgnore(oss.ToString(), title);
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
static std::atomic<AssertionHandler> GAssertionHandler = { nullptr };
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FAssertException::FAssertException(const char *msg, const wchar_t *file, unsigned line)
:   FException(msg), _file(file), _line(line) {}
//----------------------------------------------------------------------------
FAssertException::~FAssertException() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void AssertionFailed(const char *msg, const wchar_t *file, unsigned line) {
    static THREAD_LOCAL bool GIsInAssertion = false;
    static THREAD_LOCAL bool GIgnoreAssertsInThisThread = false;
    static std::atomic<bool> GIgnoreAllAsserts = ATOMIC_VAR_INIT(false);
    if (GIgnoreAllAsserts || GIgnoreAssertsInThisThread)
        return;

    LOG(Assertion, L"error: '{0}' failed !\n\t{1}({2})", msg, file, line);

    FLUSH_LOG(); // flush log before continuing to get eventual log messages

    if (GIsInAssertion)
        CORE_THROW_IT(FAssertException("Assert reentrancy !", WIDESTRING(__FILE__), __LINE__));

    GIsInAssertion = true;

    bool failure = false;

    AssertionHandler const handler = GAssertionHandler.load();

    if (handler) {
        failure = (*handler)(msg, file, line);
    }
    else if (FPlatformMisc::IsDebuggerAttached()) {
#ifdef PLATFORM_WINDOWS // breaking in this frame is much quicker for debugging
        ::DebugBreak();
#else
        FPlatformMisc::DebugBreak();
#endif
    }
    else {
        switch (AssertAbortRetryIgnore_(MakeStringView(L"Assert debug failed !"), msg, file, line)) {
        case Dialog::EResult::Abort:
            failure = true;
            break;
        case Dialog::EResult::Retry:
            if (FPlatformMisc::IsDebuggerAttached())
                FPlatformMisc::DebugBreak();
            break;
        case Dialog::EResult::Ignore:
            failure = false;
            break;
        default:
            AssertNotReached();
        }
    }

    GIsInAssertion = false;

    if (failure)
        CORE_THROW_IT(FAssertException(msg, file, line));
}
//----------------------------------------------------------------------------
void SetAssertionHandler(AssertionHandler handler) {
    GAssertionHandler.store(handler);
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
static std::atomic<AssertionReleaseHandler> GAssertionReleaseHandler = { nullptr };
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FAssertReleaseException::FAssertReleaseException(const char *msg, const wchar_t *file, unsigned line)
:   FException(msg), _file(file), _line(line) {}
//----------------------------------------------------------------------------
FAssertReleaseException::~FAssertReleaseException() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void AssertionReleaseFailed(const char *msg, const wchar_t *file, unsigned line) {
    static THREAD_LOCAL bool GIsInAssertion = false;
    static THREAD_LOCAL bool GIgnoreAssertsInThisThread = false;
    static std::atomic<bool> GIgnoreAllAsserts = ATOMIC_VAR_INIT(false);
    if (GIgnoreAllAsserts || GIgnoreAssertsInThisThread)
        return;

    LOG(Assertion, L"error: '{0}' failed !\n\t{1}({2})", msg, file, line);

    FLUSH_LOG(); // flush log before continuing to get eventual log messages

    if (GIsInAssertion)
        CORE_THROW_IT(FAssertReleaseException("Assert release reentrancy !", WIDESTRING(__FILE__), __LINE__));

    GIsInAssertion = true;

    bool failure = true; // AssertRelease() fails by default

    AssertionReleaseHandler const handler = GAssertionReleaseHandler.load();

    if (handler) {
        failure = (*handler)(msg, file, line);
    }
    else if (FPlatformMisc::IsDebuggerAttached()) {
#ifdef PLATFORM_WINDOWS // breaking in this frame is much quicker for debugging
        ::DebugBreak();
#else
        FPlatformMisc::DebugBreak();
#endif
    }
    else {
        switch (AssertAbortRetryIgnore_(MakeStringView(L"Assert release failed !"), msg, file, line)) {
        case Dialog::EResult::Abort:
            failure = true;
            break;
        case Dialog::EResult::Retry:
            if (FPlatformMisc::IsDebuggerAttached())
                FPlatformMisc::DebugBreak();
            break;
        case Dialog::EResult::Ignore:
            failure = false;
            break;
        default:
            AssertNotReached();
        }
    }

    GIsInAssertion = false;

    if (failure)
        CORE_THROW_IT(FAssertReleaseException(msg, file, line));
}
//----------------------------------------------------------------------------
void SetAssertionReleaseHandler(AssertionReleaseHandler handler) {
    GAssertionReleaseHandler.store(handler);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
#endif //!WITH_CORE_ASSERT_RELEASE
