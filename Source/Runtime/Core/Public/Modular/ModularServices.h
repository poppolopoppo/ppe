#pragma once

#include "Core_fwd.h"

#include "Modular/Modular_fwd.h"

#include "Container/FlatMap.h"
#include "Memory/RefPtr.h"
#include "Memory/UniquePtr.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FModularServices : Meta::FNonCopyableNorMovable {
public:
    FModularServices();
    explicit FModularServices(const FModularServices* parent);

    ~FModularServices();

    bool empty() const { return _services.empty(); }
    size_t size() const { return _services.size(); }

    template <typename _Interface>
    NODISCARD _Interface& Get() const NOEXCEPT;
    template <typename _Interface>
    NODISCARD _Interface* GetIFP() const NOEXCEPT;

    template <typename _Interface, typename _Service>
    void Add(_Service&& rservice);
    template <typename _Interface>
    void Remove();
    template <typename _Interface, typename _Service>
    void CheckedRemove(_Service&& rservice);

    void Clear();

private:
    using FServiceKey_ = size_t;

    class FServiceHolder_ : Meta::FNonCopyable {
    public:
        using getter_f = void* (*)(void*);
        using destructor_f = void (*)(void*) NOEXCEPT;

        FServiceHolder_() = default;

        FServiceHolder_(void* service, getter_f getter, destructor_f destructor) NOEXCEPT
        :    _service(service)
        ,    _getter(getter)
        ,    _destructor(destructor)
        {}

        FServiceHolder_(FServiceHolder_&& rvalue) NOEXCEPT
        :    FServiceHolder_() {
            swap(*this, rvalue);
        }

        ~FServiceHolder_() NOEXCEPT {
            if (_destructor)
                _destructor(_service);
        }

        void* Get() const NOEXCEPT {
            return (_getter ? _getter(_service) : _service);
        }

        friend void swap(FServiceHolder_& lhs, FServiceHolder_& rhs) {
            std::swap(lhs._service, rhs._service);
            std::swap(lhs._getter, rhs._getter);
            std::swap(lhs._destructor, rhs._destructor);
        }

    private:
        void* _service;
        getter_f _getter;
        destructor_f _destructor;
    };

    template <typename _Interface>
    static CONSTEXPR FServiceKey_ ServiceKey() NOEXCEPT {
        return Meta::type_id<_Interface>::value;
    }

    template <typename T>
    static FServiceHolder_ MakeHolder_(T* rawptr) {
        Assert(rawptr);
        return FServiceHolder_{ rawptr, nullptr, nullptr };
    }
    template <typename T>
    static FServiceHolder_ MakeHolder_(const TRefPtr<T>& refptr) {
        Assert(refptr);
        AddRef(refptr.get());
        return FServiceHolder_{ refptr.get(), nullptr,
            [](void* ptr) NOEXCEPT {
                RemoveRef(static_cast<FRefCountable*>(ptr));
            }};
    }
    template <typename T>
    static FServiceHolder_ MakeHolder_(TUniquePtr<T>&& uniqueptr) {
        Assert(uniqueptr);
        T* const rawptr = uniqueptr.get();
        POD_STORAGE(TUniquePtr<T>) no_dtor; // #HACK: steal reference without destroying
        new (&no_dtor) TUniquePtr<T>(std::move(uniqueptr));
        return FServiceHolder_{ rawptr, nullptr, TUniquePtr<T>::Deleter() };
    }

    void* GetService_(FServiceKey_ key) const NOEXCEPT;

    void AddService_(FServiceKey_ key, FServiceHolder_&& rholder);
    void RemoveService_(FServiceKey_ key);

    const FModularServices* _parent;
    FLATMAP_INSITU(Internal, FServiceKey_, FServiceHolder_, 5) _services;
};
//----------------------------------------------------------------------------
template <typename _Interface>
NODISCARD _Interface& FModularServices::Get() const NOEXCEPT {
    CONSTEXPR const FServiceKey_ key = ServiceKey<_Interface>();
    auto* const pservice = static_cast<_Interface*>(GetService_(key));
    AssertRelease(pservice);
    return (*pservice);
}
//----------------------------------------------------------------------------
template <typename _Interface>
NODISCARD _Interface* FModularServices::GetIFP() const NOEXCEPT {
    CONSTEXPR const FServiceKey_ key = ServiceKey<_Interface>();
    return static_cast<_Interface*>(GetService_(key));
}
//----------------------------------------------------------------------------
template <typename _Interface, typename _Service>
void FModularServices::Add(_Service&& rservice) {
    CONSTEXPR const FServiceKey_ key = ServiceKey<_Interface>();
    AddService_(key, MakeHolder_(std::move(rservice)));
}
//----------------------------------------------------------------------------
template <typename _Interface>
void FModularServices::Remove() {
    CONSTEXPR const FServiceKey_ key = ServiceKey<_Interface>();
    RemoveService_(key);
}
//----------------------------------------------------------------------------
template <typename _Interface, typename _Service>
void FModularServices::CheckedRemove(_Service&& rservice) {
    CONSTEXPR const FServiceKey_ key = ServiceKey<_Interface>();
    UNUSED(rservice);
    Assert_NoAssume(GetService_(key) == MakeHolder_(std::move(rservice)).Get());
    RemoveService_(key);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
