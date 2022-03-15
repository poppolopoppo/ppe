#include "stdafx.h"

#include "HAL/Windows/WindowsPlatformTime.h"

#ifdef PLATFORM_WINDOWS

#   include <time.h>

#define USE_PPE_WINDOWS_HIGHRESTIMER 1
#if USE_PPE_WINDOWS_HIGHRESTIMER
//  Set granularity of sleep and such to 1 ms.
#   include <timeapi.h> // timeBeginPeriod/timeEndPeriod()
#   pragma comment(lib, "winmm.lib")
#endif

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const double FWindowsPlatformTime::GSecondsPerCycle = FWindowsPlatformTime::SecondsPerCycle();
//----------------------------------------------------------------------------
i64 FWindowsPlatformTime::Timestamp() NOEXCEPT {
    STATIC_ASSERT(std::is_same_v<i64, ::__time64_t>);
    return ::_time64(nullptr);
}
//----------------------------------------------------------------------------
u64 FWindowsPlatformTime::NetworkTime() NOEXCEPT {
    ::FILETIME ft{};
    ::GetSystemTimePreciseAsFileTime(&ft);
    return *((const u64*)& ft);
}
//----------------------------------------------------------------------------
void FWindowsPlatformTime::LocalTime(i64 timestamp, u32& year, u32& month, u32& dayOfWeek, u32& dayOfYear, u32& dayOfMon, u32& hour, u32& min, u32& sec) NOEXCEPT {
    struct ::tm lc {};
    Verify(0 == ::_localtime64_s(&lc, reinterpret_cast<const ::__time64_t*>(&timestamp)));

    year = 1900 + checked_cast<u32>(lc.tm_year);
    month = 1 + checked_cast<u32>(lc.tm_mon);
    dayOfWeek = checked_cast<u32>(lc.tm_wday);
    dayOfYear = checked_cast<u32>(lc.tm_yday);
    dayOfMon = checked_cast<u32>(lc.tm_mday);
    hour = checked_cast<u32>(lc.tm_hour);
    min = checked_cast<u32>(lc.tm_min);
    sec = checked_cast<u32>(lc.tm_sec);
}
//----------------------------------------------------------------------------
void FWindowsPlatformTime::UtcTime(i64 timestamp, u32& year, u32& month, u32& dayOfWeek, u32& dayOfYear, u32& dayOfMon, u32& hour, u32& min, u32& sec) NOEXCEPT {
    struct ::tm lc{};
    Verify(0 == ::_gmtime64_s(&lc, reinterpret_cast<const ::__time64_t*>(&timestamp)));

    year = 1900 + checked_cast<u32>(lc.tm_year);
    month = 1 + checked_cast<u32>(lc.tm_mon);
    dayOfWeek = checked_cast<u32>(lc.tm_wday);
    dayOfYear = checked_cast<u32>(lc.tm_yday);
    dayOfMon = checked_cast<u32>(lc.tm_mday);
    hour = checked_cast<u32>(lc.tm_hour);
    min = checked_cast<u32>(lc.tm_min);
    sec = checked_cast<u32>(lc.tm_sec);
}
//----------------------------------------------------------------------------
// https://docs.microsoft.com/en-us/windows/desktop/api/timeapi/nf-timeapi-timebeginperiod
void FWindowsPlatformTime::EnterHighResolutionTimer() {
#if USE_PPE_WINDOWS_HIGHRESTIMER
    Verify(TIMERR_NOERROR == ::timeBeginPeriod(1)); // set timer resolution to 1ms (minimum value)
#endif
}
void FWindowsPlatformTime::LeaveLowResolutionTimer() {
#if USE_PPE_WINDOWS_HIGHRESTIMER
    Verify(TIMERR_NOERROR == ::timeEndPeriod(1)); // unset timer resolution from 1ms (minimum value)
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_WINDOWS
