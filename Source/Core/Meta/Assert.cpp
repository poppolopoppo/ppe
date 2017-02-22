#include "stdafx.h"

#include "Assert.h"

#if defined(WITH_CORE_ASSERT) || defined(WITH_CORE_ASSERT_RELEASE)

#include "Diagnostic/CrtDebug.h"
#include "Diagnostic/DialogBox.h"
#include "Diagnostic/Exception.h"
#include "Diagnostic/Logger.h"

#include "IO/Stream.h"
#include "IO/StringView.h"

#include "Misc/TargetPlatform.h"

#include <atomic>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
static Dialog::EResult AssertAbortRetryIgnore_(const FWStringView& title, const char *msg, const wchar_t *file, unsigned line) {
    FThreadLocalWOStringStream oss;

    oss << title << crlf
        << L"----------------------------------------------------------------" << crlf
        << file << L'(' << line << L"): " << msg;

    return Dialog::AbortRetryIgnore(MakeStringView(oss.str()), title);
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
FAssertException::FAssertException(const char *msg, const wchar_t *file, unsigned line)
:   FException(msg), _file(file), _line(line) {}
//----------------------------------------------------------------------------
FAssertException::~FAssertException() {}
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
        CORE_THROW_IT(FAssertException("Assert reentrancy !", WIDESTRING(__FILE__), __LINE__));

    gIsInAssertion = true;

    bool failure = false;

    LOG(Assertion, L"error: '{0}' failed !\n\t{1}({2})", msg, file, line);

    AssertionHandler const handler = gAssertionHandler.load();

    if (handler) {
        failure = (*handler)(msg, file, line);
    }
    else if (FPlatform::IsDebuggerAttached()) {
        FPlatform::DebugBreak();
    }
    else {
        switch (AssertAbortRetryIgnore_(MakeStringView(L"Assert debug failed !"), msg, file, line)) {
        case Dialog::EResult::Abort:
            failure = true;
            break;
        case Dialog::EResult::Retry:
            if (FPlatform::IsDebuggerAttached())
                FPlatform::DebugBreak();
            break;
        case Dialog::EResult::Ignore:
            failure = false;
            break;
        default:
            AssertNotReached();
        }
    }

    gIsInAssertion = false;

    if (failure)
        CORE_THROW_IT(FAssertException(msg, file, line));
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
FAssertReleaseException::FAssertReleaseException(const char *msg, const wchar_t *file, unsigned line)
:   FException(msg), _file(file), _line(line) {}
//----------------------------------------------------------------------------
FAssertReleaseException::~FAssertReleaseException() {}
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
        CORE_THROW_IT(FAssertReleaseException("Assert release reentrancy !", WIDESTRING(__FILE__), __LINE__));

    gIsInAssertion = true;

    bool failure = true; // AssertRelease() fails by default

    LOG(Assertion, L"error: '{0}' failed !\n\t{1}({2})", msg, file, line);

    AssertionReleaseHandler const handler = gAssertionReleaseHandler.load();

    if (handler) {
        failure = (*handler)(msg, file, line);
    }
    else if (FPlatform::IsDebuggerAttached()) {
        FPlatform::DebugBreak();
    }
    else {
        switch (AssertAbortRetryIgnore_(MakeStringView(L"Assert release failed !"), msg, file, line)) {
        case Dialog::EResult::Abort:
            failure = true;
            break;
        case Dialog::EResult::Retry:
            if (FPlatform::IsDebuggerAttached())
                FPlatform::DebugBreak();
            break;
        case Dialog::EResult::Ignore:
            failure = false;
            break;
        default:
            AssertNotReached();
        }
    }

    gIsInAssertion = false;

    if (failure)
        CORE_THROW_IT(FAssertReleaseException(msg, file, line));
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
