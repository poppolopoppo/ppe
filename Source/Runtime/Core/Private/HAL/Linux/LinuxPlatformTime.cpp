#include "stdafx.h"

#include "HAL/Linux/LinuxPlatformTime.h"

#ifdef PLATFORM_LINUX

#include "HAL/TargetPlatform.h"
#include "HAL/Linux/Errno.h"
#include "HAL/Linux/LinuxPlatformIncludes.h"

#include "Diagnostic/Logger.h"

#include <sys/time.h>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static int CalibrateAndSelectClock_() {
    static int GClocks[] {
        CLOCK_REALTIME,
        CLOCK_MONOTONIC,
        CLOCK_MONOTONIC_RAW,
		CLOCK_MONOTONIC_COARSE,
    };

    int result = CLOCK_REALTIME; // failsafe
    long minres = LONG_MAX;
    struct ::timespec ts{};
    for (const int clock : GClocks) {
        if (::clock_getres(clock, &ts) == 0 && ts.tv_nsec < minres) {
            result = clock;
            minres = ts.tv_nsec;
        }
    }

    return result;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
int FLinuxPlatformTime::GClockSource = -1;
//----------------------------------------------------------------------------
void FLinuxPlatformTime::InitTiming() {
    if (GClockSource == -1) {
		// Only ever set this ClockSource once
		GClockSource = CalibrateAndSelectClock_();
	}
}
//----------------------------------------------------------------------------
u64 FLinuxPlatformTime::NetworkTime() NOEXCEPT {
    struct ::timeval tod;
    ::gettimeofday(&tod, nullptr);
    return static_cast<u64>(static_cast<u64>(tod.tv_sec) * 1000000ULL + static_cast<u64>(tod.tv_usec));
}
//----------------------------------------------------------------------------
void FLinuxPlatformTime::SystemTime(u32& year, u32& month, u32& dayOfWeek, u32& day, u32& hour, u32& min, u32& sec, u32& msec) NOEXCEPT {
    struct ::timespec ts = {0,0};

    if (::clock_gettime(CLOCK_REALTIME, &ts)) {
        LOG(HAL, Fatal, L"clock_gettime() failed with errno: {0}", FErrno{});
        return;
    }

    struct ::tm tp = {};
    time_t timer = ts.tv_sec;
    if (::localtime_r(&timer, &tp)) {
        LOG(HAL, Fatal, L"localtime_r() failed with errno: {0}", FErrno{});
        return;
    }

    year = checked_cast<u32>(tp.tm_year);
    month = checked_cast<u32>(tp.tm_mon);
    dayOfWeek = checked_cast<u32>(tp.tm_wday);
    day = checked_cast<u32>(tp.tm_yday);
    hour = checked_cast<u32>(tp.tm_hour);
    min = checked_cast<u32>(tp.tm_min);
    sec = checked_cast<u32>(tp.tm_sec);
    msec = checked_cast<u32>(ts.tv_nsec / 1000000ull);
}
//----------------------------------------------------------------------------
void FLinuxPlatformTime::UtcTime(u32& year, u32& month, u32& dayOfWeek, u32& day, u32& hour, u32& min, u32& sec, u32& msec) NOEXCEPT {
    struct ::timespec ts = {0,0};

    if (::clock_gettime(CLOCK_REALTIME, &ts)) {
        LOG(HAL, Fatal, L"clock_gettime() failed with errno: {0}", FErrno{});
        return;
    }

    struct ::tm tp = {};
    time_t timer = ts.tv_sec;
    if (::gmtime_r(&timer, &tp)) {
        LOG(HAL, Fatal, L"localtime_r() failed with errno: {0}", FErrno{});
        return;
    }

    year = checked_cast<u32>(tp.tm_year);
    month = checked_cast<u32>(tp.tm_mon);
    dayOfWeek = checked_cast<u32>(tp.tm_wday);
    day = checked_cast<u32>(tp.tm_yday);
    hour = checked_cast<u32>(tp.tm_hour);
    min = checked_cast<u32>(tp.tm_min);
    sec = checked_cast<u32>(tp.tm_sec);
    msec = checked_cast<u32>(ts.tv_nsec / 1000000ull);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_LINUX
