#pragma once

#include "Core/Core.h"

#include "Core/Meta/Cast.h"

namespace Core {
namespace Units {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Tag, u64 _Ratio, typename _Smaller>
struct TUnitTraits;
//----------------------------------------------------------------------------
template <typename _Traits>
class TUnit;
//----------------------------------------------------------------------------
namespace Time { struct _Tag; }
namespace Distance { struct _Tag; }
namespace Mass { struct _Tag; }
namespace Storage { struct _Tag; }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Traits>
class TUnit {
public:
    template <typename _OtherUnits>
    friend class TUnit;

    typedef _Traits traits_type;
    typedef typename traits_type::tag_type tag_type;
    typedef TUnit<traits_type> self_type;

    typedef typename tag_type::value_type value_type;
    typedef typename tag_type::largest_type largest_type;

    FORCE_INLINE TUnit() : _value(0) {}
    FORCE_INLINE TUnit(value_type value) : _value(value) {}

    FORCE_INLINE TUnit(const TUnit& other) : _value(other._value) {}
    FORCE_INLINE TUnit& operator =(const TUnit& other) {
        _value = other._value;
        return *this;
    }

    template <typename _OtherTraits>
    TUnit(const TUnit<_OtherTraits>& other);
    template <typename _OtherTraits>
    TUnit& operator =(const TUnit<_OtherTraits>& other);

    value_type Value() const { return _value; }
    void SetValue(value_type value) { _value = value; }

    value_type operator *() const { return _value; }

    bool operator ==(const TUnit& other) const { return _value == other._value; }
    bool operator !=(const TUnit& other) const { return !operator ==(other); }

    bool operator <(const TUnit& other) const { return _value < other._value; }
    bool operator >=(const TUnit& other) const { return !operator <(other); }

    bool operator >(const TUnit& other) const { return _value > other._value; }
    bool operator <=(const TUnit& other) const { return !operator >(other); }

private:
    value_type _value;
};
//----------------------------------------------------------------------------
template <typename _Unit, typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    const TUnit<_Unit>& unit) {
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
#define UNITS_BEGIN(NAME) namespace NAME {
#define UNITS_END() }
#define UNITS_DECL(TAG, SYMBOL, NAME, RATIO, SMALLER) \
    typedef Core::Units::TUnit< Core::Units::TUnitTraits<Core::Units::TAG::_Tag, RATIO, SMALLER> > NAME;
//----------------------------------------------------------------------------
#include "Core/Maths/Units.Definitions-inl.h"
//----------------------------------------------------------------------------
#undef UNITS_DECL
#undef UNITS_END
#undef UNITS_BEGIN
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Time {
    struct _Tag {
        typedef double value_type;
        typedef FDays largest_type;
    };
}
//----------------------------------------------------------------------------
namespace Distance {
    struct _Tag {
        typedef double value_type;
        typedef FKilometers largest_type;
    };
}
//----------------------------------------------------------------------------
namespace Mass {
    struct _Tag {
        typedef double value_type;
        typedef FTonnes largest_type;
    };
}
//----------------------------------------------------------------------------
namespace Storage {
    struct _Tag {
        typedef double value_type;
        typedef FPetabytes largest_type;
    };
}
//----------------------------------------------------------------------------
template <typename _To, typename _From>
constexpr typename _To::value_type ConvertValue(typename _From::value_type value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Units
} //!namespace Core

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define UNITS_BEGIN(NAME)
#define UNITS_END()
#define UNITS_DECL(TAG, SYMBOL, NAME, RATIO, SMALLER) \
    typedef Core::Units::TAG::NAME NAME; \
    extern template class Core::Units::TUnit< Core::Units::TUnitTraits<Core::Units::TAG::_Tag, RATIO, SMALLER> >; \
    template <typename _Traits> \
    std::basic_ostream<char, _Traits>& operator <<( \
        std::basic_ostream<char, _Traits>& oss, \
        const Core::Units::TUnit< Core::Units::TUnitTraits<Core::Units::TAG::_Tag, RATIO, SMALLER> >& unit ) { \
        return oss << unit.Value() << " " STRINGIZE(SYMBOL); \
    } \
    template <typename _Traits> \
    std::basic_ostream<wchar_t, _Traits>& operator <<( \
        std::basic_ostream<wchar_t, _Traits>& oss, \
        const Core::Units::TUnit< Core::Units::TUnitTraits<Core::Units::TAG::_Tag, RATIO, SMALLER> >& unit ) { \
        return oss << unit.Value() << L" " WSTRINGIZE(SYMBOL); \
    }
//----------------------------------------------------------------------------
#include "Core/Maths/Units.Definitions-inl.h"
//----------------------------------------------------------------------------
#undef UNITS_DECL
#undef UNITS_END
#undef UNITS_BEGIN
//----------------------------------------------------------------------------
CORE_ASSUME_TYPE_AS_POD(Units::TUnit<_Traits>, typename _Traits)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
