#include "stdafx.h"

#include "Time/DateTime.h"

#include "Time/Timestamp.h"
#include "IO/TextWriter.h"

#include <time.h>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FDateTime FDateTime::Now() {
    return FromLocalTime(FTimestamp::Now());
}
//----------------------------------------------------------------------------
FDateTime FDateTime::FromLocalTime(const FTimestamp& t) {
    STATIC_ASSERT(sizeof(t) == sizeof(::__time64_t));
    FDateTime d;
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
FDateTime FDateTime::FromTimeUTC(const FTimestamp& t) {
    STATIC_ASSERT(sizeof(t) == sizeof(::__time64_t));
    FDateTime d;
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
FTextWriter& operator <<(FTextWriter& oss, const FDateTime& d) {
    return (*oss.FormatScope())
        << FTextFormat::PadLeft(4, '0') << d.Year
        << FTextFormat::PadLeft(1, '0') << '/'
        << FTextFormat::PadLeft(2, '0') << d.Month
        << FTextFormat::PadLeft(1, '0') << '/'
        << FTextFormat::PadLeft(2, '0') << d.Day
        << FTextFormat::PadLeft(1, '0') << ' '
        << FTextFormat::PadLeft(2, '0') << d.Hours
        << FTextFormat::PadLeft(1, '0') << ':'
        << FTextFormat::PadLeft(2, '0') << d.Minutes
        << FTextFormat::PadLeft(1, '0') << ':'
        << FTextFormat::PadLeft(2, '0') << d.Seconds;
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, const FDateTime& d) {
    return (*oss.FormatScope())
        << FTextFormat::PadLeft(4, L'0') << d.Year
        << FTextFormat::PadLeft(1, L'0') << L'/'
        << FTextFormat::PadLeft(2, L'0') << d.Month
        << FTextFormat::PadLeft(1, L'0') << L'/'
        << FTextFormat::PadLeft(2, L'0') << d.Day
        << FTextFormat::PadLeft(1, L'0') << L' '
        << FTextFormat::PadLeft(2, L'0') << d.Hours
        << FTextFormat::PadLeft(1, L'0') << L':'
        << FTextFormat::PadLeft(2, L'0') << d.Minutes
        << FTextFormat::PadLeft(1, L'0') << L':'
        << FTextFormat::PadLeft(2, L'0') << d.Seconds;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
