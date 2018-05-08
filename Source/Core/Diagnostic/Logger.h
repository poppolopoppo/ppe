#pragma once

#include "Core/Core.h"

#include "Core/Meta/Enum.h"

#if !defined(FINAL_RELEASE) || USE_CORE_FORCE_LOGGING
#   define USE_DEBUG_LOGGER
#endif

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class ELoggerVerbosity {
    Debug       = 1<<0,
    Info        = 1<<1,
    Emphasis    = 1<<2,
    Warning     = 1<<3,
    Error       = 1<<4,
    Fatal       = 1<<5,

    None        = 0,
    NoDebug     = (Info|Emphasis|Warning|Error|Fatal),
    NoDebugInfo = (Emphasis|Warning|Error|Fatal),
#ifdef PROFILING_ENABLED
    All         = NoDebugInfo
#else
    All         = (Debug|Info|Emphasis|Warning|Error|Fatal)
#endif
};
ENUM_FLAGS(ELoggerVerbosity);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#ifdef USE_DEBUG_LOGGER

#   include "Core/IO/Format.h"
#   include "Core/IO/String_fwd.h"
#   include "Core/IO/TextWriter.h"
#   include "Core/Memory/RefPtr.h"
#   include "Core/Meta/Function.h"
#   include "Core/Time/Timestamp.h"

#   include <thread>

namespace Core {
FWD_INTERFACE_REFPTR(Logger);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FLogger {
public:
    using EVerbosity = ELoggerVerbosity;

    struct FCategory {
        const wchar_t* Name;
        EVerbosity Verbosity = EVerbosity::All;
    };

    struct FSiteInfo {
        FTimestamp Timestamp;
        std::thread::id ThreadId;
        const wchar_t* Filename;
        size_t Line;

        static FSiteInfo Make(const wchar_t* filename, size_t line) {
            return FSiteInfo{ FTimestamp::Now(), std::this_thread::get_id(), filename, line };
        }
    };

    static CORE_API void Log(const FCategory& category, EVerbosity level, const FSiteInfo& site, const FWStringView& text);
    static CORE_API void LogArgs(const FCategory& category, EVerbosity level, const FSiteInfo& site, const FWStringView& format, const FWFormatArgList& args);

    template <typename _Arg0, typename... _Args>
    static void Log(const FCategory& category, EVerbosity level, const FSiteInfo& site, const FWStringView& format, _Arg0&& arg0, _Args&&... args) {
        typedef details::TBasicFormatFunctor_<wchar_t> formatfunctor_t;
        const formatfunctor_t functors[] = {
            formatfunctor_t::Make(std::forward<_Arg0>(arg0)),
            formatfunctor_t::Make(std::forward<_Args>(args))...
        };

        LogArgs(category, level, site, format, FWFormatArgList(functors));
    }

    static CORE_API void Flush(bool synchronous = true);

public:
    static CORE_API void Start();
    static CORE_API void Shutdown();

    static CORE_API void RegisterLogger(const PLogger& logger);
    static CORE_API void UnregisterLogger(const PLogger& logger);

public:
    static CORE_API PLogger MakeStdout();
    static CORE_API PLogger MakeOutputDebug();
    static CORE_API PLogger MakeAppendFile(const wchar_t* filename);
    static CORE_API PLogger MakeRollFile(const wchar_t* filename);
    static CORE_API PLogger MakeFunctor(Meta::TFunction<void(const FCategory&, EVerbosity, FSiteInfo, const FWStringView&)>&& write);
};
//----------------------------------------------------------------------------
class ILogger : FLogger, public FRefCountable {
public:
    virtual ~ILogger() {}

    using FLogger::EVerbosity;
    using FLogger::FCategory;
    using FLogger::FSiteInfo;

    virtual void Log(const FCategory& category, EVerbosity level, const FSiteInfo& site, const FWStringView& text) = 0;

    virtual void Flush(bool synchronous) = 0;
};
//----------------------------------------------------------------------------
CORE_API FTextWriter& operator <<(FTextWriter& oss, FLogger::EVerbosity level);
CORE_API FWTextWriter& operator <<(FWTextWriter& oss, FLogger::EVerbosity level);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#define LOG_CATEGORY_VERBOSITY(_API, _NAME, _VERBOSITY) \
    _API ::Core::FLogger::FCategory CONCAT(GLogCategory_, _NAME){ WIDESTRING(STRINGIZE(_NAME)), ::Core::FLogger::EVerbosity::_VERBOSITY };
#define LOG_CATEGORY(_API, _NAME) \
    LOG_CATEGORY_VERBOSITY(_API, _NAME, All)
#define EXTERN_LOG_CATEGORY(_API, _NAME) \
    extern _API ::Core::FLogger::FCategory CONCAT(GLogCategory_, _NAME);

#define LOG(_CATEGORY, _LEVEL, ...) \
    ::Core::FLogger::Log( \
        CONCAT(GLogCategory_, _CATEGORY), \
        ::Core::FLogger::EVerbosity::_LEVEL, \
        ::Core::FLogger::FSiteInfo::Make( \
            WIDESTRING(__FILE__), \
            __LINE__ ), \
        __VA_ARGS__ )

#define FLUSH_LOG() \
    ::Core::FLogger::Flush()

#else

#   include "Core/Meta/Assert.h"

#define LOG_CATEGORY_VERBOSITY(...)
#define LOG_CATEGORY(...)
#define EXTERN_LOG_CATEGORY(...)
#define FLUSH_LOG() NOOP()

#define LOG(_CATEGORY, _LEVEL, ...) \
    (void)( (!!(::Core::ELoggerVerbosity::Fatal != ::Core::ELoggerVerbosity::_LEVEL)) || (AssertReleaseFailed(L"log : fatal error"), 0) )

#endif //!#ifdef USE_DEBUG_LOGGER

#define CLOG(_CONDITION, _CATEGORY, _LEVEL, ...) \
    (void)( (!(_CONDITION)) || (LOG(_CATEGORY, _LEVEL, __VA_ARGS__), 0) )
