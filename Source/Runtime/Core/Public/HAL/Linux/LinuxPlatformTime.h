#pragma once

#include "HAL/Generic/GenericPlatformTime.h"

#ifdef PLATFORM_LINUX

#include "HAL/Linux/LinuxPlatformIncludes.h"

#include <time.h>
#include <x86intrin.h>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_CORE_API FLinuxPlatformTime : FGenericPlatformTime {
public:
    STATIC_CONST_INTEGRAL(bool, HasHighPrecision, true);

    static u64 Rdtsc() NOEXCEPT {
        unsigned int dummy;
        return ::__rdtscp(&dummy);
    }

    static i64 Cycles() NOEXCEPT {
        struct timespec ts;
		Verify(0 == ::clock_gettime(CLOCK_MONOTONIC_RAW, &ts));
        // returns microseconds
		return static_cast<i64>(static_cast<i64>(ts.tv_sec) * 1000000ULL + static_cast<i64>(ts.tv_nsec) / 1000ULL);
    }

    static double Seconds() NOEXCEPT {
        struct timespec ts;
		Verify(0 == ::clock_gettime(GClockSource, &ts));
		return static_cast<double>(ts.tv_sec) + static_cast<double>(ts.tv_nsec) / 1e9;
    }

    static i64 ToCycles(double seconds) NOEXCEPT {
        return (static_cast<i64>(seconds * 1e6));
    }
    static double ToSeconds(i64 cycles) NOEXCEPT {
        return (static_cast<double>(cycles)/* µs */ * 1e-6);
    }

    static i64 Timestamp() NOEXCEPT;
    static u64 NetworkTime() NOEXCEPT;

    static void LocalTime(i64 timestamp, u32& year, u32& month, u32& dayOfWeek, u32& dayOfYear, u32& dayOfMon, u32& hour, u32& min, u32& sec) NOEXCEPT;
    static void UtcTime(i64 timestamp, u32& year, u32& month, u32& dayOfWeek, u32& dayOfYear, u32& dayOfMon, u32& hour, u32& min, u32& sec) NOEXCEPT;

    // not relevant on linux :
    static void EnterHighResolutionTimer() {}
    static void LeaveLowResolutionTimer() {}

public: // platform specific

    static int GClockSource;

    static void InitTiming();
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_LINUX
