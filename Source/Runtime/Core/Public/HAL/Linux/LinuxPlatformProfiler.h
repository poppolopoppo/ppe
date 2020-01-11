#pragma once

#include "HAL/Generic/GenericPlatformProfiler.h"

#if USE_PPE_PLATFORM_PROFILER && defined(PLATFORM_LINUX)
#   if defined(__clang__) || defined(ARCH_X86)
#       undef USE_PPE_PLATFORM_PROFILER
#       define USE_PPE_PLATFORM_PROFILER 0
namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FLinuxPlatformProfiler : FGenericPlatformProfiler {
public:
    STATIC_CONST_INTEGRAL(bool, HasProfiler, false);

    enum ELinuxProfilerLevel {
        GlobalLevel = 1,
        ProcessLevel = 2,
        ThreadLevel = 3,
    };

    using EProfilerLevel = ELinuxProfilerLevel;

    static void Start(EProfilerLevel) {}
    static void Stop(EProfilerLevel) {}

    static void Resume(EProfilerLevel) {}
    static void Suspend(EProfilerLevel) {}

    static void Mark(u32) {}
    static void MarkAndComment(u32, const char*) {}
    static void Name(EProfilerLevel, const char*) {}

    static u32 NextMarker() { return 0; }

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
#   endif
#endif

#if USE_PPE_PLATFORM_PROFILER && defined(PLATFORM_LINUX)

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FLinuxPlatformProfiler : FGenericPlatformProfiler {
public:
    STATIC_CONST_INTEGRAL(bool, HasProfiler, true);

    enum ELinuxProfilerLevel {
        GlobalLevel = 1,
        ProcessLevel = 2,
        ThreadLevel = 3,
    };

    using EProfilerLevel = ELinuxProfilerLevel;

    static void Start(EProfilerLevel level);
    static void Stop(EProfilerLevel level);

    static void Resume(EProfilerLevel level);
    static void Suspend(EProfilerLevel level);

    static void Mark(u32 marker);
    static void MarkAndComment(u32 marker, const char* comment);
    static void Name(EProfilerLevel level, const char* identifier);

    static u32 NextMarker();

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!USE_PPE_PLATFORM_PROFILER && defined(PLATFORM_LINUX)
