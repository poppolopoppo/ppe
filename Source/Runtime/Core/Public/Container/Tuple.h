#pragma once

#include "Core_fwd.h"

#include "Memory/HashFunctions.h"
#include "Meta/TypeTraits.h"

#include <functional>
#include <tuple>
#include <type_traits>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename... _Args>
using TTuple = std::tuple<_Args...>;
//----------------------------------------------------------------------------
template <typename... _Args>
using TTupleDecay = TTuple< Meta::TDecay<_Args>... >;
//----------------------------------------------------------------------------
template <typename... _Args>
using TTupleMake = std::decay_t<decltype(
    std::make_tuple(std::declval<_Args>().../* wraps const T& to T */)
)>;
//----------------------------------------------------------------------------
template <typename... _Args>
CONSTEXPR TTuple<Meta::TRemoveReference<_Args>... > MakeTuple(_Args&&... args) NOEXCEPT {
    return std::make_tuple(std::forward<_Args>(args)...);
}
//----------------------------------------------------------------------------
template <typename... _Args>
CONSTEXPR bool is_pod_type(TTuple<_Args...>*) NOEXCEPT {
    return ( Meta::is_pod_v<_Args> && ... );
}
//----------------------------------------------------------------------------
namespace Meta {
template <typename... _Args>
CONSTEXPR TTuple<_Args...> NoInitType(TType< TTuple<_Args...> >) NOEXCEPT {
    return MakeTuple(MakeNoInit<_Args>()...);
}
template <typename... _Args>
CONSTEXPR TTuple<_Args...> ForceInitType(TType< TTuple<_Args...> >) NOEXCEPT {
    return MakeTuple(MakeForceInit<_Args>()...);
}
template <typename... _Args>
CONSTEXPR void Construct(TTuple<_Args...>* p, FNoInit) NOEXCEPT {
    Construct(p, MakeNoInit<_Args>()...);
}
template <typename... _Args>
CONSTEXPR void Construct(TTuple<_Args...>* p, FForceInit) NOEXCEPT {
    Construct(p, MakeForceInit<_Args>()...);
}
} //!Meta
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
template <typename _Lambda, typename _Tuple, size_t... _Idx>
CONSTEXPR void ForeachTuple_(_Lambda foreach, _Tuple& tuple, std::index_sequence<_Idx...>) {
    FOLD_EXPR( foreach(std::get<_Idx>(tuple)) );
}
} //!details
//----------------------------------------------------------------------------
template <typename... _Args, typename _Lambda>
CONSTEXPR void ForeachTuple(TTuple<_Args...>& tuple, _Lambda foreach) {
    details::ForeachTuple_(foreach, tuple, std::index_sequence_for<_Args...>{});
}
//----------------------------------------------------------------------------
template <typename... _Args, typename _Lambda>
CONSTEXPR void ForeachTuple(const TTuple<_Args...>& tuple, _Lambda foreach) {
    details::ForeachTuple_(foreach, tuple, std::index_sequence_for<_Args...>{});
}
//----------------------------------------------------------------------------
template <typename... _Args>
FORCE_INLINE hash_t hash_value(const TTuple<_Args...>& tuple) {
    hash_t h{ PPE_HASH_VALUE_SEED };
    ForeachTuple(tuple, [&h](const auto& x) { hash_combine(h, x); });
    return h;
}
//----------------------------------------------------------------------------
template <typename _Char, typename... _Args>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const TTuple<_Args...>& tuple) {
    ForeachTuple(tuple, [&oss](const auto& x) { oss << x; });
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
