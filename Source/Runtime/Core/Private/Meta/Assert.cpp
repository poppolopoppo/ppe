#include "stdafx.h"

#include "Meta/Assert.h"

#if USE_PPE_ASSERT || USE_PPE_ASSERT_RELEASE

#include "Diagnostic/CurrentProcess.h"
#include "Diagnostic/Exception.h"
#include "Diagnostic/IgnoreList.h"
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
static FPlatformDialog::EResult AssertIgnoreOnceAlwaysAbortRetry_(
    FPlatformDialog::EIcon icon,
    const FWStringView& title, const wchar_t* msg,
    const wchar_t *file, unsigned line ) {
    FWStringBuilder oss;

    oss << title << Crlf
        << L"----------------------------------------------------------------" << Crlf
        << file << L'(' << line << L"): " << msg;

    return FPlatformDialog::IgnoreOnceAlwaysAbortRetry(oss.ToString(), title, icon);
}
//----------------------------------------------------------------------------
static bool ReportAssertionForDebug_(
    const FWStringView& level,
    const wchar_t* msg, const wchar_t* file, unsigned line) {
#if USE_PPE_PLATFORM_DEBUG
    if (FCurrentProcess::StartedWithDebugger()) {
        wchar_t buf[1024];
        FWFixedSizeTextWriter oss(buf);
        oss << Eol
            << L"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << Eol
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
static bool PPE_DEBUG_SECTION DefaultDebugAssertHandler_(const wchar_t* msg, const wchar_t *file, unsigned line) {
#if USE_PPE_IGNORELIST
    FIgnoreList::FIgnoreKey ignoreKey;
    ignoreKey << MakeStringView("AssertDebug")
        << MakeCStringView(msg) << MakeCStringView(file) << MakeRawConstView(line);
    if (FIgnoreList::HitIFP(ignoreKey) > 0)
        return false;
#endif

    if (not ReportAssertionForDebug_(L"debug", msg, file, line)) {
        LOG(Assertion, Error, L"debug assert '{0}' failed !\n\t{1}({2})", MakeCStringView(msg), MakeCStringView(file), line);
        FLUSH_LOG(); // flush log before continuing to get eventual log messages

        switch (AssertIgnoreOnceAlwaysAbortRetry_(FPlatformDialog::Warning, L"Assert debug failed !", msg, file, line)) {
        case FPlatformDialog::Abort:
            PPE_DEBUG_CRASH();
            break;
        case FPlatformDialog::Retry:
            if (FPlatformDebug::IsDebuggerPresent())
                FPlatformDebug::DebugBreak();
            return false;
        case FGenericPlatformDialog::Ignore:
            return false;
        case FGenericPlatformDialog::IgnoreAlways:
#if USE_PPE_IGNORELIST
            FIgnoreList::AddIFP(ignoreKey);
#endif
            return false;
        default:
            AssertNotReached();
        }
    }

    return false; // debug assertions are ignored by default
}
//----------------------------------------------------------------------------
static std::atomic<FAssertHandler> GAssertionHandler{ &DefaultDebugAssertHandler_ };
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
NO_INLINE void PPE_DEBUG_SECTION AssertionFailed(const wchar_t* msg, const wchar_t *file, unsigned line) {
    // You can tweak those static variables when debugging:
    static volatile THREAD_LOCAL bool GIsInAssertion = false;
    static volatile THREAD_LOCAL bool GIgnoreAssertsInThisThread = false;
    static volatile std::atomic<bool> GIgnoreAllAsserts = ATOMIC_VAR_INIT(false);

    if (GIgnoreAllAsserts || GIgnoreAssertsInThisThread)
        return;

    if (GIsInAssertion)
        PPE_THROW_IT(FAssertException("Assert reentrancy !", file, line));

    GIsInAssertion = true;

    bool failure = false;
    if (FAssertHandler const handler = GAssertionHandler.load())
        failure = (*handler)(msg, file, line);

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
#endif //!USE_PPE_ASSERT

#if USE_PPE_ASSERT_RELEASE
namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static bool PPE_DEBUG_SECTION DefaultReleaseAssertHandler_(const wchar_t* msg, const wchar_t *file, unsigned line) {
#if USE_PPE_IGNORELIST
    FIgnoreList::FIgnoreKey ignoreKey;
    ignoreKey << MakeStringView("AssertRelease")
        << MakeCStringView(msg) << MakeCStringView(file) << MakeRawConstView(line);
    if (FIgnoreList::HitIFP(ignoreKey) > 0)
        return false;
#endif

    if (not ReportAssertionForDebug_(L"release", msg, file, line)) {
        LOG(Assertion, Error, L"release assert '{0}' failed !\n\t{1}({2})", MakeCStringView(msg), MakeCStringView(file), line);
        FLUSH_LOG(); // flush log before continuing to get eventual log messages

        switch (AssertIgnoreOnceAlwaysAbortRetry_(FPlatformDialog::Error, L"Assert release failed !", msg, file, line)) {
        case FPlatformDialog::Abort:
            break;
        case FPlatformDialog::Retry:
            if (FPlatformDebug::IsDebuggerPresent())
                FPlatformDebug::DebugBreak();
            return false;
        case FPlatformDialog::Ignore:
            return false;
        case FGenericPlatformDialog::IgnoreAlways:
#if USE_PPE_IGNORELIST
            FIgnoreList::AddIFP(ignoreKey);
#endif
            return false;
        default:
            AssertNotReached();
        }
    }

    return true; // release assertions emit errors by default
}
//----------------------------------------------------------------------------
static std::atomic<FAssertReleaseHandler> GAssertionReleaseHandler{ &DefaultReleaseAssertHandler_ };
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
NO_INLINE void PPE_DEBUG_SECTION AssertionReleaseFailed(const wchar_t* msg, const wchar_t *file, unsigned line) {
    // You can tweak those static variables when debugging:
    static THREAD_LOCAL bool GIsInAssertion = false;
    static THREAD_LOCAL bool GIgnoreAssertsInThisThread = false;
    static std::atomic<bool> GIgnoreAllAsserts = ATOMIC_VAR_INIT(false);

    if (GIgnoreAllAsserts || GIgnoreAssertsInThisThread)
        return;

    if (GIsInAssertion)
        PPE_THROW_IT(FAssertReleaseException("Assert release reentrancy !", file, line));

    GIsInAssertion = true;

    bool failure = true; // AssertRelease() fails by default
    if (FAssertReleaseHandler const handler = GAssertionReleaseHandler.load())
        failure = (*handler)(msg, file, line);

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
#endif //!USE_PPE_ASSERT_RELEASE
