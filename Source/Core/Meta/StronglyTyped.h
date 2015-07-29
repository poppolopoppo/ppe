#pragma once

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace StronglyTyped {
//----------------------------------------------------------------------------
template <typename T, typename _Tag>
struct Numeric {
    typedef T value_type;
    typedef _Tag tag_type;

    T Value;
    
    explicit Numeric(T value = T()) : Value(value) {}
    operator T () const { return Value; }

    Numeric(const Numeric& other) : Value(other.Value) {}
    Numeric& operator =(const Numeric& other) { Value =other.Value; return *this; }

    bool operator ==(const Numeric& other) const { return Value == other.Value; }
    bool operator !=(const Numeric& other) const { return !operator ==(other); }

    bool operator < (const Numeric& other) const { return Value < other.Value; }
    bool operator >=(const Numeric& other) const { return !operator < (other); }

    static Numeric MinusOne() { return Numeric(-1); }
    static Numeric One() { return Numeric(1); }
    static Numeric Zero() { return Numeric(0); }
};
//----------------------------------------------------------------------------
} //!namespace StronglyTyped
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define CORE_STRONGLYTYPED_NUMERIC_DEF(T, _Name) \
    namespace StronglyTyped { struct _Name {}; } \
    typedef ::Core::StronglyTyped::Numeric<T, StronglyTyped::_Name> _Name
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
