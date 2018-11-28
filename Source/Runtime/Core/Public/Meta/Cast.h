#pragma once

#include "Meta/Assert.h"
#include "Meta/TypeHash.h"
#include "Meta/TypeTraits.h"

#include <type_traits>

#ifdef WITH_PPE_ASSERT
#   define USE_PPE_CHECKEDCAST 1
#else
#   define USE_PPE_CHECKEDCAST 0
#endif

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Dst>
constexpr FORCE_INLINE NOALIAS _Dst checked_cast(_Dst id) noexcept {
    return id;
}
//----------------------------------------------------------------------------
template <typename _Dst, typename _Src>
NOALIAS Meta::TEnableIf<
    std::is_arithmetic<_Dst>::value &&
    std::is_arithmetic<_Src>::value &&
    !std::is_same<_Dst, _Src>::value,
    _Dst >
    checked_cast(const _Src value) noexcept {
#if USE_PPE_CHECKEDCAST
    const _Dst result = static_cast<_Dst>(value);
    Assert(static_cast<_Src>(result) == value);
    return result;
#else
    return static_cast<_Dst>(value);
#endif
}
template <typename _Dst, typename _Src>
NOALIAS Meta::TEnableIf<
    (std::is_pointer<_Dst>::value ^ std::is_pointer<_Src>::value) &&
    !std::is_same<_Dst, _Src>::value,
    _Dst >
    checked_cast(const _Src value) noexcept {
#if USE_PPE_CHECKEDCAST
    const _Dst result = reinterpret_cast<_Dst>(value);
    Assert(reinterpret_cast<_Src>(result) == value);
    return result;
#else
    return reinterpret_cast<_Dst>(value);
#endif
}
template <typename _Dst, typename _Src>
NOALIAS Meta::TEnableIf<
    (std::is_pointer<_Dst>::value && std::is_pointer<_Src>::value) &&
    !std::is_same<_Dst, _Src>::value,
    _Dst >
    checked_cast(_Src value) {
#if USE_PPE_CHECKEDCAST
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
//////////////////////////////////////////////////////////////////////////////
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
} //!namespace PPE
