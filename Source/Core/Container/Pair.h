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
using Pair = std::pair<_Key, _Value>;
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
FORCE_INLINE Pair< typename std::remove_reference<_Key>::type, typename std::remove_reference<_Value>::type > MakePair(_Key&& key, _Value&& value) {
    typedef Pair< typename std::remove_reference<_Key>::type, typename std::remove_reference<_Value>::type > pair_type;
    return pair_type(std::forward<_Key>(key), std::forward<_Value>(value));
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
FORCE_INLINE size_t hash_value(const Pair<_Key, _Value>& pair) {
    return hash_value(pair.first, pair.second);
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
std::basic_ostream<_Char, _Traits>& operator <<(std::basic_ostream<_Char, _Traits>& oss, const Pair<_Key, _Value>& pair) {
    return oss << "( " << pair.first << ", " << pair.second << " )";
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
