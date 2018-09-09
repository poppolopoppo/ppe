#pragma once

#include "HAL/Generic/GenericPlatformProfiler.h"

#if USE_PPE_PLATFORM_PROFILER && defined(PLATFORM_WINDOWS)

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FWindowsPlatformProfiler : FGenericPlatformProfiler {
public:
    STATIC_CONST_INTEGRAL(bool, HasProfiler, true);

    enum EWindowsProfilerLevel {
        GlobalLevel = 1,
        ProcessLevel = 2,
        ThreadLevel = 3,
    };

    using EProfilerLevel = EWindowsProfilerLevel;

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

#endif //!USE_PPE_PLATFORM_PROFILER && defined(PLATFORM_WINDOWS)
