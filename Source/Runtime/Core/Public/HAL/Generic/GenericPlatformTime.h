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

    static u64 Rdtsc() = delete;
    static i64 Cycles() = delete;
    static double Seconds() = delete;

    static i64 ToCycles(double seconds) = delete;
    static double ToSeconds(i64 cycles) = delete;

    static i64 Timestamp() = delete;
    static u64 CpuTime() = delete; // can't convert to time
    static u64 NetworkTime() = delete;

    static void LocalTime(i64 timestamp, u32& year, u32& month, u32& dayOfWeek, u32& dayOfYear, u32& dayOfMon, u32& hour, u32& min, u32& sec) = delete;
    static void UtcTime(i64 timestamp, u32& year, u32& month, u32& dayOfWeek, u32& dayOfYear, u32& dayOfMon, u32& hour, u32& min, u32& sec) = delete;

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
