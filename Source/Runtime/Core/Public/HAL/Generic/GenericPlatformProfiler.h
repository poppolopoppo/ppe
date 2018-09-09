#pragma once

#include "HAL/Generic/GenericPlatformProfiler.h"

#define USE_PPE_PLATFORM_PROFILER (!USE_PPE_FINAL_RELEASE)

#if USE_PPE_PLATFORM_PROFILER

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FGenericPlatformProfiler {
public: // must be defined for every platform
    STATIC_CONST_INTEGRAL(bool, HasProfiler, true);

    enum EGenericProfilerLevel {
        GlobalLevel,
        ProcessLevel,
        ThreadLevel,
    };

    using EProfilerLevel = EGenericProfilerLevel;

    static void Start(EProfilerLevel level) = delete;
    static void Stop(EProfilerLevel level) = delete;

    static void Resume(EProfilerLevel level) = delete;
    static void Suspend(EProfilerLevel level) = delete;

    static void Mark(u32 marker) = delete;
    static void MarkAndComment(u32 marker, const char* comment) = delete;
    static void Name(EProfilerLevel level, const char* identifier) = delete;

    static u32 NextMarker() = delete; // used to create unique markers
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#define PPE_PROFILING_MARK(_COMMENT) \
    static const u32 ANONYMIZE(_GProfilingMarker_) = \
        FPlatformProfiler::NextMarker(); \
    FPlatformProfiler::MarkAndComment( \
        ANONYMIZE(_GProfilingMarker_), \
        _COMMENT " - mark at " __FILE__ " (" STRINGIZE(__LINE) ")" )

#define PPE_PROFILING_SCOPE(_COMMENT) \
    static const u32 ANONYMIZE(_GProfilingMarker_) = \
        FPlatformProfiler::NextMarker(); \
    struct ANONYMIZE(_FProfilingScope_) { \
        ANONYMIZE(_FProfilingScope_)() { \
            FPlatformProfiler::MarkAndComment( \
                ANONYMIZE(_GProfilingMarker_), \
                _COMMENT " - start at " __FILE__ " (" STRINGIZE(__LINE) ")" ); \
        } \
        ~ANONYMIZE(_FProfilingScope_)() { \
            FPlatformProfiler::MarkAndComment( \
                ANONYMIZE(_GProfilingMarker_), \
                _COMMENT " - stop at " __FILE__ " (" STRINGIZE(__LINE) ")" ); \
        } \
    }   ANONYMIZE(_profilingScope){}

#else

#define PPE_PROFILING_MARK(_COMMENT) NOOP()
#define PPE_PROFILING_SCOPE(_COMMENT) NOOP()

#endif //!USE_PPE_PLATFORM_PROFILER
