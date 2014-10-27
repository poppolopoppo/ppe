#pragma once

#include "Core/Core.h"

#ifndef _FINAL
#   define USE_LOGGER
#endif

#ifdef USE_LOGGER

#include "Core/Memory/MemoryView.h"
#include "Core/Memory/UniquePtr.h"
#include "Core/Meta/Singleton.h"

#include <chrono>
#include <iosfwd>
#include <memory>
#include <mutex>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class LogCategory {
    Information = 0,
    Warning,
    Error,
    Exception,
    Assertion,
    Profiling,
    Callstack,
};
//----------------------------------------------------------------------------
MemoryView<const LogCategory> EachLogCategory();
const wchar_t* LogCategoryToWCStr(LogCategory category);
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits = std::char_traits<_Char> >
std::basic_ostream<_Char, _Traits>& operator <<(std::basic_ostream<_Char, _Traits>& oss, LogCategory category);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ILogger;
//----------------------------------------------------------------------------
class LoggerFrontend {
public:
    typedef std::chrono::steady_clock clock_type;

    LoggerFrontend();
    explicit LoggerFrontend(ILogger* impl);
    ~LoggerFrontend();

    ILogger* Impl() const { return _impl.get(); }
    void SetImpl(ILogger* impl);

    const clock_type::time_point& StartedAt() const { return _startedAt; }

    double Now() const {
        return std::chrono::duration_cast<std::chrono::duration<double>>(clock_type::now() - _startedAt).count();
    }

    void Log(LogCategory category, const wchar_t* text);

    template <typename _Arg0, typename... _Args>
    void LogFormat(LogCategory category, const wchar_t* format, _Arg0&& arg0, _Args&&... args);

private:
    mutable std::mutex _lock;
    clock_type::time_point _startedAt;
    UniquePtr<ILogger> _impl;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class Logger : Meta::Singleton<LoggerFrontend, Logger> {
    typedef Meta::Singleton<LoggerFrontend, Logger> parent_type;
public:
    using parent_type::Instance;
    using parent_type::HasInstance;
    using parent_type::Destroy;

    static void Create(ILogger* impl = nullptr) {
        parent_type::Create(impl);
    }
};
//----------------------------------------------------------------------------
void Log(LogCategory category, const wchar_t* text);
//----------------------------------------------------------------------------
template <typename _Arg0, typename... _Args>
void Log(LogCategory category, const wchar_t* format, _Arg0&& arg0, _Args&&... args);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ILogger {
public:
    virtual ~ILogger() {}
    virtual void Log(const LoggerFrontend& frontend, LogCategory category, const wchar_t* text, size_t length) = 0;
};
//----------------------------------------------------------------------------
class OutputDebugLogger : public ILogger {
public:
    virtual ~OutputDebugLogger() {}
    virtual void Log(const LoggerFrontend& frontend, LogCategory category, const wchar_t* text, size_t length) override;
};
//----------------------------------------------------------------------------
class StdErrorLogger : public ILogger {
public:
    virtual ~StdErrorLogger() {}
    virtual void Log(const LoggerFrontend& frontend, LogCategory category, const wchar_t* text, size_t length) override;
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

#include "Core/Diagnostic/Logger-inl.h"

#define LOG(_Category, ...) Core::Log(Core::LogCategory::_Category, __VA_ARGS__)

#else

#define LOG(_Category, ...) NOOP

#endif //!#ifdef USE_LOGGER
