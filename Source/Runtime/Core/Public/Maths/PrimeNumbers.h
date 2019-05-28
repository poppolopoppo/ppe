#pragma once

#include "Core.h"

#include "IO/TextWriter_fwd.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
constexpr u32 GPrimeNumbersU32Max = 1000000;
//----------------------------------------------------------------------------
extern PPE_CORE_API const u16 GPrimeNumbersU16[6542];
extern PPE_CORE_API const u32 GPrimeNumbersU32[78498];
//----------------------------------------------------------------------------
struct FPrimeNumberU16 {
using type = u16;
static PPE_CORE_API const u16 Values[6542];
static PPE_CORE_API u16 ClosestCeil(u16 v);
static PPE_CORE_API u16 ClosestFloor(u16 v);
};
//----------------------------------------------------------------------------
struct FPrimeNumberU32 {
using type = u32;
static PPE_CORE_API const u32 Values[78498];
static PPE_CORE_API u32 ClosestCeil(u32 v);
static PPE_CORE_API u32 ClosestFloor(u32 v);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Prime number product :
// Leverage prime numbers factorization properties
// Enables to test efficiently inheritance for instance
//----------------------------------------------------------------------------
template <typename _Tag, bool _Large = false>
class TPrimeNumberProduct {
public:
    using FPrimeNumber = Meta::TConditional<_Large, FPrimeNumberU16, FPrimeNumberU32>;
    using value_type = u64;

    CONSTEXPR TPrimeNumberProduct() : TPrimeNumberProduct(1) {}

    CONSTEXPR TPrimeNumberProduct(const TPrimeNumberProduct& other) : TPrimeNumberProduct(other._value) {}
    CONSTEXPR TPrimeNumberProduct& operator =(const TPrimeNumberProduct& other) { _value = other._value; return (*this); }

    CONSTEXPR value_type Value() const { return _value; }

    CONSTEXPR bool Contains(const TPrimeNumberProduct& parent) const {
        return ((_value / parent._value) * parent._value == _value);
    }

    inline friend void swap(TPrimeNumberProduct& lhs, TPrimeNumberProduct& rhs) {
        std::swap(lhs._value, rhs._value);
    }

    CONSTEXPR inline friend bool operator ==(TPrimeNumberProduct lhs, TPrimeNumberProduct rhs) {
        return (lhs._value == rhs._value);
    }

    CONSTEXPR inline friend bool operator !=(TPrimeNumberProduct lhs, TPrimeNumberProduct rhs) {
        return (lhs._value != rhs._value);
    }

    static TPrimeNumberProduct Prime(size_t index) {
        Assert(index < lengthof(FPrimeNumber::Values));
        return TPrimeNumberProduct(value_type(FPrimeNumber::Values[index]));
    }

    static TPrimeNumberProduct Combine(TPrimeNumberProduct lhs, TPrimeNumberProduct rhs) {
        Assert(lhs._value);
        Assert(rhs._value);
        return TPrimeNumberProduct(lhs._value * rhs._value);
    }

private:
    explicit TPrimeNumberProduct(value_type value) : _value(value) {}

    value_type _value;
};
//----------------------------------------------------------------------------
PPE_ASSUME_TEMPLATE_AS_POD(
    COMMA_PROTECT(TPrimeNumberProduct<_Tag, _Large>),
    typename _Tag, bool _Large)
//----------------------------------------------------------------------------
template <typename _Char, typename _Tag>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const TPrimeNumberProduct<_Tag>& number) {
    return oss << number.Value();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
