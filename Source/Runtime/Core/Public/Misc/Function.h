#pragma once

#include "Allocator/TrackingMalloc.h"
#include "Container/TupleHelpers.h"
#include "Memory/PtrRef.h"
#include "Memory/RefPtr.h"
#include "Memory/UniquePtr.h"
#include "Meta/AlignedStorage.h"
#include "Meta/TypeTraits.h"

#include "Misc/Function_fwd.h"

/*
// #TODO : special case for handling TFunction<> combinations
//
// ex : f= [](x){2*x}; g= [](x){x+1}; h= [f, g](x){f(g(x))}
//
//  f+g payloads could fit in situ,
//  but we use TFunction<f>+TFunction<g> so it will never fit !
//
//   #TODO : add an helper which bind TFunction<> with minimal in situ capacity
//           and it should work auto-magically
//   #TODO : add a special wrapper for TFunction<T, 0> (no in situ)
//   #TODO : both tasks up there will need to implement bind outside TFunction<>
//
// *OR*
//
//   #TODO : don't handle the cases mention above (they're quite hard),
//           and provide a new TFunctionExpr<> for compositions, based on the same model than TVectorExpr<>
//
*/

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
// Detects if a type is a TFunction<>
//----------------------------------------------------------------------------
template <typename T>
struct TIsFunction : std::false_type {};
template <typename T, size_t _InSitu>
struct TIsFunction< TFunction<T, _InSitu> > : std::true_type {};
//----------------------------------------------------------------------------
template <typename T>
static CONSTEXPR bool is_function_v = TIsFunction<T>::value;
//----------------------------------------------------------------------------
// TFunctionPayload<T> will hold an allocated payload when not fitting in situ
//----------------------------------------------------------------------------
template <typename T>
class TFunctionPayload : public FRefCountable, Meta::FNonCopyableNorMovable {
    // the payload is shared between all copies since it's always const
    // so FRefCountable is well suited for this job
public:
    explicit CONSTEXPR TFunctionPayload(T&& value) NOEXCEPT
        : _value(std::move(value))
    {}

    template <typename... _Extra>
    explicit CONSTEXPR TFunctionPayload(_Extra&&... extra) NOEXCEPT
        : _value(std::forward<_Extra>(extra)...)
    {}

    CONSTEXPR operator const T& () const NOEXCEPT { return _value; }

private:
    T _value;
};
//----------------------------------------------------------------------------
template <typename T>
using PFunctionPayload = TRefPtr< TFunctionPayload<T> >;
//----------------------------------------------------------------------------
// TFunctionVTable<> describes how to manipulate each function payload
//----------------------------------------------------------------------------
template <typename _Wrapper>
struct TFunctionVTable {
    typedef void(*copy_t)(void*, const void*) NOEXCEPT;
    typedef void(*move_t)(void*, void*) NOEXCEPT;
    typedef void(*destroy_t)(void*) NOEXCEPT;
    typedef bool(*equals_t)(const void*, const void*) NOEXCEPT;
    using wrapper_t = _Wrapper;

    size_t Size;
    copy_t Copy;
    move_t Move;
    destroy_t Destroy;
    equals_t Equals;
    wrapper_t Invoke;

    CONSTEXPR TFunctionVTable(
        size_t size,
        copy_t copy,
        move_t move,
        destroy_t destroy,
        equals_t equals,
        wrapper_t invoke) NOEXCEPT
    :   Size(size)
    ,   Copy(copy)
    ,   Move(move)
    ,   Destroy(destroy)
    ,   Equals(equals)
    ,   Invoke(invoke)
    {}

    CONSTEXPR bool IsDummy() const NOEXCEPT {
        return (nullptr == Invoke);
    }

    static CONSTEXPR equals_t phony_equals{
        [](const void*, const void*) CONSTEXPR NOEXCEPT {
            return false;
        } };

    //////////////////////////////////////////////////////////////////////////

    static CONSTEXPR TFunctionVTable phony(wrapper_t lambda) NOEXCEPT {
        return {
            0,
            [](void*, const void*) CONSTEXPR NOEXCEPT {},
            [](void*, void*) CONSTEXPR NOEXCEPT {},
            [](void*) CONSTEXPR NOEXCEPT {},
            phony_equals,
            lambda
        };
    }

    //////////////////////////////////////////////////////////////////////////

    template <typename T>
    static CONSTEXPR TFunctionVTable make(wrapper_t lambda) NOEXCEPT {
        return {
            sizeof(T),
            make_copy<T>(),
            make_move<T>(),
            make_destroy<T>(),
            make_equals<T>(),
            lambda
        };
    }

    template <typename T>
    static CONSTEXPR copy_t make_copy() NOEXCEPT {
        IF_CONSTEXPR(std::is_copy_constructible_v<T>)
            return [](void* dst, const void* src) CONSTEXPR NOEXCEPT{
                INPLACE_NEW(dst, T){ *static_cast<const T*>(src) };
            };
        else
            return nullptr; // crash when trying to copy
    }

    template <typename T>
    static CONSTEXPR move_t make_move() NOEXCEPT {
        IF_CONSTEXPR(std::is_move_constructible_v<T> || std::is_copy_constructible_v<T>)
            return [](void* dst, void* src) CONSTEXPR NOEXCEPT{
                INPLACE_NEW(dst, T){ std::move(*static_cast<T*>(src)) };
                Meta::Destroy(static_cast<T*>(src)); // expected to destroy after move
            };
        else
            return nullptr; // crash when trying to move
    }

    template <typename T>
    static CONSTEXPR destroy_t make_destroy() {
        return [](void* p) CONSTEXPR NOEXCEPT{
            Meta::Destroy(static_cast<T*>(p)); // simpler control flow to always call dtor
        };
    }

    template <typename T>
    static CONSTEXPR equals_t make_equals() NOEXCEPT {
        IF_CONSTEXPR(Meta::has_equals_v<T>)
            return [](const void* lhs, const void* rhs) CONSTEXPR NOEXCEPT{
                return (*static_cast<const T*>(lhs) == *static_cast<const T*>(rhs));
            };
        else
            return phony_equals; // always different
    }
};
//----------------------------------------------------------------------------
// TFunctionTuple<T> can wrap specific arguments :
// - raw pointers pointing to a FRefCountable are wrapped inside a TSafePtr<>
//----------------------------------------------------------------------------
template <typename T>
struct TFunctionTupleArg { using type = T; };
//----------------------------------------------------------------------------
// Wraps references with pointers to keep value semantics :
template <typename T>
struct TFunctionTupleArg<T&> {
    using type = TPtrRef<T>;
};
//----------------------------------------------------------------------------
// Wraps FRefCountable* within TSafePtr<T> to check for lifetime :
template <typename T>
struct TFunctionTupleArg<T*> {
    using type = Meta::TConditional<
        IsRefCountable<T>::value,
        TSafePtr<T>, TPtrRef<T> >;
};
//----------------------------------------------------------------------------
template <typename... _Args>
using TFunctionTuple = TTupleMake< typename TFunctionTupleArg<_Args>::type... >;
//----------------------------------------------------------------------------
// TFunctionTraits<> implements bindings outside of TFunction<> to be independent of _InSitu
//----------------------------------------------------------------------------
template <typename T>
struct TFunctionTraits;
//----------------------------------------------------------------------------
template <typename _Ret, typename... _Args>
struct TFunctionTraits<_Ret(_Args...)> {
    using native_type = _Ret(_Args...);
    using wrapper_type = _Ret(*)(const void*, _Args...);
    using vtable_type = TFunctionVTable<wrapper_type>;

    template <typename T, typename _R = decltype(std::declval<T>()(std::declval<_Args>()...)) >
    static typename std::is_same<_Ret, _R>::type is_callable_(int);
    template <typename T>
    static std::false_type is_callable_(...);

    template <typename T>
    static CONSTEXPR const bool is_callable_v = decltype(is_callable_<T>(0))::value;

    // using a dummy vtable instead of nullptr leads to much simpler codegen
    static CONSTEXPR const vtable_type dummy_vtable = vtable_type::phony(nullptr);

    template <native_type _FreeFunc>
    struct freefunc_t {
        static CONSTEXPR _Ret invoke(const void*, _Args... args) {
            return _FreeFunc(std::forward<_Args>(args)...);
        }
        static CONSTEXPR vtable_type vtable = vtable_type::phony(&invoke);
        STATIC_ASSERT(vtable.Invoke);
    };

    template <typename _Lambda>
    struct lambda_t {
        template <typename _Payload>
        struct bind_t {
            static CONSTEXPR _Ret invoke(const void* embed, _Args... args) {
                return _Payload::Get(embed)(std::forward<_Args>(args)...);
            }
            static CONSTEXPR vtable_type vtable = vtable_type::template make<typename _Payload::type>(&invoke);
            STATIC_ASSERT(vtable.Invoke);
        };
    };

    template <typename _OtherFunc>
    struct otherfunc_t;

    template <typename... _Extra>
    struct otherfunc_t<_Ret(*)(_Args..., _Extra...)> {
        template <typename... _ExtraArgs>
        struct extra_t {
            using type = details::TFunctionTuple<_ExtraArgs...>;
        };
        template <_Ret(*_ExtraFunc)(_Args..., _Extra...), typename _Payload>
        struct bind_t {
            static CONSTEXPR _Ret invoke(const void* embed, _Args... args) {
                return CallTupleEx(_ExtraFunc, _Payload::Get(embed), std::forward<_Args>(args)...);
            }
            static CONSTEXPR vtable_type vtable = vtable_type::template make<typename _Payload::type>(&invoke);
            STATIC_ASSERT(vtable.Invoke);
        };
    };

    template <typename _Class, typename... _Extra>
    struct otherfunc_t<_Ret(_Class::*)(_Args..., _Extra...)> {
        template <typename... _ExtraArgs>
        struct extra_t;
        template <typename... _ExtraArgs>
        struct extra_t<_Class*, _ExtraArgs...> {
            using type = details::TFunctionTuple<_Class*, _ExtraArgs...>;
        };
        template <_Ret(_Class::*_MemFunc)(_Args..., _Extra...), typename _Payload>
        struct bind_t {
            static CONSTEXPR _Ret invoke(const void* embed, _Args... args) {
                return CallTupleEx(_MemFunc, _Payload::Get(embed), std::forward<_Args>(args)...);
            }
            static CONSTEXPR vtable_type vtable = vtable_type::template make<typename _Payload::type>(&invoke);
            STATIC_ASSERT(vtable.Invoke);
        };
    };

    template <typename _Class, typename... _Extra>
    struct otherfunc_t<_Ret(_Class::*)(_Args..., _Extra...) const> {
        template <typename... _ExtraArgs>
        struct extra_t;
        template <typename... _ExtraArgs>
        struct extra_t<_Class*, _ExtraArgs...> {
            using type = details::TFunctionTuple<const _Class*, _ExtraArgs...>;
        };
        template <typename... _ExtraArgs>
        struct extra_t<const _Class*, _ExtraArgs...> {
            using type = details::TFunctionTuple<const _Class*, _ExtraArgs...>;
        };
        template <_Ret(_Class::*_MemFuncConst)(_Args..., _Extra...) const, typename _Payload>
        struct bind_t {
            static CONSTEXPR _Ret invoke(const void* embed, _Args... args) {
                return CallTupleEx(_MemFuncConst, _Payload::Get(embed), std::forward<_Args>(args)...);
            }
            static CONSTEXPR vtable_type vtable = vtable_type::template make<typename _Payload::type>(&invoke);
            STATIC_ASSERT(vtable.Invoke);
        };
    };
};
//----------------------------------------------------------------------------
// TPayloadTraits<> can wrap the payload in a PFunctionPayload<> if it won't fit in situ
//----------------------------------------------------------------------------
template <typename, typename>
struct TPayloadTraits;
//----------------------------------------------------------------------------
template <typename T>
struct TPayloadTraits< T, T > {
    using type = T;
    static CONSTEXPR const T& Get(const void* embed) NOEXCEPT {
        return (*static_cast<const T*>(embed));
    }
    template <typename... _Extra>
    static CONSTEXPR void Set(void* embed, _Extra&&... extra) NOEXCEPT {
        INPLACE_NEW(embed, T) { std::forward<_Extra>(extra)... };
    }
};
//----------------------------------------------------------------------------
template <typename T>
struct TPayloadTraits< T, PFunctionPayload<T> > {
    using type = PFunctionPayload<T>;
    static const T& Get(const void* embed) NOEXCEPT {
        return (**static_cast<const details::PFunctionPayload<T>*>(embed));
    }
    template <typename... _Extra>
    static void Set(void* embed, _Extra&&... extra) NOEXCEPT {
        INPLACE_NEW(embed, PFunctionPayload<T>) {
            NEW_REF(Function, TFunctionPayload<T>) { std::forward<_Extra>(extra)... }
        };
    }
};
//----------------------------------------------------------------------------
} //!details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Ret, typename... _Args, size_t _InSitu>
class TFunction<_Ret(_Args...), _InSitu> {
public:
    using embed_type = std::aligned_storage_t<_InSitu>;
    using traits_type = details::TFunctionTraits<_Ret(_Args...)>;
    using native_type = typename traits_type::native_type;
    using wrapper_type = typename traits_type::wrapper_type;
    using vtable_type = typename traits_type::vtable_type;

private:
    const vtable_type* _vtable;
    embed_type _embed;

public:
    CONSTEXPR TFunction() NOEXCEPT : TFunction(&traits_type::dummy_vtable) {}
    CONSTEXPR explicit TFunction(const vtable_type* vtable) NOEXCEPT
    :   _vtable(vtable)
    {}

    ~TFunction() NOEXCEPT {
        _vtable->Destroy(&_embed);
    }

    CONSTEXPR TFunction(const TFunction& other) NOEXCEPT
    :   _vtable(other._vtable) {
        _vtable->Copy(&_embed, &other._embed);
    }
    CONSTEXPR TFunction& operator =(const TFunction& other) NOEXCEPT {
        _vtable->Destroy(&_embed);
        _vtable = other._vtable;
        _vtable->Copy(&_embed, &other._embed);
        return (*this);
    }

    CONSTEXPR TFunction(TFunction&& rvalue) NOEXCEPT
    :   _vtable(rvalue._vtable) {
        _vtable->Move(&_embed, &rvalue._embed);
        rvalue._vtable = &traits_type::dummy_vtable;
    }
    CONSTEXPR TFunction& operator =(TFunction&& rvalue) NOEXCEPT {
        _vtable->Destroy(&_embed);
        _vtable = rvalue._vtable;
        _vtable->Move(&_embed, &rvalue._embed);
        rvalue._vtable = &traits_type::dummy_vtable;
        return (*this);
    }

    CONSTEXPR bool Valid() const NOEXCEPT {
        return (not _vtable->IsDummy());
    }

    CONSTEXPR operator const void* () const NOEXCEPT {
        return (_vtable->IsDummy() ? nullptr : this);
    }

    CONSTEXPR _Ret operator()(_Args... args) const {
        return _vtable->Invoke(&_embed, std::forward<_Args>(args)...);
    }

    CONSTEXPR _Ret Invoke(_Args... args) const {
        return _vtable->Invoke(&_embed, std::forward<_Args>(args)...);
    }

    CONSTEXPR void FireAndForget(_Args... args) {
        Invoke(std::forward<_Args>(args)...);
        Reset();
    }

    CONSTEXPR void Reset() NOEXCEPT {
        _vtable->Destroy(&_embed);
        _vtable = &traits_type::dummy_vtable;
    }

    template <size_t S>
    CONSTEXPR void Reset(const TFunction<native_type, S>& other) NOEXCEPT {
        AssertRelease(other._vtable->Size <= _InSitu);
        _vtable->Destroy(&_embed);
        _vtable = other._vtable;
        _vtable->Copy(&_embed, &other._embed);
    }

    inline void swap(TFunction& lhs, TFunction& rhs) { std::swap(lhs, rhs); }

    inline friend CONSTEXPR bool operator ==(const TFunction& lhs, const TFunction& rhs) NOEXCEPT {
        return (lhs._vtable == rhs._vtable && lhs._vtable->Equals(&lhs._embed, &rhs._embed));
    }
    inline friend CONSTEXPR bool operator !=(const TFunction& lhs, const TFunction& rhs) NOEXCEPT {
        return (not operator ==(lhs, rhs));
    }

    /***********************************************************************
     ** Payload (insitu or allocated)
     ***********************************************************************/

    template <typename T>
    using payload_traits_t = details::TPayloadTraits<
        T,
        Meta::TConditional<
            (sizeof(T) <= _InSitu),
            T,
            details::PFunctionPayload<T>
        >
    >;

    template <typename _PayloadTraits, typename... _Extra>
    CONSTEXPR TFunction& SetPayloadLarge(_Extra&&... extra) {
        _PayloadTraits::Set(&_embed, std::forward<_Extra>(extra)...);
        return (*this);
    }

    template <typename _PayloadTraits, typename... _Extra>
    CONSTEXPR TFunction& SetPayload(_Extra&&... extra) NOEXCEPT {
        using payload_t = typename _PayloadTraits::type;
        static_assert(sizeof(payload_t) <= _InSitu, "payload won't fit in situ");
        return SetPayloadLarge<_PayloadTraits>(std::forward<_Extra>(extra)...);
    }

    /***********************************************************************
     ** Free funcs
     ***********************************************************************/

    template <native_type _FreeFunc>
    static CONSTEXPR auto Bind() NOEXCEPT {
        return TFunction{ &traits_type::template freefunc_t<_FreeFunc>::vtable };
    }

    /***********************************************************************
     ** Lambdas
     ***********************************************************************/

    template <typename _Lambda>
    static CONSTEXPR auto Bind(_Lambda&& lambda) NOEXCEPT {
        using f = Meta::TDecay<_Lambda>;
        using payload_t = payload_traits_t<f>;
        return TFunction{ &traits_type::template lambda_t<f>::template bind_t<payload_t>::vtable }
            .SetPayload<payload_t>(std::forward<_Lambda>(lambda));
    }

    template <typename _Lambda>
    static CONSTEXPR auto BindLarge(_Lambda&& lambda) {
        using f = Meta::TDecay<_Lambda>;
        using payload_t = payload_traits_t<f>;
        return TFunction{ &traits_type::template lambda_t<f>::template bind_t<payload_t>::vtable }
            .SetPayloadLarge<payload_t>(std::forward<_Lambda>(lambda));
    }

    // lambdas have also implicit constructors :

    template <typename _Lambda>
    static CONSTEXPR bool is_callable_lambda_v = (
        (traits_type::template is_callable_v<_Lambda>) &&
        (not details::is_function_v< Meta::TDecay<_Lambda> >) );

    template <typename _Lambda, class = Meta::TEnableIf<is_callable_lambda_v<_Lambda>> >
    CONSTEXPR TFunction(_Lambda&& lambda) NOEXCEPT
    :   TFunction( &traits_type
            ::template lambda_t< Meta::TDecay<_Lambda> >
            ::template bind_t< payload_traits_t< Meta::TDecay<_Lambda> > >
            ::vtable ) {
        using payload_t = payload_traits_t< Meta::TDecay<_Lambda> >;
        SetPayload<payload_t>(std::forward<_Lambda>(lambda));
    }

    template <typename _Lambda, class = Meta::TEnableIf<is_callable_lambda_v<_Lambda>> >
    CONSTEXPR TFunction(Meta::FForceInit, _Lambda&& lambda)
    :   TFunction( &traits_type
            ::template lambda_t< Meta::TDecay<_Lambda> >
            ::template bind_t< payload_traits_t< Meta::TDecay<_Lambda> > >
            ::vtable ) {
        using payload_t = payload_traits_t< Meta::TDecay<_Lambda> >;
        SetPayloadLarge<payload_t>(std::forward<_Lambda>(lambda));
    }

    /***********************************************************************
     ** Bind other types generically
     ***********************************************************************/

    template <auto _OtherFunc, typename... _Extra>
    static CONSTEXPR auto Bind(_Extra... extra) NOEXCEPT {
        using func_t = typename traits_type::template otherfunc_t<decltype(_OtherFunc)>;
        using payload_t = payload_traits_t< typename func_t::template extra_t<_Extra...>::type >;
        return TFunction{ &func_t::template bind_t<_OtherFunc, payload_t>::vtable }
            .SetPayload<payload_t>(std::forward<_Extra>(extra)...);
    }

    template <auto _OtherFunc, typename... _Extra>
    static CONSTEXPR auto BindLarge(_Extra... extra) {
        using func_t = typename traits_type::template otherfunc_t<decltype(_OtherFunc)>;
        using payload_t = payload_traits_t< typename func_t::template extra_t<_Extra...>::type >;
        return TFunction{ &func_t::template bind_t<_OtherFunc, payload_t>::vtable }
            .SetPayloadLarge<payload_t>(std::forward<_Extra>(extra)...);
    }

}; //!TFunction
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// MakeFunction()
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
template <typename T>
struct infer_function_t;
//----------------------------------------------------------------------------
template <typename _Ret, typename... _Args>
struct infer_function_t<_Ret(_Args...)> {
using type = TFunction<_Ret(_Args...)>;
};
//----------------------------------------------------------------------------
template <typename _Ret, typename... _Args>
struct infer_function_t<_Ret(*)(_Args...)> {
using type = TFunction<_Ret(_Args...)>;
};
//----------------------------------------------------------------------------
template <typename _Ret, typename _Class, typename... _Args>
struct infer_function_t<_Ret(_Class::*)(_Args...)> {
using type = TFunction<_Ret(_Args...)>;
};
//----------------------------------------------------------------------------
template <typename _Ret, typename _Class, typename... _Args>
struct infer_function_t<_Ret(_Class::*)(_Args...) const> {
using type = TFunction<_Ret(_Args...)>;
};
//----------------------------------------------------------------------------
} //!details
//----------------------------------------------------------------------------
template <auto _Func, typename... _Extra>
CONSTEXPR auto MakeFunction(_Extra&&... extra) NOEXCEPT {
    using func_t = typename details::infer_function_t<decltype(_Func)>::type;
    return func_t::template Bind<_Func>(std::forward<_Extra>(extra)...);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// NoFunction
//----------------------------------------------------------------------------
struct FNoFunction {
    template <typename T, size_t N>
    CONSTEXPR operator TFunction<T, N> () const NOEXCEPT {
        return TFunction<T, N>{};
    }
};
//----------------------------------------------------------------------------
CONSTEXPR const FNoFunction NoFunction;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
