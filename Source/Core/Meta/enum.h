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
template <class T, class = typename std::enable_if< std::is_enum<T>::value >::type >
inline constexpr T CombineFlags(T&& flags...) {
    return T(0 | flags...);
}
//----------------------------------------------------------------------------
template <class T, class = typename std::enable_if< std::is_integral<T>::value >::type >
inline constexpr T MakeFlag(T&& values...) {
    return T(0 | (1<<flags)...);
}
//----------------------------------------------------------------------------
#define ENUM_FLAGS(_ENUMTYPE) \
    STATIC_ASSERT(std::is_enum<_ENUMTYPE>::value); \
    inline _ENUMTYPE operator &(_ENUMTYPE lhs, _ENUMTYPE rhs) { return _ENUMTYPE(u64(lhs)&u64(rhs)); } \
    inline _ENUMTYPE operator |(_ENUMTYPE lhs, _ENUMTYPE rhs) { return _ENUMTYPE(u64(lhs)|u64(rhs)); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace Core
