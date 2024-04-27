#pragma once

#include "Core.h"

#include "IO/TextWriter_fwd.h"
#include "Meta/Cast.h"

namespace PPE {
namespace Units {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _UnitTag, u64 _Ratio, typename _Smaller>
struct TUnitTraits;
//----------------------------------------------------------------------------
template <typename _Traits>
class TUnit;
//----------------------------------------------------------------------------
namespace Time { struct _UnitTag; }
namespace Distance { struct _UnitTag; }
namespace Mass { struct _UnitTag; }
namespace Storage { struct _UnitTag; }
namespace Angle { struct _UnitTag; }
namespace Speed { struct _UnitTag; }
namespace AngularSpeed { struct _UnitTag; }
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

    NODISCARD CONSTEXPR value_type Value() const { return _value; }
    CONSTEXPR void SetValue(value_type value) { _value = value; }

    NODISCARD CONSTEXPR value_type operator *() const { return _value; }

    NODISCARD CONSTEXPR TUnit operator +(const TUnit& other) const { return TUnit{ _value + other._value }; }
    NODISCARD CONSTEXPR TUnit operator -(const TUnit& other) const { return TUnit{ _value - other._value }; }

    CONSTEXPR TUnit& operator +=(const TUnit& other) { _value += other._value; return (*this); }
    CONSTEXPR TUnit& operator -=(const TUnit& other) { _value -= other._value; return (*this); }

    NODISCARD CONSTEXPR bool operator ==(const TUnit& other) const { return _value == other._value; }
    NODISCARD CONSTEXPR bool operator !=(const TUnit& other) const { return !operator ==(other); }

    NODISCARD CONSTEXPR bool operator <(const TUnit& other) const { return _value < other._value; }
    NODISCARD CONSTEXPR bool operator >=(const TUnit& other) const { return !operator <(other); }

    NODISCARD CONSTEXPR bool operator >(const TUnit& other) const { return _value > other._value; }
    NODISCARD CONSTEXPR bool operator <=(const TUnit& other) const { return !operator >(other); }

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
    using NAME = PPE::Units::TUnit< PPE::Units::TUnitTraits<PPE::Units::TAG::_UnitTag, RATIO, SMALLER> >;
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
    struct _UnitTag {
        typedef double value_type;
        typedef FDays largest_type;
    };
}
//----------------------------------------------------------------------------
namespace Distance {
    struct _UnitTag {
        typedef double value_type;
        typedef FKilometers largest_type;
    };
}
//----------------------------------------------------------------------------
namespace Mass {
    struct _UnitTag {
        typedef double value_type;
        typedef FTonnes largest_type;
    };
}
//----------------------------------------------------------------------------
namespace Storage {
    struct _UnitTag {
        typedef double value_type;
        typedef FPetabytes largest_type;
    };
}
//----------------------------------------------------------------------------
namespace Angle {
    struct _UnitTag {
        typedef double value_type;
        typedef FRadians largest_type;
    };
}
//----------------------------------------------------------------------------
namespace Speed {
    struct _UnitTag {
        typedef double value_type;
        typedef FKilometerSeconds largest_type;
    };
}
//----------------------------------------------------------------------------
namespace AngularSpeed {
    struct _UnitTag {
        typedef double value_type;
        typedef FRadianSeconds largest_type;
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
    EXTERN_TEMPLATE_CLASS_DECL(PPE_CORE_API) PPE::Units::TUnit< PPE::Units::TUnitTraits<PPE::Units::TAG::_UnitTag, RATIO, SMALLER> >; \
    PPE_CORE_API FTextWriter& operator <<(FTextWriter& oss, \
        const PPE::Units::TUnit< PPE::Units::TUnitTraits<PPE::Units::TAG::_UnitTag, RATIO, SMALLER> >& unit); \
    PPE_CORE_API FWTextWriter& operator <<(FWTextWriter& oss, \
        const PPE::Units::TUnit< PPE::Units::TUnitTraits<PPE::Units::TAG::_UnitTag, RATIO, SMALLER> >& unit); \
    UNITS_LITERAL_OP(SYMBOL, PPE::Units::TUnit< PPE::Units::TUnitTraits<PPE::Units::TAG::_UnitTag COMMA RATIO COMMA SMALLER> >)
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
NODISCARD inline CONSTEXPR FMeters operator *(FMeterSeconds speed, FSeconds time) { return FMeters{ *speed * *time }; }
NODISCARD inline CONSTEXPR FMeters operator *(FSeconds time, FMeterSeconds speed) { return FMeters{ *speed * *time }; }
NODISCARD inline CONSTEXPR FSeconds operator /(FMeters distance, FMeterSeconds speed) { return FSeconds{ *distance / *speed }; }
//----------------------------------------------------------------------------
NODISCARD inline CONSTEXPR FRadians operator *(FRadianSeconds speed, FSeconds time) { return FRadians{ *speed * *time }; }
NODISCARD inline CONSTEXPR FRadians operator *(FSeconds time, FRadianSeconds speed) { return FRadians{ *speed * *time }; }
NODISCARD inline CONSTEXPR FSeconds operator /(FRadians angle, FMeterSeconds speed) { return FSeconds{ *angle / *speed }; }
//----------------------------------------------------------------------------
template <typename _Char, typename _Unit>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const Units::TUnit<_Unit>& unit) {
    return oss << unit.Value();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
