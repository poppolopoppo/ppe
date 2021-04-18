#pragma once

#include "Core_fwd.h"

#include "Modular/Modular_fwd.h"

#include "Container/FlatMap.h"
#include "Memory/RefPtr.h"
#include "Memory/UniquePtr.h"
#include "Meta/TypeInfo.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FModularServices : Meta::FNonCopyableNorMovable {
public:
    explicit FModularServices(const FStringView& name) NOEXCEPT;
    FModularServices(const FStringView& name, const FModularServices* parent) NOEXCEPT;
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

    void ReleaseMemory() NOEXCEPT; // will call ReleaseMemory() on every service implementing it

private:
    using FServiceKey_ = Meta::type_info_t;

    template <typename _Interface>
    static CONSTEXPR FServiceKey_ ServiceKey_() NOEXCEPT {
        return Meta::type_info<_Interface>;
    }

    class FServiceHolder_ : Meta::FNonCopyable {
    public:
        using getter_f = void* (*)(void*);
        using destructor_f = void (*)(void*) NOEXCEPT;
        using release_memory_f = void (*)(void*) NOEXCEPT;

        FServiceHolder_() = default;

        FServiceHolder_(void* service, getter_f getter, destructor_f destructor, release_memory_f releaseMemory) NOEXCEPT
        :   _service(service)
        ,   _getter(getter)
        ,   _destructor(destructor)
        ,   _releaseMemory(releaseMemory) {
            Assert_NoAssume(_service);
        }

        FServiceHolder_(FServiceHolder_&& rvalue) NOEXCEPT
        :   FServiceHolder_() {
            swap(*this, rvalue);
        }

        FServiceHolder_& operator =(FServiceHolder_&& rvalue) NOEXCEPT {
            if (_destructor)
                _destructor(_service);

            _service = nullptr;
            _getter = nullptr;
            _destructor = nullptr;
            _releaseMemory = nullptr;

            swap(*this, rvalue);

            return (*this);
        }

        ~FServiceHolder_() NOEXCEPT {
            if (_destructor)
                _destructor(_service);
        }

        void* Get() const NOEXCEPT {
            return (_getter ? _getter(_service) : _service);
        }

        bool ReleaseMemoryIFP() const NOEXCEPT {
            if (_releaseMemory) {
                _releaseMemory(_service);
                return true;
            }
            return false;
        }

        bool operator ==(const FServiceHolder_& other) const NOEXCEPT {
            Assert(other._service != _service ||
                (_getter == other._getter && _destructor == other._destructor && _releaseMemory == other._releaseMemory));
            return (_service == other._service &&
                    _getter == other._getter &&
                    _destructor == other._destructor &&
                    _releaseMemory == other._releaseMemory );
        }
        bool operator !=(const FServiceHolder_& other) const NOEXCEPT {
            return not operator==(other);
        }

        friend void swap(FServiceHolder_& lhs, FServiceHolder_& rhs) NOEXCEPT {
            std::swap(lhs._service, rhs._service);
            std::swap(lhs._getter, rhs._getter);
            std::swap(lhs._destructor, rhs._destructor);
            std::swap(lhs._releaseMemory, rhs._releaseMemory);
        }

        template <typename T>
        static constexpr release_memory_f MakeReleaseMemoryIFP(const T*) {
            IF_CONSTEXPR(Meta::has_defined_v<if_has_release_memory_, T>) {
                return [](void* p) NOEXCEPT {
                    static_cast<T*>(p)->ReleaseMemory();
                };
            }
            return nullptr;
        }

    private:
        template <typename T>
        using if_has_release_memory_ = decltype(std::declval<T&>().ReleaseMemory());

        void* _service{ nullptr };
        getter_f _getter{ nullptr };
        destructor_f _destructor{ nullptr };
        release_memory_f _releaseMemory{ nullptr };
    };

    template <typename T>
    static FServiceHolder_ MakeHolder_(T* rawptr) {
        Assert(rawptr);
        return FServiceHolder_{
            rawptr,
            nullptr,
            nullptr,
            FServiceHolder_::MakeReleaseMemoryIFP(rawptr)
        };
    }
    template <typename T>
    static FServiceHolder_ MakeHolder_(const TRefPtr<T>& refptr) {
        Assert(refptr);
        AddRef(refptr.get());
        return FServiceHolder_{
            refptr.get(),
            [](void* ptr) NOEXCEPT {
                RemoveRef(static_cast<FRefCountable*>(ptr));
            },
            FServiceHolder_::MakeReleaseMemoryIFP(refptr.get())
        };
    }
    template <typename T>
    static FServiceHolder_ MakeHolder_(TUniquePtr<T>&& uniqueptr) {
        Assert(uniqueptr);
        T* const rawptr = uniqueptr.get();
        POD_STORAGE(TUniquePtr<T>) no_dtor; // #HACK: steal reference without destroying
        new (&no_dtor) TUniquePtr<T>(std::move(uniqueptr));
        return FServiceHolder_{
            rawptr,
            TUniquePtr<T>::Deleter(),
            FServiceHolder_::MakeReleaseMemoryIFP(rawptr)
        };
    }

    void* GetService_(const FServiceKey_& key) const NOEXCEPT;

    void AddService_(const FServiceKey_& key, FServiceHolder_&& rholder);
    void RemoveService_(const FServiceKey_& key);

    const FStringView _name;
    const FModularServices* _parent;
    FLATMAP_INSITU(Internal, FServiceKey_, FServiceHolder_, 5) _services;
};
//----------------------------------------------------------------------------
template <typename _Interface>
NODISCARD _Interface& FModularServices::Get() const NOEXCEPT {
    CONSTEXPR const FServiceKey_ key = ServiceKey_<_Interface>();
    auto* const pservice = static_cast<_Interface*>(GetService_(key));
    AssertRelease(pservice);
    return (*pservice);
}
//----------------------------------------------------------------------------
template <typename _Interface>
NODISCARD _Interface* FModularServices::GetIFP() const NOEXCEPT {
    CONSTEXPR const FServiceKey_ key = ServiceKey_<_Interface>();
    return static_cast<_Interface*>(GetService_(key));
}
//----------------------------------------------------------------------------
template <typename _Interface, typename _Service>
void FModularServices::Add(_Service&& rservice) {
    CONSTEXPR const FServiceKey_ key = ServiceKey_<_Interface>();
    AddService_(key, MakeHolder_(std::move(rservice)));
}
//----------------------------------------------------------------------------
template <typename _Interface>
void FModularServices::Remove() {
    CONSTEXPR const FServiceKey_ key = ServiceKey_<_Interface>();
    RemoveService_(key);
}
//----------------------------------------------------------------------------
template <typename _Interface, typename _Service>
void FModularServices::CheckedRemove(_Service&& rservice) {
    CONSTEXPR const FServiceKey_ key = ServiceKey_<_Interface>();
    UNUSED(rservice);
    Assert_NoAssume(GetService_(key) == MakeHolder_(std::move(rservice)).Get());
    RemoveService_(key);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
