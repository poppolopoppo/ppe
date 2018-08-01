#include "stdafx.h"

#include "WindowsPlatformTime.h"

#ifdef PLATFORM_WINDOWS

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static double FetchSecondsPerCycle_() {
    ::LARGE_INTEGER frequency;
    Verify(::QueryPerformanceFrequency(&frequency));
    return (1.0 / frequency.QuadPart);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const double FWindowsPlatformTime::GSecondsPerCycle = FetchSecondsPerCycle_();
//----------------------------------------------------------------------------
void FWindowsPlatformTime::SystemTime(u32& year, u32& month, u32& dayOfWeek, u32& day, u32& hour, u32& min, u32& sec, u32& msec) {
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
void FWindowsPlatformTime::UtcTime(u32& year, u32& month, u32& dayOfWeek, u32& day, u32& hour, u32& min, u32& sec, u32& msec) {
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
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_WINDOWS
