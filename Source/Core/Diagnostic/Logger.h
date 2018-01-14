#pragma once

#include "Core/Core.h"

#include "Core/IO/Format.h"
#include "Core/IO/String_fwd.h"
#include "Core/IO/TextWriter_fwd.h"

#if !defined(FINAL_RELEASE) || USE_CORE_FORCE_LOGGING
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
CORE_API TMemoryView<const ELogCategory> EachLogCategory();
CORE_API FStringView LogCategoryToCStr(ELogCategory category);
CORE_API FWStringView LogCategoryToWCStr(ELogCategory category);
//----------------------------------------------------------------------------
CORE_API FTextWriter& operator <<(FTextWriter& oss, ELogCategory category);
CORE_API FWTextWriter& operator <<(FWTextWriter& oss, ELogCategory category);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ILogger {
public:
    virtual ~ILogger() {}
    virtual void Log(ELogCategory category, const FWStringView& format, const FWFormatArgList& args) = 0;
    virtual void Flush() = 0;
};
//----------------------------------------------------------------------------
class CORE_API FAbstractThreadSafeLogger : public ILogger {
public:
    virtual ~FAbstractThreadSafeLogger() {}
    virtual void Log(ELogCategory category, const FWStringView& format, const FWFormatArgList& args) override;
    virtual void Flush() override;
protected:
    virtual void LogThreadSafe(ELogCategory category, const FWStringView& format, const FWFormatArgList& args) = 0;
    virtual void FlushThreadSafe() = 0;
private:
    std::recursive_mutex _barrier;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#ifdef USE_DEBUG_LOGGER

#include "Core/IO/TextWriter.h"
#include "Core/Memory/MemoryStream.h"

#include <memory>
#include <mutex>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
CORE_API ILogger* SetLoggerImpl(ILogger* logger);
//----------------------------------------------------------------------------
CORE_API void FlushLog();
//----------------------------------------------------------------------------
CORE_API void Log(ELogCategory category, const FWStringView& text);
//----------------------------------------------------------------------------
CORE_API void LogArgs(ELogCategory category, const FWStringView& format, const FWFormatArgList& args);
//----------------------------------------------------------------------------
template <typename _Arg0, typename... _Args>
void Log(ELogCategory category, const FWStringView& format, _Arg0&& arg0, _Args&&... args) {
    typedef details::TBasicFormatFunctor_<wchar_t> formatfunctor_t;
    const formatfunctor_t functors[] = {
        formatfunctor_t::Make(std::forward<_Arg0>(arg0)),
        formatfunctor_t::Make(std::forward<_Args>(args))...
    };

    LogArgs(category, format, FWFormatArgList(functors));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class CORE_API FOutputDebugLogger : public FAbstractThreadSafeLogger {
protected:
    virtual void LogThreadSafe(ELogCategory category, const FWStringView& format, const FWFormatArgList& args) override;
    virtual void FlushThreadSafe() override;
};
//----------------------------------------------------------------------------
class CORE_API FStdoutLogger : public FAbstractThreadSafeLogger {
protected:
    virtual void LogThreadSafe(ELogCategory category, const FWStringView& format, const FWFormatArgList& args) override;
    virtual void FlushThreadSafe() override;
};
//----------------------------------------------------------------------------
class CORE_API FStderrLogger : public FAbstractThreadSafeLogger {
protected:
    virtual void LogThreadSafe(ELogCategory category, const FWStringView& format, const FWFormatArgList& args) override;
    virtual void FlushThreadSafe() override;
};
//----------------------------------------------------------------------------
class CORE_API FStreamLogger : public FAbstractThreadSafeLogger {
public:
    FStreamLogger(class IBufferedStreamWriter* stream);
    ~FStreamLogger();

protected:
    virtual void LogThreadSafe(ELogCategory category, const FWStringView& format, const FWFormatArgList& args) override;
    virtual void FlushThreadSafe() override;

private:
    class IBufferedStreamWriter* _stream;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class CORE_API FLoggerStartup {
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
