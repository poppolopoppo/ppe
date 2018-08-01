#pragma once

#include "Core/HAL/TargetPlatform.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct CORE_API FGenericPlatformTime {
public: // must be defined for every platform
    STATIC_CONST_INTEGRAL(bool, HasHighPrecision, false);

    static i64 Cycles() = delete;
    static double Seconds() = delete;
    static double SecondsPerCycle() = delete;

    static double ToSeconds(i64 cycles) = delete;
    static i64 ToTicks(double secs) = delete;

    static void SystemTime(u32& year, u32& month, u32& dayOfWeek, u32& day, u32& hour, u32& min, u32& sec, u32& msec) = delete;
    static void UtcTime(u32& year, u32& month, u32& dayOfWeek, u32& day, u32& hour, u32& min, u32& sec, u32& msec) = delete;

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
