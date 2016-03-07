#pragma once

#include <type_traits>

namespace Core {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// helpers to ease manipulations of enum classes
//----------------------------------------------------------------------------
template <class T, class = typename std::enable_if< std::is_enum<T>::value >::type >
inline constexpr bool HasFlag(T value, T flag) {
    typedef typename std::conditional<(sizeof(T) <= sizeof(size_t)), size_t, u64 >::type uint_type;
    STATIC_ASSERT(sizeof(uint_type) >= sizeof(T));
    return uint_type(flag) == (uint_type(value) & uint_type(flag));
}
//----------------------------------------------------------------------------
template <class T, class = typename std::enable_if< std::is_enum<T>::value >::type >
FORCE_INLINE void SetFlag(T *pvalue, T flag) {
    typedef typename std::conditional<(sizeof(T) <= sizeof(size_t)), size_t, u64 >::type uint_type;
    STATIC_ASSERT(sizeof(uint_type) >= sizeof(T));
    *pvalue = (T)(uint_type(*pvalue) | uint_type(flag));
}
//----------------------------------------------------------------------------
template <class T, class = typename std::enable_if< std::is_enum<T>::value >::type >
FORCE_INLINE void UnsetFlag(T *pvalue, T flag) {
    typedef typename std::conditional<(sizeof(T) <= sizeof(size_t)), size_t, u64 >::type uint_type;
    STATIC_ASSERT(sizeof(uint_type) >= sizeof(T));
    *pvalue = (T)(uint_type(*pvalue) & (~uint_type(flag)) );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace Core
