#include "stdafx.h"

#include "Logger.h"
#include "Misc/TargetPlatform.h"

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
    static constexpr ELogCategory sCategories[] = {
        ELogCategory::Info,
        ELogCategory::Warning,
        ELogCategory::Error,
        ELogCategory::Exception,
        ELogCategory::Debug,
        ELogCategory::Assertion,
        ELogCategory::Profiling,
        ELogCategory::Callstack,
    };

    STATIC_ASSERT(8 == lengthof(sCategories));
}
//----------------------------------------------------------------------------
TMemoryView<const ELogCategory> EachLogCategory() {
    return MakeView(sCategories);
}
//----------------------------------------------------------------------------
FWStringView LogCategoryToWCStr(ELogCategory category) {
    switch (category)
    {
    case Core::ELogCategory::Info:
        return L"Info";
    case Core::ELogCategory::Warning:
        return L"Warning";
    case Core::ELogCategory::Error:
        return L"Error";
    case Core::ELogCategory::Exception:
        return L"Exception";
    case Core::ELogCategory::Debug:
        return L"Debug";
    case Core::ELogCategory::Assertion:
        return L"Assertion";
    case Core::ELogCategory::Profiling:
        return L"Profiling";
    case Core::ELogCategory::Callstack:
        return L"Callstack";
    }
    AssertNotImplemented();
    return FWStringView();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#ifdef USE_DEBUG_LOGGER

#include "Diagnostic/CurrentProcess.h"

#include "IO/Stream.h"
#include "IO/String.h"
#include "IO/StringView.h"
#include "Thread/AtomicSpinLock.h"

#include <iostream>
#include <sstream>

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
        oss << eol;
        FPlatform::OutputDebug(oss.NullTerminatedStr());
    }

    virtual void Flush() override {}

private:
    FWStringView _prefix;
};
static const FBasicLogger_ gLoggerBeforeMain (L"BEFORE_MAIN");
static const FBasicLogger_ gLoggerAfterMain  (L"AFTER_MAIN");
//----------------------------------------------------------------------------
static std::atomic<ILogger*> gLoggerCurrentImpl(remove_const(&gLoggerBeforeMain));
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
ILogger* SetLoggerImpl(ILogger* logger) { // return previous handler
    Assert(logger);
    return gLoggerCurrentImpl.exchange(logger);
}
//----------------------------------------------------------------------------
void FlushLog() {
    gLoggerCurrentImpl.load()->Flush();
}
//----------------------------------------------------------------------------
void Log(ELogCategory category, const FWStringView& text) {
    Assert(text.size());
    Assert('\0' == text.data()[text.size()]); // text must be null terminated !
    gLoggerCurrentImpl.load()->Log(category, text, FormatArgListW());
}
//----------------------------------------------------------------------------
void LogArgs(ELogCategory category, const FWStringView& format, const FormatArgListW& args) {
    Assert(format.size());
    Assert('\0' == format.data()[format.size()]); // text must be null terminated !
    gLoggerCurrentImpl.load()->Log(category, format, args);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FAbstractThreadSafeLogger::Log(ELogCategory category, const FWStringView& format, const FormatArgListW& args) {
    const std::lock_guard<std::recursive_mutex> scopeLock(_barrier);
    LogThreadSafe(category, format, args);
}
//----------------------------------------------------------------------------
void FAbstractThreadSafeLogger::Flush() {
    const std::lock_guard<std::recursive_mutex> scopeLock(_barrier);
    FlushThreadSafe();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FOutputDebugLogger::LogThreadSafe(ELogCategory category, const FWStringView& text, const FormatArgListW& args) {
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

    oss << eol;

    FPlatform::OutputDebug(oss.str().c_str());
}
//----------------------------------------------------------------------------
void FOutputDebugLogger::FlushThreadSafe() {}
//----------------------------------------------------------------------------
void FStdoutLogger::LogThreadSafe(ELogCategory category, const FWStringView& text, const FormatArgListW& args) {
    if (ELogCategory::Callstack != category)
        Format(std::wcout, L"[{0:12f}][{1}]", FCurrentProcess::ElapsedSeconds(), category);

    if (args.empty())
        std::wcout << text;
    else
        FormatArgs(std::wcout, text, args);
    Assert(!std::wcout.bad());

    std::wcout << eol;
}
//----------------------------------------------------------------------------
void FStdoutLogger::FlushThreadSafe() {
    std::wcout.flush();
}
//----------------------------------------------------------------------------
void FStderrLogger::LogThreadSafe(ELogCategory category, const FWStringView& text, const FormatArgListW& args) {
    if (ELogCategory::Callstack != category)
        Format(std::wcerr, L"[{0:12f}][{1}]", FCurrentProcess::ElapsedSeconds(), category);

    if (args.empty())
        std::wcerr << text;
    else
        FormatArgs(std::wcerr, text, args);
    Assert(!std::wcerr.bad());

    std::wcerr << eol;
}
//----------------------------------------------------------------------------
void FStderrLogger::FlushThreadSafe() {
    std::wcerr.flush(); // shouldn't be necessary for cerr which is unbuffered by default, but just in case ...
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
STATIC_ASSERT(sizeof(FAbstractThreadSafeLogger) == sizeof(FOutputDebugLogger));
STATIC_ASSERT(sizeof(FAbstractThreadSafeLogger) == sizeof(FStdoutLogger));
STATIC_ASSERT(sizeof(FAbstractThreadSafeLogger) == sizeof(FStderrLogger));
static POD_STORAGE(FAbstractThreadSafeLogger) gLoggerDefaultStorage;
//----------------------------------------------------------------------------
void FLoggerStartup::Start() {
    Assert(&gLoggerBeforeMain == gLoggerCurrentImpl.load());

    ILogger* logger = nullptr;

    if (FPlatform::IsDebuggerAttached())
        logger = new ((void*)&gLoggerDefaultStorage) FOutputDebugLogger();
    else
        logger = new ((void*)&gLoggerDefaultStorage) FStdoutLogger();

    Assert(logger == (ILogger*)&gLoggerDefaultStorage);

    SetLoggerImpl(logger);
}
//----------------------------------------------------------------------------
void FLoggerStartup::Shutdown() {
    ILogger* logger = SetLoggerImpl(remove_const(&gLoggerAfterMain));

    Assert(logger);
    Assert(logger != &gLoggerBeforeMain);

    if (logger == (ILogger*)&gLoggerDefaultStorage)
        logger->~ILogger();
    else
        checked_delete(logger);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#endif //!USE_DEBUG_LOGGER
