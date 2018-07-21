#pragma once

#include "Core/HAL/Generic/GenericPlatformTime.h"

#ifdef PLATFORM_WINDOWS

#include "Core/HAL/Windows/WindowsPlatformIncludes.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct CORE_API FWindowsPlatformTime : FGenericPlatformTime {
public:
    STATIC_CONST_INTEGRAL(bool, HasHighPrecision, true);

    static FORCE_INLINE i64 Cycles() {
        ::LARGE_INTEGER cycles;
        ::QueryPerformanceCounter(&cycles);
        return cycles.QuadPart;
    }

    static FORCE_INLINE double Seconds() {
        ::LARGE_INTEGER cycles;
        ::QueryPerformanceCounter(&cycles);
        return (cycles.QuadPart * GSecondsPerCycle);
    }

    static FORCE_INLINE double SecondsPerCycle() NOEXCEPT { return GSecondsPerCycle; }

    static FORCE_INLINE double ToSeconds(i64 cycles) NOEXCEPT { return (cycles * GSecondsPerCycle); }
    static FORCE_INLINE i64 ToTicks(double secs) NOEXCEPT { return i64(secs / GSecondsPerCycle); }

    static void SystemTime(u32& year, u32& month, u32& dayOfWeek, u32& day, u32& hour, u32& min, u32& sec, u32& msec);
    static void UtcTime(u32& year, u32& month, u32& dayOfWeek, u32& day, u32& hour, u32& min, u32& sec, u32& msec);

public: // platform specific

    static const double GSecondsPerCycle;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#endif //!PLATFORM_WINDOWS
