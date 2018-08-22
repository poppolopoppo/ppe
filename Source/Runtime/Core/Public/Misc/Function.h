#pragma once

#include "Memory/RefPtr.h"
#include "Meta/AlignedStorage.h"
#include "Meta/TypeTraits.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// TFunction<> is using an insitu storage to avoid allocation, unlike std::function<>
//----------------------------------------------------------------------------
class PPE_CORE_API FBaseFunction {
public:
    ~FBaseFunction();

    bool Valid() const { return (0 != _data); }
    PPE_FAKEBOOL_OPERATOR_DECL() { return (void*)_data; }

    void Reset();

    bool Equals(const FBaseFunction& other) const;

    void Swap(FBaseFunction& other);

    inline friend bool operator ==(const FBaseFunction& lhs, const FBaseFunction& rhs) { return (lhs.Equals(rhs)); }
    inline friend bool operator !=(const FBaseFunction& lhs, const FBaseFunction& rhs) { return (not operator ==(lhs, rhs)); }

    inline friend void swap(FBaseFunction& lhs, FBaseFunction& rhs) { lhs.Swap(rhs); }

protected:
    static constexpr intptr_t GMaskPOD = (intptr_t(1) << CODE3264(30, 62));
    static constexpr intptr_t GMaskWrapped = (intptr_t(1) << CODE3264(31, 63));
    static constexpr intptr_t GMaskAll = (GMaskPOD | GMaskWrapped);
    static constexpr size_t GInSituSize = (32 - sizeof(intptr_t));

    template <typename T>
    using TObjectRef_ = Meta::TConditional<
        std::is_base_of_v<FRefCountable, Meta::TDecay<T>>,
        TSafePtr<T>,
        T*
    >;

    struct IPayload_ {
        virtual ~IPayload_() {}
        virtual void CopyTo(void* dst) const = 0;
    };

    FBaseFunction() {}
    explicit FBaseFunction(intptr_t data);

    FBaseFunction(const FBaseFunction& other) { assign_copy_(other); }
    FBaseFunction& operator =(const FBaseFunction& other);

    FBaseFunction(FBaseFunction&& rvalue) : _data(0) { Swap(rvalue); }
    FBaseFunction& operator =(FBaseFunction&& rvalue);

    intptr_t data_() const { return _data; }
    IPayload_* payload_() const { return ((IPayload_*)&_inSitu); }

    bool is_pod_() const { return (0 != (_data&GMaskPOD)); }
    bool is_wrapped_() const { return (0 != (_data&GMaskWrapped)); }
    bool is_destructible_() const { return (GMaskWrapped == (_data&GMaskAll)); }

    template <typename T, typename _Ret, typename... _Args>
    void assign_wrapped_(T&& payload, Meta::TType<_Ret(*)(const void*, _Args...)> wrapper) {
        assign_wrapped_(std::move(payload), typename std::is_trivially_destructible<T>::type{}, wrapper);
    }

private:
    intptr_t _data;
    ALIGNED_STORAGE(GInSituSize, 1) _inSitu;

    template <typename T, bool _FitInSitu = (sizeof(T) <= GInSituSize)>
    struct TPayload_ : IPayload_ {
        T Data; // data is created in situ, avoiding allocation
        TPayload_(T&& data) : Data(std::move(data)) {}
        virtual void CopyTo(void* dst) const override {
            INPLACE_NEW(dst, TPayload_)(T(Data));
        }
        template <typename... _Args>
        auto operator ()(_Args&&... args) const {
            return Data(std::forward<_Args>(args)...);
        }
    };

    template <typename T> // use an allocation only when not fitting in situ
    struct TPayload_<T, false> : IPayload_ {
        class FHolder : public FRefCountable {
        public: // this holder is ref counted so it can be easily shared
            T Data;
            explicit FHolder(T&& data) : Data(std::move(data)) {}
        };
        TRefPtr<FHolder> HolderRef;
        // this is definitely the slow path due to this allocation and should be avoided as much as possible
        // one problem with this design is that it's impossible to wrap/combine other functions without allocation
        TPayload_(T&& data) : HolderRef(NEW_REF(Function, FHolder)(std::move(data))) {}
        TPayload_(const TPayload_& other) : HolderRef(other.HolderRef) {}
        virtual void CopyTo(void* dst) const override {
            INPLACE_NEW(dst, TPayload_)(*this);
        }
        template <typename... _Args>
        auto operator ()(_Args&&... args) const {
            return HolderRef->Data(std::forward<_Args>(args)...);
        }
    };

    void assign_copy_(const FBaseFunction& other);

    template <typename T, typename _Ret, typename... _Args>
    void assign_wrapped_(T&& payload, std::true_type, Meta::TType<_Ret(*)(const void*, _Args...)> wrapper) {
        assign_wrapped_impl_<T>(std::move(payload), GMaskAll, wrapper);
    }

    template <typename T, typename _Ret, typename... _Args>
    void assign_wrapped_(T&& payload, std::false_type, Meta::TType<_Ret(*)(const void*, _Args...)> wrapper) {
        assign_wrapped_impl_<TPayload_<T>>(std::move(payload), GMaskWrapped, wrapper);
    }

    template <typename _Payload, typename T, typename _Ret, typename... _Args>
    void assign_wrapped_impl_(T&& arg, intptr_t flags, Meta::TType<_Ret(*)(const void*, _Args...)>) {
        STATIC_ASSERT(Meta::TCheckFitInSize<_Payload, decltype(_inSitu)>::value);
        INPLACE_NEW(payload_(), _Payload)(std::move(arg));
        typedef _Ret(*wrapper_type)(const void*, _Args&&...);
        const wrapper_type w = [](const void* inSitu, _Args&&... args) -> _Ret {
            return (*(_Payload*)inSitu)(std::forward<_Args>(args)...);
        };
        Assert(0 == (intptr_t(w) & GMaskAll));
        STATIC_ASSERT(sizeof(w) == sizeof(intptr_t));
        _data = (intptr_t(w) | flags);
    }
};
//----------------------------------------------------------------------------
template <typename T>
class TFunction;
template <typename _Ret, typename... _Args>
class TFunction<_Ret(_Args...)> : public FBaseFunction {
    template <typename T, typename _R = decltype(std::declval<T>()(std::declval<_Args>()...)) >
    static typename std::is_same<_Ret, _R>::type is_callable_(int);
    template <typename T>
    static std::false_type is_callable_(...);

public:
    using FBaseFunction::Equals;
    using FBaseFunction::Reset;
    using FBaseFunction::Swap;
    using FBaseFunction::Valid;

    typedef _Ret (*func_type)(_Args...);

    template <typename T>
    static constexpr bool is_callable_v{ decltype(is_callable_<T>(0))::value };

    TFunction() : FBaseFunction(0) {}

    TFunction(const TFunction& other) : FBaseFunction(other) {}
    TFunction& operator =(const TFunction& other) { FBaseFunction::operator =(other); return (*this); }

    TFunction(TFunction&& rvalue) : FBaseFunction(std::move(rvalue)) {}
    TFunction& operator =(TFunction&& rvalue) { FBaseFunction::operator =(std::move(rvalue)); return (*this); }

    TFunction(func_type fn)
    :   FBaseFunction(intptr_t(fn)) {
        Assert(not is_wrapped_());
    }

    template <typename _Class>
    TFunction(_Class* obj, _Ret(_Class::*member)(_Args...)) {
        Assert(obj);
        Assert(member);
        struct member_t {
            TObjectRef_<_Class> Obj;
            _Ret(_Class::*Member)(_Args...);
            _Ret operator ()(_Args&&... args) const {
                return (Obj->*Member)(std::forward<_Args>(args)...);
            }
        };
        assign_func_(member_t{ { obj }, member });
    }

    template <typename _Class>
    TFunction(const _Class* obj, _Ret(_Class::*member)(_Args...) const) {
        Assert(obj);
        Assert(member);
        struct member_const_t {
            TObjectRef_<const _Class> Obj;
            _Ret(_Class::*Member)(_Args...) const;
            _Ret operator ()(_Args&&... args) const {
                return (Obj->*Member)(std::forward<_Args>(args)...);
            }
        };
        assign_func_(member_const_t{ { obj }, member });
    }

    template <typename T, typename = Meta::TEnableIf<is_callable_v<T>> >
    TFunction(T&& lambda) {
        assign_func_(std::move(lambda));
    }

    _Ret operator ()(_Args... args) const {
        return Invoke(std::forward<_Args>(args)...);
    }

    _Ret Invoke(_Args... args) const {
        Assert(data_());
        return ((is_wrapped_())
            ? ((wrapper_type)(data_()&~GMaskAll))(payload_(), std::forward<_Args>(args)...)
            : ((func_type)data_())(std::forward<_Args>(args)...));

        STATIC_ASSERT(sizeof(intptr_t) == sizeof(func_type));
        STATIC_ASSERT(sizeof(intptr_t) == sizeof(wrapper_type));
    }

    void FireAndForget(_Args... args) {
        Invoke(std::forward<_Args>(args)...);
        Reset(); // unbind after first call
    }

private:
    typedef _Ret(*wrapper_type)(const void*, _Args&&...);

    template <typename T>
    void assign_func_(T&& data) {
        assign_wrapped_(std::move(data), Meta::TType<wrapper_type>{});
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
} //!namespace PPE
