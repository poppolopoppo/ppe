#pragma once

#include <type_traits>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace StronglyTyped {
//----------------------------------------------------------------------------
template <typename T, typename _Tag, T _DefaultValue = T() >
struct TNumeric {
    static_assert(std::is_arithmetic<T>::value, "T must be an arithmetic type");

    typedef T value_type;
    typedef _Tag tag_type;

    STATIC_CONST_INTEGRAL(T, DefaultValue, _DefaultValue);

    T Value;

    explicit TNumeric(T value = _DefaultValue) : Value(value) {}
    operator T () const { return Value; }

    TNumeric(const TNumeric& other) : Value(other.Value) {}
    TNumeric& operator =(const TNumeric& other) { Value =other.Value; return *this; }

    bool IsDefaultValue() const { return (_DefaultValue == Value); }

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
} //!namespace StronglyTyped
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define CORE_STRONGLYTYPED_NUMERIC_DEFAULTVALUE_DEF(T, _Name, _DefaultValue) \
    namespace StronglyTyped { struct _Name {}; } \
    typedef ::Core::StronglyTyped::TNumeric<T, StronglyTyped::_Name, _DefaultValue > _Name
//----------------------------------------------------------------------------
#define CORE_STRONGLYTYPED_NUMERIC_DEF(T, _Name) \
    CORE_STRONGLYTYPED_NUMERIC_DEFAULTVALUE_DEF(T, _Name, ((T)0))
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
#define CORE_FAKEBOOL_OPERATOR_DECL() operator const void* () const
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
