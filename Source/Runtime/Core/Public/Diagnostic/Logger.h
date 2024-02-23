#pragma once

#include "Core_fwd.h"

#include "Diagnostic/Logger_fwd.h"
#include "IO/StaticString.h"
#include "Meta/Enum.h"
#include "Memory/RefPtr.h"
#include "Time/Timestamp.h"

// structured logging without allocation:
// https://godbolt.org/z/xh3P8GvP4

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class ELoggerVerbosity : u8 {
    Debug       = 1<<0,
    Verbose     = 1<<1,
    Info        = 1<<2,
    Profiling   = 1<<3,
    Emphasis    = 1<<4,
    Warning     = 1<<5,
    Error       = 1<<6,
    Fatal       = 1<<7,

    None        = 0,
    NoDebug     = (Info|Profiling|Emphasis|Warning|Error|Fatal),
    NoDebugInfo = (Profiling|Emphasis|Warning|Error|Fatal),

#if USE_PPE_PROFILING || USE_PPE_FINAL_RELEASE
    All         = NoDebugInfo
#elif !USE_PPE_DEBUG || USE_PPE_FASTDEBUG
    All         = (Verbose|NoDebug)
#else
    All         = (Debug|Verbose|NoDebug)
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
#   include "Time/Timepoint.h"

#   include "IO/StringView.h"
#   include "Memory/PtrRef.h"
#   include "Misc/Function_fwd.h"
#   include "Misc/Opaque.h"

#   include <thread>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FLoggerCategory {
    enum EFlags : u32 {
        Unknown         = 0,
        Immediate       = 1<<0,
#if !USE_PPE_FINAL_RELEASE
        BreakOnError    = 1<<1,
        BreakOnWarning  = 1<<2,
#endif
    };
    ENUM_FLAGS_FRIEND(EFlags);

    FStringLiteral Name;
    ELoggerVerbosity Verbosity{ ELoggerVerbosity::All };
    EFlags Flags{ Default };
    hash_t HashValue{ 0 };

    template <size_t _Len>
    CONSTEXPR FLoggerCategory(
        const char (&staticName)[_Len],
        ELoggerVerbosity verbosity = ELoggerVerbosity::All,
        EFlags flags = Default)
    :   Name(staticName)
    ,   Verbosity(verbosity)
    ,   Flags(flags)
    ,   HashValue(hash_arr_constexpr(staticName))
    {
        Assert_NoAssume(HashValue > 0);
    }
};
//----------------------------------------------------------------------------
struct FLoggerSiteInfo {
    FTimepoint LogTime;
    FConstChar SourceFile;
    std::thread::id ThreadId;
    u32 SourceLine : 23;
    u32 PackedLevel : 8;
    bool IsTextAllocated : 1;

    ELoggerVerbosity Level() const { return static_cast<ELoggerVerbosity>(PackedLevel); }

    FLoggerSiteInfo() = default;
    FLoggerSiteInfo(ELoggerVerbosity level, FConstChar sourceFile, u32 sourceLine)
    :   LogTime(FTimepoint::Now())
    ,   SourceFile(sourceFile)
    ,   ThreadId(std::this_thread::get_id())
    ,   SourceLine(sourceLine)
    ,   PackedLevel(static_cast<u8>(level))
    ,   IsTextAllocated(false)
    {}
};
PPE_ASSUME_TYPE_AS_POD(FLoggerSiteInfo);
//----------------------------------------------------------------------------
struct FLoggerMessage {
    TPtrRef<const FLoggerCategory> Category;
    TPtrRef<const Opaq::object_view> Data;
    FConstChar Text;
    FLoggerSiteInfo Site;

    ELoggerVerbosity Level() const { return Site.Level(); }
    bool IsTextAllocated() const { return Site.IsTextAllocated; };

    FLoggerMessage() = default;
    FLoggerMessage(
        const FLoggerCategory& category, FLoggerSiteInfo&& site,
        FConstChar text, bool isTextAllocated,
        TPtrRef<const Opaq::object_view>&& pOptionalData) NOEXCEPT
    :   Category(category)
    ,   Data(std::move(pOptionalData))
    ,   Text(std::move(text))
    ,   Site(std::move(site)) {
        Site.IsTextAllocated = isTextAllocated;
    }

    FLoggerMessage(
        const FLoggerCategory& category, FLoggerSiteInfo&& site,
        FConstChar text, bool isTextAllocated) NOEXCEPT
    :   Category(category)
    ,   Text(std::move(text))
    ,   Site(std::move(site)) {
        Site.IsTextAllocated = isTextAllocated;
    }
};
PPE_ASSUME_TYPE_AS_POD(FLoggerMessage);
//----------------------------------------------------------------------------
class ILogger {
public:
    virtual ~ILogger() = default;

    virtual void LogMessage(const FLoggerMessage& msg) = 0;
    virtual void Flush(bool synchronous) = 0;
};
//----------------------------------------------------------------------------
class FLogger {
public:
    using FCategory = FLoggerCategory;
    using FMessage = FLoggerMessage;
    using FSiteInfo = FLoggerSiteInfo;
    using EVerbosity = ELoggerVerbosity;

    static PPE_CORE_API void Log(const FCategory& category, FSiteInfo&& site, FStringLiteral text);
    static PPE_CORE_API void LogFmt(const FCategory& category, FSiteInfo&& site, FStringLiteral format, const FFormatArgList& args);
    static PPE_CORE_API void LogStructured(const FCategory& category, FSiteInfo&& site, FStringLiteral text, Opaq::object_init&& object);
    static PPE_CORE_API void LogStructured(const FCategory& category, FSiteInfo&& site, FStringView textToCopy, Opaq::object_init&& object);
    static PPE_CORE_API void Printf(const FCategory& category, FSiteInfo&& site, FStringLiteral format, va_list args);
    static PPE_CORE_API void LogDirect(const FCategory& category, FSiteInfo&& site, const TFunction<void(FTextWriter&)>& direct);
    static PPE_CORE_API void RecordArgs(const FCategory& category, FSiteInfo&& site, const FFormatArgList& record);
    static PPE_CORE_API void Flush(bool synchronous = true);

    static void LogFmtT(const FCategory& category, FSiteInfo&& site, FStringLiteral formatWithoutArgs) {
        Log(category, std::move(site), formatWithoutArgs);
    }
    template <typename _Arg0, typename... _Args>
    static void LogFmtT(const FCategory& category, FSiteInfo&& site, FStringLiteral format, _Arg0&& arg0, _Args&&... args) {
        LogFmt(category, std::move(site), format, {
            MakeFormatArg<char>(std::forward<_Arg0>(arg0)),
            MakeFormatArg<char>(std::forward<_Args>(args))... });
    }

    static void Printf(const FCategory& category, FSiteInfo&& site, FStringLiteral format, .../* va_list */) {
        va_list args;
        va_start(args, format);
        Printf(category, std::move(site), format, args);
        va_end(args);
    }

    template <typename _Arg0, typename... _Args>
    static void RecordArgsT(const FCategory& category, FSiteInfo&& site, _Arg0&& arg0, _Args&&... args) {
        RecordArgs(category, std::move(site), {
            MakeFormatArg<char>(std::forward<_Arg0>(arg0)),
            MakeFormatArg<char>(std::forward<_Args>(args))... });
    }

public:
    static PPE_CORE_API void Start();
    static PPE_CORE_API void Shutdown();

    static PPE_CORE_API void RegisterLogger(TPtrRef<ILogger> logger, bool autoDelete = true);
    static PPE_CORE_API void UnregisterLogger(TPtrRef<ILogger> logger);

    NODISCARD static PPE_CORE_API ELoggerVerbosity GlobalVerbosity();
    static PPE_CORE_API void SetGlobalVerbosity(ELoggerVerbosity verbosity);

    NODISCARD static CONSTEXPR bool ShouldCompileMessage(ELoggerVerbosity verbosity) {
#if 0
        return (verbosity ^ ELoggerVerbosity::All);
#else
        using underlying_type = Meta::TEnumOrd<ELoggerVerbosity>;
        return !!(static_cast<underlying_type>(ELoggerVerbosity::All) & static_cast<underlying_type>(verbosity));
#endif
    }

    static PPE_CORE_API void RegisterStdoutLogger(bool useColors);
    static PPE_CORE_API void RegisterOutputDebugLogger();
    static PPE_CORE_API void RegisterAppendFileLogger(FConstWChar filename);
    static PPE_CORE_API void RegisterAppendJsonLogger(FConstWChar filename);
    static PPE_CORE_API void RegisterRollFileLogger(FConstWChar filename);
    static PPE_CORE_API void RegisterRollJsonLogger(FConstWChar filename);
    static PPE_CORE_API void RegisterSystemTraceLogger();

};
//----------------------------------------------------------------------------
PPE_CORE_API FConstChar ToString(FLogger::EVerbosity level) NOEXCEPT;
PPE_CORE_API FConstWChar ToWString(FLogger::EVerbosity level) NOEXCEPT;
//----------------------------------------------------------------------------
PPE_CORE_API FTextWriter& operator <<(FTextWriter& oss, FLogger::EVerbosity level);
PPE_CORE_API FWTextWriter& operator <<(FWTextWriter& oss, FLogger::EVerbosity level);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#define _PPE_LOG_MAKESITE(_CATEGORY, _LEVEL) \
    LOG_CATEGORY_GET(_CATEGORY), \
    ::PPE::FLogger::FSiteInfo{ \
        ::PPE::FLogger::EVerbosity::_LEVEL, \
        _PPE_LOG_STRING(__FILE__), \
        __LINE__ }

#define _PPE_LOG_VALIDATEFORMAT(_FORMAT, ...) \
    static_assert( /* validate format strings statically */ \
        (::PPE::EValidateFormat::Valid == ::PPE::ValidateFormatString( _FORMAT, PP_NUM_ARGS(__VA_ARGS__) )), \
        "invalid format : check arguments -> " STRINGIZE(PP_NUM_ARGS(__VA_ARGS__)) );

// #TODO: remove this workaround when MSVC is fixed... (ShouldCompileMessage invalidly detected as not constexpr)
#if defined(_MSC_VER) && !defined(__clang__) /* clang-cl works just fine */
#   define _PPE_LOG_IF_SHOULDCOMPILE(_LEVEL) IF_CONSTEXPR(::PPE::FLogger::EVerbosity::_LEVEL ^ ::PPE::FLogger::EVerbosity::All)
#else
#   define _PPE_LOG_IF_SHOULDCOMPILE(_LEVEL) IF_CONSTEXPR(::PPE::FLogger::ShouldCompileMessage(::PPE::FLogger::EVerbosity::_LEVEL))
#endif

#define LOG_CATEGORY_EX(_API, _NAME, _VERBOSITY, _FLAGS) \
    _API ::PPE::FLoggerCategory& LOG_CATEGORY_GET(_NAME) { \
        ONE_TIME_INITIALIZE(::PPE::FLoggerCategory, GLogCategory, \
            _PPE_LOG_STRINGIZE(_NAME), \
            ::PPE::FLogger::EVerbosity::_VERBOSITY, \
            ::PPE::FLoggerCategory::EFlags::_FLAGS ); \
        return GLogCategory; \
    }
#define LOG_CATEGORY_VERBOSITY(_API, _NAME, _VERBOSITY) \
    LOG_CATEGORY_EX(_API, _NAME, _VERBOSITY, Unknown)
#define LOG_CATEGORY(_API, _NAME) \
    LOG_CATEGORY_VERBOSITY(_API, _NAME, All)

#define PPE_LOG(_CATEGORY, _LEVEL, ...) do { \
    _PPE_LOG_IF_SHOULDCOMPILE(_LEVEL) { \
        EXPAND( _PPE_LOG_VALIDATEFORMAT(__VA_ARGS__) ) \
        ::PPE::FLogger::LogFmtT( _PPE_LOG_MAKESITE(_CATEGORY, _LEVEL), __VA_ARGS__ ); \
    } } while(0)

#define PPE_LOG_ARGS(_CATEGORY, _LEVEL, _FORMAT, _FORMAT_ARG_LIST) do { \
    _PPE_LOG_IF_SHOULDCOMPILE(_LEVEL) { \
        ::PPE::FLogger::LogFmt( _PPE_LOG_MAKESITE(_CATEGORY, _LEVEL), _FORMAT, _FORMAT_ARG_LIST ); \
    } } while(0)

#define PPE_SLOG(_CATEGORY, _LEVEL, ...) do { \
    _PPE_LOG_IF_SHOULDCOMPILE(_LEVEL) { \
        ::PPE::FLogger::LogStructured( _PPE_LOG_MAKESITE(_CATEGORY, _LEVEL), __VA_ARGS__ ); \
    } } while(0)

#define PPE_LOG_DIRECT(_CATEGORY, _LEVEL, ...) do { \
    _PPE_LOG_IF_SHOULDCOMPILE(_LEVEL) { \
        ::PPE::FLogger::LogDirect( _PPE_LOG_MAKESITE(_CATEGORY, _LEVEL), __VA_ARGS__ ); \
    } } while(0)

#define PPE_LOG_PRINTF(_CATEGORY, _LEVEL, ...) do { \
    _PPE_LOG_IF_SHOULDCOMPILE(_LEVEL) { \
        ::PPE::FLogger::Printf( _PPE_LOG_MAKESITE(_CATEGORY, _LEVEL), __VA_ARGS__ ); \
    } } while(0)

#define PPE_LOG_RECORD(_CATEGORY, _LEVEL, ...) do { \
    _PPE_LOG_IF_SHOULDCOMPILE(_LEVEL) { \
        ::PPE::FLogger::RecordArgsT( _PPE_LOG_MAKESITE(_CATEGORY, _LEVEL), __VA_ARGS__ ); \
    } } while(0)

#define PPE_LOG_FLUSH() \
    ::PPE::FLogger::Flush()

#else

#   include "Meta/Assert.h"

#   if USE_PPE_FINAL_RELEASE
#       define _PPE_LOG_LEVEL_Debug() NOOP()
#       define _PPE_LOG_LEVEL_Verbose() NOOP()
#       define _PPE_LOG_LEVEL_Info() NOOP()
#       define _PPE_LOG_LEVEL_Profiling() NOOP()
#       define _PPE_LOG_LEVEL_Emphasis() NOOP()
#       define _PPE_LOG_LEVEL_Warning() NOOP()
#       define _PPE_LOG_LEVEL_Error() NOOP()
#       define _PPE_LOG_LEVEL_Fatal() AssertReleaseFailed("log : fatal error")
#       define _PPE_LOG_LEVEL(_LEVEL) EXPAND( CONCAT(_PPE_LOG_LEVEL_, _LEVEL) _LPARENTHESIS _RPARENTHESIS )
#   else
#       define _PPE_LOG_LEVEL(_LEVEL) NOOP()
#   endif

#   define LOG_CATEGORY_EX(_API, _NAME, _VERBOSITY, _FLAGS)
#   define LOG_CATEGORY_VERBOSITY(_API, _NAME, _VERBOSITY)
#   define LOG_CATEGORY(_API, _NAME)

#   define PPE_LOG(_CATEGORY, _LEVEL, ...) _PPE_LOG_LEVEL(_LEVEL)
#   define PPE_LOG_ARGS(_CATEGORY, _LEVEL, _FORMAT, _FORMAT_ARG_LIST) _PPE_LOG_LEVEL(_LEVEL)
#   define PPE_SLOG(_CATEGORY, _LEVEL, ...) _PPE_LOG_LEVEL(_LEVEL)
#   define PPE_LOG_DIRECT(_CATEGORY, _LEVEL, ...) _PPE_LOG_LEVEL(_LEVEL)
#   define PPE_LOG_PRINTF(_CATEGORY, _LEVEL, ...) _PPE_LOG_LEVEL(_LEVEL)
#   define PPE_LOG_RECORD(_CATEGORY, _LEVEL, ...) _PPE_LOG_LEVEL(_LEVEL)

#   define PPE_LOG_FLUSH() NOOP()

#endif //!#if USE_PPE_LOGGER

#if USE_PPE_FINAL_RELEASE
#   define PPE_CLOG(_CONDITION, _CATEGORY, _LEVEL, ...) NOOP()
#else
#   define PPE_CLOG(_CONDITION, _CATEGORY, _LEVEL, ...) do { \
        if (_CONDITION) PPE_LOG(_CATEGORY, _LEVEL, __VA_ARGS__); \
    } while (0)
#endif

#define PPE_LOG_CHECKEX( _CATEGORY, _RETURN, ... ) do {\
    if (Ensure(__VA_ARGS__)) {} \
    else { \
        PPE_LOG( _CATEGORY, Error, _PPE_LOG_STRING("failed expr: '{0}', at: {1}:{2}"), _PPE_LOG_STRINGIZE(__VA_ARGS__), _PPE_LOG_STRING(__FILE__), _PPE_LOG_STRINGIZE(__LINE__) ); \
        return (_RETURN); \
    }} while (0)
#define PPE_LOG_CHECK( _CATEGORY, ... ) PPE_LOG_CHECKEX( _CATEGORY, ::PPE::Default, __VA_ARGS__ )
#define PPE_LOG_CHECKVOID( _CATEGORY, ... ) PPE_LOG_CHECKEX( _CATEGORY, void(), __VA_ARGS__ )

#define PPE_LOG_UNSUPPORTED_FUNCTION( _CATEGORY ) \
    LOG( _CATEGORY, Warning, _PPE_LOG_STRING("unsupported: {0}, at {1}:{2}"), \
        ::PPE::MakeStringView(PPE_PRETTY_FUNCTION), \
        _PPE_LOG_STRING(__FILE__), \
        __LINE__ )
