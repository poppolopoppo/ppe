#pragma once

#include "Core/Core.h"

#include "Core/Memory/HashFunctions.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class DateTime;
class Timestamp {
public:
    typedef i64 value_type;

    Timestamp() : _value(0) {}
    Timestamp(value_type value) : _value(value) {}

    Timestamp(const Timestamp& other) : _value(other._value) {}
    Timestamp& operator =(const Timestamp& other) { _value = other._value; return *this; }

    value_type Value() const { return _value; }
    void SetValue(value_type value) { _value = value; }

    DateTime ToDateTime() const;
    DateTime ToDateTimeUTC() const;

    static Timestamp Now();

    friend hash_t hash_value(const Timestamp& t) { return hash_as_pod(t._value); }
    friend void swap(Timestamp& lhs, Timestamp& rhs) { std::swap(lhs._value, rhs._value); }

    friend bool operator ==(const Timestamp& lhs, const Timestamp& rhs) { return lhs._value == rhs._value; }
    friend bool operator !=(const Timestamp& lhs, const Timestamp& rhs) { return lhs._value != rhs._value; }

    friend bool operator < (const Timestamp& lhs, const Timestamp& rhs) { return lhs._value <  rhs._value; }
    friend bool operator >=(const Timestamp& lhs, const Timestamp& rhs) { return lhs._value >= rhs._value; }

private:
    value_type _value;
};
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(std::basic_ostream<_Char, _Traits>& oss, const Timestamp& t) {
    return oss << t.ToDateTime();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
