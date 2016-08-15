#pragma once

#include "Core/Core.h"

#include "Core/IO/Format.h"
#include "Core/IO/StringSlice.h"

#include <iosfwd>

#if !defined(FINAL_RELEASE) && !defined(PROFILING_ENABLED)
#   define USE_DEBUG_LOGGER
#endif

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class LogCategory {
    Info = 0,
    Warning,
    Error,
    Exception,
    Debug,
    Assertion,
    Profiling,
    Callstack,
};
//----------------------------------------------------------------------------
MemoryView<const LogCategory> EachLogCategory();
const wchar_t* LogCategoryToWCStr(LogCategory category);
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits >
std::basic_ostream<_Char, _Traits>& operator <<(std::basic_ostream<_Char, _Traits>& oss, LogCategory category) {
    return oss << LogCategoryToWCStr(category);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ILogger {
public:
    virtual ~ILogger() {}
    virtual void Log(LogCategory category, const WStringSlice& format, const FormatArgListW& args) = 0;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#ifdef USE_DEBUG_LOGGER

#include "Core/IO/FormatHelpers.h"
#include "Core/IO/Stream.h"

#include <memory>
#include <sstream>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ILogger* SetLoggerImpl(ILogger* logger);
//----------------------------------------------------------------------------
void Log(LogCategory category, const WStringSlice& text);
//----------------------------------------------------------------------------
void LogArgs(LogCategory category, const WStringSlice& format, const FormatArgListW& args);
//----------------------------------------------------------------------------
template <typename _Arg0, typename... _Args>
void Log(LogCategory category, const WStringSlice& format, _Arg0&& arg0, _Args&&... args) {
    typedef details::_FormatFunctor<wchar_t> formatfunctor_t;
    const formatfunctor_t functors[] = {
        formatfunctor_t::Make(std::forward<_Arg0>(arg0)),
        formatfunctor_t::Make(std::forward<_Args>(args))...
    };

    LogArgs(category, format, FormatArgListW(functors));
}
//----------------------------------------------------------------------------
class LoggerStream : public ThreadLocalWOStringStream {
public:
    LoggerStream(LogCategory category) : _category(category) {}
    ~LoggerStream() { Log(_category, MakeStringSlice(str())); }

private:
    LogCategory _category;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class OutputDebugLogger : public ILogger {
public:
    virtual ~OutputDebugLogger() {}
    virtual void Log(LogCategory category, const WStringSlice& format, const FormatArgListW& args) override;
};
//----------------------------------------------------------------------------
class StdcoutLogger : public ILogger {
public:
    virtual ~StdcoutLogger() {}
    virtual void Log(LogCategory category, const WStringSlice& format, const FormatArgListW& args) override;
};
//----------------------------------------------------------------------------
class StderrLogger : public ILogger {
public:
    virtual ~StderrLogger() {}
    virtual void Log(LogCategory category, const WStringSlice& format, const FormatArgListW& args) override;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class LoggerStartup {
public:
    static void Start();
    static void Shutdown();

    LoggerStartup() { Start(); }
    ~LoggerStartup() { Shutdown(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#define LOG(_Category, ...) \
    Core::Log(Core::LogCategory::_Category, __VA_ARGS__)

#else

#define LOG(_Category, ...) NOOP

#endif //!#ifdef USE_DEBUG_LOGGER
