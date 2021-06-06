#pragma once

#include "Meta/Assert.h"
#include "Meta/TypeTraits.h"

#include <type_traits>

#if USE_PPE_ASSERT
#   define USE_PPE_CHECKEDCAST 1
#else
#   define USE_PPE_CHECKEDCAST 0
#endif

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Dst>
CONSTEXPR NOALIAS _Dst checked_cast(_Dst id) NOEXCEPT {
    return id;
}
//----------------------------------------------------------------------------
template <typename _Dst, typename _Src>
CONSTEXPR NOALIAS Meta::TEnableIf<
    std::is_arithmetic<_Dst>::value &&
    std::is_arithmetic<_Src>::value &&
    !std::is_same<_Dst, _Src>::value,
    _Dst >
    checked_cast(const _Src value) NOEXCEPT {
#if USE_PPE_CHECKEDCAST
    const _Dst result = static_cast<_Dst>(value);
    Assert(static_cast<_Src>(result) == value);
    return result;
#else
    return static_cast<_Dst>(value);
#endif
}
//----------------------------------------------------------------------------
template <typename _Dst, typename _Src>
NOALIAS Meta::TEnableIf<
    (std::is_pointer<_Dst>::value ^ std::is_pointer<_Src>::value) &&
    !std::is_same<_Dst, _Src>::value,
    _Dst >
    checked_cast(const _Src value) NOEXCEPT {
#if USE_PPE_CHECKEDCAST
    const _Dst result = reinterpret_cast<_Dst>(value);
    Assert(reinterpret_cast<_Src>(result) == value);
    return result;
#else
    return reinterpret_cast<_Dst>(value);
#endif
}
//----------------------------------------------------------------------------
template <typename _Dst, typename _Src>
#if !USE_PPE_CHECKEDCAST
    CONSTEXPR
#endif
NOALIAS Meta::TEnableIf<
    (std::is_pointer<_Dst>::value && std::is_pointer<_Src>::value) &&
    !std::is_same<_Dst, _Src>::value,
    _Dst >
    checked_cast(_Src value) NOEXCEPT {
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
// https://en.cppreference.com/w/cpp/numeric/bit_cast
template <class To, class From>
Meta::TEnableIf<
    sizeof(To) == sizeof(From) &&
    std::is_trivially_copyable_v<From> &&
    std::is_trivially_copyable_v<To>,
    To>
// constexpr support needs compiler magic
bit_cast(const From& src) noexcept {
    static_assert(std::is_trivially_constructible_v<To>,
        "This implementation additionally requires destination type to be trivially constructible");

    To dst;
    std::memcpy(&dst, &src, sizeof(To));
    return dst;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR NOALIAS T& remove_const(const T& value) NOEXCEPT {
    return const_cast< T& >(value);
}
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR NOALIAS T* remove_const(const T* pvalue) NOEXCEPT {
    return const_cast< T* >(pvalue);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
