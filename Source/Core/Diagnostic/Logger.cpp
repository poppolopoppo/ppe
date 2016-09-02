#include "stdafx.h"

#include "Logger.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
STATIC_ASSERT(0 == size_t(LogCategory::Info));
STATIC_ASSERT(1 == size_t(LogCategory::Warning));
STATIC_ASSERT(2 == size_t(LogCategory::Error));
STATIC_ASSERT(3 == size_t(LogCategory::Exception));
STATIC_ASSERT(4 == size_t(LogCategory::Debug));
STATIC_ASSERT(5 == size_t(LogCategory::Assertion));
STATIC_ASSERT(6 == size_t(LogCategory::Profiling));
STATIC_ASSERT(7 == size_t(LogCategory::Callstack));
//----------------------------------------------------------------------------
namespace {
    static const LogCategory sCategories[] = {
        LogCategory::Info,
        LogCategory::Warning,
        LogCategory::Error,
        LogCategory::Exception,
        LogCategory::Debug,
        LogCategory::Assertion,
        LogCategory::Profiling,
        LogCategory::Callstack,
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
MemoryView<const LogCategory> EachLogCategory() {
    return MakeView(sCategories);
}
//----------------------------------------------------------------------------
const wchar_t* LogCategoryToWCStr(LogCategory category) {
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
class BasicLogger_ : public ILogger {
public:
    BasicLogger_(const WStringView& prefix) : _prefix(prefix) {}

    // Used before main, no dependencies on allocators
    virtual void Log(LogCategory category, const WStringView& text, const FormatArgListW& args) override {
        wchar_t buffer[2048];
        WOCStrStream oss(buffer);
        if (LogCategory::Callstack != category &&
            LogCategory::Info != category ) {
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
    WStringView _prefix;
};
static const BasicLogger_ gLoggerBeforeMain (L"BEFORE_MAIN");
static const BasicLogger_ gLoggerAfterMain  (L"AFTER_MAIN");
//----------------------------------------------------------------------------
static AtomicSpinLock gLoggerSpinLock;
static std::atomic<ILogger*> gLoggerCurrentImpl = remove_const(&gLoggerBeforeMain);
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
ILogger* SetLoggerImpl(ILogger* logger) { // return previous handler
    Assert(logger);
    const AtomicSpinLock::Scope scopeLock(gLoggerSpinLock);
    return gLoggerCurrentImpl.exchange(logger);
}
//----------------------------------------------------------------------------
void Log(LogCategory category, const WStringView& text) {
    Assert(text.size());
    Assert('\0' == text.data()[text.size()]); // text must be null terminated !
    const AtomicSpinLock::Scope scopeLock(gLoggerSpinLock);
    gLoggerCurrentImpl.load()->Log(category, text, FormatArgListW());
}
//----------------------------------------------------------------------------
void LogArgs(LogCategory category, const WStringView& format, const FormatArgListW& args) {
    Assert(format.size());
    Assert('\0' == format.data()[format.size()]); // text must be null terminated !
    const AtomicSpinLock::Scope scopeLock(gLoggerSpinLock);
    gLoggerCurrentImpl.load()->Log(category, format, args);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void OutputDebugLogger::Log(LogCategory category, const WStringView& text, const FormatArgListW& args) {
    ThreadLocalWOStringStream oss;

#if 0
    if (LogCategory::Callstack != category) {
        Format(oss, L"[{0:12f7}][{1}]", CurrentProcess::ElapsedSeconds(), category);
    }
#else
    if (LogCategory::Callstack != category &&
        LogCategory::Info != category ) {
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
void StdcoutLogger::Log(LogCategory category, const WStringView& text, const FormatArgListW& args) {
    if (LogCategory::Callstack != category)
        Format(std::wcout, L"[{0:12f}][{1}]", CurrentProcess::ElapsedSeconds(), category);

    if (args.empty())
        std::wcout << text;
    else
        FormatArgs(std::wcout, text, args);

    std::wcout << std::endl;
}
//----------------------------------------------------------------------------
void StderrLogger::Log(LogCategory category, const WStringView& text, const FormatArgListW& args) {
    if (LogCategory::Callstack != category)
        Format(std::wcerr, L"[{0:12f}][{1}]", CurrentProcess::ElapsedSeconds(), category);

    if (args.empty())
        std::wcerr << text;
    else
        FormatArgs(std::wcerr, text, args);

    std::wcerr << std::endl;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void LoggerStartup::Start() {
    Assert(&gLoggerBeforeMain == gLoggerCurrentImpl.load());
    SetLoggerImpl(new OutputDebugLogger);
}
//----------------------------------------------------------------------------
void LoggerStartup::Shutdown() {
    ILogger* logger = SetLoggerImpl(remove_const(&gLoggerAfterMain));
    Assert(logger);
    Assert(logger != &gLoggerBeforeMain);
    checked_delete(logger);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#endif //!USE_DEBUG_LOGGER
