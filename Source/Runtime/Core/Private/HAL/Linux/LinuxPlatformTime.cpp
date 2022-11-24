// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

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
i64 FLinuxPlatformTime::Timestamp() NOEXCEPT {
    STATIC_ASSERT(std::is_same_v<::time_t, i64>);
    return ::time(nullptr);
}
//----------------------------------------------------------------------------
u64 FLinuxPlatformTime::NetworkTime() NOEXCEPT {
    struct ::timeval tod;
    ::gettimeofday(&tod, nullptr);
    return static_cast<u64>(static_cast<u64>(tod.tv_sec) * 1000000ULL + static_cast<u64>(tod.tv_usec));
}
//----------------------------------------------------------------------------
void FLinuxPlatformTime::LocalTime(i64 timestamp, u32& year, u32& month, u32& dayOfWeek, u32& dayOfYear, u32& dayOfMon, u32& hour, u32& min, u32& sec) NOEXCEPT {
    struct ::tm tp = {};
    time_t timer = timestamp;
    if (::localtime_r(&timer, &tp) == nullptr) {
        LOG(HAL, Fatal, L"localtime_r() failed with errno: {0}", FErrno{});
        return;
    }

    year = 1900 + checked_cast<u32>(tp.tm_year);
    month = 1 + checked_cast<u32>(tp.tm_mon);
    dayOfWeek = checked_cast<u32>(tp.tm_wday);
    dayOfYear = checked_cast<u32>(tp.tm_yday);
    dayOfMon = checked_cast<u32>(tp.tm_mday);
    hour = checked_cast<u32>(tp.tm_hour);
    min = checked_cast<u32>(tp.tm_min);
    sec = checked_cast<u32>(tp.tm_sec);
}
//----------------------------------------------------------------------------
void FLinuxPlatformTime::UtcTime(i64 timestamp, u32& year, u32& month, u32& dayOfWeek, u32& dayOfYear, u32& dayOfMon, u32& hour, u32& min, u32& sec) NOEXCEPT {
    struct ::tm tp = {};
    time_t timer = timestamp;
    if (::gmtime_r(&timer, &tp)) {
        LOG(HAL, Fatal, L"gmtime_r() failed with errno: {0}", FErrno{});
        return;
    }

    year = 1900 + checked_cast<u32>(tp.tm_year);
    month = 1 + checked_cast<u32>(tp.tm_mon);
    dayOfWeek = checked_cast<u32>(tp.tm_wday);
    dayOfYear = checked_cast<u32>(tp.tm_yday);
    dayOfMon = checked_cast<u32>(tp.tm_mday);
    hour = checked_cast<u32>(tp.tm_hour);
    min = checked_cast<u32>(tp.tm_min);
    sec = checked_cast<u32>(tp.tm_sec);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_LINUX
