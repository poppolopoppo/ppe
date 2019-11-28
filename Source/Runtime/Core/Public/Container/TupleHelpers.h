#pragma once

#include "Core_fwd.h"

#include "Container/Array.h"
#include "Container/Tuple.h"

#include "Memory/PtrRef.h"
#include "Memory/RefPtr.h"

#include <utility>
#include <type_traits>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Merge 2 different tuples together
//----------------------------------------------------------------------------
namespace details {
template <typename _Lhs, typename _Rhs>
struct TTupleMerger {
    template <typename T>
    struct TTupleWrap {
        typedef TTuple< Meta::TRemoveReference<T> > type;
        static CONSTEXPR type make(Meta::TRemoveReference<T>&& value) NOEXCEPT {
            return std::make_tuple(std::forward<T>(value));
        }
    };

    template <typename... _Args>
    struct TTupleWrap< TTuple<_Args...> > {
        typedef TTuple<_Args...> type;
        static CONSTEXPR type make(type&& value) NOEXCEPT {
            return std::forward<type>(value);
        }
    };

    template <typename U, typename V>
    struct TTupleMerged {};

    template <typename... _LhsArgs, typename... _RhsArgs>
    struct TTupleMerged< TTuple<_LhsArgs...>, TTuple<_RhsArgs...> > {
        typedef TTuple<_LhsArgs..., _RhsArgs...> type;
    };

    typedef TTupleWrap< Meta::TRemoveReference<_Lhs> > lhs_tuple;
    typedef TTupleWrap< Meta::TRemoveReference<_Rhs> > rhs_tuple;

    typedef typename TTupleMerged<
        typename lhs_tuple::type,
        typename rhs_tuple::type
    >::type type;

    static CONSTEXPR type cat(_Lhs&& lhs, _Rhs&& rhs) NOEXCEPT {
        return std::tuple_cat(
            lhs_tuple::make(std::move(lhs)),
            rhs_tuple::make(std::move(rhs)) );
    }
};
}//!details
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
CONSTEXPR auto MergeTuple(_Lhs&& lhs, _Rhs&& rhs) NOEXCEPT
-> typename details::TTupleMerger<_Lhs, _Rhs>::type {
    return details::TTupleMerger<_Lhs, _Rhs>::cat(
        std::forward<_Lhs>(lhs), std::forward<_Rhs>(rhs));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Call any function or member function pointer with a tuple containing all args
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
template<typename _Return, typename... _Args, typename... _Prms, size_t... _Index>
FORCE_INLINE _Return CallHelper_(_Return (*func)(_Args...), const TTuple<_Prms...>& args, std::index_sequence<_Index...>) {
    NOOP(args);
    return func(std::get<_Index>(args)...);
}
template<typename _Return, typename... _Args, typename... _Prms, size_t... _Index>
FORCE_INLINE _Return CallHelper_(_Return (*func)(_Args...), TTuple<_Prms...>& args, std::index_sequence<_Index...>) {
    NOOP(args);
    return func(std::get<_Index>(args)...);
}
//----------------------------------------------------------------------------
template<typename _Return, typename _Class, typename... _Args, typename... _Prms, size_t... _Index>
FORCE_INLINE _Return CallHelper_(_Return (_Class::*member)(_Args...), _Class* src, const TTuple<_Prms...>& args, std::index_sequence<_Index...>) {
    NOOP(args);
    return (src->*member)(std::get<_Index>(args)...);
}
template<typename _Return, typename _Class, typename... _Args, typename... _Prms, size_t... _Index>
FORCE_INLINE _Return CallHelper_(_Return(_Class::*member)(_Args...), _Class* src, TTuple<_Prms...>& args, std::index_sequence<_Index...>) {
    NOOP(args);
    return (src->*member)(std::get<_Index>(args)...);
}
//----------------------------------------------------------------------------
template<typename _Return, typename _Class, typename... _Args, typename... _Prms, size_t... _Index>
FORCE_INLINE _Return CallHelper_(_Return(_Class::*member)(_Args...) const, const _Class* src, const TTuple<_Prms...>& args, std::index_sequence<_Index...>) {
    NOOP(args);
    return (src->*member)(std::get<_Index>(args)...);
}
template<typename _Return, typename _Class, typename... _Args, typename... _Prms, size_t... _Index>
FORCE_INLINE _Return CallHelper_(_Return(_Class::*member)(_Args...) const, const _Class* src, TTuple<_Prms...>& args, std::index_sequence<_Index...>) {
    NOOP(args);
    return (src->*member)(std::get<_Index>(args)...);
}
//----------------------------------------------------------------------------
} //!details
//----------------------------------------------------------------------------
template<typename _Return, typename... _Args, typename... _Prms>
_Return CallTuple(_Return (*func)(_Args...), const TTuple<_Prms...>& args) {
    STATIC_ASSERT(sizeof...(_Args) == sizeof...(_Prms));
    return details::CallHelper_(func, args, std::index_sequence_for<_Args...>{});
}
template<typename _Return, typename... _Args, typename... _Prms>
_Return CallTuple(_Return(*func)(_Args...), TTuple<_Prms...>& args) {
    STATIC_ASSERT(sizeof...(_Args) == sizeof...(_Prms));
    return details::CallHelper_(func, args, std::index_sequence_for<_Args...>{});
}
//----------------------------------------------------------------------------
template<typename _Return, typename _Class, typename... _Args, typename... _Prms>
_Return CallTuple(_Return (_Class::*member)(_Args...), _Class* src, const TTuple<_Prms...>& args) {
    STATIC_ASSERT(sizeof...(_Args) == sizeof...(_Prms));
    return details::CallHelper_(member, src, args, std::index_sequence_for<_Args...>{});
}
template<typename _Return, typename _Class, typename... _Args, typename... _Prms>
_Return CallTuple(_Return(_Class::*member)(_Args...), _Class* src, TTuple<_Prms...>& args) {
    STATIC_ASSERT(sizeof...(_Args) == sizeof...(_Prms));
    return details::CallHelper_(member, src, args, std::index_sequence_for<_Args...>{});
}
//----------------------------------------------------------------------------
template<typename _Return, typename _Class, typename... _Args, typename... _Prms>
_Return CallTuple(_Return(_Class::*member)(_Args...) const, const _Class* src, const TTuple<_Prms...>& args) {
    STATIC_ASSERT(sizeof...(_Args) == sizeof...(_Prms));
    return details::CallHelper_(member, src, args, std::index_sequence_for<_Args...>{});
}
template<typename _Return, typename _Class, typename... _Args, typename... _Prms>
_Return CallTuple(_Return(_Class::*member)(_Args...) const, const _Class* src, TTuple<_Prms...>& args) {
    STATIC_ASSERT(sizeof...(_Args) == sizeof...(_Prms));
    return details::CallHelper_(member, src, args, std::index_sequence_for<_Args...>{});
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Call any function or member function pointer with a parameter pack and a tuple containing each part of args
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
template <typename _Ret, typename... _Params, typename... _Extra, typename... _Args, size_t... _Indices>
constexpr _Ret CallTupleEx_(_Ret(*f)(_Params...), const TTuple<_Extra...>& extra, std::index_sequence<_Indices...>, _Args&&... args) {
    return (*f)(std::forward<_Args>(args)..., std::get<_Indices>(extra)...);
}
//----------------------------------------------------------------------------
template <typename _Ret, typename _Class, typename... _Params, typename... _Extra, typename... _Args, size_t... _Indices>
constexpr _Ret CallTupleEx_(_Ret(_Class::*m)(_Params...), const TTuple<_Class*, _Extra...>& extra, std::index_sequence<_Indices...>, _Args&&... args) {
    return (std::get<0>(extra)->*m)(std::forward<_Args>(args)..., std::get<1 + _Indices>(extra)...);
}
template <typename _Ret, typename _Class, typename... _Params, typename... _Extra, typename... _Args, size_t... _Indices>
constexpr _Ret CallTupleEx_(_Ret(_Class::*m)(_Params...), const TTuple<TPtrRef<_Class>, _Extra...>& extra, std::index_sequence<_Indices...>, _Args&&... args) {
    return (std::get<0>(extra)->*m)(std::forward<_Args>(args)..., std::get<1 + _Indices>(extra)...);
}
template <typename _Ret, typename _Class, typename... _Params, typename... _Extra, typename... _Args, size_t... _Indices>
constexpr _Ret CallTupleEx_(_Ret(_Class::*m)(_Params...), const TTuple<TSafePtr<_Class>, _Extra...>& extra, std::index_sequence<_Indices...>, _Args&&... args) {
    return (std::get<0>(extra)->*m)(std::forward<_Args>(args)..., std::get<1 + _Indices>(extra)...);
}
template <typename _Ret, typename _Class, typename... _Params, typename... _Extra, typename... _Args, size_t... _Indices>
constexpr _Ret CallTupleEx_(_Ret(_Class::*m)(_Params...), const TTuple<TRefPtr<_Class>, _Extra...>& extra, std::index_sequence<_Indices...>, _Args&&... args) {
    return (std::get<0>(extra)->*m)(std::forward<_Args>(args)..., std::get<1 + _Indices>(extra)...);
}
//----------------------------------------------------------------------------
template <typename _Ret, typename _Class, typename... _Params, typename... _Extra, typename... _Args, size_t... _Indices>
constexpr _Ret CallTupleEx_(_Ret(_Class::*mc)(_Params...) const, const TTuple<const _Class*, _Extra...>& extra, std::index_sequence<_Indices...>, _Args&&... args) {
    return (std::get<0>(extra)->*mc)(std::forward<_Args>(args)..., std::get<1 + _Indices>(extra)...);
}
template <typename _Ret, typename _Class, typename... _Params, typename... _Extra, typename... _Args, size_t... _Indices>
constexpr _Ret CallTupleEx_(_Ret(_Class::*mc)(_Params...) const, const TTuple<TPtrRef<const _Class>, _Extra...>& extra, std::index_sequence<_Indices...>, _Args&&... args) {
    return (std::get<0>(extra)->*mc)(std::forward<_Args>(args)..., std::get<1 + _Indices>(extra)...);
}
template <typename _Ret, typename _Class, typename... _Params, typename... _Extra, typename... _Args, size_t... _Indices>
constexpr _Ret CallTupleEx_(_Ret(_Class::*mc)(_Params...) const, const TTuple<TSafePtr<const _Class>, _Extra...>& extra, std::index_sequence<_Indices...>, _Args&&... args) {
    return (std::get<0>(extra)->*mc)(std::forward<_Args>(args)..., std::get<1 + _Indices>(extra)...);
}
template <typename _Ret, typename _Class, typename... _Params, typename... _Extra, typename... _Args, size_t... _Indices>
constexpr _Ret CallTupleEx_(_Ret(_Class::*mc)(_Params...) const, const TTuple<TRefPtr<const _Class>, _Extra...>& extra, std::index_sequence<_Indices...>, _Args&&... args) {
    return (std::get<0>(extra)->*mc)(std::forward<_Args>(args)..., std::get<1 + _Indices>(extra)...);
}
//----------------------------------------------------------------------------
} //details
//----------------------------------------------------------------------------
template <typename _Ret, typename... _Params, typename... _Extra, typename... _Args>
constexpr _Ret CallTupleEx(_Ret(*f)(_Params...), const TTuple<_Extra...>& extra, _Args&&... args) {
    return details::CallTupleEx_(f, extra, std::index_sequence_for<_Extra...>{}, std::forward<_Args>(args)...);
}
//----------------------------------------------------------------------------
template <typename _Ret, typename _Class, typename... _Params, typename... _Extra, typename... _Args>
constexpr _Ret CallTupleEx(_Ret(_Class::*m)(_Params...), const TTuple<_Class*, _Extra...>& extra, _Args&&... args) {
    return details::CallTupleEx_(m, extra, std::index_sequence_for<_Extra...>{}, std::forward<_Args>(args)...);
}
template <typename _Ret, typename _Class, typename... _Params, typename... _Extra, typename... _Args>
constexpr _Ret CallTupleEx(_Ret(_Class::*m)(_Params...), const TTuple<TPtrRef<_Class>, _Extra...>& extra, _Args&&... args) {
    return details::CallTupleEx_(m, extra, std::index_sequence_for<_Extra...>{}, std::forward<_Args>(args)...);
}
template <typename _Ret, typename _Class, typename... _Params, typename... _Extra, typename... _Args>
constexpr _Ret CallTupleEx(_Ret(_Class::*m)(_Params...), const TTuple<TSafePtr<_Class>, _Extra...>& extra, _Args&&... args) {
    return details::CallTupleEx_(m, extra, std::index_sequence_for<_Extra...>{}, std::forward<_Args>(args)...);
}
template <typename _Ret, typename _Class, typename... _Params, typename... _Extra, typename... _Args>
constexpr _Ret CallTupleEx(_Ret(_Class::*m)(_Params...), const TTuple<TRefPtr<_Class>, _Extra...>& extra, _Args&&... args) {
    return details::CallTupleEx_(m, extra, std::index_sequence_for<_Extra...>{}, std::forward<_Args>(args)...);
}
//----------------------------------------------------------------------------
template <typename _Ret, typename _Class, typename... _Params, typename... _Extra, typename... _Args>
constexpr _Ret CallTupleEx(_Ret(_Class::*mc)(_Params...) const, const TTuple<const _Class*, _Extra...>& extra, _Args&&... args) {
    return details::CallTupleEx_(mc, extra, std::index_sequence_for<_Extra...>{}, std::forward<_Args>(args)...);
}
template <typename _Ret, typename _Class, typename... _Params, typename... _Extra, typename... _Args>
constexpr _Ret CallTupleEx(_Ret(_Class::*mc)(_Params...) const, const TTuple<TPtrRef<const _Class>, _Extra...>& extra, _Args&&... args) {
    return details::CallTupleEx_(mc, extra, std::index_sequence_for<_Extra...>{}, std::forward<_Args>(args)...);
}
template <typename _Ret, typename _Class, typename... _Params, typename... _Extra, typename... _Args>
constexpr _Ret CallTupleEx(_Ret(_Class::*mc)(_Params...) const, const TTuple<TSafePtr<const _Class>, _Extra...>& extra, _Args&&... args) {
    return details::CallTupleEx_(mc, extra, std::index_sequence_for<_Extra...>{}, std::forward<_Args>(args)...);
}
template <typename _Ret, typename _Class, typename... _Params, typename... _Extra, typename... _Args>
constexpr _Ret CallTupleEx(_Ret(_Class::*mc)(_Params...) const, const TTuple<TRefPtr<const _Class>, _Extra...>& extra, _Args&&... args) {
    return details::CallTupleEx_(mc, extra, std::index_sequence_for<_Extra...>{}, std::forward<_Args>(args)...);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Call a function on every member of tuple and return an array of results
//----------------------------------------------------------------------------
namespace details {
template <typename T, typename _Lambda, typename... _Args, size_t... _Idx>
CONSTEXPR TArray<T, sizeof...(_Args)> MapTuple_(_Lambda map, const TTuple<_Args...>& tuple, std::index_sequence<_Idx...>) {
    return { map(std::get<_Idx>(tuple))... };
}
} //!details
//----------------------------------------------------------------------------
template <typename T, typename... _Args, typename _Lambda>
CONSTEXPR TArray<T, sizeof...(_Args)> MapTuple(const TTuple<_Args...>& tuple, _Lambda map) {
    return details::MapTuple_<T>(map, tuple, std::index_sequence_for<_Args...>{});
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Call a function on every member of tuple and return a tuple of results
//----------------------------------------------------------------------------
namespace details {
template <typename _Lambda, typename... _Args, size_t... _Idx>
CONSTEXPR auto ProjectTuple_(_Lambda projector, const TTuple<_Args...>& tuple, std::index_sequence<_Idx...>) {
    return std::make_tuple(projector(std::get<_Idx>(tuple))... );
}
} //!details
//----------------------------------------------------------------------------
template <typename... _Args, typename _Lambda>
CONSTEXPR auto ProjectTuple(const TTuple<_Args...>& tuple, _Lambda projector) {
    return details::ProjectTuple_(projector, tuple, std::index_sequence_for<_Args...>{});
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
