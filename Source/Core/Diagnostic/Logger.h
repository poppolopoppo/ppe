#pragma once

#include "Core/Core.h"

#include "Core/IO/Format.h"
#include "Core/IO/StringView.h"

#include <iosfwd>
#include <mutex>

#if !defined(FINAL_RELEASE)
#   define USE_DEBUG_LOGGER
#endif

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class ELogCategory {
    Info = 0,
    Emphasis,
    Warning,
    Error,
    Exception,
    Debug,
    Assertion,
    Profiling,
    Callstack,
};
//----------------------------------------------------------------------------
TMemoryView<const ELogCategory> EachLogCategory();
FWStringView LogCategoryToWCStr(ELogCategory category);
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits >
std::basic_ostream<_Char, _Traits>& operator <<(std::basic_ostream<_Char, _Traits>& oss, ELogCategory category) {
    return oss << LogCategoryToWCStr(category);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ILogger {
public:
    virtual ~ILogger() {}
    virtual void Log(ELogCategory category, const FWStringView& format, const FFormatArgListW& args) = 0;
    virtual void Flush() = 0;
};
//----------------------------------------------------------------------------
class FAbstractThreadSafeLogger : public ILogger {
public:
    virtual ~FAbstractThreadSafeLogger() {}
    virtual void Log(ELogCategory category, const FWStringView& format, const FFormatArgListW& args) override;
    virtual void Flush() override;
protected:
    virtual void LogThreadSafe(ELogCategory category, const FWStringView& format, const FFormatArgListW& args) = 0;
    virtual void FlushThreadSafe() = 0;
private:
    std::recursive_mutex _barrier;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#ifdef USE_DEBUG_LOGGER

#include "Core/IO/Stream.h"

#include <memory>
#include <sstream>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ILogger* SetLoggerImpl(ILogger* logger);
//----------------------------------------------------------------------------
void FlushLog();
//----------------------------------------------------------------------------
void Log(ELogCategory category, const FWStringView& text);
//----------------------------------------------------------------------------
void LogArgs(ELogCategory category, const FWStringView& format, const FFormatArgListW& args);
//----------------------------------------------------------------------------
template <typename _Arg0, typename... _Args>
void Log(ELogCategory category, const FWStringView& format, _Arg0&& arg0, _Args&&... args) {
    typedef details::TFormatFunctor_<wchar_t> formatfunctor_t;
    const formatfunctor_t functors[] = {
        formatfunctor_t::Make(std::forward<_Arg0>(arg0)),
        formatfunctor_t::Make(std::forward<_Args>(args))...
    };

    LogArgs(category, format, FFormatArgListW(functors));
}
//----------------------------------------------------------------------------
class FLoggerStream : public FThreadLocalWOStringStream {
public:
    FLoggerStream(ELogCategory category) : _category(category) {}
    ~FLoggerStream() { Log(_category, MakeStringView(str())); }

private:
    ELogCategory _category;
};
//----------------------------------------------------------------------------
class FStackLocalLoggerStream : public FWOCStrStream {
public:
    STATIC_CONST_INTEGRAL(size_t, Capacity, 2048);

    FStackLocalLoggerStream(ELogCategory category)
        : FWOCStrStream(_localBuffer), _category(category) {}
    ~FStackLocalLoggerStream() { Log(_category, FWOCStrStream::MakeView_NullTerminated()); }

private:
    ELogCategory _category;
    wchar_t _localBuffer[Capacity];
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FOutputDebugLogger : public FAbstractThreadSafeLogger {
protected:
    virtual void LogThreadSafe(ELogCategory category, const FWStringView& format, const FFormatArgListW& args) override;
    virtual void FlushThreadSafe() override;
};
//----------------------------------------------------------------------------
class FStdoutLogger : public FAbstractThreadSafeLogger {
protected:
    virtual void LogThreadSafe(ELogCategory category, const FWStringView& format, const FFormatArgListW& args) override;
    virtual void FlushThreadSafe() override;
};
//----------------------------------------------------------------------------
class FStderrLogger : public FAbstractThreadSafeLogger {
protected:
    virtual void LogThreadSafe(ELogCategory category, const FWStringView& format, const FFormatArgListW& args) override;
    virtual void FlushThreadSafe() override;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FLoggerStartup {
public:
    static void Start();
    static void Shutdown();

    FLoggerStartup() { Start(); }
    ~FLoggerStartup() { Shutdown(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#define LOG(_Category, ...) \
    ::Core::Log(Core::ELogCategory::_Category, __VA_ARGS__)
#define FLUSH_LOG() \
    ::Core::FlushLog()

#else

#define LOG(_Category, ...) NOOP()
#define FLUSH_LOG() NOOP()

#endif //!#ifdef USE_DEBUG_LOGGER
