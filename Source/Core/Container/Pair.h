#pragma once

#include "Core/Core.h"

#include <iosfwd>
#include <utility>
#include <type_traits>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
using TPair = std::pair<_Key, _Value>;
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
FORCE_INLINE TPair< typename std::remove_reference<_Key>::type, typename std::remove_reference<_Value>::type > MakePair(_Key&& key, _Value&& value) {
    typedef TPair< typename std::remove_reference<_Key>::type, typename std::remove_reference<_Value>::type > pair_type;
    return pair_type(std::forward<_Key>(key), std::forward<_Value>(value));
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
FORCE_INLINE hash_t hash_value(const TPair<_Key, _Value>& pair) {
    using Core::hash_value;
    return hash_tuple(hash_value(pair.first), hash_value(pair.second));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <
    typename _Key,
    typename _Value,
    typename _Char,
    typename _Traits
>
std::basic_ostream<_Char, _Traits>& operator <<(std::basic_ostream<_Char, _Traits>& oss, const TPair<_Key, _Value>& pair) {
    return oss << "( " << pair.first << ", " << pair.second << " )";
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
