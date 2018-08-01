#pragma once

#include "Maths/Units.h"

namespace PPE {
namespace Units {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
template <typename _Smaller>
struct TNextUnitIndex {
    typedef typename _Smaller::traits_type smaller_traits_type;
    static constexpr u64 Value = smaller_traits_type::Index + 1;
};
template <>
struct TNextUnitIndex<void> {
    static constexpr u64 Value = 0;
};
} //!details
//----------------------------------------------------------------------------
template <typename _Tag, u64 _Ratio, typename _Smaller>
struct TUnitTraits {
    typedef _Tag tag_type;
    typedef _Smaller smaller_type;
    typedef typename tag_type::value_type value_type;
    static constexpr u64 Ratio = _Ratio;
    static constexpr u64 Index = details::TNextUnitIndex<smaller_type>::Value;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
template <typename _To, typename _From>
struct TConvertionRatio_LargerToSmaller {
    typedef typename _To::traits_type to_traits_type;
    typedef typename _From::traits_type from_traits_type;

    static_assert(to_traits_type::Index < from_traits_type::Index, "invalid conversion ratio search");

    typedef typename from_traits_type::smaller_type from_smaller_type;
    typedef typename from_smaller_type::traits_type from_smaller_traits_type;

    static constexpr u64 Value = from_smaller_traits_type::Ratio * TConvertionRatio_LargerToSmaller<_To, from_smaller_type>::Value;
};
//----------------------------------------------------------------------------
template <typename _ToFrom>
struct TConvertionRatio_LargerToSmaller<_ToFrom, _ToFrom> {
    static constexpr u64 Value = 1;
};
//----------------------------------------------------------------------------
} //!details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
template <typename _To, typename _From>
struct TConvertionRatio {
    typedef typename _To::traits_type to_traits_type;
    typedef typename _From::traits_type from_traits_type;

    static_assert(std::is_same< typename to_traits_type::tag_type,
                                typename from_traits_type::tag_type>::value,
                    "convertion between different units");

    static constexpr bool LargerToSmaller = (to_traits_type::Index <= from_traits_type::Index);

    typedef typename to_traits_type::value_type value_type;
    typedef typename std::conditional<
        LargerToSmaller,
        TConvertionRatio_LargerToSmaller<_To, _From>,
        TConvertionRatio_LargerToSmaller<_From, _To>
    >::type convertionratio_type;

    static constexpr u64 Value = convertionratio_type::Value;
};
//----------------------------------------------------------------------------
template <typename _ToFrom>
struct TConvertionRatio<_ToFrom, _ToFrom> {
    static constexpr u64 Value = 1;
};
//----------------------------------------------------------------------------
} //!details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
template <
    typename _To, typename _From
,   bool _LargerToSmaller = TConvertionRatio<_To, _From>::LargerToSmaller >
struct TConverter {
    typedef typename _To::traits_type from_traits_type;
    typedef typename from_traits_type::value_type value_type;
    static constexpr value_type Convert(value_type from) {
        return value_type(TConvertionRatio<_To, _From>::Value * from);
    }
};
//----------------------------------------------------------------------------
template <typename _To, typename _From>
struct TConverter<_To, _From, false> {
    typedef typename _To::traits_type from_traits_type;
    typedef typename from_traits_type::value_type value_type;
    static constexpr value_type Convert(value_type from) {
        return value_type(from / TConvertionRatio<_To, _From>::Value);
    }
};
//----------------------------------------------------------------------------
template <typename _To, typename _From>
constexpr typename TConverter<_To, _From>::value_type ConvertValue(typename TConverter<_To, _From>::value_type value) {
    return TConverter<_To, _From>::Convert(value);
}
//----------------------------------------------------------------------------
} //!details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Traits>
template <typename _OtherTraits>
TUnit<_Traits>::TUnit(const TUnit<_OtherTraits>& other)
:   _value(details::ConvertValue<self_type, TUnit<_OtherTraits> >(other._value)) {}
//----------------------------------------------------------------------------
template <typename _Traits>
template <typename _OtherTraits>
TUnit<_Traits>& TUnit<_Traits>::operator =(const TUnit<_OtherTraits>& other) {
    _value = details::ConvertValue<self_type, TUnit<_OtherTraits> >(other._value);
    return *this;
}
//----------------------------------------------------------------------------
template <typename _To, typename _From>
constexpr typename _To::value_type ConvertValue(typename _From::value_type value) {
    STATIC_ASSERT(std::is_same<typename _To::tag_type, typename _From::tag_type>::value);
    STATIC_ASSERT(std::is_same<typename _To::value_type, typename _From::value_type>::value);
    return details::ConvertValue<_To, _From>(value);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Units
} //!namespace PPE
