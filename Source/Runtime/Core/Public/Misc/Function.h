#pragma once

#include "Allocator/TrackingMalloc.h"
#include "Container/TupleHelpers.h"
#include "Memory/PtrRef.h"
#include "Memory/RefPtr.h"
#include "Memory/WeakPtr.h"
#include "Meta/PointerWFlags.h"
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
enum class EFunctionFlags : u32 {
    None            = 0,
    FireAndForget   = 1<<0,
    FitInSitu       = 1<<1,
};
ENUM_FLAGS(EFunctionFlags);
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
// Deduce different types of functions pointers generically
//----------------------------------------------------------------------------
template <typename T>
struct TInferFunction;
//----------------------------------------------------------------------------
template <typename _Ret, typename... _Args>
struct TInferFunction<_Ret(_Args...)> {
    using bind_type = TFunction<_Ret(_Args...)>;
    using noexcept_type = _Ret(_Args...) NOEXCEPT;
};
//----------------------------------------------------------------------------
template <typename _Ret, typename... _Args>
struct TInferFunction<_Ret(_Args...) NOEXCEPT> {
    using bind_type = TFunction<_Ret(_Args...) NOEXCEPT>;
    using noexcept_type = _Ret(_Args...) NOEXCEPT;
};
//----------------------------------------------------------------------------
template <typename _Ret, typename... _Args>
struct TInferFunction<_Ret(*)(_Args...)> {
    using bind_type = TFunction<_Ret(_Args...)>;
    using noexcept_type = _Ret(*)(_Args...) NOEXCEPT;
};
//----------------------------------------------------------------------------
template <typename _Ret, typename... _Args>
struct TInferFunction<_Ret(*)(_Args...) NOEXCEPT> {
    using bind_type = TFunction<_Ret(_Args...) NOEXCEPT>;
    using noexcept_type = _Ret(*)(_Args...) NOEXCEPT;
};
//----------------------------------------------------------------------------
template <typename _Ret, typename _Class, typename... _Args>
struct TInferFunction<_Ret(_Class::*)(_Args...)> {
    using bind_type = TFunction<_Ret(_Args...)>;
    using noexcept_type = _Ret(_Class::*)(_Args...) NOEXCEPT;
};
//----------------------------------------------------------------------------
template <typename _Ret, typename _Class, typename... _Args>
struct TInferFunction<_Ret(_Class::*)(_Args...) NOEXCEPT> {
    using bind_type = TFunction<_Ret(_Args...) NOEXCEPT>;
    using noexcept_type = _Ret(_Class::*)(_Args...) NOEXCEPT;
};
//----------------------------------------------------------------------------
template <typename _Ret, typename _Class, typename... _Args>
struct TInferFunction<_Ret(_Class::*)(_Args...) const> {
    using bind_type = TFunction<_Ret(_Args...)>;
    using noexcept_type = _Ret(_Class::*)(_Args...) const NOEXCEPT;
};
//----------------------------------------------------------------------------
template <typename _Ret, typename _Class, typename... _Args>
struct TInferFunction<_Ret(_Class::*)(_Args...) const NOEXCEPT> {
    using bind_type = TFunction<_Ret(_Args...) NOEXCEPT>;
    using noexcept_type = _Ret(_Class::*)(_Args...) const NOEXCEPT;
};
//----------------------------------------------------------------------------
template <typename T, bool _NoExcept>
struct TNoExceptFunction {
    using type = Meta::TConditional<_NoExcept,
        typename TInferFunction<T>::noexcept_type,
        T >;
};
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
using PFunctionPayload = TRefPtr< const TFunctionPayload<T> >;
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
            return[](void* dst, void* src) CONSTEXPR NOEXCEPT{
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
        IF_CONSTEXPR(Meta::has_trivial_equal_v<T>)
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
template <>
struct TFunctionTupleArg<void*> { using type = void*; };
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
        IsRefCountable<T>::value ||
        IsWeakRefCountable<T>::value,
        TSafePtr<T>, TPtrRef<T> >;
};
//----------------------------------------------------------------------------
template <typename... _Args>
using TFunctionTuple = TTupleMake< typename TFunctionTupleArg<_Args>::type... >;
//----------------------------------------------------------------------------
// TFunctionTraits<> implements bindings outside of TFunction<> to be independent of _InSitu
//----------------------------------------------------------------------------
// Note: use template specialization here since MSVC struggles with overloading based on NOEXCEPT_IF(bool)
// Because of this we have to copy/paste TFunctionTraits<> both with and without NOEXCEPT :'(
template <typename _Ret, bool _NoExcept, typename... _Args>
struct TFunctionTraits;
//----------------------------------------------------------------------------
template <typename _Ret, typename... _Args>
struct TFunctionTraits<_Ret, false, _Args...> {
    STATIC_CONST_INTEGRAL(bool, NoExcept, false);
    using native_type = typename TNoExceptFunction<_Ret(_Args...), NoExcept>::type;
    using wrapper_type = typename TNoExceptFunction<_Ret(*)(const void*, _Args...), NoExcept>::type;
    using vtable_type = TFunctionVTable<wrapper_type>;

    // check if T is callable with current function signature
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
    struct otherfunc_t {
        template <typename... _ExtraArgs>
        struct extra_t;
    };

    template <typename... _Extra>
    struct otherfunc_t<_Ret(*)(_Args..., _Extra...)> {
        template <typename... _ExtraArgs>
        struct extra_t {
            using type = TFunctionTuple<_ExtraArgs...>;
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
        using class_type = _Class;
        template <typename... _ExtraArgs>
        struct extra_t;
        template <typename... _ExtraArgs>
        struct extra_t<class_type*, _ExtraArgs...> {
            using type = TFunctionTuple<class_type*, _ExtraArgs...>;
        };
        template <typename... _ExtraArgs>
        struct extra_t<TPtrRef<class_type>, _ExtraArgs...> {
            using type = TFunctionTuple<TPtrRef<class_type>, _ExtraArgs...>;
        };
        template <typename... _ExtraArgs>
        struct extra_t<TSafePtr<class_type>, _ExtraArgs...> {
            using type = TFunctionTuple<TSafePtr<class_type>, _ExtraArgs...>;
        };
        template <typename... _ExtraArgs>
        struct extra_t<TRefPtr<class_type>, _ExtraArgs...> {
            using type = TFunctionTuple<TRefPtr<class_type>, _ExtraArgs...>;
        };
        template <_Ret(class_type::* _MemFunc)(_Args..., _Extra...), typename _Payload>
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
        using class_type = _Class;
        template <typename... _ExtraArgs>
        struct extra_t;
        template <typename... _ExtraArgs>
        struct extra_t<class_type*, _ExtraArgs...> {
            using type = TFunctionTuple<const class_type*, _ExtraArgs...>;
        };
        template <typename... _ExtraArgs>
        struct extra_t<TPtrRef<class_type>, _ExtraArgs...> {
            using type = TFunctionTuple<TPtrRef<const class_type>, _ExtraArgs...>;
        };
        template <typename... _ExtraArgs>
        struct extra_t<TSafePtr<class_type>, _ExtraArgs...> {
            using type = TFunctionTuple<TSafePtr<const class_type>, _ExtraArgs...>;
        };
        template <typename... _ExtraArgs>
        struct extra_t<TRefPtr<class_type>, _ExtraArgs...> {
            using type = TFunctionTuple<TRefPtr<const class_type>, _ExtraArgs...>;
        };
        template <typename... _ExtraArgs>
        struct extra_t<const class_type*, _ExtraArgs...> {
            using type = TFunctionTuple<const class_type*, _ExtraArgs...>;
        };
        template <typename... _ExtraArgs>
        struct extra_t<TPtrRef<const class_type>, _ExtraArgs...> {
            using type = TFunctionTuple<TPtrRef<const class_type>, _ExtraArgs...>;
        };
        template <typename... _ExtraArgs>
        struct extra_t<TSafePtr<const class_type>, _ExtraArgs...> {
            using type = TFunctionTuple<TSafePtr<const class_type>, _ExtraArgs...>;
        };
        template <typename... _ExtraArgs>
        struct extra_t<TRefPtr<const class_type>, _ExtraArgs...> {
            using type = TFunctionTuple<TRefPtr<const class_type>, _ExtraArgs...>;
        };
        template <_Ret(class_type::* _MemFuncConst)(_Args..., _Extra...) const, typename _Payload>
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
template <typename _Ret, typename... _Args>
struct TFunctionTraits<_Ret, true, _Args...> {
    STATIC_CONST_INTEGRAL(bool, NoExcept, true);
    using native_type = typename TNoExceptFunction<_Ret(_Args...), NoExcept>::type;
    using wrapper_type = typename TNoExceptFunction<_Ret(*)(const void*, _Args...), NoExcept>::type;
    using vtable_type = TFunctionVTable<wrapper_type>;

    // check if T is callable with current function signature
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
        static CONSTEXPR _Ret invoke(const void*, _Args... args) NOEXCEPT {
            return _FreeFunc(std::forward<_Args>(args)...);
        }
        static CONSTEXPR vtable_type vtable = vtable_type::phony(&invoke);
        STATIC_ASSERT(vtable.Invoke);
    };

    template <typename _Lambda>
    struct lambda_t {
        template <typename _Payload>
        struct bind_t {
            static CONSTEXPR _Ret invoke(const void* embed, _Args... args) NOEXCEPT {
                return _Payload::Get(embed)(std::forward<_Args>(args)...);
            }
            static CONSTEXPR vtable_type vtable = vtable_type::template make<typename _Payload::type>(&invoke);
            STATIC_ASSERT(vtable.Invoke);
        };
    };

    template <typename _OtherFunc>
    struct otherfunc_t {
        template <typename... _ExtraArgs>
        struct extra_t;
    };

    template <typename... _Extra>
    struct otherfunc_t<_Ret(*)(_Args..., _Extra...) NOEXCEPT> {
        template <typename... _ExtraArgs>
        struct extra_t {
            using type = TFunctionTuple<_ExtraArgs...>;
        };
        template <_Ret(*_ExtraFunc)(_Args..., _Extra...) NOEXCEPT, typename _Payload>
        struct bind_t {
            static CONSTEXPR _Ret invoke(const void* embed, _Args... args) NOEXCEPT {
                return CallTupleEx(_ExtraFunc, _Payload::Get(embed), std::forward<_Args>(args)...);
            }
            static CONSTEXPR vtable_type vtable = vtable_type::template make<typename _Payload::type>(&invoke);
            STATIC_ASSERT(vtable.Invoke);
        };
    };

    template <typename _Class, typename... _Extra>
    struct otherfunc_t<_Ret(_Class::*)(_Args..., _Extra...) NOEXCEPT> {
        using class_type = _Class;
        template <typename... _ExtraArgs>
        struct extra_t;
        template <typename... _ExtraArgs>
        struct extra_t<class_type*, _ExtraArgs...> {
            using type = TFunctionTuple<class_type*, _ExtraArgs...>;
        };
        template <typename... _ExtraArgs>
        struct extra_t<TPtrRef<class_type>, _ExtraArgs...> {
            using type = TFunctionTuple<TPtrRef<class_type>, _ExtraArgs...>;
        };
        template <typename... _ExtraArgs>
        struct extra_t<TSafePtr<class_type>, _ExtraArgs...> {
            using type = TFunctionTuple<TSafePtr<class_type>, _ExtraArgs...>;
        };
        template <typename... _ExtraArgs>
        struct extra_t<TRefPtr<class_type>, _ExtraArgs...> {
            using type = TFunctionTuple<TRefPtr<class_type>, _ExtraArgs...>;
        };
        template <_Ret(class_type::* _MemFunc)(_Args..., _Extra...) NOEXCEPT, typename _Payload>
        struct bind_t {
            static CONSTEXPR _Ret invoke(const void* embed, _Args... args) NOEXCEPT {
                return CallTupleEx(_MemFunc, _Payload::Get(embed), std::forward<_Args>(args)...);
            }
            static CONSTEXPR vtable_type vtable = vtable_type::template make<typename _Payload::type>(&invoke);
            STATIC_ASSERT(vtable.Invoke);
        };
    };

    template <typename _Class, typename... _Extra>
    struct otherfunc_t<_Ret(_Class::*)(_Args..., _Extra...) const NOEXCEPT> {
        using class_type = _Class;
        template <typename... _ExtraArgs>
        struct extra_t;
        template <typename... _ExtraArgs>
        struct extra_t<class_type*, _ExtraArgs...> {
            using type = TFunctionTuple<const class_type*, _ExtraArgs...>;
        };
        template <typename... _ExtraArgs>
        struct extra_t<TPtrRef<class_type>, _ExtraArgs...> {
            using type = TFunctionTuple<TPtrRef<const class_type>, _ExtraArgs...>;
        };
        template <typename... _ExtraArgs>
        struct extra_t<TSafePtr<class_type>, _ExtraArgs...> {
            using type = TFunctionTuple<TSafePtr<const class_type>, _ExtraArgs...>;
        };
        template <typename... _ExtraArgs>
        struct extra_t<TRefPtr<class_type>, _ExtraArgs...> {
            using type = TFunctionTuple<TRefPtr<const class_type>, _ExtraArgs...>;
        };
        template <typename... _ExtraArgs>
        struct extra_t<const class_type*, _ExtraArgs...> {
            using type = TFunctionTuple<const class_type*, _ExtraArgs...>;
        };
        template <typename... _ExtraArgs>
        struct extra_t<TPtrRef<const class_type>, _ExtraArgs...> {
            using type = TFunctionTuple<TPtrRef<const class_type>, _ExtraArgs...>;
        };
        template <typename... _ExtraArgs>
        struct extra_t<TSafePtr<const class_type>, _ExtraArgs...> {
            using type = TFunctionTuple<TSafePtr<const class_type>, _ExtraArgs...>;
        };
        template <typename... _ExtraArgs>
        struct extra_t<TRefPtr<const class_type>, _ExtraArgs...> {
            using type = TFunctionTuple<TRefPtr<const class_type>, _ExtraArgs...>;
        };
        template <_Ret(class_type::* _MemFuncConst)(_Args..., _Extra...) const NOEXCEPT, typename _Payload>
        struct bind_t {
            static CONSTEXPR _Ret invoke(const void* embed, _Args... args) NOEXCEPT {
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
            NEW_REF(Function, TFunctionPayload<T>, std::forward<_Extra>(extra)...)
        };
    }
};
//----------------------------------------------------------------------------
} //!details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
// Factor noexcept(true/false) variants with TBaseFunction<> CRTP
//----------------------------------------------------------------------------
template <typename _Impl, typename _Traits, size_t _InSitu>
class TBaseFunction {
    STATIC_ASSERT(is_function_v<_Impl>);
public:
    using traits_type = _Traits;
    using embed_type = std::aligned_storage_t<_InSitu>;
    using native_type = typename traits_type::native_type;
    using wrapper_type = typename traits_type::wrapper_type;
    using vtable_type = typename traits_type::vtable_type;

    STATIC_CONST_INTEGRAL(bool, NoExcept, traits_type::NoExcept);

private:
    Meta::TPointerWFlags<const vtable_type> _vtable;
    embed_type _embed;

    STATIC_ASSERT(u32(EFunctionFlags::FitInSitu) == 2);
    void SetFitInSitu_(bool enabled) { _vtable.SetFlag1(enabled); }

protected:
    CONSTEXPR const vtable_type& VTable_() const NOEXCEPT { return *_vtable; }
    CONSTEXPR embed_type* EmbedPtr_() NOEXCEPT { return std::addressof(_embed); }
    CONSTEXPR const embed_type* EmbedPtr_() const NOEXCEPT { return std::addressof(_embed); }

public:
    CONSTEXPR TBaseFunction() NOEXCEPT : TBaseFunction(&traits_type::dummy_vtable) {}
    CONSTEXPR explicit TBaseFunction(const vtable_type* vtable) NOEXCEPT {
        _vtable.Reset(vtable);
    }

    ~TBaseFunction() NOEXCEPT {
        _vtable->Destroy(&_embed);
    }

    CONSTEXPR TBaseFunction(const _Impl& other)
    :   _vtable(other._vtable) {
        _vtable->Copy(&_embed, &other._embed);
    }
    CONSTEXPR _Impl& operator =(const _Impl& other) {
        _vtable->Destroy(&_embed);
        _vtable = other._vtable;
        _vtable->Copy(&_embed, &other._embed);
        return static_cast<_Impl&>(*this);
    }

    CONSTEXPR TBaseFunction(_Impl&& rvalue) NOEXCEPT
    :   _vtable(rvalue._vtable) {
        _vtable->Move(&_embed, &rvalue._embed);
        rvalue._vtable.Reset(&traits_type::dummy_vtable);
    }
    CONSTEXPR _Impl& operator =(_Impl&& rvalue) NOEXCEPT {
        _vtable->Destroy(&_embed);
        _vtable = rvalue._vtable;
        _vtable->Move(&_embed, &rvalue._embed);
        rvalue._vtable.Reset(&traits_type::dummy_vtable);
        return static_cast<_Impl&>(*this);
    }

    CONSTEXPR EFunctionFlags Flags() const {
        return static_cast<EFunctionFlags>(_vtable.Flag01());
    }

    STATIC_ASSERT(u32(EFunctionFlags::FitInSitu) == 2);
    CONSTEXPR bool FitInSitu() const { return _vtable.Flag1(); }

    STATIC_ASSERT(u32(EFunctionFlags::FireAndForget) == 1);
    CONSTEXPR bool IsFireAndForget() const { return _vtable.Flag0(); }
    CONSTEXPR void SetFireAndForget(bool enabled) { _vtable.SetFlag0(enabled); }

    CONSTEXPR bool Valid() const NOEXCEPT {
        return (not _vtable->IsDummy());
    }

    CONSTEXPR operator const void* () const NOEXCEPT {
        return (_vtable->IsDummy() ? nullptr : this);
    }

    CONSTEXPR void Reset() NOEXCEPT {
        _vtable->Destroy(&_embed);
        _vtable.Reset(&traits_type::dummy_vtable);
    }

    template <size_t S>
    CONSTEXPR void Reset(const TFunction<native_type, S>& other) NOEXCEPT {
        AssertRelease(other._vtable->Size <= _InSitu);
        _vtable->Destroy(&_embed);
        _vtable = other._vtable;
        _vtable->Copy(&_embed, &other._embed);
    }

    friend CONSTEXPR bool operator ==(const _Impl& lhs, const _Impl& rhs) NOEXCEPT {
        return (lhs._vtable == rhs._vtable && lhs._vtable->Equals(&lhs._embed, &rhs._embed));
    }
    FORCE_INLINE friend CONSTEXPR bool operator !=(const _Impl& lhs, const _Impl& rhs) NOEXCEPT {
        return (not operator ==(lhs, rhs));
    }

    /***********************************************************************
     ** Payload (insitu or allocated)
     ***********************************************************************/

    template <typename T>
    using payload_traits_t = TPayloadTraits<
        T,
        Meta::TConditional<
            (sizeof(T) <= _InSitu),
            T,
            PFunctionPayload<T>
        >
    >;

    template <typename _PayloadTraits, typename... _Extra>
    CONSTEXPR _Impl& SetPayloadLarge(_Extra&&... extra) {
        _PayloadTraits::Set(&_embed, std::forward<_Extra>(extra)...);
        SetFitInSitu_(sizeof(typename _PayloadTraits::type) <= _InSitu);
        return static_cast<_Impl&>(*this);
    }

    template <typename _PayloadTraits, typename... _Extra>
    CONSTEXPR _Impl& SetPayload(_Extra&&... extra) NOEXCEPT {
        using payload_t = typename _PayloadTraits::type;
        static_assert(sizeof(payload_t) <= _InSitu, "payload won't fit in situ");
        return SetPayloadLarge<_PayloadTraits>(std::forward<_Extra>(extra)...);
    }

    /***********************************************************************
     ** Free funcs
     ***********************************************************************/

    template <native_type _FreeFunc>
    static CONSTEXPR auto Bind() NOEXCEPT {
        return _Impl{ &traits_type::template freefunc_t<_FreeFunc>::vtable };
    }

    /***********************************************************************
     ** Lambdas
     ***********************************************************************/

    template <typename _Lambda>
    static CONSTEXPR auto Bind(_Lambda&& lambda) NOEXCEPT {
        using f = Meta::TDecay<_Lambda>;
        using payload_t = payload_traits_t<f>;
        return _Impl{ &traits_type::template lambda_t<f>::template bind_t<payload_t>::vtable }
            .template SetPayload<payload_t>(std::forward<_Lambda>(lambda));
    }

    template <typename _Lambda>
    static CONSTEXPR auto BindLarge(_Lambda&& lambda) {
        using f = Meta::TDecay<_Lambda>;
        using payload_t = payload_traits_t<f>;
        return _Impl{ &traits_type::template lambda_t<f>::template bind_t<payload_t>::vtable }
            .template SetPayloadLarge<payload_t>(std::forward<_Lambda>(lambda));
    }

    // lambdas have also implicit constructors :

    template <typename _Lambda>
    static CONSTEXPR bool is_callable_lambda_v = (
        (traits_type::template is_callable_v<_Lambda>) &&
        (not details::is_function_v< Meta::TDecay<_Lambda> >) );

    template <typename _Lambda, class = Meta::TEnableIf<is_callable_lambda_v<_Lambda>> >
    CONSTEXPR TBaseFunction(_Lambda&& lambda) NOEXCEPT
    :   TBaseFunction( &traits_type
            ::template lambda_t< Meta::TDecay<_Lambda> >
            ::template bind_t< payload_traits_t< Meta::TDecay<_Lambda> > >
            ::vtable ) {
        using payload_t = payload_traits_t< Meta::TDecay<_Lambda> >;
        SetPayload<payload_t>(std::forward<_Lambda>(lambda));
    }

    template <typename _Lambda, class = Meta::TEnableIf<is_callable_lambda_v<_Lambda>> >
    CONSTEXPR TBaseFunction(Meta::FForceInit, _Lambda&& lambda)
    :   TBaseFunction( &traits_type
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
    struct bind_t {
        using func_t = typename traits_type::template otherfunc_t<decltype(_OtherFunc)>;
        using extra_t = typename func_t::template extra_t<_Extra...>;
        using payload_t = payload_traits_t< typename extra_t::type >;
        using type = typename func_t::template bind_t<_OtherFunc, payload_t>;
    };

    template <auto _OtherFunc, typename... _Extra>
    static CONSTEXPR auto Bind(_Extra... extra) NOEXCEPT {
        using binding_t = bind_t<_OtherFunc, _Extra...>;
        return _Impl{ &binding_t::type::vtable }
            .template SetPayload<typename binding_t::payload_t>(std::forward<_Extra>(extra)...);
    }

    template <auto _OtherFunc, typename... _Extra>
    static CONSTEXPR auto BindLarge(_Extra... extra) {
        using binding_t = bind_t<_OtherFunc, _Extra...>;
        return _Impl{ &binding_t::type::vtable }
            .template SetPayloadLarge<typename binding_t::payload_t>(std::forward<_Extra>(extra)...);
    }

}; //!TBaseFunction
//----------------------------------------------------------------------------
} //!details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Ret, typename... _Args, size_t _InSitu>
class TFunction<_Ret(_Args...), _InSitu> : public details::TBaseFunction<
    TFunction<_Ret(_Args...), _InSitu>,
    details::TFunctionTraits<_Ret, false, _Args...>,
    _InSitu
> {
    using parent_type = details::TBaseFunction<
        TFunction<_Ret(_Args...), _InSitu>,
        details::TFunctionTraits<_Ret, false, _Args...>,
        _InSitu
    >;
public:
    using parent_type::parent_type;
    using parent_type::operator=;
    using parent_type::NoExcept;
    using parent_type::Bind;
    using parent_type::BindLarge;

    TFunction() = default;

    CONSTEXPR TFunction(const TFunction& other) : parent_type(other) {}
    CONSTEXPR TFunction& operator =(const TFunction& other) {
        parent_type::operator =(other);
        return (*this);
    }

    CONSTEXPR TFunction(TFunction&& rvalue) NOEXCEPT : parent_type(std::move(rvalue)) {}
    CONSTEXPR TFunction& operator =(TFunction&& rvalue) NOEXCEPT {
        parent_type::operator =(std::move(rvalue));
        return (*this);
    }

    FORCE_INLINE CONSTEXPR _Ret operator()(_Args... args) {
        return parent_type::VTable_().Invoke(parent_type::EmbedPtr_(), std::forward<_Args>(args)...);
    }

    FORCE_INLINE CONSTEXPR _Ret Invoke(_Args... args) {
        return parent_type::VTable_().Invoke(parent_type::EmbedPtr_(), std::forward<_Args>(args)...);
    }

    FORCE_INLINE CONSTEXPR _Ret operator()(_Args... args) const {
        return parent_type::VTable_().Invoke(parent_type::EmbedPtr_(), std::forward<_Args>(args)...);
    }

    FORCE_INLINE CONSTEXPR _Ret Invoke(_Args... args) const {
        return parent_type::VTable_().Invoke(parent_type::EmbedPtr_(), std::forward<_Args>(args)...);
    }

    CONSTEXPR void FireAndForget(_Args... args) {
        Invoke(std::forward<_Args>(args)...);
        parent_type::Reset();
    }

};
//----------------------------------------------------------------------------
template <typename _Ret, typename... _Args, size_t _InSitu>
class TFunction<_Ret(_Args...) NOEXCEPT, _InSitu> : public details::TBaseFunction<
    TFunction<_Ret(_Args...) NOEXCEPT, _InSitu>,
    details::TFunctionTraits<_Ret, true, _Args...>,
    _InSitu
> {
    using parent_type = details::TBaseFunction<
        TFunction<_Ret(_Args...) NOEXCEPT, _InSitu>,
        details::TFunctionTraits<_Ret, true, _Args...>,
        _InSitu
    >;
public:
    using parent_type::parent_type;
    using parent_type::operator=;
    using parent_type::NoExcept;
    using parent_type::Bind;
    using parent_type::BindLarge;

    TFunction() = default;

    CONSTEXPR TFunction(const TFunction & other) : parent_type(other) {}
    CONSTEXPR TFunction& operator =(const TFunction & other) {
        parent_type::operator =(other);
        return (*this);
    }

    CONSTEXPR TFunction(TFunction && rvalue) NOEXCEPT : parent_type(std::move(rvalue)) {}
    CONSTEXPR TFunction& operator =(TFunction && rvalue) NOEXCEPT {
        parent_type::operator =(std::move(rvalue));
        return (*this);
    }

    FORCE_INLINE CONSTEXPR _Ret operator()(_Args... args) NOEXCEPT {
        return parent_type::VTable_().Invoke(parent_type::EmbedPtr_(), std::forward<_Args>(args)...);
    }

    FORCE_INLINE CONSTEXPR _Ret Invoke(_Args... args) NOEXCEPT {
        return parent_type::VTable_().Invoke(parent_type::EmbedPtr_(), std::forward<_Args>(args)...);
    }

    FORCE_INLINE CONSTEXPR _Ret operator()(_Args... args) const NOEXCEPT {
        return parent_type::VTable_().Invoke(parent_type::EmbedPtr_(), std::forward<_Args>(args)...);
    }

    FORCE_INLINE CONSTEXPR _Ret Invoke(_Args... args) const NOEXCEPT {
        return parent_type::VTable_().Invoke(parent_type::EmbedPtr_(), std::forward<_Args>(args)...);
    }

    CONSTEXPR void FireAndForget(_Args... args) NOEXCEPT {
        Invoke(std::forward<_Args>(args)...);
        parent_type::Reset();
    }

};
//----------------------------------------------------------------------------
template <auto _Func, typename... _Extra>
CONSTEXPR auto MakeFunction(_Extra&&... extra) NOEXCEPT {
    using func_t = typename details::TInferFunction<decltype(_Func)>::bind_type;
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

    template <typename... _Args>
    Meta::FDefaultValue operator ()(_Args...) const NOEXCEPT {
        return Default;
    }

    template <typename _Return, typename... _Args>
    static _Return FreeFunc(_Args...) NOEXCEPT {
        return Default;
    }

    // can't use NOEXCEPT_IF() due to MSVC...
    template <typename _Return, typename... _Args>
    using TFreeFunc = _Return(*)(_Args...);
    template <typename _Return, typename... _Args>
    using TFreeFuncNoExcept = _Return(*)(_Args...) NOEXCEPT;

    template <typename _Return, typename... _Args>
    operator TFreeFunc<_Return, _Args...>() const NOEXCEPT {
        return &FreeFunc<_Return, _Args...>;
    }

    template <typename _Return, typename... _Args>
    operator TFreeFuncNoExcept<_Return, _Args...>() const NOEXCEPT {
        return &FreeFunc<_Return, _Args...>;
    }
};
//----------------------------------------------------------------------------
inline constexpr FNoFunction NoFunction;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
