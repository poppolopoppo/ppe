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

    FTimestamp() : _value(0) {}
    FTimestamp(value_type value) : _value(value) {}

    FTimestamp(const FTimestamp& other) : _value(other._value) {}
    FTimestamp& operator =(const FTimestamp& other) { _value = other._value; return *this; }

    value_type Value() const { return _value; }
    void SetValue(value_type value) { _value = value; }

    FTimestamp SecondsAgo(value_type seconds) const {
        Assert_NoAssume(_value > seconds);
        return FTimestamp{ _value - seconds };
    }
    FTimestamp SecondsLater(value_type seconds) const {
        Assert_NoAssume(INT64_MAX - seconds >= _value);
        return FTimestamp{ _value + seconds };
    }

    FTimestamp MinutesAgo(value_type minutes) const { return SecondsAgo(minutes * 60); }
    FTimestamp MinutesLater(value_type minutes) const { return SecondsLater(minutes * 60); }

    FTimestamp HoursAgo(value_type hours) const { return SecondsAgo(hours * 3600); }
    FTimestamp HoursLater(value_type hours) const { return SecondsLater(hours * 3600); }

    FTimestamp DaysAgo(value_type days) const { return SecondsAgo(days * 3600 * 24); }
    FTimestamp DaysLater(value_type days) const { return SecondsLater(days * 3600 * 24); }

    FDateTime ToDateTime() const;
    FDateTime ToDateTimeUTC() const;

    size_t HashValue() const;

    static FTimestamp Now();

    friend hash_t hash_value(const FTimestamp& t) { return t.HashValue(); }
    friend void swap(FTimestamp& lhs, FTimestamp& rhs) { std::swap(lhs._value, rhs._value); }

    friend bool operator ==(const FTimestamp& lhs, const FTimestamp& rhs) { return lhs._value == rhs._value; }
    friend bool operator !=(const FTimestamp& lhs, const FTimestamp& rhs) { return lhs._value != rhs._value; }

    friend bool operator < (const FTimestamp& lhs, const FTimestamp& rhs) { return lhs._value <  rhs._value; }
    friend bool operator >=(const FTimestamp& lhs, const FTimestamp& rhs) { return lhs._value >= rhs._value; }

    friend bool operator <=(const FTimestamp& lhs, const FTimestamp& rhs) { return lhs._value <= rhs._value; }
    friend bool operator > (const FTimestamp& lhs, const FTimestamp& rhs) { return lhs._value >  rhs._value; }

private:
    value_type _value;
};
PPE_ASSUME_TYPE_AS_POD(FTimestamp)
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const FTimestamp& t) {
    return oss << t.ToDateTime();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
