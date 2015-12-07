#include "stdafx.h"

#include "DateTime.h"

#include "Timestamp.h"

#include <time.h>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DateTime DateTime::Now() {
    return FromLocalTime(Timestamp::Now());
}
//----------------------------------------------------------------------------
DateTime DateTime::FromLocalTime(const Timestamp& t) {
    STATIC_ASSERT(sizeof(t) == sizeof(::__time64_t));
    DateTime d;
    ::tm lc;
    const ::errno_t err = ::_localtime64_s(&lc, reinterpret_cast<const ::__time64_t*>(&t));
    AssertRelease(0 == err);
    d.DayOfWeek = lc.tm_wday;
    Assert(lc.tm_wday == d.DayOfWeek);
    d.DayOfYear = lc.tm_yday;
    Assert(lc.tm_yday == d.DayOfYear);
    d.Day = lc.tm_mday;
    Assert(lc.tm_mday == d.Day);
    d.Month = 1 + lc.tm_mon;
    Assert(lc.tm_mon + 1 == d.Month);
    d.Year = 1900 + lc.tm_year;
    Assert(lc.tm_year + 1900 == d.Year);
    d.Hours = lc.tm_hour;
    Assert(lc.tm_hour == d.Hours);
    d.Minutes = lc.tm_min;
    Assert(lc.tm_min == d.Minutes);
    d.Seconds = lc.tm_sec;
    Assert(lc.tm_sec == d.Seconds);
    return d;
}
//----------------------------------------------------------------------------
DateTime DateTime::FromTimeUTC(const Timestamp& t) {
    STATIC_ASSERT(sizeof(t) == sizeof(::__time64_t));
    DateTime d;
    ::tm lc;
    const ::errno_t err = ::_gmtime64_s(&lc, reinterpret_cast<const ::__time64_t*>(&t));
    AssertRelease(0 == err);
    d.DayOfWeek = lc.tm_wday;
    Assert(lc.tm_wday == d.DayOfWeek);
    d.DayOfYear = lc.tm_yday;
    Assert(lc.tm_yday == d.DayOfYear);
    d.Day = lc.tm_mday;
    Assert(lc.tm_mday == d.Day);
    d.Month = 1 + lc.tm_mon;
    Assert(lc.tm_mon + 1 == d.Month);
    d.Year = 1900 + lc.tm_year;
    Assert(lc.tm_year + 1900 == d.Year);
    d.Hours = lc.tm_hour;
    Assert(lc.tm_hour == d.Hours);
    d.Minutes = lc.tm_min;
    Assert(lc.tm_min == d.Minutes);
    d.Seconds = lc.tm_sec;
    Assert(lc.tm_sec == d.Seconds);
    return d;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core