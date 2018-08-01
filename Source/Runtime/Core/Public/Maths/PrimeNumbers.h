#pragma once

#include "Core.h"

#include "IO/TextWriter_fwd.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
extern PPE_API const u16 GPrimeNumbersU16[6542];
//----------------------------------------------------------------------------
PPE_API u16 ClosestPrimeU16Ceil(u16 val);
PPE_API u16 ClosestPrimeU16Floor(u16 val);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Prime number product :
// Leverage prime numbers factorization properties
// Enables to test efficiently inheritance for instance
//----------------------------------------------------------------------------
template <typename _Tag>
class TPrimeNumberProduct {
public:
    TPrimeNumberProduct() : TPrimeNumberProduct(0) {}

    TPrimeNumberProduct(const TPrimeNumberProduct& other) : TPrimeNumberProduct(other._value) {}
    TPrimeNumberProduct& operator =(const TPrimeNumberProduct& other) { _value = other._value; return (*this); }

    u64 Value() const { return _value; }

    bool Contains(const TPrimeNumberProduct& parent) const {
        Assert(_value != 0);
        Assert(parent._value != 0);
        return ((_value / parent._value) * parent._value == _value);
    }

    inline friend void swap(TPrimeNumberProduct& lhs, TPrimeNumberProduct& rhs) {
        std::swap(lhs._value, rhs._value);
    }

    inline friend bool operator ==(TPrimeNumberProduct lhs, TPrimeNumberProduct rhs) {
        return (lhs._value == rhs._value);
    }

    inline friend bool operator !=(TPrimeNumberProduct lhs, TPrimeNumberProduct rhs) {
        return (lhs._value != rhs._value);
    }

    static TPrimeNumberProduct Prime(size_t index) {
        Assert(index < lengthof(GPrimeNumbersU16));
        return TPrimeNumberProduct(u64(GPrimeNumbersU16[index]));
    }

    static TPrimeNumberProduct Combine(TPrimeNumberProduct lhs, TPrimeNumberProduct rhs) {
        Assert(lhs._value);
        Assert(rhs._value);
        return TPrimeNumberProduct(lhs._value * rhs._value);
    }

private:
    explicit TPrimeNumberProduct(u64 value) : _value(value) {}

    u64 _value;
};
//----------------------------------------------------------------------------
template <typename _Char, typename _Tag>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const TPrimeNumberProduct<_Tag>& number) {
    return oss << number.Value();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
