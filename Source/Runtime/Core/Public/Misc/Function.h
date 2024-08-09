#pragma once

#include "Misc/Function_fwd.h"

#include "Allocator/TrackingMalloc.h"
#include "Memory/InSituPtr.h"
#include "Memory/PtrRef.h"
#include "Memory/RefPtr.h"
#include "Memory/WeakPtr.h"
#include "Meta/Clonable.h"
#include "Meta/TypeTraits.h"

#include <functional> // std::apply
#include <tuple>

/*
// https://godbolt.org/z/88oqEYq3j
*/

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// TFunctionRef<>
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
struct FFunctionRefBase {
    union storage_t {
        void *_p{ nullptr };
        const void *_cp;
        void (*_fp)();

        CONSTEXPR storage_t() NOEXCEPT = default;

        template <typename T, std::enable_if_t<std::is_object_v<T>>* = nullptr>
        CONSTEXPR storage_t(T* p) NOEXCEPT
            : _p(p)
        {}

        template <typename T, std::enable_if_t<std::is_object_v<T>>* = nullptr>
        CONSTEXPR storage_t(const T* cp) NOEXCEPT
            : _cp(cp)
        {}

        template <typename T, std::enable_if_t<std::is_function_v<T>>* = nullptr>
        CONSTEXPR storage_t(T* p) NOEXCEPT
            : _fp(reinterpret_cast<decltype(_fp)>(p))
        {}

        NODISCARD inline friend CONSTEXPR bool operator ==(storage_t lhs, storage_t rhs) NOEXCEPT {
            return (lhs._p == rhs._p);
        }
        NODISCARD inline friend CONSTEXPR bool operator !=(storage_t lhs, storage_t rhs) NOEXCEPT {
            return (not operator ==(lhs, rhs));
        }
    };

    template <typename T>
    NODISCARD CONSTEXPR static auto get(storage_t obj) NOEXCEPT {
        IF_CONSTEXPR(std::is_const_v<T>)
            return static_cast<T*>(obj._cp);
        else IF_CONSTEXPR(std::is_object_v<T>)
            return static_cast<T*>(obj._p);
        else
            return reinterpret_cast<T*>(obj._fp);
    }
};
//----------------------------------------------------------------------------
} //!details
//----------------------------------------------------------------------------
template <typename _Function, typename _Return, typename... _Args>
class TFunctionRef<_Function, _Return(_Args...)> : private details::FFunctionRefBase {
    using callable_type = Meta::TCallableObject<_Function, void>;

    static constexpr bool is_noexcept_v = callable_type::is_noexcept_v;
    template <typename... T>
    static constexpr bool is_invocable_v = callable_type::template is_invocable_v<T...>;

    template <typename T>
    using value_type = callable_type::template value_type<T>;
    template <typename T>
    using pointer = value_type<T> *;
    template <typename T>
    using reference = value_type<T> &;

    using forward_f = _Return (*)(storage_t, _Args...) NOEXCEPT_IF(is_noexcept_v);

public:
    CONSTEXPR TFunctionRef() NOEXCEPT = default;

    CONSTEXPR TFunctionRef(const TFunctionRef& ) NOEXCEPT = default;
    CONSTEXPR TFunctionRef& operator =(const TFunctionRef& ) NOEXCEPT = default;

    CONSTEXPR TFunctionRef(TFunctionRef&& rvalue) NOEXCEPT
        : _fwd(std::move(rvalue._fwd))
        , _obj(std::move(rvalue._obj)) {
        rvalue = {};
    }
    CONSTEXPR TFunctionRef& operator =(TFunctionRef&& rvalue) NOEXCEPT {
        _fwd = std::move(rvalue._fwd);
        _obj = std::move(rvalue._obj);
        rvalue = {};
        return (*this);
    }

    template <typename F, std::enable_if_t<std::is_function_v<F> && is_invocable_v<F>>* = nullptr>
    CONSTEXPR TFunctionRef(F* fn) NOEXCEPT
        : _fwd(
            [](storage_t fn, _Args... args) NOEXCEPT_IF(is_noexcept_v) {
                return get<F>(fn)(std::forward<_Args>(args)...);
            })
        , _obj(fn)
    {}

    template <auto F, std::enable_if_t<is_invocable_v<decltype(F)>>* = nullptr>
    CONSTEXPR TFunctionRef(Meta::TStaticFunction<F>) NOEXCEPT
        : _fwd(
            [](storage_t fn, _Args... args) NOEXCEPT_IF(is_noexcept_v) {
                return Meta::StaticFunction<F>(std::forward<_Args>(args)...);
            })
    {}

    template <auto F, class T, std::enable_if_t<is_invocable_v<decltype(F), reference<T>>>* = nullptr>
    CONSTEXPR TFunctionRef(Meta::TStaticFunction<F> , T& obj) NOEXCEPT
        : _fwd(
            [](storage_t obj, _Args... args) NOEXCEPT_IF(is_noexcept_v) {
                return Meta::StaticFunction<F>(get<T>(obj), std::forward<_Args>(args)...);
            })
        , _obj(std::addressof(obj))
    {}

    template <auto F, class T, std::enable_if_t<is_invocable_v<decltype(F), pointer<T>>>* = nullptr>
    CONSTEXPR TFunctionRef(Meta::TStaticFunction<F> , pointer<T> obj) NOEXCEPT
        : _fwd(
            [](storage_t obj, _Args... args) NOEXCEPT_IF(is_noexcept_v) {
                return Meta::StaticFunction<F>(get<T>(obj), std::forward<_Args>(args)...);
            })
        , _obj(obj)
    {}

    template <typename _Lambda, std::enable_if_t<is_invocable_v<const _Lambda&>>* = nullptr>
    CONSTEXPR TFunctionRef(const _Lambda& lambda) NOEXCEPT
        : _fwd(
            [](storage_t lambda, _Args... args) NOEXCEPT_IF(is_noexcept_v) {
                return (*get<_Lambda>(lambda))(std::forward<_Args>(args)...);
            })
        , _obj(std::addressof(lambda))
    {}

    CONSTEXPR _Return operator ()(_Args... args) const NOEXCEPT_IF(is_noexcept_v) {
        return _fwd(_obj, std::forward<_Args>(args)...);
    }

    CONSTEXPR _Return FireAndForget(_Args... args) NOEXCEPT_IF(is_noexcept_v) {
        TFunctionRef tmp{ std::move(this) };
        return tmp(std::forward<_Args>(args)...);
    }

    NODISCARD CONSTEXPR bool Valid() const NOEXCEPT {
        return (!!_fwd);
    }

    CONSTEXPR void Reset() NOEXCEPT {
        *this = {};
    }

    NODISCARD inline friend CONSTEXPR hash_t hash_value(const TFunctionRef& fn) NOEXCEPT {
        return hash_combine(fn._fwd, fn._obj._p);
    }
    NODISCARD inline friend CONSTEXPR bool operator ==(const TFunctionRef& lhs, const TFunctionRef& rhs) NOEXCEPT {
        return (lhs._fwd == rhs._fwd and lhs._obj == rhs._obj);
    }
    NODISCARD inline friend CONSTEXPR bool operator !=(const TFunctionRef& lhs, const TFunctionRef& rhs) NOEXCEPT {
        return (not operator ==(lhs, rhs));
    }

private:
    forward_f _fwd{ nullptr };
    storage_t _obj;
};
//----------------------------------------------------------------------------
template <typename F> requires std::is_function_v<F>
TFunctionRef(F* ) -> TFunctionRef<F>;
//----------------------------------------------------------------------------
template <auto F>
TFunctionRef(Meta::TStaticFunction<F> ) -> TFunctionRef<typename Meta::TStaticFunction<F>::function_type>;
//----------------------------------------------------------------------------
template <auto F>
TFunctionRef(Meta::TStaticFunction<F> , auto ) -> TFunctionRef<typename Meta::TStaticFunction<F>::function_type>;
//----------------------------------------------------------------------------
template <typename _Lambda>
TFunctionRef(const _Lambda& ) -> TFunctionRef<typename decltype(Meta::TCallableObject{ std::declval<const _Lambda&>() })::function_type>;
//----------------------------------------------------------------------------
template <typename _Function, class _>
TFunctionRef(const TFunctionRef<_Function, _>& ) -> TFunctionRef<_Function, _>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// TFunction<>
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
// TFunctionPayloadExtra<T> can wrap specific arguments :
// - raw pointers pointing to a FRefCountable are wrapped inside a TSafePtr<>
//----------------------------------------------------------------------------
template <typename T>
struct TFunctionPayloadExtra {
    using type = T;
};
//----------------------------------------------------------------------------
template <>
struct TFunctionPayloadExtra<void*> {
    using type = void*;
};
//----------------------------------------------------------------------------
// Wraps rvalue references with a copy :
template <typename T>
struct TFunctionPayloadExtra<T&&> {
    using type = T;
};
//----------------------------------------------------------------------------
// Wraps references with pointers to keep value semantics :
template <typename T>
struct TFunctionPayloadExtra<T&> {
    using type = TPtrRef<T>;
};
//----------------------------------------------------------------------------
// Wraps FRefCountable* within TSafePtr<T> to check for lifetime :
template <typename T>
struct TFunctionPayloadExtra<T*> {
    using type = Meta::TConditional<
        std::conjunction_v<IsRefCountable<T>, IsWeakRefCountable<T>>,
        TSafePtr<T>, TPtrRef<T> >;
};
//----------------------------------------------------------------------------
// TFunctionPayload<> stores a payload either inlined, either allocated with ref counting
//----------------------------------------------------------------------------
template <typename _Payload, bool _bDynamic>
struct TFunctionPayload : public _Payload {
    template <typename... _It, std::enable_if_t<std::is_constructible_v<_Payload, _It&&...>>* = nullptr>
    CONSTEXPR TFunctionPayload(_It&&... it) NOEXCEPT_IF(std::is_nothrow_constructible_v<_Payload, _It&&...>)
        : _Payload(std::forward<_It>(it)...)
    {}

    NODISCARD CONSTEXPR _Payload& get() NOEXCEPT {
        return (*this);
    }

    NODISCARD CONSTEXPR const _Payload& get() const NOEXCEPT {
        return (*this);
    }
};
//----------------------------------------------------------------------------
template <typename _Payload>
struct TFunctionPayload<_Payload, true> {
    TRefPtr<TRefCountable<_Payload>> Shared;

    template <typename... _It, std::enable_if_t<std::is_constructible_v<_Payload, _It&&...>>* = nullptr>
    TFunctionPayload(_It&&... it)
        : Shared(NEW_REF(Function, TRefCountable<_Payload>, std::forward<_It>(it)...))
    {}

    NODISCARD _Payload& get() const NOEXCEPT {
        return (*Shared);
    }
};
//----------------------------------------------------------------------------
} //!details
//----------------------------------------------------------------------------
template <typename _Function, typename _Return, typename... _Args, size_t _InSituSize>
class TFunction<_Function, _InSituSize, _Return(_Args...)> {
    using args_tuple = decltype(std::forward_as_tuple(std::declval<_Args>()...));
    using callable_type = Meta::TCallableObject<_Function, void>;
    using return_type = _Return;

    static constexpr bool is_noexcept_v = callable_type::is_noexcept_v;

public:
    CONSTEXPR TFunction() NOEXCEPT
        : _invokable{FBadInvocable_{}}
    {}

    CONSTEXPR TFunction(Meta::FDefaultValue ) NOEXCEPT
        : TFunction()
    {}

    template <typename _Callable, typename... _Extras, decltype( Meta::TCallableObject{std::declval<_Callable>()}(std::declval<_Extras&&>()..., std::declval<_Args>()...) )* = nullptr>
    CONSTEXPR TFunction(_Callable callable, _Extras&&... extras) NOEXCEPT_IF(std::is_nothrow_constructible_v<decltype(_invokable), decltype(TInvocable_{ Meta::TCallableObject{ callable }, std::forward<_Extras>(extras)... })>)
        : _invokable{TInvocable_{Meta::TCallableObject{ callable }, std::forward<_Extras>(extras)...}}
    {}

    template <auto _FunctionPtr, typename... _Extras, decltype( std::declval<Meta::TStaticFunction<_FunctionPtr>>()(std::declval<_Extras&&>()..., std::declval<_Args>()...) )* = nullptr>
    CONSTEXPR TFunction(Meta::TStaticFunction<_FunctionPtr> staticFunction, _Extras&&... extras) NOEXCEPT_IF(std::is_nothrow_constructible_v<decltype(_invokable), decltype(TInvocable_{ staticFunction, std::forward<_Extras>(extras)... })>)
        : _invokable(TInvocable_{staticFunction, std::forward<_Extras>(extras)...})
    {}

    template <typename _Lambda, std::enable_if_t<std::is_class_v<std::decay_t<_Lambda>>, decltype( std::declval<_Lambda&&>()(std::declval<_Args>()...) )>* = nullptr>
    CONSTEXPR TFunction(_Lambda&& lambda) NOEXCEPT_IF(std::is_nothrow_constructible_v<decltype(_invokable), decltype(TInvocable_{ std::move(lambda) })>)
        : _invokable{TInvocable_{std::move(lambda)}}
    {}

    template <auto _FunctionPtr, typename... _Extras, decltype( std::declval<Meta::TStaticFunction<_FunctionPtr>>()(std::declval<_Extras&&>()..., std::declval<_Args&&>()...) )* = nullptr>
    NODISCARD CONSTEXPR static TFunction Bind(_Extras&&... extras) NOEXCEPT_IF(std::is_nothrow_constructible_v<TFunction, Meta::TStaticFunction<_FunctionPtr>, _Extras&&...>) {
        return TFunction{ Meta::TStaticFunction<_FunctionPtr>{}, std::forward<_Extras>(extras)... };
    }

    NODISCARD bool Valid() const NOEXCEPT {
        return _invokable->Valid();
    }

    _Return operator ()(_Args... args) const NOEXCEPT_IF(is_noexcept_v) {
        return _invokable->Invoke(std::forward_as_tuple(std::forward<_Args>(args)...));
    }

private:
    struct IInvocable_ : Meta::IClonable {
        virtual ~IInvocable_() = default;
        NODISCARD virtual bool Valid() const NOEXCEPT = 0;
        virtual _Return Invoke(args_tuple&& args) const NOEXCEPT_IF(is_noexcept_v) = 0;
    };

    template <typename _Payload>
    struct EMPTY_BASES TPayloadWithVTable_ : IInvocable_, _Payload {};

    TInSituPtr<IInvocable_, _InSituSize + sizeof(TPayloadWithVTable_<std::tuple</* empty */>>)/* sizeof VTable */> _invokable;

    template <typename _Payload>
    NODISCARD static CONSTEVAL bool IsPayloadNotFittingInSitu_() {
        return (sizeof(TPayloadWithVTable_<_Payload>) > sizeof(_invokable));
    }

    template <typename _Payload>
    using TPayloadStorage_ = details::TFunctionPayload<_Payload, IsPayloadNotFittingInSitu_<_Payload>()>;

    template <typename _Callable, typename... _Extras>
    using TPayload_ = TPayloadStorage_<decltype(std::make_tuple(
        std::declval<_Callable>(),
        std::make_tuple(typename details::TFunctionPayloadExtra<_Extras>::type{ std::declval<_Extras&&>() }...)
    ))>;

    template <typename _Crtp, bool _bValid = true>
    struct TBasicInvocable_ : public IInvocable_ {
        // IInvocable interface

        NODISCARD virtual bool Valid() const NOEXCEPT override final {
            return _bValid;
        }

        // IClonable interface

        virtual void ConstructMove(FAllocatorBlock dst) NOEXCEPT override final {
            new (dst.Data) _Crtp{ std::move(*static_cast<_Crtp*>(this)) };
        }

        virtual void ConstructCopy(FAllocatorBlock dst) const override final {
            new (dst.Data) _Crtp{ *static_cast<const _Crtp*>(this) };
        }
    };

    template <typename _Callable, typename... _Extras>
    struct EMPTY_BASES TInvocable_ final : public TBasicInvocable_<TInvocable_<_Callable, _Extras...>>, private TPayload_<_Callable, _Extras...> {
        using payload_type = TPayload_<_Callable, _Extras...>;

        CONSTEXPR explicit TInvocable_(_Callable callable, _Extras&&... extras) noexcept(std::is_nothrow_constructible_v<payload_type, _Callable, decltype(std::make_tuple(std::forward<_Extras>(extras)...))>)
            : payload_type{std::forward<_Callable>(callable), std::make_tuple(std::forward<_Extras>(extras)...)}
        {}

        // IInvocable interface

        virtual _Return Invoke(args_tuple&& args) const NOEXCEPT_IF(is_noexcept_v) override final {
            const auto& payload = payload_type::get();
            return std::apply(std::get<0>(payload), std::tuple_cat(std::get<1>(payload), std::move(args)));
        }
    };

    struct FBadInvocable_ final : public TBasicInvocable_<FBadInvocable_, false> {
        // IInvocable interface

        virtual _Return Invoke(args_tuple&& ) const NOEXCEPT_IF(is_noexcept_v) override final {
            IF_CONSTEXPR(not std::is_same_v<void, _Return>)
                throw std::bad_function_call{};
        }
    };

    template <typename _Callable, typename... _Extras>
    TInvocable_(_Callable callable, _Extras&&... extras) -> TInvocable_<_Callable, _Extras...>;
};
//----------------------------------------------------------------------------
template <typename _Callable, typename... _Extras>
TFunction(_Callable callable, _Extras&&... extras) -> TFunction<typename decltype(Meta::TCallableObject{ callable })::function_type, GFunctionInSitu>;
//----------------------------------------------------------------------------
template <auto _FunctionPtr, typename... _Extras>
TFunction(Meta::TStaticFunction<_FunctionPtr> staticFunction, _Extras&&... extras) -> TFunction<typename Meta::TStaticFunction<_FunctionPtr>::function_type, GFunctionInSitu>;
//----------------------------------------------------------------------------
template <typename _Lambda>
TFunction(_Lambda&& lambda) -> TFunction<typename decltype(Meta::TCallableObject{ lambda })::function_type, GFunctionInSitu>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
