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
    return size_t(flag) == (size_t(value) & size_t(flag));
}
//----------------------------------------------------------------------------
template <class T, class = typename std::enable_if< std::is_enum<T>::value >::type >
FORCE_INLINE void SetFlag(T *pvalue, T flag) {
    *pvalue = (T)(size_t(*pvalue) | size_t(flag));
}
//----------------------------------------------------------------------------
template <class T, class = typename std::enable_if< std::is_enum<T>::value >::type >
FORCE_INLINE void UnsetFlag(T *pvalue, T flag) {
    *pvalue = (T)(size_t(*pvalue) & (~size_t(flag)) );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace Core
