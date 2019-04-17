#include "stdafx.h"

#include "HAL/Windows/WindowsPlatformTime.h"

#ifdef PLATFORM_WINDOWS

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
u64 FWindowsPlatformTime::NetworkTime() NOEXCEPT {
    ::FILETIME ft;
    ::GetSystemTimePreciseAsFileTime(&ft);
    return *((const u64*)& ft);
}
//----------------------------------------------------------------------------
void FWindowsPlatformTime::SystemTime(u32& year, u32& month, u32& dayOfWeek, u32& day, u32& hour, u32& min, u32& sec, u32& msec) NOEXCEPT {
    ::SYSTEMTIME st;
    ::GetLocalTime(&st);

    year = checked_cast<u32>(st.wYear);
    month = checked_cast<u32>(st.wMonth);
    dayOfWeek = checked_cast<u32>(st.wDayOfWeek);
    day = checked_cast<u32>(st.wDay);
    hour = checked_cast<u32>(st.wHour);
    min = checked_cast<u32>(st.wMinute);
    sec = checked_cast<u32>(st.wSecond);
    msec = checked_cast<u32>(st.wMilliseconds);
}
//----------------------------------------------------------------------------
void FWindowsPlatformTime::UtcTime(u32& year, u32& month, u32& dayOfWeek, u32& day, u32& hour, u32& min, u32& sec, u32& msec) NOEXCEPT {
    ::SYSTEMTIME st;
    ::GetSystemTime(&st);

    year = checked_cast<u32>(st.wYear);
    month = checked_cast<u32>(st.wMonth);
    dayOfWeek = checked_cast<u32>(st.wDayOfWeek);
    day = checked_cast<u32>(st.wDay);
    hour = checked_cast<u32>(st.wHour);
    min = checked_cast<u32>(st.wMinute);
    sec = checked_cast<u32>(st.wSecond);
    msec = checked_cast<u32>(st.wMilliseconds);
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
