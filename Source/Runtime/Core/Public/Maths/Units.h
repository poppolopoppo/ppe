#pragma once

#include "Core.h"

#include "IO/TextWriter_fwd.h"
#include "Meta/Cast.h"

namespace PPE {
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

    CONSTEXPR TUnit() : _value(0) {}
    CONSTEXPR TUnit(value_type value) : _value(value) {}

    CONSTEXPR TUnit(const TUnit& other) : _value(other._value) {}
    CONSTEXPR TUnit& operator =(const TUnit& other) {
        _value = other._value;
        return *this;
    }

    template <typename _OtherTraits>
    CONSTEXPR TUnit(const TUnit<_OtherTraits>& other);
    template <typename _OtherTraits>
    CONSTEXPR TUnit& operator =(const TUnit<_OtherTraits>& other);

    CONSTEXPR value_type Value() const { return _value; }
    CONSTEXPR void SetValue(value_type value) { _value = value; }

    CONSTEXPR value_type operator *() const { return _value; }

    CONSTEXPR bool operator ==(const TUnit& other) const { return _value == other._value; }
    CONSTEXPR bool operator !=(const TUnit& other) const { return !operator ==(other); }

    CONSTEXPR bool operator <(const TUnit& other) const { return _value < other._value; }
    CONSTEXPR bool operator >=(const TUnit& other) const { return !operator <(other); }

    CONSTEXPR bool operator >(const TUnit& other) const { return _value > other._value; }
    CONSTEXPR bool operator <=(const TUnit& other) const { return !operator >(other); }

private:
    value_type _value;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Units
} //!namespace PPE

#include "Maths/Units-inl.h"

namespace PPE {
namespace Units {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define UNITS_BEGIN(NAME) namespace NAME {
#define UNITS_END() }
//----------------------------------------------------------------------------
#define UNITS_DECL(TAG, SYMBOL, NAME, RATIO, SMALLER) \
    using NAME = PPE::Units::TUnit< PPE::Units::TUnitTraits<PPE::Units::TAG::_Tag, RATIO, SMALLER> >;
//----------------------------------------------------------------------------
#include "Maths/Units.Definitions-inl.h"
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
CONSTEXPR typename _To::value_type ConvertValue(typename _From::value_type value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Units
} //!namespace PPE

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// C++11 added support for user-defined literal operators :
// ex:  10_s    <=> FSeconds{ 10 }
//      22_ms   <=> FMicroseconds{ 22 }
//      127_kg  <=> FKilograms{ 127 }
//
#define UNITS_LITERAL_OP(SYMBOL, TYPE) \
    CONSTEXPR TYPE operator "" CONCAT(_, SYMBOL)(long double value) { \
        return TYPE{ static_cast<TYPE::value_type>(value) }; \
    }
//----------------------------------------------------------------------------
#define UNITS_BEGIN(NAME)
#define UNITS_END()
#define UNITS_DECL(TAG, SYMBOL, NAME, RATIO, SMALLER) \
    using NAME = PPE::Units::TAG::NAME; \
    PPE_CORE_API extern template class PPE::Units::TUnit< PPE::Units::TUnitTraits<PPE::Units::TAG::_Tag, RATIO, SMALLER> >; \
    PPE_CORE_API FTextWriter& operator <<(FTextWriter& oss, \
        const PPE::Units::TUnit< PPE::Units::TUnitTraits<PPE::Units::TAG::_Tag, RATIO, SMALLER> >& unit); \
    PPE_CORE_API FWTextWriter& operator <<(FWTextWriter& oss, \
        const PPE::Units::TUnit< PPE::Units::TUnitTraits<PPE::Units::TAG::_Tag, RATIO, SMALLER> >& unit); \
    UNITS_LITERAL_OP(SYMBOL, PPE::Units::TUnit< PPE::Units::TUnitTraits<PPE::Units::TAG::_Tag COMMA RATIO COMMA SMALLER> >)
//----------------------------------------------------------------------------
#include "Maths/Units.Definitions-inl.h"
//----------------------------------------------------------------------------
#undef UNITS_DECL
#undef UNITS_END
#undef UNITS_BEGIN
#undef UNITS_LITERAL_OP
//----------------------------------------------------------------------------
PPE_ASSUME_TEMPLATE_AS_POD(Units::TUnit<_Traits>, typename _Traits)
//----------------------------------------------------------------------------
template <typename _Char, typename _Unit>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const Units::TUnit<_Unit>& unit) {
    return oss << unit.Value();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
