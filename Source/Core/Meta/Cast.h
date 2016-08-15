#pragma once

#include <type_traits>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Dst, typename _Src>
inline NOALIAS typename std::enable_if<
    std::is_arithmetic<_Dst>::value &&
    std::is_arithmetic<_Src>::value &&
    !std::is_same<_Dst, _Src>::value,
    _Dst>::type
    checked_cast(const _Src value) noexcept {
#ifdef _DEBUG
    const _Dst result = static_cast<_Dst>(value);

    Assert(static_cast<_Src>(result) == value);
    return result;
#else
    return static_cast<_Dst>(value);
#endif
}
//----------------------------------------------------------------------------
template <typename _Dst, typename _Src>
inline NOALIAS typename std::enable_if<
    (std::is_pointer<_Dst>::value ^ std::is_pointer<_Src>::value) &&
    !std::is_same<_Dst, _Src>::value,
    _Dst>::type
    checked_cast(const _Src value) noexcept {
#ifdef _DEBUG
    const _Dst result = reinterpret_cast<_Dst>(value);
    Assert(reinterpret_cast<_Src>(result) == value);
    return result;
#else
    return reinterpret_cast<_Dst>(value);
#endif
}
//----------------------------------------------------------------------------
template <typename _Dst, typename _Src>
inline NOALIAS typename std::enable_if<
    (std::is_pointer<_Dst>::value && std::is_pointer<_Src>::value) &&
    !std::is_same<_Dst, _Src>::value,
    _Dst>::type
    checked_cast(_Src value) {
#ifdef _DEBUG
    if (nullptr == value)
        return nullptr;
    const _Dst result = static_cast<_Dst>(value);
    Assert(dynamic_cast<_Dst>(value));
    return result;
#else
    return static_cast<_Dst>(value);
#endif
}
//----------------------------------------------------------------------------
template <typename _Dst>
FORCE_INLINE NOALIAS const _Dst& checked_cast(const _Dst& value) noexcept {
    return value;
}
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE NOALIAS T& remove_const(const T& value) {
    return const_cast< T& >(value);
}
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE NOALIAS T* remove_const(const T* pvalue) {
    return const_cast< T* >(pvalue);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
