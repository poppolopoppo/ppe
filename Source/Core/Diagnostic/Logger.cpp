#include "stdafx.h"

#include "Logger.h"

#include "Misc/TargetPlatform.h"

#include <fcntl.h>
#include <io.h>

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
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#ifdef USE_DEBUG_LOGGER

#include "Diagnostic/Callstack.h"
#include "Diagnostic/CurrentProcess.h"
#include "Diagnostic/DecodedCallstack.h"

#include "IO/Stream.h"
#include "IO/String.h"
#include "IO/StringView.h"
#include "Thread/AtomicSpinLock.h"

#include <iostream>
#include <sstream>

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
static void DumpThreadCallstack_(std::basic_ostream<_Char>& oss) {
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
    virtual void Log(ELogCategory category, const FWStringView& text, const FFormatArgListW& args) override {
        if (not FPlatform::IsDebuggerAttached())
            return; // because we are using OutputDebug()

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
        AssertRelease(!oss.bad());

        FPlatform::OutputDebug(oss.NullTerminatedStr());
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
        CurrentLogger_()->Log(category, text, FFormatArgListW());
    }
}
//----------------------------------------------------------------------------
void LogArgs(ELogCategory category, const FWStringView& format, const FFormatArgListW& args) {
    Assert(format.size());
    Assert('\0' == format.data()[format.size()]); // text must be null terminated !
    CurrentLogger_()->Log(category, format, args);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FAbstractThreadSafeLogger::Log(ELogCategory category, const FWStringView& format, const FFormatArgListW& args) {
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
void FOutputDebugLogger::LogThreadSafe(ELogCategory category, const FWStringView& text, const FFormatArgListW& args) {
    if (ELogCategory::Emphasis == category)
        FPlatform::OutputDebug(L"------------------------------------------------------------------------------\n");

    wchar_t buffer[2048];
    FWOCStrStream oss(buffer);

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

    if (text.size() + oss.size() + 16 * args.size() < 2048) {
        // still remaining place in stack buffer
        if (args.empty()) {
            oss << text;
        }
        else {
            FormatArgs(oss, text, args);
        }

        oss << eol;

        FPlatform::OutputDebug(oss.NullTerminatedStr());
    }
    else {
        // different strategy for large logs, assuming no args is given for them
        AssertRelease(args.empty()); // TODO : handle splitting formatted texts

        FPlatform::OutputDebug(oss.NullTerminatedStr());

        FWStringView input = text;
        do {
            const size_t amount = Min(2047ul, input.size());
            const FWStringView print = input.CutBefore(amount);
            input = input.CutStartingAt(amount);

            ::memcpy(buffer, print.data(), print.SizeInBytes());
            buffer[amount] = L'\0';

            FPlatform::OutputDebug(buffer);

        } while (not input.empty());
    }
}
//----------------------------------------------------------------------------
void FOutputDebugLogger::FlushThreadSafe() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FStdoutLogger::LogThreadSafe(ELogCategory category, const FWStringView& text, const FFormatArgListW& args) {
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
            GPreviousTextAttribute = ::WORD(-1);
        }
    }
#endif

    if (ELogCategory::Callstack != category &&
        ELogCategory::Emphasis != category )
        Format(std::wcout, L"[{0:12f}][{1}]", FCurrentProcess::ElapsedSeconds(), category);

    if (args.empty())
        std::wcout << text;
    else
        FormatArgs(std::wcout, text, args);
    Assert(!std::wcout.bad());

    std::wcout << eol;

#if CORE_DUMP_CALLSTACK_ON_WARNING
    if (ELogCategory::Warning == category)
        DumpThreadCallstack_(std::wcout);
#endif
#if CORE_DUMP_CALLSTACK_ON_ERROR
    if (ELogCategory::Error == category)
        DumpThreadCallstack_(std::wcout);
#endif
}
//----------------------------------------------------------------------------
void FStdoutLogger::FlushThreadSafe() {
    std::wcout.flush();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FStderrLogger::LogThreadSafe(ELogCategory category, const FWStringView& text, const FFormatArgListW& args) {
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
#ifdef PLATFORM_WINDOWS
class FConsoleWriterW : public std::basic_streambuf<wchar_t> {
public:
    explicit FConsoleWriterW(::HANDLE out) : _out(out) {}

    virtual int_type overflow(int_type c = traits_type::eof()) {
        ::DWORD written;
        const wchar_t wch = wchar_t(c);
        const ::BOOL res = ::WriteConsoleW(_out, &wch, 1, &written, NULL);
        (void)res;
        return 1;
    }

    static FConsoleWriterW& Stdout() {
        static FConsoleWriterW GInstance(::GetStdHandle(STD_OUTPUT_HANDLE));
        return GInstance;
    }

private:
    ::HANDLE _out;
};
std::basic_streambuf<wchar_t>* GStdWCoutOriginalReadBuffer = nullptr;
#endif
//----------------------------------------------------------------------------
STATIC_ASSERT(sizeof(FAbstractThreadSafeLogger) == sizeof(FOutputDebugLogger));
STATIC_ASSERT(sizeof(FAbstractThreadSafeLogger) == sizeof(FStdoutLogger));
STATIC_ASSERT(sizeof(FAbstractThreadSafeLogger) == sizeof(FStderrLogger));
//----------------------------------------------------------------------------
static POD_STORAGE(FAbstractThreadSafeLogger) GLoggerDefaultStorage;
//----------------------------------------------------------------------------
void FLoggerStartup::Start() {
    Assert(LowLevelLogger_() == CurrentLogger_());

    // set locale to UTF-8
    //_setmode(_fileno(stdout), _O_U16TEXT);
    //_setmode(_fileno(stderr), _O_U16TEXT);

    // don't buffer output
    if (::setvbuf(stdout, nullptr, _IONBF, 0)) AssertNotReached();
    if (::setvbuf(stderr, nullptr, _IONBF, 0)) AssertNotReached();
    /*
    std::cout .setf(std::ios::unitbuf);
    std::wcout.setf(std::ios::unitbuf);
    std::cerr .setf(std::ios::unitbuf);
    std::wcerr.setf(std::ios::unitbuf);
    */

    // workaround for windows and unicode
#if PLATFORM_WINDOWS
    Assert(nullptr == GStdWCoutOriginalReadBuffer);
    GStdWCoutOriginalReadBuffer = std::wcout.rdbuf(&FConsoleWriterW::Stdout());
#endif

    // throw exceptions instead of silently failing
    std::cout .exceptions(std::ostream::failbit  | std::ostream::badbit );
    std::wcout.exceptions(std::wostream::failbit | std::wostream::badbit);
    std::cerr .exceptions(std::ostream::failbit  | std::ostream::badbit );
    std::wcerr.exceptions(std::wostream::failbit | std::wostream::badbit);
    std::cin  .exceptions(std::istream::failbit  | std::istream::badbit );
    std::wcin .exceptions(std::wistream::failbit | std::wistream::badbit);

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

    // workaround for windows and unicode
#if PLATFORM_WINDOWS
    Assert(GStdWCoutOriginalReadBuffer);
    std::wcout.rdbuf(GStdWCoutOriginalReadBuffer);
    GStdWCoutOriginalReadBuffer = nullptr;
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#endif //!USE_DEBUG_LOGGER
