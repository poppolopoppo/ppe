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

    static FTimepoint Now();
    static value_type Ticks(const FTimespan& duration);
    static FTimespan Duration(const FTimepoint& start, const FTimepoint& stop);
    static FTimespan ElapsedSince(const FTimepoint& t) { return Duration(t, Now()); }

private:
    value_type _value;
};
PPE_ASSUME_TYPE_AS_POD(FTimepoint)
//----------------------------------------------------------------------------
constexpr FTimespan::value_type Timespan_120hz() { return Units::ConvertValue<FTimespan, Units::Time::FSeconds>(1.0 / 120); }
constexpr FTimespan::value_type Timespan_60hz()  { return Units::ConvertValue<FTimespan, Units::Time::FSeconds>(1.0 /  60); }
constexpr FTimespan::value_type Timespan_30hz()  { return Units::ConvertValue<FTimespan, Units::Time::FSeconds>(1.0 /  30); }
constexpr FTimespan::value_type Timespan_15hz()  { return Units::ConvertValue<FTimespan, Units::Time::FSeconds>(1.0 /  15); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
