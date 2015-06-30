#pragma once

#include "Core/Core.h"

#include "Core/Maths/Units.h"

// - /!\ -
// Chrono has no real high precision clock on VS2013 (and since VS2010)
// Then I am rolling my own simple time helpers
// - /!\ -
//#include <chrono>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
typedef Units::Time::Microseconds Timespan;
//----------------------------------------------------------------------------
class Timepoint {
public:
    typedef u64 value_type;

    Timepoint() : _value(0) {}
    Timepoint(value_type value) : _value(value) {}
    ~Timepoint() {}

    Timepoint(const Timepoint& other) : _value(other._value) {}
    Timepoint& operator =(const Timepoint& other) { _value = other._value; return *this; }

    value_type Value() const { return _value; }
    void SetValue(value_type value) { _value = value; }

    Timepoint operator +(value_type value) { return _value + value; }
    Timepoint operator -(value_type value) { Assert(_value >= value); return _value - value; }

    Timepoint operator +(const Timespan& duration) { return operator +(Ticks(duration)); }
    Timepoint operator -(const Timespan& duration) { return operator -(Ticks(duration)); }

    bool operator ==(const Timepoint& other) const { return _value == other._value; }
    bool operator !=(const Timepoint& other) const { return _value != other._value; }

    bool operator <(const Timepoint& other) const  { return _value <  other._value; }
    bool operator >=(const Timepoint& other) const { return _value >= other._value; }

    bool operator >(const Timepoint& other) const  { return _value >  other._value; }
    bool operator <=(const Timepoint& other) const { return _value <= other._value; }

    static Timepoint Now();
    static value_type Ticks(const Timespan& duration);
    static Timespan Duration(const Timepoint& start, const Timepoint& stop);

private:
    value_type _value;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
