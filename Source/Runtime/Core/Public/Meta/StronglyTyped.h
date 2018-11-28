#pragma once

#include <type_traits>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Tag>
struct TNumeric {
    typedef T value_type;
    typedef _Tag tag_type;

    static CONSTEXPR T DefaultValue() {
        return tag_type::DefaultValue();
    }

    T Value;

    CONSTEXPR explicit TNumeric(T value = DefaultValue()) : Value(value) {
        STATIC_ASSERT(std::is_arithmetic_v<T>);
    }

    CONSTEXPR operator T () const { return Value; }

    CONSTEXPR TNumeric(const TNumeric& other) : Value(other.Value) {}
    CONSTEXPR TNumeric& operator =(const TNumeric& other) { Value =other.Value; return *this; }

    CONSTEXPR bool IsDefaultValue() const { return (DefaultValue() == Value); }

    CONSTEXPR friend bool operator ==(const TNumeric& lhs, const TNumeric& rhs) { return lhs.Value == rhs.Value; }
    CONSTEXPR friend bool operator !=(const TNumeric& lhs, const TNumeric& rhs) { return lhs.Value != rhs.Value; }

    CONSTEXPR friend bool operator < (const TNumeric& lhs, const TNumeric& rhs) { return lhs.Value <  rhs.Value; }
    CONSTEXPR friend bool operator >=(const TNumeric& lhs, const TNumeric& rhs) { return lhs.Value >= rhs.Value; }

    CONSTEXPR friend bool operator > (const TNumeric& lhs, const TNumeric& rhs) { return lhs.Value >  rhs.Value; }
    CONSTEXPR friend bool operator <=(const TNumeric& lhs, const TNumeric& rhs) { return lhs.Value <= rhs.Value; }

    friend void swap(TNumeric& lhs, TNumeric& rhs) { std::swap(lhs.Value, rhs.Value); }
    friend inline hash_t hash_value(const TNumeric& value) { return hash_as_pod(value.Value); }

    CONSTEXPR static TNumeric MinusOne() { return TNumeric(T(-1)); }
    CONSTEXPR static TNumeric One() { return TNumeric(T(1)); }
    CONSTEXPR static TNumeric Zero() { return TNumeric(T(0)); }
};
//----------------------------------------------------------------------------
template <typename T, typename _Tag, T _DefaultValue, typename = Meta::TEnableIf<std::is_integral_v<T>/* C++ forbid float as template arg */> >
struct TNumericDefault : public TNumeric<T, TNumericDefault<T, _Tag, _DefaultValue> > {
    static CONSTEXPR T DefaultValue() { return (_DefaultValue); }
    using parent_type = TNumeric<T, TNumericDefault>;
    using typename parent_type::value_type;
    using parent_type::parent_type;
    using parent_type::operator T;
    using parent_type::DefaultValue;
    using parent_type::IsDefaultValue;
    using parent_type::MinusOne;
    using parent_type::One;
    using parent_type::Zero;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define PPE_STRONGLYTYPED_NUMERIC_DEFAULTVALUE_DEF(T, _Name, _DefaultValue) \
    namespace StronglyTyped { struct _Name { \
        static constexpr T DefaultValue() { return (_DefaultValue); } \
    }; } \
    using _Name = ::PPE::TNumeric<T, StronglyTyped::_Name>
//----------------------------------------------------------------------------
#define PPE_STRONGLYTYPED_NUMERIC_DEF(T, _Name) \
    PPE_STRONGLYTYPED_NUMERIC_DEFAULTVALUE_DEF(T, _Name, T{})
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FFakeBool {
    bool Value;
    FFakeBool() = default;
    FFakeBool(const FFakeBool& other) = default;
    FFakeBool& operator =(const FFakeBool& other) = default;
    FFakeBool(bool value) : Value(value) {}
    FFakeBool& operator =(bool value) { Value = value; return *this; }
    operator bool () const { return Value; }
    inline friend void swap(FFakeBool& lhs, FFakeBool& rhs) { std::swap(lhs.Value, rhs.Value); }
};
//----------------------------------------------------------------------------
#define PPE_FAKEBOOL_OPERATOR_DECL() operator const void* () const
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
