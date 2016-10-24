#pragma once

#include "Core/Core.h"

#include "Core/IO/Format.h"
#include "Core/IO/StringView.h"

#include <iosfwd>

#if !defined(FINAL_RELEASE) && !defined(PROFILING_ENABLED)
#   define USE_DEBUG_LOGGER
#endif

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class ELogCategory {
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
TMemoryView<const ELogCategory> EachLogCategory();
const wchar_t* LogCategoryToWCStr(ELogCategory category);
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
    virtual void Log(ELogCategory category, const FWStringView& format, const FormatArgListW& args) = 0;
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
void Log(ELogCategory category, const FWStringView& text);
//----------------------------------------------------------------------------
void LogArgs(ELogCategory category, const FWStringView& format, const FormatArgListW& args);
//----------------------------------------------------------------------------
template <typename _Arg0, typename... _Args>
void Log(ELogCategory category, const FWStringView& format, _Arg0&& arg0, _Args&&... args) {
    typedef details::_FormatFunctor<wchar_t> formatfunctor_t;
    const formatfunctor_t functors[] = {
        formatfunctor_t::Make(std::forward<_Arg0>(arg0)),
        formatfunctor_t::Make(std::forward<_Args>(args))...
    };

    LogArgs(category, format, FormatArgListW(functors));
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
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FOutputDebugLogger : public ILogger {
public:
    virtual ~FOutputDebugLogger() {}
    virtual void Log(ELogCategory category, const FWStringView& format, const FormatArgListW& args) override;
};
//----------------------------------------------------------------------------
class FStdcoutLogger : public ILogger {
public:
    virtual ~FStdcoutLogger() {}
    virtual void Log(ELogCategory category, const FWStringView& format, const FormatArgListW& args) override;
};
//----------------------------------------------------------------------------
class FStderrLogger : public ILogger {
public:
    virtual ~FStderrLogger() {}
    virtual void Log(ELogCategory category, const FWStringView& format, const FormatArgListW& args) override;
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
    Core::Log(Core::ELogCategory::_Category, __VA_ARGS__)

#else

#define LOG(_Category, ...) NOOP

#endif //!#ifdef USE_DEBUG_LOGGER
