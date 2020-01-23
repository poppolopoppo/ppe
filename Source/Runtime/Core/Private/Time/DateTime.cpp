#include "stdafx.h"

#include "Time/DateTime.h"

#include "HAL/PlatformTime.h"
#include "IO/TextWriter.h"
#include "Time/Timestamp.h"

#include <time.h>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FDateTime::Set(u32 year, u32 mon, u32 wday, u32 yday, u32 mday, u32 hour, u32 min, u32 sec) NOEXCEPT {
    Year = year;
    Month = mon;
    DayOfWeek = wday;
    DayOfYear = yday;
    Day = mday;
    Hours = hour;
    Minutes = min;
    Seconds = sec;

    Assert_NoAssume(Year == year);
    Assert_NoAssume(Month == mon);
    Assert_NoAssume(DayOfYear == yday);
    Assert_NoAssume(DayOfWeek == wday);
    Assert_NoAssume(Day == mday);
    Assert_NoAssume(Hours == hour);
    Assert_NoAssume(Minutes == min);
    Assert_NoAssume(Seconds == sec);
}
//----------------------------------------------------------------------------
FDateTime FDateTime::Now() {
    return FromLocalTime(FTimestamp::Now());
}
//----------------------------------------------------------------------------
FDateTime FDateTime::FromLocalTime(const FTimestamp& t) {
    u32 year, mon, wday, yday, mday, hour, min, sec;
    FPlatformTime::LocalTime(t.Value(), year, mon, wday, yday, mday, hour, min, sec);
    return FDateTime{ year, mon, wday, yday, mday, hour, min, sec };
}
//----------------------------------------------------------------------------
FDateTime FDateTime::FromTimeUTC(const FTimestamp& t) {
    u32 year, mon, wday, yday, mday, hour, min, sec;
    FPlatformTime::UtcTime(t.Value(), year, mon, wday, yday, mday, hour, min, sec);
    return FDateTime{ year, mon, wday, yday, mday, hour, min, sec };
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
