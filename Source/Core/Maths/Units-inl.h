#pragma once

#include "Core/Maths/Units.h"

namespace Core {
namespace Units {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
template <typename _Smaller>
struct NextUnitIndex {
    typedef typename _Smaller::traits_type smaller_traits_type;
    enum : u64 { Value = smaller_traits_type::Index + 1 };
};
template <>
struct NextUnitIndex<void> {
    enum : u64 { Value = 0 };
};
} //!details
//----------------------------------------------------------------------------
template <typename _Tag, u64 _Ratio, typename _Smaller>
struct UnitTraits {
    typedef _Tag tag_type;
    typedef _Smaller smaller_type;
    typedef typename tag_type::value_type value_type;
    enum : u64 {
        Ratio = _Ratio,
        Index = details::NextUnitIndex<smaller_type>::Value
    };
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
template <typename _To, typename _From>
struct ConvertionRatio_LargerToSmaller {
    typedef typename _To::traits_type to_traits_type;
    typedef typename _From::traits_type from_traits_type;

    static_assert(to_traits_type::Index < from_traits_type::Index, "invalid conversion ratio search");

    typedef typename from_traits_type::smaller_type from_smaller_type;
    typedef typename from_smaller_type::traits_type from_smaller_traits_type;

    enum : u64 { Value = from_smaller_traits_type::Ratio * ConvertionRatio_LargerToSmaller<_To, from_smaller_type>::Value };
};
//----------------------------------------------------------------------------
template <typename _ToFrom>
struct ConvertionRatio_LargerToSmaller<_ToFrom, _ToFrom> {
    enum : u64 { Value = 1 };
};
//----------------------------------------------------------------------------
} //!details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
template <typename _To, typename _From>
struct ConvertionRatio {
    typedef typename _To::traits_type to_traits_type;
    typedef typename _From::traits_type from_traits_type;

    static_assert(std::is_same< typename to_traits_type::tag_type,
                                typename from_traits_type::tag_type>::value,
                    "convertion between different units");

    enum : bool { LargerToSmaller = (to_traits_type::Index <= from_traits_type::Index) };

    typedef typename to_traits_type::value_type value_type;
    typedef typename std::conditional<
        LargerToSmaller,
        ConvertionRatio_LargerToSmaller<_To, _From>,
        ConvertionRatio_LargerToSmaller<_From, _To>
    >::type convertionratio_type;

    enum : u64 { Value = convertionratio_type::Value };
};
//----------------------------------------------------------------------------
template <typename _ToFrom>
struct ConvertionRatio<_ToFrom, _ToFrom> {
    enum : size_t { Value = 1 };
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
,   bool _LargerToSmaller = ConvertionRatio<_To, _From>::LargerToSmaller >
struct Converter {
    typedef typename _To::traits_type from_traits_type;
    typedef typename from_traits_type::value_type value_type;
    static constexpr value_type Convert(value_type from) {
        return value_type(ConvertionRatio<_To, _From>::Value * from);
    }
};
//----------------------------------------------------------------------------
template <typename _To, typename _From>
struct Converter<_To, _From, false> {
    typedef typename _To::traits_type from_traits_type;
    typedef typename from_traits_type::value_type value_type;
    static constexpr value_type Convert(value_type from) {
        return value_type(from / ConvertionRatio<_To, _From>::Value);
    }
};
//----------------------------------------------------------------------------
template <typename _To, typename _From>
constexpr typename Converter<_To, _From>::value_type ConvertValue(typename Converter<_To, _From>::value_type value) {
    return Converter<_To, _From>::Convert(value);
}
//----------------------------------------------------------------------------
} //!details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Traits>
template <typename _OtherTraits>
Unit<_Traits>::Unit(const Unit<_OtherTraits>& other)
:   _value(details::ConvertValue<self_type, Unit<_OtherTraits> >(other._value)) {}
//----------------------------------------------------------------------------
template <typename _Traits>
template <typename _OtherTraits>
Unit<_Traits>& Unit<_Traits>::operator =(const Unit<_OtherTraits>& other) {
    _value = details::ConvertValue<self_type, Unit<_OtherTraits> >(other._value);
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
} //!namespace Core
