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
} //!namespace PPE

#ifdef USE_DEBUG_LOGGER

#   include "IO/Format.h"
#   include "IO/String_fwd.h"
#   include "IO/TextWriter.h"
#   include "Memory/RefPtr.h"
#   include "Misc/Function.h"
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

    static PPE_API void Log(const FCategory& category, EVerbosity level, const FSiteInfo& site, const FWStringView& text);
    static PPE_API void LogArgs(const FCategory& category, EVerbosity level, const FSiteInfo& site, const FWStringView& format, const FWFormatArgList& args);

    template <typename _Arg0, typename... _Args>
    static void Log(const FCategory& category, EVerbosity level, const FSiteInfo& site, const FWStringView& format, _Arg0&& arg0, _Args&&... args) {
        typedef details::TBasicFormatFunctor_<wchar_t> formatfunctor_t;
        const formatfunctor_t functors[] = {
            formatfunctor_t::Make(std::forward<_Arg0>(arg0)),
            formatfunctor_t::Make(std::forward<_Args>(args))...
        };

        LogArgs(category, level, site, format, FWFormatArgList(functors));
    }

    static PPE_API void Flush(bool synchronous = true);

public:
    static PPE_API void Start();
    static PPE_API void Shutdown();

    static PPE_API void RegisterLogger(const PLogger& logger);
    static PPE_API void UnregisterLogger(const PLogger& logger);

public:
    static PPE_API PLogger MakeStdout();
    static PPE_API PLogger MakeOutputDebug();
    static PPE_API PLogger MakeAppendFile(const wchar_t* filename);
    static PPE_API PLogger MakeRollFile(const wchar_t* filename);
    static PPE_API PLogger MakeFunctor(TFunction<void(const FCategory&, EVerbosity, FSiteInfo, const FWStringView&)>&& write);
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
PPE_API FTextWriter& operator <<(FTextWriter& oss, FLogger::EVerbosity level);
PPE_API FWTextWriter& operator <<(FWTextWriter& oss, FLogger::EVerbosity level);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#define LOG_CATEGORY_VERBOSITY(_API, _NAME, _VERBOSITY) \
    _API ::Core::FLoggerCategory CONCAT(GLogCategory_, _NAME){ WIDESTRING(STRINGIZE(_NAME)), ::Core::FLogger::EVerbosity::_VERBOSITY };
#define LOG_CATEGORY(_API, _NAME) \
    LOG_CATEGORY_VERBOSITY(_API, _NAME, All)

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

#   include "Meta/Assert.h"

#define LOG_CATEGORY_VERBOSITY(...)
#define LOG_CATEGORY(...)
#define FLUSH_LOG() NOOP()

#define LOG(_CATEGORY, _LEVEL, ...) \
    (void)( (!!(::Core::ELoggerVerbosity::Fatal != ::Core::ELoggerVerbosity::_LEVEL)) || (AssertReleaseFailed(L"log : fatal error"), 0) )

#endif //!#ifdef USE_DEBUG_LOGGER

#define CLOG(_CONDITION, _CATEGORY, _LEVEL, ...) \
    (void)( (!(_CONDITION)) || (LOG(_CATEGORY, _LEVEL, __VA_ARGS__), 0) )
