#pragma once

#include "Meta/Hash_fwd.h"

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

    CONSTEXPR TNumeric() : Value(DefaultValue()) {}
    CONSTEXPR explicit TNumeric(T value) : Value(value) {
        STATIC_ASSERT(
            std::is_arithmetic_v<T> ||
            std::is_same_v<T, void*> ||
            std::is_same_v<T, const void*> );
    }

    CONSTEXPR TNumeric(Meta::FUnsignedMax umax) : Value(umax) {
        STATIC_ASSERT(
            std::is_integral_v<T> &&
            std::is_unsigned_v<T> );
    }

    CONSTEXPR operator T () const { return Value; }

    CONSTEXPR T operator * () const { return Value; }
    CONSTEXPR T& operator * () NOEXCEPT { return Value; }

    CONSTEXPR TNumeric(const TNumeric& other) : Value(other.Value) {}
    CONSTEXPR TNumeric& operator =(const TNumeric& other) { Value =other.Value; return *this; }

    CONSTEXPR bool IsDefaultValue() const { return (DefaultValue() == Value); }
    CONSTEXPR void Assign(T value) { Value = value; }

    CONSTEXPR friend bool operator ==(const TNumeric& lhs, const TNumeric& rhs) { return lhs.Value == rhs.Value; }
    CONSTEXPR friend bool operator !=(const TNumeric& lhs, const TNumeric& rhs) { return lhs.Value != rhs.Value; }

    CONSTEXPR friend bool operator < (const TNumeric& lhs, const TNumeric& rhs) { return lhs.Value <  rhs.Value; }
    CONSTEXPR friend bool operator >=(const TNumeric& lhs, const TNumeric& rhs) { return lhs.Value >= rhs.Value; }

    CONSTEXPR friend bool operator > (const TNumeric& lhs, const TNumeric& rhs) { return lhs.Value >  rhs.Value; }
    CONSTEXPR friend bool operator <=(const TNumeric& lhs, const TNumeric& rhs) { return lhs.Value <= rhs.Value; }

    friend void swap(TNumeric& lhs, TNumeric& rhs) NOEXCEPT { std::swap(lhs.Value, rhs.Value); }
    friend inline hash_t hash_value(const TNumeric& value) NOEXCEPT { return hash_as_pod(value.Value); }

    CONSTEXPR static TNumeric MinusOne() { return TNumeric(T(-1)); }
    CONSTEXPR static TNumeric One() { return TNumeric(T(1)); }
    CONSTEXPR static TNumeric Zero() { return TNumeric(T(0)); }
    CONSTEXPR static TNumeric Null() { return TNumeric(T(0)); }
};
PPE_ASSUME_TEMPLATE_AS_POD(TNumeric<T COMMA _Tag>, typename T, typename _Tag);
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
PPE_ASSUME_TEMPLATE_AS_POD(TNumericDefault<T COMMA _Tag COMMA _DefaultValue>, typename T, typename _Tag, T _DefaultValue);
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
    PPE_STRONGLYTYPED_NUMERIC_DEFAULTVALUE_DEF(T, _Name, ::PPE::Meta::DefaultValue<T>())
//----------------------------------------------------------------------------
#define PPE_FAKEBOOL_OPERATOR_DECL() explicit CONSTF operator bool () const NOEXCEPT
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_STRONGLYTYPED_NUMERIC_DEF(size_t, FSizeInBytes);
//----------------------------------------------------------------------------
inline CONSTEXPR FSizeInBytes operator "" _B (unsigned long long sizeInBytes) { return FSizeInBytes(checked_cast<FSizeInBytes::value_type>(sizeInBytes)); }
inline CONSTEXPR FSizeInBytes operator "" _b (unsigned long long sizeInBytes) { return FSizeInBytes(checked_cast<FSizeInBytes::value_type>(sizeInBytes)); }
//----------------------------------------------------------------------------
inline CONSTEXPR FSizeInBytes OneKikiByte{ 1024u };
inline CONSTEXPR FSizeInBytes OneMebiByte{ 1024u * OneKikiByte };
inline CONSTEXPR FSizeInBytes OneGibiByte{ 1024u * OneMebiByte };
//----------------------------------------------------------------------------
inline CONSTEXPR FSizeInBytes operator "" _KiB (unsigned long long sizeInKikiBytes) { return FSizeInBytes(checked_cast<FSizeInBytes::value_type>(sizeInKikiBytes * OneKikiByte)); }
inline CONSTEXPR FSizeInBytes operator "" _MiB (unsigned long long sizeInMebiBytes) { return FSizeInBytes(checked_cast<FSizeInBytes::value_type>(sizeInMebiBytes * OneMebiByte)); }
inline CONSTEXPR FSizeInBytes operator "" _GiB (unsigned long long sizeInGibiBytes) { return FSizeInBytes(checked_cast<FSizeInBytes::value_type>(sizeInGibiBytes * OneGibiByte)); }
//----------------------------------------------------------------------------
inline CONSTEXPR FSizeInBytes OneKiloByte{ 1000u };
inline CONSTEXPR FSizeInBytes OneMegaByte{ 1000u * OneKiloByte };
inline CONSTEXPR FSizeInBytes OneGigaByte{ 1000u * OneMegaByte };
//----------------------------------------------------------------------------
inline CONSTEXPR FSizeInBytes operator "" _kB (unsigned long long sizeInKiloBytes) { return FSizeInBytes(checked_cast<FSizeInBytes::value_type>(sizeInKiloBytes * OneKiloByte)); }
inline CONSTEXPR FSizeInBytes operator "" _MB (unsigned long long sizeInMegaBytes) { return FSizeInBytes(checked_cast<FSizeInBytes::value_type>(sizeInMegaBytes * OneMegaByte)); }
inline CONSTEXPR FSizeInBytes operator "" _GB (unsigned long long sizeInGigaBytes) { return FSizeInBytes(checked_cast<FSizeInBytes::value_type>(sizeInGigaBytes * OneGigaByte)); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
