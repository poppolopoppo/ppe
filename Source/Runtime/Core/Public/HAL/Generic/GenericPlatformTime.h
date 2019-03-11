#pragma once

#include "HAL/TargetPlatform.h"

#include <chrono>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_CORE_API FGenericPlatformTime {
public: // must be defined for every platform
    STATIC_CONST_INTEGRAL(bool, HasHighPrecision, false);

    static i64 Cycles() = delete;
    static double Seconds() = delete;
    static double SecondsPerCycle() = delete;

    static u64 ThreadCpuCycles() = delete; // can't convert to time

    static double ToSeconds(i64 cycles) = delete;
    static double ToMicroseconds(i64 cycles) = delete;

    static i64 ToTicks(double secs) = delete;

    static void SystemTime(u32& year, u32& month, u32& dayOfWeek, u32& day, u32& hour, u32& min, u32& sec, u32& msec) = delete;
    static void UtcTime(u32& year, u32& month, u32& dayOfWeek, u32& day, u32& hour, u32& min, u32& sec, u32& msec) = delete;

    static double ChronoMicroseconds() NOEXCEPT {
        using chrono_type = std::conditional_t<
            std::chrono::high_resolution_clock::is_steady,
            std::chrono::high_resolution_clock,
            std::chrono::steady_clock >;
        using microseconds_type = std::chrono::duration<
            double,
            std::chrono::microseconds::period >;
        return microseconds_type(chrono_type::now().time_since_epoch()).count();
    }

    static void EnterHighResolutionTimer() = delete;
    static void LeaveLowResolutionTimer() = delete;

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
