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
#define PPE_CALLTUPLE_IMPL_(_FUNCALL, _NOEXCEPT) \
    STATIC_ASSERT(sizeof...(_Args) == sizeof...(_Prms)); \
    return Meta::static_for<sizeof...(_Args)>([&](auto... idx) _NOEXCEPT { \
        return (_FUNCALL)(std::get<idx>(args)...); \
    })
#define PPE_CALLTUPLE_DEF(_CONST, _NOEXCEPT) \
    /* -- free functions -- */ \
    template<typename _Return, typename... _Args, typename... _Prms> \
    FORCE_INLINE _Return CallTuple(_Return (*func)(_Args...) _NOEXCEPT, _CONST TTuple<_Prms...>& args) _NOEXCEPT { \
        PPE_CALLTUPLE_IMPL_(*func, _NOEXCEPT); \
    } \
    /* -- member function -- */ \
    template<typename _Return, typename _Class, typename... _Args, typename... _Prms> \
    FORCE_INLINE _Return CallTuple(_Return (_Class::*member)(_Args...) _NOEXCEPT, _Class* src, _CONST TTuple<_Prms...>& args) _NOEXCEPT { \
        PPE_CALLTUPLE_IMPL_(src->*member, _NOEXCEPT); \
    } \
    /* -- const member functions -- */ \
    template<typename _Return, typename _Class, typename... _Args, typename... _Prms> \
    FORCE_INLINE _Return CallTuple(_Return(_Class::*member)(_Args...) const _NOEXCEPT, const _Class* src, _CONST TTuple<_Prms...>& args) _NOEXCEPT { \
        PPE_CALLTUPLE_IMPL_(src->*member, _NOEXCEPT); \
    }
PPE_CALLTUPLE_DEF(     ,         )
PPE_CALLTUPLE_DEF(     , noexcept)
PPE_CALLTUPLE_DEF(const,         )
PPE_CALLTUPLE_DEF(const, noexcept)
#undef PPE_CALLTUPLE_DEF
#undef PPE_CALLTUPLE_IMPL_
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Call any function or member function pointer with a parameter pack and a tuple containing each part of args
//----------------------------------------------------------------------------
#define PPE_CALLTUPLE_EX_IMPL_(_FUNCALL, _NOEXCEPT, _OFFSET) \
    STATIC_ASSERT(sizeof...(_Args) + sizeof...(_Extra) == sizeof...(_Params)); \
    return Meta::static_for<sizeof...(_Extra)>([&](auto... idx) _NOEXCEPT { \
        return (_FUNCALL)(std::forward<_Args>(args)..., std::get<_OFFSET + idx>(extra)...); \
    })
#define PPE_CALLTUPLE_EX_DEF_(_CONST, _NOEXCEPT) \
    /* -- free functions -- */ \
    template <typename _Ret, typename... _Params, typename... _Extra, typename... _Args> \
    FORCE_INLINE constexpr _Ret CallTupleEx(_Ret(*f)(_Params...) _NOEXCEPT, _CONST TTuple<_Extra...>& extra, _Args&&... args) _NOEXCEPT { \
        PPE_CALLTUPLE_EX_IMPL_(*f, _NOEXCEPT, 0); \
    } \
    /* -- member functions -- */ \
    template <typename _Ret, typename _Class, typename... _Params, typename... _Extra, typename... _Args> \
    FORCE_INLINE constexpr _Ret CallTupleEx(_Ret(_Class::*m)(_Params...) _NOEXCEPT, _CONST TTuple<_Class*, _Extra...>& extra, _Args&&... args) _NOEXCEPT { \
        PPE_CALLTUPLE_EX_IMPL_(std::get<0>(extra)->*m, _NOEXCEPT, 1); \
    } \
    template <typename _Ret, typename _Class, typename... _Params, typename... _Extra, typename... _Args> \
    FORCE_INLINE constexpr _Ret CallTupleEx(_Ret(_Class::*m)(_Params...) _NOEXCEPT, _CONST TTuple<TPtrRef<_Class>, _Extra...>& extra, _Args&&... args) _NOEXCEPT { \
        PPE_CALLTUPLE_EX_IMPL_(std::get<0>(extra).get()->*m, _NOEXCEPT, 1); \
    } \
    template <typename _Ret, typename _Class, typename... _Params, typename... _Extra, typename... _Args> \
    FORCE_INLINE constexpr _Ret CallTupleEx(_Ret(_Class::*m)(_Params...) _NOEXCEPT, _CONST TTuple<TSafePtr<_Class>, _Extra...>& extra, _Args&&... args) _NOEXCEPT { \
        PPE_CALLTUPLE_EX_IMPL_(std::get<0>(extra).get()->*m, _NOEXCEPT, 1); \
    } \
    template <typename _Ret, typename _Class, typename... _Params, typename... _Extra, typename... _Args> \
    FORCE_INLINE constexpr _Ret CallTupleEx(_Ret(_Class::*m)(_Params...) _NOEXCEPT, _CONST TTuple<TRefPtr<_Class>, _Extra...>& extra, _Args&&... args) _NOEXCEPT { \
        PPE_CALLTUPLE_EX_IMPL_(std::get<0>(extra).get()->*m, _NOEXCEPT, 1); \
    } \
    /* - const member functions -- */ \
    template <typename _Ret, typename _Class, typename... _Params, typename... _Extra, typename... _Args> \
    FORCE_INLINE constexpr _Ret CallTupleEx(_Ret(_Class::*mc)(_Params...) const _NOEXCEPT, _CONST TTuple<const _Class*, _Extra...>& extra, _Args&&... args) _NOEXCEPT { \
        PPE_CALLTUPLE_EX_IMPL_(std::get<0>(extra)->*mc, _NOEXCEPT, 1); \
    } \
    template <typename _Ret, typename _Class, typename... _Params, typename... _Extra, typename... _Args> \
    FORCE_INLINE constexpr _Ret CallTupleEx(_Ret(_Class::*mc)(_Params...) const _NOEXCEPT, _CONST TTuple<TPtrRef<const _Class>, _Extra...>& extra, _Args&&... args) _NOEXCEPT { \
        PPE_CALLTUPLE_EX_IMPL_(std::get<0>(extra).get()->*mc, _NOEXCEPT, 1); \
    } \
    template <typename _Ret, typename _Class, typename... _Params, typename... _Extra, typename... _Args> \
    FORCE_INLINE constexpr _Ret CallTupleEx(_Ret(_Class::*mc)(_Params...) const _NOEXCEPT, _CONST TTuple<TSafePtr<const _Class>, _Extra...>& extra, _Args&&... args) _NOEXCEPT { \
        PPE_CALLTUPLE_EX_IMPL_(std::get<0>(extra).get()->*mc, _NOEXCEPT, 1); \
    } \
    template <typename _Ret, typename _Class, typename... _Params, typename... _Extra, typename... _Args> \
    FORCE_INLINE constexpr _Ret CallTupleEx(_Ret(_Class::*mc)(_Params...) const _NOEXCEPT, _CONST TTuple<TRefPtr<const _Class>, _Extra...>& extra, _Args&&... args) _NOEXCEPT { \
        PPE_CALLTUPLE_EX_IMPL_(std::get<0>(extra).get()->*mc, _NOEXCEPT, 1); \
    }
PPE_CALLTUPLE_EX_DEF_(     ,         )
PPE_CALLTUPLE_EX_DEF_(     , noexcept)
PPE_CALLTUPLE_EX_DEF_(const,         )
PPE_CALLTUPLE_EX_DEF_(const, noexcept)
#undef PPE_CALLTUPLE_EX_DEF_
#undef PPE_CALLTUPLE_EX_IMPL_
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Call a function on every member of tuple and return an array of results
//----------------------------------------------------------------------------
template <typename T, typename... _Args, typename _Lambda>
CONSTEXPR TStaticArray<T, sizeof...(_Args)> MapTuple(const TTuple<_Args...>& tuple, _Lambda map) {
    return Meta::static_for<sizeof...(_Args)>([&](auto... idx) -> TStaticArray<T, sizeof...(_Args)> {
       return { map(std::get<idx>(tuple))... };
    });
}
//----------------------------------------------------------------------------
// Call a function on every member of tuple and return a tuple of results
//----------------------------------------------------------------------------
template <typename... _Args, typename _Lambda>
CONSTEXPR auto ProjectTuple(const TTuple<_Args...>& tuple, _Lambda projector) {
    Meta::static_for<sizeof...(_Args)>([&](auto... idx) {
       return std::make_tuple(projector(std::get<idx>(tuple))...);
    });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
