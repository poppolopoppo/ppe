#pragma once

#include "Core.h"

#include "Maths/Units.h"

// Use FTimepoint only to measure durations, if you want a true date use FTimestamp/FDateTime instead

// - /!\ -
// Chrono has no real high precision clock on VS2013 (and since VS2010)
// Then I am rolling my own simple time helpers
// - /!\ -
//#include <chrono>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
typedef Units::Time::FMilliseconds FTimespan;
//----------------------------------------------------------------------------
class FTimepoint {
public:
    typedef i64 value_type;

    FTimepoint() : _value(0) {}
    FTimepoint(value_type value) : _value(value) {}

    FTimepoint(const FTimepoint& other) : _value(other._value) {}
    FTimepoint& operator =(const FTimepoint& other) { _value = other._value; return *this; }

    value_type Value() const { return _value; }
    void SetValue(value_type value) { _value = value; }

    value_type operator *() const { return _value; }

    FTimepoint operator +(value_type value) { return _value + value; }
    FTimepoint operator -(value_type value) { Assert(_value >= value); return _value - value; }

    FTimepoint operator +(const FTimespan& duration) { return operator +(Ticks(duration)); }
    FTimepoint operator -(const FTimespan& duration) { return operator -(Ticks(duration)); }

    bool operator ==(const FTimepoint& other) const { return _value == other._value; }
    bool operator !=(const FTimepoint& other) const { return _value != other._value; }

    bool operator <(const FTimepoint& other) const  { return _value <  other._value; }
    bool operator >=(const FTimepoint& other) const { return _value >= other._value; }

    bool operator >(const FTimepoint& other) const  { return _value >  other._value; }
    bool operator <=(const FTimepoint& other) const { return _value <= other._value; }

    static PPE_CORE_API FTimepoint Now();
    static PPE_CORE_API value_type Ticks(const FTimespan& duration);
    static PPE_CORE_API FTimespan Duration(const FTimepoint& start, const FTimepoint& stop);
    static FTimespan ElapsedSince(const FTimepoint& t) { return Duration(t, Now()); }

private:
    value_type _value;
};
PPE_ASSUME_TYPE_AS_POD(FTimepoint)
//----------------------------------------------------------------------------
CONSTEXPR FTimespan::value_type operator "" _hz(unsigned long long value) {
    Assert(value > 0);
    return Units::ConvertValue<FTimespan, Units::Time::FSeconds>(1.0 / value);
}
//----------------------------------------------------------------------------
constexpr FTimespan Timespan_120hz{ 120_hz };
constexpr FTimespan Timespan_60hz{ 60_hz };
constexpr FTimespan Timespan_30hz{ 30_hz };
constexpr FTimespan Timespan_15hz{ 15_hz };
constexpr FTimespan Timespan_5hz{ 5_hz };
constexpr FTimespan Timespan_1hz{ 1_hz };
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
