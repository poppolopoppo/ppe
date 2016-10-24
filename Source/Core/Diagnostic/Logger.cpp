#include "stdafx.h"

#include "Logger.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
STATIC_ASSERT(0 == size_t(ELogCategory::Info));
STATIC_ASSERT(1 == size_t(ELogCategory::Warning));
STATIC_ASSERT(2 == size_t(ELogCategory::Error));
STATIC_ASSERT(3 == size_t(ELogCategory::Exception));
STATIC_ASSERT(4 == size_t(ELogCategory::Debug));
STATIC_ASSERT(5 == size_t(ELogCategory::Assertion));
STATIC_ASSERT(6 == size_t(ELogCategory::Profiling));
STATIC_ASSERT(7 == size_t(ELogCategory::Callstack));
//----------------------------------------------------------------------------
namespace {
    static const ELogCategory sCategories[] = {
        ELogCategory::Info,
        ELogCategory::Warning,
        ELogCategory::Error,
        ELogCategory::Exception,
        ELogCategory::Debug,
        ELogCategory::Assertion,
        ELogCategory::Profiling,
        ELogCategory::Callstack,
    };

    static const wchar_t* sCategoriesWCStr[] = {
        L"Info",
        L"Warning",
        L"Error",
        L"Exception",
        L"Debug",
        L"Assertion",
        L"Profiling",
        L"Callstack",
    };

    STATIC_ASSERT(lengthof(sCategoriesWCStr) == lengthof(sCategories));
    STATIC_ASSERT(8 == lengthof(sCategories));
}
//----------------------------------------------------------------------------
TMemoryView<const ELogCategory> EachLogCategory() {
    return MakeView(sCategories);
}
//----------------------------------------------------------------------------
const wchar_t* LogCategoryToWCStr(ELogCategory category) {
    Assert(size_t(category) < lengthof(sCategoriesWCStr));
    return sCategoriesWCStr[size_t(category)];
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!naemspace Core

#ifdef USE_DEBUG_LOGGER

#include "Diagnostic/CurrentProcess.h"

#include "IO/Stream.h"
#include "IO/String.h"
#include "IO/StringView.h"
#include "Thread/AtomicSpinLock.h"

#include <iostream>
#include <sstream>

#ifdef CPP_VISUALSTUDIO
#   include <Windows.h>
#endif

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
class FBasicLogger_ : public ILogger {
public:
    FBasicLogger_(const FWStringView& prefix) : _prefix(prefix) {}

    // Used before main, no dependencies on allocators
    virtual void Log(ELogCategory category, const FWStringView& text, const FormatArgListW& args) override {
        wchar_t buffer[2048];
        FWOCStrStream oss(buffer);
        if (ELogCategory::Callstack != category &&
            ELogCategory::Info != category ) {
            Format(oss, L"[{0}][{1}]", _prefix, category);
        }
        else {
            Format(oss, L"[{0}]", _prefix);
        }
        FormatArgs(oss, text, args);
        oss << std::endl;
        OutputDebugStringW(oss.NullTerminatedStr());
    }

private:
    FWStringView _prefix;
};
static const FBasicLogger_ gLoggerBeforeMain (L"BEFORE_MAIN");
static const FBasicLogger_ gLoggerAfterMain  (L"AFTER_MAIN");
//----------------------------------------------------------------------------
static FAtomicSpinLock gLoggerSpinLock;
static std::atomic<ILogger*> gLoggerCurrentImpl = remove_const(&gLoggerBeforeMain);
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
ILogger* SetLoggerImpl(ILogger* logger) { // return previous handler
    Assert(logger);
    const FAtomicSpinLock::FScope scopeLock(gLoggerSpinLock);
    return gLoggerCurrentImpl.exchange(logger);
}
//----------------------------------------------------------------------------
void Log(ELogCategory category, const FWStringView& text) {
    Assert(text.size());
    Assert('\0' == text.data()[text.size()]); // text must be null terminated !
    const FAtomicSpinLock::FScope scopeLock(gLoggerSpinLock);
    gLoggerCurrentImpl.load()->Log(category, text, FormatArgListW());
}
//----------------------------------------------------------------------------
void LogArgs(ELogCategory category, const FWStringView& format, const FormatArgListW& args) {
    Assert(format.size());
    Assert('\0' == format.data()[format.size()]); // text must be null terminated !
    const FAtomicSpinLock::FScope scopeLock(gLoggerSpinLock);
    gLoggerCurrentImpl.load()->Log(category, format, args);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FOutputDebugLogger::Log(ELogCategory category, const FWStringView& text, const FormatArgListW& args) {
    FThreadLocalWOStringStream oss;

#if 0
    if (ELogCategory::Callstack != category) {
        Format(oss, L"[{0:12f7}][{1}]", FCurrentProcess::ElapsedSeconds(), category);
    }
#else
    if (ELogCategory::Callstack != category &&
        ELogCategory::Info != category ) {
        Format(oss, L"[{0}]", category);
    }
#endif

    if (args.empty())
        oss << text;
    else
        FormatArgs(oss, text, args);

    oss << std::endl;

    OutputDebugStringW(oss.str().c_str());
}
//----------------------------------------------------------------------------
void FStdcoutLogger::Log(ELogCategory category, const FWStringView& text, const FormatArgListW& args) {
    if (ELogCategory::Callstack != category)
        Format(std::wcout, L"[{0:12f}][{1}]", FCurrentProcess::ElapsedSeconds(), category);

    if (args.empty())
        std::wcout << text;
    else
        FormatArgs(std::wcout, text, args);

    std::wcout << std::endl;
}
//----------------------------------------------------------------------------
void FStderrLogger::Log(ELogCategory category, const FWStringView& text, const FormatArgListW& args) {
    if (ELogCategory::Callstack != category)
        Format(std::wcerr, L"[{0:12f}][{1}]", FCurrentProcess::ElapsedSeconds(), category);

    if (args.empty())
        std::wcerr << text;
    else
        FormatArgs(std::wcerr, text, args);

    std::wcerr << std::endl;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
static FOutputDebugLogger gLoggerDefault;
//----------------------------------------------------------------------------
void FLoggerStartup::Start() {
    Assert(&gLoggerBeforeMain == gLoggerCurrentImpl.load());
    SetLoggerImpl(&gLoggerDefault);
}
//----------------------------------------------------------------------------
void FLoggerStartup::Shutdown() {
    ILogger* logger = SetLoggerImpl(remove_const(&gLoggerAfterMain));
    Assert(logger);
    Assert(logger != &gLoggerBeforeMain);
    if (logger != &gLoggerDefault)
        checked_delete(logger);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#endif //!USE_DEBUG_LOGGER
