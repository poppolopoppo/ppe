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

    static constexpr T DefaultValue() {
        return tag_type::DefaultValue();
    }

    T Value;

    explicit TNumeric(T value = DefaultValue()) : Value(value) {
        STATIC_ASSERT(std::is_arithmetic_v<T>);
    }

    operator T () const { return Value; }

    TNumeric(const TNumeric& other) : Value(other.Value) {}
    TNumeric& operator =(const TNumeric& other) { Value =other.Value; return *this; }

    bool IsDefaultValue() const { return (DefaultValue() == Value); }

    friend bool operator ==(const TNumeric& lhs, const TNumeric& rhs) { return lhs.Value == rhs.Value; }
    friend bool operator !=(const TNumeric& lhs, const TNumeric& rhs) { return lhs.Value != rhs.Value; }

    friend bool operator < (const TNumeric& lhs, const TNumeric& rhs) { return lhs.Value <  rhs.Value; }
    friend bool operator >=(const TNumeric& lhs, const TNumeric& rhs) { return lhs.Value >= rhs.Value; }

    friend bool operator > (const TNumeric& lhs, const TNumeric& rhs) { return lhs.Value >  rhs.Value; }
    friend bool operator <=(const TNumeric& lhs, const TNumeric& rhs) { return lhs.Value <= rhs.Value; }

    friend void swap(TNumeric& lhs, TNumeric& rhs) { std::swap(lhs.Value, rhs.Value); }
    friend inline hash_t hash_value(const TNumeric& value) { return hash_as_pod(value.Value); }

    static TNumeric MinusOne() { return TNumeric(T(-1)); }
    static TNumeric One() { return TNumeric(T(1)); }
    static TNumeric Zero() { return TNumeric(T(0)); }
};
//----------------------------------------------------------------------------
template <typename T, typename _Tag, T _DefaultValue, typename = Meta::TEnableIf<std::is_integral_v<T>/* C++ forbid float as template arg */> >
struct TNumericDefault : public TNumeric<T, TNumericDefault<T, _Tag, _DefaultValue> > {
    static constexpr T DefaultValue() { return (_DefaultValue); }
    using parent_type = TNumeric<T, TNumericDefault>;
    using parent_type::DefaultValue;
    using parent_type::parent_type;
    using parent_type::operator T;
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
    using _Name = ::Core::TNumeric<T, StronglyTyped::_Name>
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
