// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Meta/Assert.h"

#if USE_PPE_ASSERT || USE_PPE_ASSERT_RELEASE

#include "Diagnostic/CurrentProcess.h"
#include "Diagnostic/Exception.h"
#include "Diagnostic/IgnoreList.h"
#include "Diagnostic/Logger.h"
#include "HAL/PlatformDebug.h"
#include "HAL/PlatformDialog.h"
#include "HAL/PlatformProcess.h"
#include "IO/StringBuilder.h"
#include "IO/StringView.h"
#include "Memory/MemoryView.h"

namespace PPE {
LOG_CATEGORY(PPE_CORE_API, Assertion)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
static FPlatformDialog::EResult AssertIgnoreOnceAlwaysAbortRetry_(
    FPlatformDialog::EIcon icon,
    const FStringView& title, const char* msg,
    const char *file, unsigned line ) {
    FWStringBuilder oss;

    oss << title << Crlf
        << L"----------------------------------------------------------------" << Crlf
        << MakeCStringView(file) << L'(' << line << L"): " << MakeCStringView(msg);

    return FPlatformDialog::IgnoreOnceAlwaysAbortRetry(oss.ToString(), UTF_8_TO_WCHAR(title), icon);
}
//----------------------------------------------------------------------------
static bool ReportAssertionForDebug_(
    const FStringView& level,
    const char* msg, const char* file, unsigned line) {
#if USE_PPE_PLATFORM_DEBUG
    if (FCurrentProcess::StartedWithDebugger()) {
        char buf[1024];
        FFixedSizeTextWriter oss(buf);
        oss << Eol
            << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << Eol
            << " !!! " << level << " condition failed !!!" << Eol
            << "   text:  " << MakeCStringView(msg) << Eol
            << "   file:  " << MakeCStringView(file) << "(" << line << ")" << Eol
            << "  -> breaking the debugger" << Eol
            << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << Eol
            << Eos;
        FPlatformDebug::OutputDebug(buf);
        PPE_DEBUG_BREAK();
        return true;
    }
#else
    Unused(level);
    Unused(msg);
    Unused(file);
    Unused(line);
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
static bool PPE_DEBUG_SECTION DefaultDebugAssertHandler_(const char* msg, const char *file, unsigned line, bool isEnsure) {
#if USE_PPE_IGNORELIST
    FIgnoreList::FIgnoreKey ignoreKey;
    ignoreKey
        << (isEnsure ? MakeStringView("Ensure") : MakeStringView("AssertDebug"))
        << MakeCStringView(msg) << MakeCStringView(file) << MakePodView(line);
    if (FIgnoreList::HitIFP(ignoreKey) > 0)
        return false;
#endif

    if (not ReportAssertionForDebug_((isEnsure ? MakeStringView("ensure") : MakeStringView("debug assert")), msg, file, line)) {
        PPE_LOG(Assertion, Error, "{3} '{0}' failed !\n\t{1}({2})", MakeCStringView(msg), MakeCStringView(file), line,
            (isEnsure ? MakeStringView("ensure") : MakeStringView("debug assert")));
        PPE_LOG_FLUSH(); // flush log before continuing to get eventual log messages

        switch (AssertIgnoreOnceAlwaysAbortRetry_(FPlatformDialog::Warning,
            (isEnsure ? MakeStringView("Ensure failed !") : MakeStringView("Assert debug failed !")),
            msg, file, line)) {
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
static std::atomic<FAssertionHandler> GAssertionHandler{ &DefaultDebugAssertHandler_ };
static THREAD_LOCAL FAssertionHandler GAssertionHandlerForCurrentThread{ nullptr };
//----------------------------------------------------------------------------
static void PPE_DEBUG_SECTION DebugPredicateFailed_(const char* msg, const char *file, unsigned line, bool isEnsure) {
    // You can tweak those static variables when debugging:
    static volatile THREAD_LOCAL bool GIsInAssertion = false;
    static volatile THREAD_LOCAL bool GIgnoreAssertsInThisThread = false;
    static volatile std::atomic<bool> GIgnoreAllAsserts = ATOMIC_VAR_INIT(false);

    if (GIgnoreAllAsserts || GIgnoreAssertsInThisThread)
        return;

    if (GIsInAssertion)
        PPE_THROW_IT(FAssertException(isEnsure ? "Ensure reentrancy !" : "Assert reentrancy !", file, line));

    GIsInAssertion = true;

    bool failure = false;

    FAssertionHandler handler{ GAssertionHandlerForCurrentThread };
    if (Likely(nullptr == handler))
        handler = GAssertionHandler.load();

    if (Likely(!!handler))
        failure = (*handler)(msg, file, line, isEnsure);

    GIsInAssertion = false;

    if (failure && not isEnsure)
        PPE_THROW_IT(FAssertException("Assert debug failed !", file, line));
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FAssertException::FAssertException(const char *msg, const char *file, unsigned line)
:   FException(msg), _file(file), _line(line) {}
//----------------------------------------------------------------------------
FAssertException::~FAssertException() = default;
//----------------------------------------------------------------------------
#if USE_PPE_EXCEPTION_DESCRIPTION
FTextWriter& FAssertException::Description(FTextWriter& oss) const {
    return oss
        << "debug assert '" << MakeCStringView(What()) << "' failed !" << Eol
        << MakeCStringView(File()) << ':' << Line();
}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
NO_INLINE void PPE_DEBUG_SECTION AssertionFailed(const char* msg, const char *file, unsigned line) {
    DebugPredicateFailed_(msg, file, line, false);
}
//----------------------------------------------------------------------------
NO_INLINE void PPE_DEBUG_SECTION EnsureFailed(const char* msg, const char *file, unsigned line) {
    DebugPredicateFailed_(msg, file, line, true);
}
//----------------------------------------------------------------------------
FAssertionHandler SetAssertionHandler(FAssertionHandler handler) NOEXCEPT {
    FAssertionHandler previous = GAssertionHandler.load(std::memory_order_relaxed);
    for (i32 backoff = 0;;) {
        if (GAssertionHandler.compare_exchange_weak(previous, handler, std::memory_order_release, std::memory_order_relaxed))
            return previous;

        FPlatformProcess::SleepForSpinning(backoff);
    }
}
//----------------------------------------------------------------------------
FAssertionHandler SetAssertionHandlerForCurrentThread(FAssertionHandler handler) NOEXCEPT {
    std::swap(handler, GAssertionHandlerForCurrentThread);
    return handler;
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
static bool PPE_DEBUG_SECTION DefaultReleaseAssertHandler_(const char* msg, const char *file, unsigned line) {
#if USE_PPE_IGNORELIST
    FIgnoreList::FIgnoreKey ignoreKey;
    ignoreKey << MakeStringView("AssertRelease")
        << MakeCStringView(msg) << MakeCStringView(file) << MakePodConstView(line);
    if (FIgnoreList::HitIFP(ignoreKey) > 0)
        return false;
#endif

    if (not ReportAssertionForDebug_("release assert", msg, file, line)) {
        PPE_LOG(Assertion, Error, "release assert '{0}' failed !\n\t{1}({2})", MakeCStringView(msg), MakeCStringView(file), line);
        PPE_LOG_FLUSH(); // flush log before continuing to get eventual log messages

        switch (AssertIgnoreOnceAlwaysAbortRetry_(FPlatformDialog::Error, "Assert release failed !", msg, file, line)) {
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
static std::atomic<FReleaseAssertionHandler> GAssertionReleaseHandler{ &DefaultReleaseAssertHandler_ };
static THREAD_LOCAL FReleaseAssertionHandler GAssertionReleaseHandlerForCurrentThread{ nullptr };
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FAssertReleaseException::FAssertReleaseException(const char *msg, const char *file, unsigned line)
:   FException(msg), _file(file), _line(line) {}
//----------------------------------------------------------------------------
FAssertReleaseException::~FAssertReleaseException() = default;
//----------------------------------------------------------------------------
#if USE_PPE_EXCEPTION_DESCRIPTION
FTextWriter& FAssertReleaseException::Description(FTextWriter& oss) const {
    return oss
        << "release assert '" << MakeCStringView(What()) << "' failed !" << Eol
        << MakeCStringView(File()) << ':' << Line();
}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
NO_INLINE void PPE_DEBUG_SECTION AssertionReleaseFailed(const char* msg, const char *file, unsigned line) {
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

    FReleaseAssertionHandler handler{ GAssertionReleaseHandlerForCurrentThread };
    if (Likely(nullptr == handler))
        handler = GAssertionReleaseHandler.load();

    if (Likely(!!handler))
        failure = (*handler)(msg, file, line);

    GIsInAssertion = false;

    if (failure)
        PPE_THROW_IT(FAssertReleaseException("Assert release failed !", file, line));
}
//----------------------------------------------------------------------------
FReleaseAssertionHandler SetAssertionReleaseHandler(FReleaseAssertionHandler handler) NOEXCEPT {
    FReleaseAssertionHandler previous = GAssertionReleaseHandler.load(std::memory_order_relaxed);
    for (i32 backoff = 0;;) {
        if (GAssertionReleaseHandler.compare_exchange_weak(previous, handler, std::memory_order_release, std::memory_order_relaxed))
            return previous;

        FPlatformProcess::SleepForSpinning(backoff);
    }
}
//----------------------------------------------------------------------------
FReleaseAssertionHandler SetAssertionReleaseHandlerForCurrentThrad(FReleaseAssertionHandler handler) NOEXCEPT {
    std::swap(handler, GAssertionReleaseHandlerForCurrentThread);
    return handler;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
#endif //!USE_PPE_ASSERT_RELEASE
