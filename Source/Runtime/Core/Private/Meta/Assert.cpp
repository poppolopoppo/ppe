#include "stdafx.h"

#include "Meta/Assert.h"

#if USE_PPE_ASSERT || USE_PPE_ASSERT_RELEASE

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
static FPlatformDialog::EResult AssertAbortRetryIgnore_(
    FPlatformDialog::EIcon icon,
    const FWStringView& title, const wchar_t* msg,
    const wchar_t *file, unsigned line ) {
    FWStringBuilder oss;

    oss << title << Crlf
        << L"----------------------------------------------------------------" << Crlf
        << file << L'(' << line << L"): " << msg;

    return FPlatformDialog::AbortRetryIgnore(oss.ToString(), title, icon);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!USE_PPE_ASSERT || USE_PPE_ASSERT_RELEASE

#if USE_PPE_ASSERT
namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static std::atomic<FAssertHandler> GAssertionHandler = { nullptr };
//----------------------------------------------------------------------------
static bool ReportAssertionForDebug_(
    const FWStringView& level,
    const wchar_t* msg, const wchar_t* file, unsigned line) {
#if USE_PPE_PLATFORM_DEBUG
    if (FPlatformDebug::IsDebuggerPresent()) {
        wchar_t buf[1024];
        FWFixedSizeTextWriter oss(buf);
        oss << L"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << Eol
            << L" !!! " << level << L" assertion failed !!!" << Eol
            << L"   text:  " << MakeCStringView(msg) << Eol
            << L"   file:  " << MakeCStringView(file) << L"(" << line << L")" << Eol
            << L"  -> breaking the debugger" << Eol
            << L"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << Eol
            << Eos;
        FPlatformDebug::OutputDebug(buf);
        PPE_DEBUG_BREAK();
        return true;
    }
#else
    UNUSED(level);
    UNUSED(msg);
    UNUSED(file);
    UNUSED(line);
#endif
    return false;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FAssertException::FAssertException(const char *msg, const wchar_t *file, unsigned line)
:   FException(msg), _file(file), _line(line) {}
//----------------------------------------------------------------------------
FAssertException::~FAssertException() = default;
//----------------------------------------------------------------------------
#if USE_PPE_EXCEPTION_DESCRIPTION
FWTextWriter& FAssertException::Description(FWTextWriter& oss) const {
    return oss
        << L"debug assert '" << MakeCStringView(What()) << L"' failed !" << Eol
        << MakeCStringView(File()) << L':' << Line();
}
#endif
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

    bool failure = false;

    FAssertHandler const handler = GAssertionHandler.load();

    if (handler) {
        failure = (*handler)(msg, file, line);
    }
    else if (not ReportAssertionForDebug_(L"debug", msg, file, line)) {
        LOG(Assertion, Error, L"debug assert '{0}' failed !\n\t{1}({2})", MakeCStringView(msg), MakeCStringView(file), line);
        FLUSH_LOG(); // flush log before continuing to get eventual log messages

        switch (AssertAbortRetryIgnore_(FPlatformDialog::Warning, L"Assert debug failed !", msg, file, line)) {
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
void SetAssertionHandler(FAssertHandler handler) {
    GAssertionHandler.store(handler);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
#endif //!WITH_PPE_ASSERT

#if USE_PPE_ASSERT_RELEASE
namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static std::atomic<FAssertReleaseHandler> GAssertionReleaseHandler = { nullptr };
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FAssertReleaseException::FAssertReleaseException(const char *msg, const wchar_t *file, unsigned line)
:   FException(msg), _file(file), _line(line) {}
//----------------------------------------------------------------------------
FAssertReleaseException::~FAssertReleaseException() = default;
//----------------------------------------------------------------------------
#if USE_PPE_EXCEPTION_DESCRIPTION
FWTextWriter& FAssertReleaseException::Description(FWTextWriter& oss) const {
    return oss
        << L"release assert '" << MakeCStringView(What()) << L"' failed !" << Eol
        << MakeCStringView(File()) << L':' << Line();
}
#endif
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

    bool failure = true; // AssertRelease() fails by default

    FAssertReleaseHandler const handler = GAssertionReleaseHandler.load();

    if (handler) {
        failure = (*handler)(msg, file, line);
    }
    else if (not ReportAssertionForDebug_(L"release", msg, file, line)) {
        LOG(Assertion, Error, L"release assert '{0}' failed !\n\t{1}({2})", MakeCStringView(msg), MakeCStringView(file), line);
        FLUSH_LOG(); // flush log before continuing to get eventual log messages

        switch (AssertAbortRetryIgnore_(FPlatformDialog::Error, L"Assert release failed !", msg, file, line)) {
        case FPlatformDialog::Abort:
            failure = true;
            break;
        case FPlatformDialog::Retry:
            if (FPlatformDebug::IsDebuggerPresent())
                FPlatformDebug::DebugBreak();
            break;
        case FPlatformDialog::Ignore:
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
void SetAssertionReleaseHandler(FAssertReleaseHandler handler) {
    GAssertionReleaseHandler.store(handler);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
#endif //!WITH_PPE_ASSERT_RELEASE
