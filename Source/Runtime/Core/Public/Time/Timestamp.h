#pragma once

#include "Time_fwd.h"

#include "IO/TextWriter_fwd.h"
#include "Maths/Units.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FTimestamp {
public:
    typedef i64 value_type;

    CONSTEXPR FTimestamp() : _value(0) {}
    CONSTEXPR FTimestamp(value_type value) : _value(value) {}

    CONSTEXPR FTimestamp(const FTimestamp& other) : _value(other._value) {}
    CONSTEXPR FTimestamp& operator =(const FTimestamp& other) { _value = other._value; return *this; }

    NODISCARD CONSTEXPR value_type Value() const { return _value; }
    CONSTEXPR void SetValue(value_type value) { _value = value; }

    NODISCARD CONSTEXPR FTimestamp SecondsAgo(value_type seconds) const {
        Assert_NoAssume(_value > seconds);
        return FTimestamp{ _value - seconds };
    }
    NODISCARD CONSTEXPR FTimestamp SecondsLater(value_type seconds) const {
        Assert_NoAssume(INT64_MAX - seconds >= _value);
        return FTimestamp{ _value + seconds };
    }

    NODISCARD CONSTEXPR FTimestamp MinutesAgo(value_type minutes) const { return SecondsAgo(minutes * 60); }
    NODISCARD CONSTEXPR FTimestamp MinutesLater(value_type minutes) const { return SecondsLater(minutes * 60); }

    NODISCARD CONSTEXPR FTimestamp HoursAgo(value_type hours) const { return SecondsAgo(hours * 3600); }
    NODISCARD CONSTEXPR FTimestamp HoursLater(value_type hours) const { return SecondsLater(hours * 3600); }

    NODISCARD CONSTEXPR FTimestamp DaysAgo(value_type days) const { return SecondsAgo(days * 3600 * 24); }
    NODISCARD CONSTEXPR FTimestamp DaysLater(value_type days) const { return SecondsLater(days * 3600 * 24); }

    NODISCARD CONSTEXPR FTimestamp Ago(FTimespan span) const {
        return SecondsAgo(static_cast<value_type>(*Units::Time::FSeconds{ span }));
    }
    NODISCARD CONSTEXPR FTimestamp Later(FTimespan span) const {
        return SecondsLater(static_cast<value_type>(*Units::Time::FSeconds{ span }));
    }

    NODISCARD FDateTime ToDateTime() const;
    NODISCARD FDateTime ToDateTimeUTC() const;

    NODISCARD size_t HashValue() const;

    NODISCARD static FTimestamp Now();

    friend hash_t hash_value(const FTimestamp& t) NOEXCEPT { return t.HashValue(); }

    friend void swap(FTimestamp& lhs, FTimestamp& rhs) NOEXCEPT { std::swap(lhs._value, rhs._value); }

    NODISCARD CONSTEXPR friend bool operator ==(const FTimestamp& lhs, const FTimestamp& rhs) { return lhs._value == rhs._value; }
    NODISCARD CONSTEXPR friend bool operator !=(const FTimestamp& lhs, const FTimestamp& rhs) { return lhs._value != rhs._value; }

    NODISCARD CONSTEXPR friend bool operator < (const FTimestamp& lhs, const FTimestamp& rhs) { return lhs._value <  rhs._value; }
    NODISCARD CONSTEXPR friend bool operator >=(const FTimestamp& lhs, const FTimestamp& rhs) { return lhs._value >= rhs._value; }

    NODISCARD CONSTEXPR friend bool operator <=(const FTimestamp& lhs, const FTimestamp& rhs) { return lhs._value <= rhs._value; }
    NODISCARD CONSTEXPR friend bool operator > (const FTimestamp& lhs, const FTimestamp& rhs) { return lhs._value >  rhs._value; }

    NODISCARD CONSTEXPR friend FTimestamp operator +(FTimestamp stamp, FTimespan span) {
        return stamp.Later(span);
    }
    NODISCARD CONSTEXPR friend FTimestamp operator -(FTimestamp stamp, FTimespan span) {
        return stamp.Ago(span);
    }

private:
    value_type _value;
};
//----------------------------------------------------------------------------
PPE_CORE_API FTextWriter& operator <<(FTextWriter& oss, const FTimestamp& t);
PPE_CORE_API FWTextWriter& operator <<(FWTextWriter& oss, const FTimestamp& t);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
