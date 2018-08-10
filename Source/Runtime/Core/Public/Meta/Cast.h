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

namespace PPE {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <template <typename... > class _Typed>
class TDynamicCastable {
public:
    virtual ~TDynamicCastable() {}

    virtual hash_t InnerTypeHash() const = 0;

    template <typename... _Args>
    bool IsA() const { 
        return (InnerTypeHash() == _Typed<_Args...>::StaticInnerTypeHash);
    }

    template <typename... _Args>
    _Typed<_Args...>* Cast() {
        AssertRelease(IsA<_Args...>());
        return checked_cast<_Typed<_Args...>*>(this);
    }

    template <typename... _Args>
    const _Typed<_Args...>* Cast() const {
        AssertRelease(IsA<_Args...>());
        return checked_cast<const _Typed<_Args...>*>(this);
    }

    template <typename... _Args>
    _Typed<_Args...>* As() {
        return (IsA<_Args...>() ? checked_cast<_Typed<_Args...>*>(this) : nullptr);
    }

    template <typename... _Args>
    const _Typed<_Args...>* As() const {
        return (IsA<_Args...>() ? checked_cast<const _Typed<_Args...>*>(this) : nullptr);
    }
};
//----------------------------------------------------------------------------
#define META_DYNAMIC_CASTABLE_IMPL(_SELF_TYPE) \
    virtual ::PPE::hash_t InnerTypeHash() const override final { return StaticInnerTypeHash; } \
    STATIC_CONST_INTEGRAL(size_t, StaticInnerTypeHash, ::PPE::Meta::TTypeHash<_SELF_TYPE>::value())
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace PPE
