#pragma once

#include "Core/Core.h"

#include "Core/Meta/Cast.h"

namespace Core {
namespace Units {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Tag, u64 _Ratio, typename _Smaller>
struct UnitTraits;
//----------------------------------------------------------------------------
template <typename _Traits>
class Unit;
//----------------------------------------------------------------------------
namespace Time { struct _Tag; }
namespace Distance { struct _Tag; }
namespace Mass { struct _Tag; }
namespace Storage { struct _Tag; }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define UNITS_DECL(NAME, TAG, RATIO, SMALLER) \
    using NAME = Core::Units::Unit< Core::Units::UnitTraits<TAG, RATIO, SMALLER> >
//----------------------------------------------------------------------------
#include "Core/Maths/Units.Definitions-inl.h"
//----------------------------------------------------------------------------
#undef UNITS_DECL
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Traits>
class Unit {
public:
    template <typename _OtherUnits>
    friend class Unit;

    typedef _Traits traits_type;
    typedef typename traits_type::tag_type tag_type;
    typedef Unit<traits_type> self_type;

    typedef typename tag_type::value_type value_type;
    typedef typename tag_type::largest_type largest_type;

    Unit() : _value(0) {}
    Unit(value_type value) : _value(value) {}

    Unit(const Unit& other) : _value(other._value) {}
    Unit& operator =(const Unit& other) {
        _value = other._value;
        return *this;
    }

    template <typename _OtherTraits>
    Unit(const Unit<_OtherTraits>& other);
    template <typename _OtherTraits>
    Unit& operator =(const Unit<_OtherTraits>& other);

    value_type Value() const { return _value; }
    void SetValue(value_type value) { _value = value; }

    bool operator ==(const Unit& other) const { return _value == other._value; }
    bool operator !=(const Unit& other) const { return !operator ==(other); }

    bool operator <(const Unit& other) const { return _value < other._value; }
    bool operator >=(const Unit& other) const { return !operator <(other); }

    bool operator >(const Unit& other) const { return _value > other._value; }
    bool operator <=(const Unit& other) const { return !operator >(other); }

private:
    value_type _value;
};
//----------------------------------------------------------------------------
template <typename _Unit, typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    const Unit<_Unit>& unit) {
    return oss << std::fixed << unit.Value();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Units
} //!namespace Core

#include "Core/Maths/Units-inl.h"

namespace Core {
namespace Units {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Time {
    struct _Tag {
        typedef double value_type;
        typedef Days largest_type;
    };
}
//----------------------------------------------------------------------------
namespace Distance {
    struct _Tag {
        typedef double value_type;
        typedef Kilometers largest_type;
    };
}
//----------------------------------------------------------------------------
namespace Mass {
    struct _Tag {
        typedef double value_type;
        typedef Tonnes largest_type;
    };
}
//----------------------------------------------------------------------------
namespace Storage {
    struct _Tag {
        typedef double value_type;
        typedef Petabytes largest_type;
    };
}
//----------------------------------------------------------------------------
#define UNITS_DECL(NAME, TAG, RATIO, SMALLER) \
    extern template class Core::Units::Unit< Core::Units::UnitTraits<TAG, RATIO, SMALLER> >
//----------------------------------------------------------------------------
#include "Core/Maths/Units.Definitions-inl.h"
//----------------------------------------------------------------------------
#undef UNITS_DECL
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Units
} //!namespace Core
