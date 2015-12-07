#pragma once

#include "Core/Core.h"

#include "Core/Memory/HashFunctions.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class Timestamp;
class DateTime {
public:
    DateTime() : DateTime(0) {}
    explicit DateTime(u64 ord) { *reinterpret_cast<u64*>(this) = ord; }

    DateTime(const DateTime& other) { *reinterpret_cast<u64*>(this) = other.Ord(); }
    DateTime& operator =(const DateTime& other) { *reinterpret_cast<u64*>(this) = other.Ord(); return *this; }

    u64 DayOfYear   : 9;
    u64 DayOfWeek   : 3;

    u64 Day         : 5;
    u64 Month       : 4;
    u64 Year        : 26;

    u64 Hours       : 5;
    u64 Minutes     : 6;
    u64 Seconds     : 6;

    u64 Ord() const { return *reinterpret_cast<const u64*>(this); }

    static DateTime Now();
    static DateTime FromLocalTime(const Timestamp& t);
    static DateTime FromTimeUTC(const Timestamp& t);

    friend hash_t hash_value(const DateTime& d) { return hash_as_pod(d.Ord()); }
    friend void swap(DateTime& lhs, DateTime& rhs) { std::swap(*reinterpret_cast<u64*>(&lhs), *reinterpret_cast<u64*>(&rhs)); }

    friend bool operator ==(const DateTime& lhs, const DateTime& rhs) { return lhs.Ord() == rhs.Ord(); }
    friend bool operator !=(const DateTime& lhs, const DateTime& rhs) { return lhs.Ord() != rhs.Ord(); }
};
STATIC_ASSERT(sizeof(DateTime) == sizeof(u64));
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(std::basic_ostream<_Char, _Traits>& oss, const DateTime& d) {
    const _Char f = oss.fill();
    const std::streamsize w = oss.width();
    return oss << std::setfill('0')
        << std::setw(4) << d.Year
        << std::setw(1) << "/"
        << std::setw(2) << d.Month
        << std::setw(1) << "/"
        << std::setw(2) << d.Day
        << std::setw(1) << " "
        << std::setw(2) << d.Hours
        << std::setw(1) << ":"
        << std::setw(2) << d.Minutes
        << std::setw(1) << ":"
        << std::setw(2) << d.Seconds
        << std::setfill(f) << std::setw(w);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
