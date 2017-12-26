#pragma once

#include "Core/Meta/AlignedStorage.h"
#include "Core/Meta/TypeTraits.h"

namespace Core {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// TFunction<> is guaranteed to never allocate, unlike std::function<>
//----------------------------------------------------------------------------
template <typename T>
class TFunction;
template <typename _Ret, typename... _Args>
class TFunction<_Ret(_Args...)> {
public:
    typedef _Ret (*func_type)(_Args...);

    template <typename T>
    static constexpr bool is_callable_v = std::bool_constant<
            decltype(is_callable_<T>(0))::value &&
            // Forbid wrapping TFunction in another TFunction :
            not std::is_same_v<Meta::TDecay<T>, TFunction>
        >::value;

    TFunction() : _func(nullptr) {}

    TFunction(const TFunction& other) { assign_copy_(other); }
    TFunction& operator =(const TFunction& other) {
        Reset();
        assign_copy_(other);
        return (*this);
    }

    TFunction(TFunction&& rvalue) : TFunction() {
        Swap(rvalue);
    }
    TFunction& operator =(TFunction&& rvalue) {
        Reset();
        Swap(rvalue);
        return (*this);
    }

    TFunction(func_type fn)
        : _func(fn) {
        Assert(not is_wrapped_());
    }

    template <typename _Class>
    TFunction(_Class* obj, _Ret(_Class::*member)(_Args...)) {
        Assert(obj);
        Assert(member);
        struct member_t {
            _Class* Obj;
            _Ret(_Class::*Member)(_Args...);
            _Ret operator ()(_Args... args) const {
                return (Obj->*Member)(std::forward<_Args>(args)...);
            }
        };
        assign_wrapped_(member_t{ obj, member });
        Assert(is_pod_());
    }

    template <typename _Class>
    TFunction(const _Class* obj, _Ret(_Class::*member)(_Args...) const) {
        Assert(obj);
        Assert(member);
        struct member_const_t {
            const _Class* Obj;
            _Ret(_Class::*Member)(_Args...) const;
            _Ret operator ()(_Args... args) const {
                return (Obj->*Member)(std::forward<_Args>(args)...);
            }
        };
        assign_wrapped_(member_const_t{ obj, member });
        Assert(is_pod_());
    }

    template <typename T, typename = Meta::TEnableIf<is_callable_v<T>> >
    TFunction(T&& lambda) {
        assign_wrapped_(std::move(lambda));
    }

    ~TFunction() {
        if (is_destructible_())
            ((IWrapper_*)&_inSitu)->~IWrapper_();
    }

    CORE_FAKEBOOL_OPERATOR_DECL() { return _func; }
    bool Valid() const { return (nullptr != _func); }

    _Ret operator ()(_Args... args) const {
        return Invoke(std::forward<_Args>(args)...);
    }

    _Ret Invoke(_Args... args) const {
        Assert(_func);
        return ((is_wrapped_())
            ? ((wrapper_type)(_wrapper&~GMaskAll))(&_inSitu, std::forward<_Args>(args)...)
            : _func(std::forward<_Args>(args)...));
    }

    void Reset() {
        if (is_destructible_())
            ((IWrapper_*)&_inSitu)->~IWrapper_();

        _func = nullptr;
    }

    bool Equals(const TFunction& other) const {
        return (_func == _func && _inSitu == other._inSitu);
    }

    void Swap(TFunction& other) {
        std::swap(_func, other._func);
        std::swap(_inSitu, other._inSitu);
    }

    inline friend bool operator ==(const TFunction& lhs, const TFunction& rhs) { return (lhs.Equals(rhs)); }
    inline friend bool operator !=(const TFunction& lhs, const TFunction& rhs) { return (not operator ==(lhs, rhs)); }

    inline friend void swap(TFunction& lhs, TFunction& rhs) { lhs.Swap(rhs); }

private:
    static constexpr intptr_t GMaskPOD = (intptr_t(1) << CODE3264(30, 62));
    static constexpr intptr_t GMaskWrapped = (intptr_t(1) << CODE3264(31, 63));
    static constexpr intptr_t GMaskAll = (GMaskPOD | GMaskWrapped);

    typedef _Ret(*wrapper_type)(const void*, _Args...);

    union {
        func_type _func;
        intptr_t _wrapper;
    };
    STATIC_ASSERT(sizeof(intptr_t) == sizeof(func_type));
    STATIC_ASSERT(sizeof(intptr_t) == sizeof(wrapper_type));

    bool is_pod_() const { return (0 != (_wrapper&GMaskPOD)); }
    bool is_wrapped_() const { return (0 != (_wrapper&GMaskWrapped)); }
    bool is_destructible_() const { return (GMaskWrapped == (_wrapper&GMaskAll)); }

    static constexpr size_t GInSituSize = (32 - sizeof(intptr_t));
    ALIGNED_STORAGE(GInSituSize, 1) _inSitu;

    template <typename T, typename _R = decltype(std::declval<T>()(std::declval<_Args>()...)) >
    static typename std::is_same<_Ret, _R>::type is_callable_(int);
    template <typename T>
    static std::false_type is_callable_(...);

    struct IWrapper_ {
        virtual ~IWrapper_() {}
        virtual void CopyTo(void* dst) const = 0;
    };

    template <typename T>
    struct TWrapper_ : IWrapper_ {
        T Data;
        TWrapper_(T&& data) : Data(std::move(data)) {}
        virtual void CopyTo(void* dst) const override {
            new (dst) TWrapper_<T>(T(Data));
        }
    };

    template <typename _Data>
    void assign_wrapped_(_Data&& data) {
        assign_wrapped_(std::move(data), std::is_trivially_destructible<_Data>::type{});
    }

    template <typename _Data>
    void assign_wrapped_(_Data&& data, std::true_type) {
        using wrapped_type = _Data;
        STATIC_ASSERT(sizeof(wrapped_type) <= GInSituSize);
        new (&_inSitu) wrapped_type(std::move(data));
        const wrapper_type w = [](const void* inSitu, _Args... args) -> _Ret {
            return (*(wrapped_type*)inSitu)(std::forward<_Args>(args)...);
        };
        Assert(0 == (intptr_t(w) & GMaskAll));
        _wrapper = (intptr_t(w) | GMaskAll);
    }

    template <typename _Data>
    void assign_wrapped_(_Data&& data, std::false_type) {
        using wrapped_type = TWrapper_<_Data>;
        STATIC_ASSERT(sizeof(wrapped_type) <= GInSituSize);
        new (&_inSitu) wrapped_type(std::move(data));
        const wrapper_type w = [](const void* inSitu, _Args... args) -> _Ret {
            return ((wrapped_type*)inSitu)->Data(std::forward<_Args>(args)...);
        };
        Assert(0 == (intptr_t(w) & GMaskAll));
        _wrapper = (intptr_t(w) | GMaskWrapped);
    }

    void assign_copy_(const TFunction& other) {
        _func = other._func;
        if (_func && is_wrapped_()) {
            if (is_destructible_())
                ((const IWrapper_*)&other._inSitu)->CopyTo(&_inSitu);
            else
                ::memcpy(&_inSitu, &other._inSitu, sizeof(_inSitu));
        }
    }
};
//----------------------------------------------------------------------------
template <typename _Ret, typename... _Args>
TFunction<_Ret(_Args...)> MakeFunction(_Ret(*func)(_Args...)) {
    return TFunction<_Ret(_Args...)>(func);
}
//----------------------------------------------------------------------------
template <typename _Ret, typename _Class, typename... _Args>
TFunction<_Ret(_Args...)> MakeFunction(_Class* obj, _Ret(_Class::*member)(_Args...)) {
    return TFunction<_Ret(_Args...)>(obj, member);
}
//----------------------------------------------------------------------------
template <typename _Ret, typename _Class, typename... _Args>
TFunction<_Ret(_Args...)> MakeFunction(const _Class* obj, _Ret(_Class::*member)(_Args...) const) {
    return TFunction<_Ret(_Args...)>(obj, member);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace Core
