#pragma once

#include "Time_fwd.h"

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

    NODISCARD PPE_CORE_API FTimestamp Timestamp() const;

    NODISCARD static PPE_CORE_API FTimepoint Now();
    NODISCARD static PPE_CORE_API value_type Ticks(const FTimespan& duration);
    NODISCARD static PPE_CORE_API FTimespan Duration(const FTimepoint& start, const FTimepoint& stop);
    NODISCARD static PPE_CORE_API FTimespan SignedDuration(const FTimepoint& start, const FTimepoint& stop);
    NODISCARD static FTimespan ElapsedSince(const FTimepoint& t) { return Duration(t, Now()); }

    NODISCARD friend FTimespan operator -(const FTimepoint& finish, const FTimepoint& start) {
        return Duration(start, finish);
    }

private:
    value_type _value;
};
//----------------------------------------------------------------------------
CONSTEXPR FTimespan operator "" _hz(unsigned long long value) {
    Assert(value > 0);
    return (1000.0 / value);
}
//----------------------------------------------------------------------------
inline CONSTEXPR FTimespan Timespan_120hz{ 120_hz };
inline CONSTEXPR FTimespan Timespan_60hz{ 60_hz };
inline CONSTEXPR FTimespan Timespan_30hz{ 30_hz };
inline CONSTEXPR FTimespan Timespan_15hz{ 15_hz };
inline CONSTEXPR FTimespan Timespan_5hz{ 5_hz };
inline CONSTEXPR FTimespan Timespan_1hz{ 1_hz };
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
