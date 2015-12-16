#pragma once

#include <type_traits>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace StronglyTyped {
//----------------------------------------------------------------------------
template <typename T, typename _Tag, T _DefaultValue = T() >
struct Numeric {
    static_assert(std::is_arithmetic<T>::value, "T must be an arithmetic type");

    typedef T value_type;
    typedef _Tag tag_type;

    STATIC_CONST_INTEGRAL(T, DefaultValue, _DefaultValue);

    T Value;

    explicit Numeric(T value = _DefaultValue) : Value(value) {}
    operator T () const { return Value; }

    Numeric(const Numeric& other) : Value(other.Value) {}
    Numeric& operator =(const Numeric& other) { Value =other.Value; return *this; }

    friend bool operator ==(const Numeric& lhs, const Numeric& rhs) { return lhs.Value == rhs.Value; }
    friend bool operator !=(const Numeric& lhs, const Numeric& rhs) { return lhs.Value != rhs.Value; }

    friend bool operator < (const Numeric& lhs, const Numeric& rhs) { return lhs.Value <  rhs.Value; }
    friend bool operator >=(const Numeric& lhs, const Numeric& rhs) { return lhs.Value >= rhs.Value; }

    friend bool operator > (const Numeric& lhs, const Numeric& rhs) { return lhs.Value >  rhs.Value; }
    friend bool operator <=(const Numeric& lhs, const Numeric& rhs) { return lhs.Value <= rhs.Value; }

    friend void swap(Numeric& lhs, Numeric& rhs) { std::swap(lhs.Value, rhs.Value); }

    static Numeric MinusOne() { return Numeric(T(-1)); }
    static Numeric One() { return Numeric(T(1)); }
    static Numeric Zero() { return Numeric(T(0)); }
};
//----------------------------------------------------------------------------
} //!namespace StronglyTyped
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define CORE_STRONGLYTYPED_NUMERIC_DEFAULTVALUE_DEF(T, _Name, _DefaultValue) \
    namespace StronglyTyped { struct _Name {}; } \
    typedef ::Core::StronglyTyped::Numeric<T, StronglyTyped::_Name, _DefaultValue > _Name
//----------------------------------------------------------------------------
#define CORE_STRONGLYTYPED_NUMERIC_DEF(T, _Name) \
    CORE_STRONGLYTYPED_NUMERIC_DEFAULTVALUE_DEF(T, _Name, ((T)0))
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct fake_bool {
    bool Value;
    fake_bool() = default;
    fake_bool(const fake_bool& other) = default;
    fake_bool& operator =(const fake_bool& other) = default;
    fake_bool(bool value) : Value(value) {}
    fake_bool& operator =(bool value) { Value = value; return *this; }
    operator bool () const { return Value; }
    friend void swap(fake_bool& lhs, fake_bool& rhs) { std::swap(lhs.Value, rhs.Value); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
