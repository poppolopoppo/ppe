#include "stdafx.h"

#include "Meta/Assert.h"

#if defined(WITH_PPE_ASSERT) || defined(WITH_PPE_ASSERT_RELEASE)

#include "Diagnostic/Exception.h"
#include "Diagnostic/Logger.h"
#include "HAL/PlatformDebug.h"
#include "HAL/PlatformDialog.h"
#include "IO/StringBuilder.h"
#include "IO/StringView.h"

namespace PPE {
LOG_CATEGORY(PPE_CORE_API, Assertion)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
static FPlatformDialog::EResult AssertAbortRetryIgnore_(const FWStringView& title, const wchar_t* msg, const wchar_t *file, unsigned line) {
    FWStringBuilder oss;

    oss << title << Crlf
        << L"----------------------------------------------------------------" << Crlf
        << file << L'(' << line << L"): " << msg;

    return FPlatformDialog::AbortRetryIgnore(oss.ToString(), title);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!defined(WITH_PPE_ASSERT) || defined(WITH_PPE_ASSERT_RELEASE)

#ifdef WITH_PPE_ASSERT
namespace PPE {
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
NO_INLINE void AssertionFailed(const wchar_t* msg, const wchar_t *file, unsigned line) {
    static THREAD_LOCAL bool GIsInAssertion = false;
    static THREAD_LOCAL bool GIgnoreAssertsInThisThread = false;
    static std::atomic<bool> GIgnoreAllAsserts = ATOMIC_VAR_INIT(false);
    if (GIgnoreAllAsserts || GIgnoreAssertsInThisThread)
        return;

    if (GIsInAssertion)
        PPE_THROW_IT(FAssertException("Assert reentrancy !", file, line));

    GIsInAssertion = true;

    LOG(Assertion, Error, L"debug assert '{0}' failed !\n\t{1}({2})", MakeCStringView(msg), MakeCStringView(file), line);

    FLUSH_LOG(); // flush log before continuing to get eventual log messages

    bool failure = false;

    AssertionHandler const handler = GAssertionHandler.load();

    if (handler) {
        failure = (*handler)(msg, file, line);
    }
    else if (FPlatformDebug::IsDebuggerPresent()) {
#ifdef PLATFORM_WINDOWS // breaking in this frame is much quicker for debugging
        ::DebugBreak();
#else
        FPlatformMisc::DebugBreak();
#endif
    }
    else {
        switch (AssertAbortRetryIgnore_(L"Assert debug failed !", msg, file, line)) {
        case FPlatformDialog::Abort:
            failure = true;
            break;
        case FPlatformDialog::Retry:
            if (FPlatformDebug::IsDebuggerPresent())
                FPlatformDebug::DebugBreak();
            break;
        case FGenericPlatformDialog::Ignore:
            failure = false;
            break;
        default:
            AssertNotReached();
        }
    }

    GIsInAssertion = false;

    if (failure)
        PPE_THROW_IT(FAssertException("Assert debug failed !", file, line));
}
//----------------------------------------------------------------------------
void SetAssertionHandler(AssertionHandler handler) {
    GAssertionHandler.store(handler);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
#endif //!WITH_PPE_ASSERT

#ifdef WITH_PPE_ASSERT_RELEASE
namespace PPE {
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
NO_INLINE void AssertionReleaseFailed(const wchar_t* msg, const wchar_t *file, unsigned line) {
    static THREAD_LOCAL bool GIsInAssertion = false;
    static THREAD_LOCAL bool GIgnoreAssertsInThisThread = false;
    static std::atomic<bool> GIgnoreAllAsserts = ATOMIC_VAR_INIT(false);
    if (GIgnoreAllAsserts || GIgnoreAssertsInThisThread)
        return;

    if (GIsInAssertion)
        PPE_THROW_IT(FAssertReleaseException("Assert release reentrancy !", file, line));

    GIsInAssertion = true;

    LOG(Assertion, Error, L"release assert '{0}' failed !\n\t{1}({2})", MakeCStringView(msg), MakeCStringView(file), line);

    FLUSH_LOG(); // flush log before continuing to get eventual log messages

    bool failure = true; // AssertRelease() fails by default

    AssertionReleaseHandler const handler = GAssertionReleaseHandler.load();

    if (handler) {
        failure = (*handler)(msg, file, line);
    }
    else if (FPlatformDebug::IsDebuggerPresent()) {
#ifdef PLATFORM_WINDOWS // breaking in this frame is much quicker for debugging
        ::DebugBreak();
#else
        FPlatformMisc::DebugBreak();
#endif
    }
    else {
        switch (AssertAbortRetryIgnore_(L"Assert release failed !", msg, file, line)) {
        case FPlatformDialog::Abort:
            failure = true;
            break;
        case FPlatformDialog::Retry:
            if (FPlatformDebug::IsDebuggerPresent())
                FPlatformDebug::DebugBreak();
            break;
        case FGenericPlatformDialog::Ignore:
            failure = false;
            break;
        default:
            AssertNotReached();
        }
    }

    GIsInAssertion = false;

    if (failure)
        PPE_THROW_IT(FAssertReleaseException("Assert release failed !", file, line));
}
//----------------------------------------------------------------------------
void SetAssertionReleaseHandler(AssertionReleaseHandler handler) {
    GAssertionReleaseHandler.store(handler);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
#endif //!WITH_PPE_ASSERT_RELEASE