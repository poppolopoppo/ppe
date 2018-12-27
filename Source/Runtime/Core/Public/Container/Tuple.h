#pragma once

#include "Core_fwd.h"

#include "Memory/HashFunctions.h"

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
std::tuple<Meta::TRemoveReference<_Args>... > MakeTuple(_Args&&... args) {
    return std::make_tuple(std::forward<_Args>(args)...);
}
//----------------------------------------------------------------------------
namespace Meta {
template <typename... _Args>
CONSTEXPR bool is_pod(TType< TTuple<_Args...> >) NOEXCEPT {
    return TIsPod_v<_Args...>;
}
template <typename... _Args>
TTuple<_Args...> NoInitType(TType< TTuple<_Args...> >) {
    return MakeTuple(MakeNoInit<_Args>()...);
}
template <typename... _Args>
TTuple<_Args...> ForceInitType(TType< TTuple<_Args...> >) {
    return MakeTuple(MakeForceInit<_Args>()...);
}
template <typename... _Args>
void Construct(TTuple<_Args...>* p, FNoInit) {
    Construct(p, MakeNoInit<_Args>()...);
}
template <typename... _Args>
void Construct(TTuple<_Args...>* p, FForceInit) {
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
