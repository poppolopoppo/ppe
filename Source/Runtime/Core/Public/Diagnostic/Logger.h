#pragma once

#include "Core.h"

#include "Diagnostic/Logger_fwd.h"
#include "Meta/Enum.h"
#include "Memory/RefPtr.h"
#include "Time/Timestamp.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class ELoggerVerbosity {
    Debug       = 1<<0,
    Verbose     = 1<<1,
    Info        = 1<<2,
    Emphasis    = 1<<3,
    Warning     = 1<<4,
    Error       = 1<<5,
    Fatal       = 1<<6,

    None        = 0,
    NoDebug     = (Info|Emphasis|Warning|Error|Fatal),
    NoDebugInfo = (Emphasis|Warning|Error|Fatal),

#if USE_PPE_PROFILING
    All         = NoDebugInfo
#else
    All         = (Debug|Verbose|Info|Emphasis|Warning|Error|Fatal)
#endif
};
ENUM_FLAGS(ELoggerVerbosity);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#if USE_PPE_LOGGER

#   include "IO/Format.h"
#   include "IO/String_fwd.h"
#   include "IO/TextWriter.h"
#   include "Memory/RefPtr.h"
#   include "Time/Timestamp.h"

#   include <thread>

namespace PPE {
FWD_INTERFACE_REFPTR(Logger);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FLoggerCategory {
    const wchar_t* Name;
    ELoggerVerbosity Verbosity = ELoggerVerbosity::All;
};
//----------------------------------------------------------------------------
class FLogger {
public:
    using EVerbosity = ELoggerVerbosity;

    using FCategory = FLoggerCategory;

    struct FSiteInfo {
        FTimestamp Timestamp;
        std::thread::id ThreadId;
        const wchar_t* Filename;
        size_t Line;

        static FSiteInfo Make(const wchar_t* filename, size_t line) {
            return FSiteInfo{ FTimestamp::Now(), std::this_thread::get_id(), filename, line };
        }
    };

    static PPE_CORE_API void Log(const FCategory& category, EVerbosity level, const FSiteInfo& site, const FWStringView& text);
    static PPE_CORE_API void LogArgs(const FCategory& category, EVerbosity level, const FSiteInfo& site, const FWStringView& format, const FWFormatArgList& args);

    template <typename _Arg0, typename... _Args>
    static void Log(const FCategory& category, EVerbosity level, const FSiteInfo& site, const FWStringView& format, _Arg0&& arg0, _Args&&... args) {
        typedef details::TBasicFormatFunctor_<wchar_t> formatfunctor_t;
        const formatfunctor_t functors[] = {
            MakeFormatArg<wchar_t>(arg0),
            MakeFormatArg<wchar_t>(args)...
        };

        LogArgs(category, level, site, format, FWFormatArgList(functors));
    }

    static PPE_CORE_API void Flush(bool synchronous = true);

public:
    static PPE_CORE_API void Start();
    static PPE_CORE_API void Shutdown();

    static PPE_CORE_API void RegisterLogger(const PLogger& logger);
    static PPE_CORE_API void UnregisterLogger(const PLogger& logger);

public:
    static PPE_CORE_API PLogger MakeStdout();
    static PPE_CORE_API PLogger MakeOutputDebug();
    static PPE_CORE_API PLogger MakeAppendFile(const wchar_t* filename);
    static PPE_CORE_API PLogger MakeRollFile(const wchar_t* filename);
    static PPE_CORE_API PLogger MakeSystemTrace();
};
//----------------------------------------------------------------------------
class ILogger : FLogger, public FRefCountable {
public:
    virtual ~ILogger() = default;

    using FLogger::EVerbosity;
    using FLogger::FCategory;
    using FLogger::FSiteInfo;

    virtual void Log(const FCategory& category, EVerbosity level, const FSiteInfo& site, const FWStringView& text) = 0;

    virtual void Flush(bool synchronous) = 0;
};
//----------------------------------------------------------------------------
PPE_CORE_API FTextWriter& operator <<(FTextWriter& oss, FLogger::EVerbosity level);
PPE_CORE_API FWTextWriter& operator <<(FWTextWriter& oss, FLogger::EVerbosity level);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#define LOG_CATEGORY_VERBOSITY(_API, _NAME, _VERBOSITY) \
    _API ::PPE::FLoggerCategory& LOG_CATEGORY_GET(_NAME) { \
        ONE_TIME_INITIALIZE(::PPE::FLoggerCategory, GLogCategory, \
            WIDESTRING(STRINGIZE(_NAME)), ::PPE::FLogger::EVerbosity::_VERBOSITY ); \
        return GLogCategory; \
    }
#define LOG_CATEGORY(_API, _NAME) \
    LOG_CATEGORY_VERBOSITY(_API, _NAME, All)

#define LOG(_CATEGORY, _LEVEL, _FORMAT, ...) do { \
    static_assert( /* validate format strings statically */ \
        ::PPE::ValidateFormatString( _FORMAT, PP_NUM_ARGS(__VA_ARGS__) ), \
        "invalid format : check arguments -> " STRINGIZE(PP_NUM_ARGS(__VA_ARGS__)) ); \
    ::PPE::FLogger::Log( \
        LOG_CATEGORY_GET(_CATEGORY), \
        ::PPE::FLogger::EVerbosity::_LEVEL, \
        ::PPE::FLogger::FSiteInfo::Make( \
            WIDESTRING(__FILE__), \
            __LINE__ ), \
        _FORMAT, __VA_ARGS__ ); \
    } while (0)

#define LOG_ARGS(_CATEGORY, _LEVEL, _FORMAT, _FORMAT_ARG_LIST) do { \
    ::PPE::FLogger::LogArgs( \
        LOG_CATEGORY_GET(_CATEGORY), \
        ::PPE::FLogger::EVerbosity::_LEVEL, \
        ::PPE::FLogger::FSiteInfo::Make( \
            WIDESTRING(__FILE__), \
            __LINE__ ), \
        _FORMAT, _FORMAT_ARG_LIST ); \
    } while (0)

#define LOG_DIRECT(_CATEGORY, _LEVEL, _MESSAGE) do { \
    ::PPE::FLogger::Log( \
        LOG_CATEGORY_GET(_CATEGORY), \
        ::PPE::FLogger::EVerbosity::_LEVEL, \
        ::PPE::FLogger::FSiteInfo::Make( \
            WIDESTRING(__FILE__), \
            __LINE__ ), \
        _MESSAGE ); \
    } while (0)

#define FLUSH_LOG() \
    ::PPE::FLogger::Flush()

#else

#   include "Meta/Assert.h"

#define LOG_CATEGORY_VERBOSITY(...)
#define LOG_CATEGORY(...)
#define FLUSH_LOG() NOOP()

#   if USE_PPE_FINAL_RELEASE
#       define LOG(_CATEGORY, _LEVEL, _FORMAT, ...) NOOP()
#       define LOG_ARGS(_CATEGORY, _LEVEL, _FORMAT, _FORMAT_ARG_LIST) NOOP()
#       define LOG_DIRECT(_CATEGORY, _LEVEL, _MESSAGE) NOOP()
#   else
#       define _LOG_Debug() NOOP()
#       define _LOG_Info() NOOP()
#       define _LOG_Emphasis() NOOP()
#       define _LOG_Warning() NOOP()
#       define _LOG_Error() NOOP()
#       define _LOG_Fatal() AssertReleaseFailed(L"log : fatal error")

#       define LOG(_CATEGORY, _LEVEL, _FORMAT, ...) EXPAND( CONCAT(_LOG_, _Level) _LPARENTHESIS _RPARENTHESIS )
#       define LOG_ARGS(_CATEGORY, _LEVEL, _FORMAT, _FORMAT_ARG_LIST) EXPAND( CONCAT(_LOG_, _Level) _LPARENTHESIS _RPARENTHESIS )
#       define LOG_DIRECT(_CATEGORY, _LEVEL, _MESSAGE) EXPAND( CONCAT(_LOG_, _Level) _LPARENTHESIS _RPARENTHESIS )
#   endif

#endif //!#if USE_PPE_LOGGER

#if USE_PPE_FINAL_RELEASE
#   define CLOG(_CONDITION, _CATEGORY, _LEVEL, _FORMAT, ...) NOOP()
#else
#   if !defined(_MSC_VER) || (defined(_MSVC_TRADITIONAL) && _MSVC_TRADITIONAL)
#       define CLOG(_CONDITION, _CATEGORY, _LEVEL, _FORMAT, ...) do { \
        if (_CONDITION) LOG(_CATEGORY, _LEVEL, _FORMAT, __VA_ARGS__); \
    } while (0)
#   else
#       define CLOG(_CONDITION, _CATEGORY, _LEVEL, _FORMAT, ...) do { \
        if (_CONDITION) LOG(_CATEGORY, _LEVEL, _FORMAT, __VA_ARGS__); \
    } while (0)
#   endif
#endif
