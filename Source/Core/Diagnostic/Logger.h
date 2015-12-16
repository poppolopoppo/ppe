#pragma once

#include "Core/Core.h"

#include "Core/Memory/MemoryView.h"

#include <iosfwd>

#ifndef FINAL_RELEASE
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
template <typename _Char, typename _Traits = std::char_traits<_Char> >
std::basic_ostream<_Char, _Traits>& operator <<(std::basic_ostream<_Char, _Traits>& oss, LogCategory category);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ILogger {
public:
    virtual ~ILogger() {}

    virtual void Log(LogCategory category, const wchar_t* text) = 0;

    template <typename _Arg0, typename... _Args>
    void Log(LogCategory category, const wchar_t* format, _Arg0&& arg0, _Args&&... args);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#ifdef USE_DEBUG_LOGGER

#include "Core/IO/FormatHelpers.h"
#include "Core/IO/Stream.h"
#include "Core/Memory/UniquePtr.h"
#include "Core/Meta/Singleton.h"

#include <memory>
#include <mutex>
#include <sstream>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class LoggerFrontend {
public:
    typedef std::chrono::steady_clock clock_type;

    LoggerFrontend();
    explicit LoggerFrontend(ILogger* impl);
    ~LoggerFrontend();

    ILogger* Impl() const { return _impl.get(); }
    void SetImpl(ILogger* impl);

    void Log(LogCategory category, const wchar_t* text);

    template <typename _Arg0, typename... _Args>
    void Log(LogCategory category, const wchar_t* format, _Arg0&& arg0, _Args&&... args);

private:
    mutable std::mutex _lock;
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
template <typename... _Args>
void Log(LogCategory category, const wchar_t* format, _Args&&... args);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class LoggerStream : public ThreadLocalWOStringStream {
public:
    LoggerStream(LogCategory category) : _category(category) {}
    ~LoggerStream() { Logger::Instance().Log(_category, str().c_str()); }

private:
    LogCategory _category;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class OutputDebugLogger : public ILogger {
public:
    virtual ~OutputDebugLogger() {}
    virtual void Log(LogCategory category, const wchar_t* text) override;
};
//----------------------------------------------------------------------------
class StdErrorLogger : public ILogger {
public:
    virtual ~StdErrorLogger() {}
    virtual void Log(LogCategory category, const wchar_t* text) override;
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

#endif //!#ifdef USE_DEBUG_LOGGER
