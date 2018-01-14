#include "stdafx.h"

#include "Logger.h"

#include "IO/BufferedStream.h"
#include "IO/FileStream.h"
#include "IO/StringView.h"
#include "IO/TextWriter.h"
#include "Memory/MemoryView.h"
#include "Misc/TargetPlatform.h"

#ifdef PLATFORM_WINDOWS
#   include "Misc/Platform_Windows.h"
#endif

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
STATIC_ASSERT(0 == size_t(ELogCategory::Info));
STATIC_ASSERT(1 == size_t(ELogCategory::Emphasis));
STATIC_ASSERT(2 == size_t(ELogCategory::Warning));
STATIC_ASSERT(3 == size_t(ELogCategory::Error));
STATIC_ASSERT(4 == size_t(ELogCategory::Exception));
STATIC_ASSERT(5 == size_t(ELogCategory::Debug));
STATIC_ASSERT(6 == size_t(ELogCategory::Assertion));
STATIC_ASSERT(7 == size_t(ELogCategory::Profiling));
STATIC_ASSERT(8 == size_t(ELogCategory::Callstack));
//----------------------------------------------------------------------------
namespace {
    static constexpr ELogCategory sCategories[] = {
        ELogCategory::Info,
        ELogCategory::Emphasis,
        ELogCategory::Warning,
        ELogCategory::Error,
        ELogCategory::Exception,
        ELogCategory::Debug,
        ELogCategory::Assertion,
        ELogCategory::Profiling,
        ELogCategory::Callstack,
    };

    STATIC_ASSERT(9 == lengthof(sCategories));
}
//----------------------------------------------------------------------------
TMemoryView<const ELogCategory> EachLogCategory() {
    return MakeView(sCategories);
}
//----------------------------------------------------------------------------
FStringView LogCategoryToCStr(ELogCategory category) {
    switch (category)
    {
    case Core::ELogCategory::Info:
        return "Info";
    case Core::ELogCategory::Emphasis:
        return "Emphasis";
    case Core::ELogCategory::Warning:
        return "Warning";
    case Core::ELogCategory::Error:
        return "Error";
    case Core::ELogCategory::Exception:
        return "Exception";
    case Core::ELogCategory::Debug:
        return "Debug";
    case Core::ELogCategory::Assertion:
        return "Assertion";
    case Core::ELogCategory::Profiling:
        return "Profiling";
    case Core::ELogCategory::Callstack:
        return "Callstack";
    }
    AssertNotImplemented();
    return FStringView();
}
//----------------------------------------------------------------------------
FWStringView LogCategoryToWCStr(ELogCategory category) {
    switch (category)
    {
    case Core::ELogCategory::Info:
        return L"Info";
    case Core::ELogCategory::Emphasis:
        return L"Emphasis";
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
FTextWriter& operator <<(FTextWriter& oss, ELogCategory category) {
    return oss << LogCategoryToCStr(category);
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, ELogCategory category) {
    return oss << LogCategoryToWCStr(category);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#ifdef USE_DEBUG_LOGGER

#include "Diagnostic/Callstack.h"
#include "Diagnostic/CurrentProcess.h"
#include "Diagnostic/DecodedCallstack.h"

#include "IO/StreamProvider.h"
#include "IO/String.h"
#include "IO/StringView.h"
#include "Memory/MemoryProvider.h"
#include "Thread/AtomicSpinLock.h"

#define CORE_DUMP_CALLSTACK_ON_WARNING  0
#define CORE_DUMP_CALLSTACK_ON_ERROR    0

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
#if CORE_DUMP_CALLSTACK_ON_WARNING || CORE_DUMP_CALLSTACK_ON_ERROR
template <typename _Char>
static void DumpThreadCallstack_(TBasicTextWriter<_Char>& oss) {
    FCallstack callstack;
    FCallstack::Capture(&callstack, 4);

    if (not callstack.Depth())
        return;

    FDecodedCallstack decoded;
    if (not callstack.Decode(&decoded))
        return;

    oss << decoded;
}
#endif
//----------------------------------------------------------------------------
class FBasicLogger_ : public ILogger {
public:
    FBasicLogger_(const FWStringView& prefix)
        : _prefix(prefix)
    {}

    // Used before main, no dependencies on allocators
    virtual void Log(ELogCategory category, const FWStringView& text, const FWFormatArgList& args) override {
        if (not FPlatformMisc::IsDebuggerAttached())
            return; // because we are using OutputDebug()

        wchar_t buffer[2048];
        FWFixedSizeTextWriter oss(buffer);

        if (ELogCategory::Callstack != category &&
            ELogCategory::Info != category ) {
            Format(oss, L"[{0}][{1}]", _prefix, category);
        }
        else {
            Format(oss, L"[{0}]", _prefix);
        }
        FormatArgs(oss, text, args);

        oss << Eol << Eos;

        FPlatformMisc::OutputDebug(buffer);
    }

    virtual void Flush() override {}

private:
    FWStringView _prefix;
};
//----------------------------------------------------------------------------
static ILogger* LowLevelLogger_() {
    static FBasicLogger_ GLoggerBasic(L"LOWLEVEL");
    return (&GLoggerBasic);
}
//----------------------------------------------------------------------------
static ILogger* GLoggerCurrentImpl = nullptr;
static ILogger* CurrentLogger_() {
    if (nullptr == GLoggerCurrentImpl)
        GLoggerCurrentImpl = LowLevelLogger_();
    return GLoggerCurrentImpl;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
ILogger* SetLoggerImpl(ILogger* logger) { // return previous handler
    Assert(logger);
    ILogger* previous = GLoggerCurrentImpl;
    GLoggerCurrentImpl = logger;
    return previous;
}
//----------------------------------------------------------------------------
void FlushLog() {
    CurrentLogger_()->Flush();
}
//----------------------------------------------------------------------------
void Log(ELogCategory category, const FWStringView& text) {
    if (not text.empty()) {
        Assert('\0' == text.data()[text.size()]); // text must be null terminated !
        CurrentLogger_()->Log(category, text, FWFormatArgList());
    }
}
//----------------------------------------------------------------------------
void LogArgs(ELogCategory category, const FWStringView& format, const FWFormatArgList& args) {
    Assert(format.size());
    Assert('\0' == format.data()[format.size()]); // text must be null terminated !
    CurrentLogger_()->Log(category, format, args);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FAbstractThreadSafeLogger::Log(ELogCategory category, const FWStringView& format, const FWFormatArgList& args) {
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
void FOutputDebugLogger::LogThreadSafe(ELogCategory category, const FWStringView& text, const FWFormatArgList& args) {
    if (ELogCategory::Emphasis == category)
        FPlatformMisc::OutputDebug(L"------------------------------------------------------------------------------\n");

    wchar_t buffer[2048];
    FWFixedSizeTextWriter oss(buffer);

#if 0
    if (ELogCategory::Callstack != category) {
        Format(oss, L"[{0:12f7}][{1}]", FCurrentProcess::ElapsedSeconds(), category);
    }
#else
    if (ELogCategory::Callstack != category &&
        ELogCategory::Emphasis != category &&
        ELogCategory::Info != category) {
        Format(oss, L"[{0}]", category);
    }
#endif

    if (text.size() + oss.Written().size() + 16 * args.size() < lengthof(buffer)) {
        // still remaining place in stack buffer
        if (args.empty()) {
            oss << text;
        }
        else {
            FormatArgs(oss, text, args);
        }

        oss << Eol << Eos;

        FPlatformMisc::OutputDebug(buffer);
    }
    else {
        // different strategy for large logs, assuming no args is given for them
        AssertRelease(args.empty()); // TODO : handle splitting formatted texts

        oss << Eos;

        FPlatformMisc::OutputDebug(buffer);

        FWStringView input = text;
        do {
            const size_t amount = Min(2047ul, input.size());
            const FWStringView print = input.CutBefore(amount);
            input = input.CutStartingAt(amount);

            ::memcpy(buffer, print.data(), print.SizeInBytes());
            buffer[amount] = L'\0';

            FPlatformMisc::OutputDebug(buffer);

        } while (not input.empty());
    }
}
//----------------------------------------------------------------------------
void FOutputDebugLogger::FlushThreadSafe() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FStdoutLogger::LogThreadSafe(ELogCategory category, const FWStringView& text, const FWFormatArgList& args) {
#ifdef PLATFORM_WINDOWS
    constexpr ::WORD fWhite = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    constexpr ::WORD fYellow = FOREGROUND_RED | FOREGROUND_GREEN;
    constexpr ::WORD fCyan = FOREGROUND_GREEN | FOREGROUND_BLUE;
    constexpr ::WORD bBlack = 0;

    static ::WORD GPreviousTextAttribute = ::WORD(-1);

    ::WORD textAttribute;
    switch (category)
    {
    case Core::ELogCategory::Info:
        textAttribute = (fWhite | bBlack);
        break;
    case Core::ELogCategory::Emphasis:
        textAttribute = (FOREGROUND_GREEN | BACKGROUND_BLUE | FOREGROUND_INTENSITY);
        break;
    case Core::ELogCategory::Warning:
        textAttribute = (fYellow | bBlack | FOREGROUND_INTENSITY);
        break;
    case Core::ELogCategory::Error:
        textAttribute = (FOREGROUND_RED | bBlack);
        break;
    case Core::ELogCategory::Exception:
        textAttribute = (fWhite | BACKGROUND_RED | BACKGROUND_INTENSITY);
        break;
    case Core::ELogCategory::Debug:
        textAttribute = (fCyan | bBlack | FOREGROUND_INTENSITY);
        break;
    case Core::ELogCategory::Assertion:
        textAttribute = (fYellow | BACKGROUND_RED | FOREGROUND_INTENSITY);
        break;
    case Core::ELogCategory::Profiling:
        textAttribute = (fCyan | bBlack);
        break;
    case Core::ELogCategory::Callstack:
        textAttribute = (FOREGROUND_GREEN);
    default:
        textAttribute = (fWhite | bBlack);
        break;
    }

    if (GPreviousTextAttribute != textAttribute) {
        if (::HANDLE hConsole = ::GetStdHandle(STD_OUTPUT_HANDLE)) {
            GPreviousTextAttribute = textAttribute;
            ::SetConsoleTextAttribute(hConsole, textAttribute);
        }
        else {
            AssertNotReached();
            GPreviousTextAttribute = ::WORD(-1);
        }
    }
#endif

    auto oss = GWStdout;

    if (ELogCategory::Callstack != category &&
        ELogCategory::Emphasis != category )
        Format(oss, L"[{0:12f}][{1}]", FCurrentProcess::ElapsedSeconds(), category);

    if (args.empty())
        oss << text;
    else
        FormatArgs(oss, text, args);

    oss << Eos;

#if CORE_DUMP_CALLSTACK_ON_WARNING
    if (ELogCategory::Warning == category)
        DumpThreadCallstack_(oss);
#endif
#if CORE_DUMP_CALLSTACK_ON_ERROR
    if (ELogCategory::Error == category)
        DumpThreadCallstack_(oss);
#endif
}
//----------------------------------------------------------------------------
void FStdoutLogger::FlushThreadSafe() {
    GWStdout.Flush();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FStderrLogger::LogThreadSafe(ELogCategory category, const FWStringView& text, const FWFormatArgList& args) {
    auto oss = GWStderr;

    if (ELogCategory::Callstack != category)
        Format(oss, L"[{0:12f}][{1}]", FCurrentProcess::ElapsedSeconds(), category);

    if (args.empty())
        oss << text;
    else
        FormatArgs(oss, text, args);

    oss << Eol;
}
//----------------------------------------------------------------------------
void FStderrLogger::FlushThreadSafe() {
    GWStderr.Flush();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FStreamLogger::FStreamLogger(class IBufferedStreamWriter* stream)
    : _stream(stream) {
    Assert(_stream);
}
//----------------------------------------------------------------------------
FStreamLogger::~FStreamLogger() {
    _stream->Flush();
}
//----------------------------------------------------------------------------
void FStreamLogger::LogThreadSafe(ELogCategory category, const FWStringView& text, const FWFormatArgList& args) {
    FWTextWriter oss(_stream);

    if (ELogCategory::Callstack != category)
        Format(oss, L"[{0:12f}][{1}]", FCurrentProcess::ElapsedSeconds(), category);

    if (args.empty())
        oss << text;
    else
        FormatArgs(oss, text, args);

    oss << Eol;
}
//----------------------------------------------------------------------------
void FStreamLogger::FlushThreadSafe() {
    _stream->Flush();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
STATIC_ASSERT(sizeof(FAbstractThreadSafeLogger) == sizeof(FOutputDebugLogger));
STATIC_ASSERT(sizeof(FAbstractThreadSafeLogger) == sizeof(FStdoutLogger));
STATIC_ASSERT(sizeof(FAbstractThreadSafeLogger) == sizeof(FStderrLogger));
//----------------------------------------------------------------------------
static POD_STORAGE(FAbstractThreadSafeLogger) GLoggerDefaultStorage;
//----------------------------------------------------------------------------
void FLoggerStartup::Start() {
    Assert(LowLevelLogger_() == CurrentLogger_());

    void* const storage = (void*)std::addressof(GLoggerDefaultStorage);

    ILogger* const logger = (FCurrentProcess::Instance().StartedWithDebugger())
        ? (ILogger*)new (storage) FOutputDebugLogger()
        : (ILogger*)new (storage) FStdoutLogger();

    Assert(logger == (ILogger*)&GLoggerDefaultStorage);

    SetLoggerImpl(logger);
}
//----------------------------------------------------------------------------
void FLoggerStartup::Shutdown() {
    ILogger* logger = SetLoggerImpl(LowLevelLogger_());

    Assert(logger);
    Assert(logger != LowLevelLogger_());

    if (logger == (ILogger*)&GLoggerDefaultStorage)
        logger->~ILogger();
    else
        checked_delete(logger);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#endif //!USE_DEBUG_LOGGER
