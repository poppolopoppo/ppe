#pragma once

#include "HAL/Generic/GenericPlatformTime.h"

#ifdef PLATFORM_WINDOWS

#include "HAL/Windows/WindowsPlatformIncludes.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_CORE_API FWindowsPlatformTime : FGenericPlatformTime {
public:
    STATIC_CONST_INTEGRAL(bool, HasHighPrecision, true);

    static FORCE_INLINE i64 Cycles() NOEXCEPT {
        ::LARGE_INTEGER cycles;
        ::QueryPerformanceCounter(&cycles);
        return cycles.QuadPart;
    }

    static double SecondsPerCycle() NOEXCEPT {
        ::LARGE_INTEGER frequency;
        Verify(::QueryPerformanceFrequency(&frequency));
        return (1.0 / frequency.QuadPart);
    }

    static double MicrosecondsPerCycle() NOEXCEPT {
        ::LARGE_INTEGER frequency;
        Verify(::QueryPerformanceFrequency(&frequency));
        return (1000000.0 / frequency.QuadPart);
    }

    static double ToSeconds(i64 cycles) NOEXCEPT {
        return (cycles * GSecondsPerCycle);
    }

    static i64 ToCycles(double seconds) NOEXCEPT {
        return i64(seconds / GSecondsPerCycle);
    }

    static FORCE_INLINE u64 ThreadCpuCycles() NOEXCEPT {
        ::HANDLE hThread = ::GetCurrentThread();
        ::ULONG64 cycleTime;
        ::QueryThreadCycleTime(hThread, &cycleTime);
        return cycleTime;
    }

    static u64 NetworkTime() NOEXCEPT;
    static void SystemTime(u32& year, u32& month, u32& dayOfWeek, u32& day, u32& hour, u32& min, u32& sec, u32& msec) NOEXCEPT;
    static void UtcTime(u32& year, u32& month, u32& dayOfWeek, u32& day, u32& hour, u32& min, u32& sec, u32& msec) NOEXCEPT;

    static void EnterHighResolutionTimer();
    static void LeaveLowResolutionTimer();

public: // platform specific

    static const double GSecondsPerCycle;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_WINDOWS
