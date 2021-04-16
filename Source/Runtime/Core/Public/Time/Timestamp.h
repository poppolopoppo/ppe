#pragma once

#include "Core.h"

#include "IO/TextWriter_fwd.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FDateTime;
class PPE_CORE_API FTimestamp {
public:
    typedef i64 value_type;

    CONSTEXPR FTimestamp() : _value(0) {}
    CONSTEXPR FTimestamp(value_type value) : _value(value) {}

    CONSTEXPR FTimestamp(const FTimestamp& other) : _value(other._value) {}
    CONSTEXPR FTimestamp& operator =(const FTimestamp& other) { _value = other._value; return *this; }

    CONSTEXPR value_type Value() const { return _value; }
    CONSTEXPR void SetValue(value_type value) { _value = value; }

    CONSTEXPR FTimestamp SecondsAgo(value_type seconds) const {
        Assert_NoAssume(_value > seconds);
        return FTimestamp{ _value - seconds };
    }
    CONSTEXPR FTimestamp SecondsLater(value_type seconds) const {
        Assert_NoAssume(INT64_MAX - seconds >= _value);
        return FTimestamp{ _value + seconds };
    }

    CONSTEXPR FTimestamp MinutesAgo(value_type minutes) const { return SecondsAgo(minutes * 60); }
    CONSTEXPR FTimestamp MinutesLater(value_type minutes) const { return SecondsLater(minutes * 60); }

    CONSTEXPR FTimestamp HoursAgo(value_type hours) const { return SecondsAgo(hours * 3600); }
    CONSTEXPR FTimestamp HoursLater(value_type hours) const { return SecondsLater(hours * 3600); }

    CONSTEXPR FTimestamp DaysAgo(value_type days) const { return SecondsAgo(days * 3600 * 24); }
    CONSTEXPR FTimestamp DaysLater(value_type days) const { return SecondsLater(days * 3600 * 24); }

    FDateTime ToDateTime() const;
    FDateTime ToDateTimeUTC() const;

    size_t HashValue() const;

    static FTimestamp Now();

    friend hash_t hash_value(const FTimestamp& t) NOEXCEPT { return t.HashValue(); }

    friend void swap(FTimestamp& lhs, FTimestamp& rhs) NOEXCEPT { std::swap(lhs._value, rhs._value); }

    CONSTEXPR friend bool operator ==(const FTimestamp& lhs, const FTimestamp& rhs) { return lhs._value == rhs._value; }
    CONSTEXPR friend bool operator !=(const FTimestamp& lhs, const FTimestamp& rhs) { return lhs._value != rhs._value; }

    CONSTEXPR friend bool operator < (const FTimestamp& lhs, const FTimestamp& rhs) { return lhs._value <  rhs._value; }
    CONSTEXPR friend bool operator >=(const FTimestamp& lhs, const FTimestamp& rhs) { return lhs._value >= rhs._value; }

    CONSTEXPR friend bool operator <=(const FTimestamp& lhs, const FTimestamp& rhs) { return lhs._value <= rhs._value; }
    CONSTEXPR friend bool operator > (const FTimestamp& lhs, const FTimestamp& rhs) { return lhs._value >  rhs._value; }

private:
    value_type _value;
};
PPE_ASSUME_TYPE_AS_POD(FTimestamp)
//----------------------------------------------------------------------------
PPE_CORE_API FTextWriter& operator <<(FTextWriter& oss, const FTimestamp& t);
PPE_CORE_API FWTextWriter& operator <<(FWTextWriter& oss, const FTimestamp& t);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
