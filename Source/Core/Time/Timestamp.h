#pragma once

#include "Core/Core.h"

#include "Core/Memory/HashFunctions.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FDateTime;
class FTimestamp {
public:
    typedef i64 value_type;

    FTimestamp() : _value(0) {}
    FTimestamp(value_type value) : _value(value) {}

    FTimestamp(const FTimestamp& other) : _value(other._value) {}
    FTimestamp& operator =(const FTimestamp& other) { _value = other._value; return *this; }

    value_type Value() const { return _value; }
    void SetValue(value_type value) { _value = value; }

    FDateTime ToDateTime() const;
    FDateTime ToDateTimeUTC() const;

    static FTimestamp Now();

    friend hash_t hash_value(const FTimestamp& t) { return hash_as_pod(t._value); }
    friend void swap(FTimestamp& lhs, FTimestamp& rhs) { std::swap(lhs._value, rhs._value); }

    friend bool operator ==(const FTimestamp& lhs, const FTimestamp& rhs) { return lhs._value == rhs._value; }
    friend bool operator !=(const FTimestamp& lhs, const FTimestamp& rhs) { return lhs._value != rhs._value; }

    friend bool operator < (const FTimestamp& lhs, const FTimestamp& rhs) { return lhs._value <  rhs._value; }
    friend bool operator >=(const FTimestamp& lhs, const FTimestamp& rhs) { return lhs._value >= rhs._value; }

private:
    value_type _value;
};
CORE_ASSUME_TYPE_AS_POD(FTimestamp)
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(std::basic_ostream<_Char, _Traits>& oss, const FTimestamp& t) {
    return oss << t.ToDateTime();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
