#pragma once

#include "Core.h"

#include "IO/TextWriter_fwd.h"
#include "Memory/HashFunctions.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FTimestamp;
class PPE_CORE_API FDateTime {
public:
    u64 DayOfWeek   : 3;
    u64 DayOfYear   : 9;

    u64 Day         : 5;
    u64 Month       : 4;
    u64 Year        : 26;

    u64 Hours       : 5;
    u64 Minutes     : 6;
    u64 Seconds     : 6;

    FDateTime() NOEXCEPT : FDateTime(0) {}

    CONSTEXPR FDateTime(u32 year, u32 mon, u32 wday, u32 yday, u32 mday, u32 hour, u32 min, u32 sec) NOEXCEPT
    :    DayOfWeek(wday)
    ,    DayOfYear(yday)
    ,    Day(mday)
    ,    Month(mon)
    ,    Year(year)
    ,    Hours(hour)
    ,    Minutes(min)
    ,    Seconds(sec) {
        Assert_NoAssume(Year == year);
        Assert_NoAssume(Month == mon);
        Assert_NoAssume(DayOfYear == yday);
        Assert_NoAssume(DayOfWeek == wday);
        Assert_NoAssume(Day == mday);
        Assert_NoAssume(Hours == hour);
        Assert_NoAssume(Minutes == min);
        Assert_NoAssume(Seconds == sec);
    }

    explicit FDateTime(u64 ord) NOEXCEPT { *reinterpret_cast<u64*>(this) = ord; }
    FDateTime(const FDateTime& other) NOEXCEPT { *reinterpret_cast<u64*>(this) = other.Ord(); }
    FDateTime& operator =(const FDateTime& other) NOEXCEPT { *reinterpret_cast<u64*>(this) = other.Ord(); return *this; }

    u64 Ord() const { return *reinterpret_cast<const u64*>(this); }

    CONSTEXPR void Set(u32 year, u32 mon, u32 wday, u32 yday, u32 mday, u32 hour, u32 min, u32 sec) NOEXCEPT {
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

    static FDateTime Now();
    static FDateTime FromLocalTime(const FTimestamp& t);
    static FDateTime FromTimeUTC(const FTimestamp& t);

    friend hash_t hash_value(const FDateTime& d) NOEXCEPT { return hash_as_pod(d.Ord()); }
    friend void swap(FDateTime& lhs, FDateTime& rhs) NOEXCEPT { std::swap(*reinterpret_cast<u64*>(&lhs), *reinterpret_cast<u64*>(&rhs)); }

    friend bool operator ==(const FDateTime& lhs, const FDateTime& rhs) { return lhs.Ord() == rhs.Ord(); }
    friend bool operator !=(const FDateTime& lhs, const FDateTime& rhs) { return lhs.Ord() != rhs.Ord(); }
};
STATIC_ASSERT(sizeof(FDateTime) == sizeof(u64));
PPE_ASSUME_TYPE_AS_POD(FDateTime)
//----------------------------------------------------------------------------
PPE_CORE_API FTextWriter& operator <<(FTextWriter& oss, const FDateTime& d);
PPE_CORE_API FWTextWriter& operator <<(FWTextWriter& oss, const FDateTime& d);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
