#pragma once

#include "Core/Core.h"

#include "Core/Maths/Units.h"

// Use FTimepoint only to measure durations, if you want a true date use FTimestamp/FDateTime instead

// - /!\ -
// Chrono has no real high precision clock on VS2013 (and since VS2010)
// Then I am rolling my own simple time helpers
// - /!\ -
//#include <chrono>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
typedef Units::Time::Milliseconds Timespan;
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

    FTimepoint operator +(const Timespan& duration) { return operator +(Ticks(duration)); }
    FTimepoint operator -(const Timespan& duration) { return operator -(Ticks(duration)); }

    bool operator ==(const FTimepoint& other) const { return _value == other._value; }
    bool operator !=(const FTimepoint& other) const { return _value != other._value; }

    bool operator <(const FTimepoint& other) const  { return _value <  other._value; }
    bool operator >=(const FTimepoint& other) const { return _value >= other._value; }

    bool operator >(const FTimepoint& other) const  { return _value >  other._value; }
    bool operator <=(const FTimepoint& other) const { return _value <= other._value; }

    static FTimepoint Now();
    static value_type Ticks(const Timespan& duration);
    static Timespan Duration(const FTimepoint& start, const FTimepoint& stop);
    static Timespan ElapsedSince(const FTimepoint& t) { return Duration(t, Now()); }

private:
    value_type _value;
};
//----------------------------------------------------------------------------
constexpr Timespan::value_type Timespan_60hz() { return Units::ConvertValue<Timespan, Units::Time::Seconds>( 1.0/60 ); }
constexpr Timespan::value_type Timespan_30hz() { return Units::ConvertValue<Timespan, Units::Time::Seconds>( 1.0/30 ); }
constexpr Timespan::value_type Timespan_15hz() { return Units::ConvertValue<Timespan, Units::Time::Seconds>( 1.0/15 ); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
