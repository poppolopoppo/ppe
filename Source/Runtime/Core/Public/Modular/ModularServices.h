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
    explicit FModularServices(FModularServices* parent);

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

    void Clear();

private:
    using FServiceKey_ = size_t;

    struct IServiceHolder_ {
        FServiceKey_ Key;

        IServiceHolder_(FServiceKey_ key) : Key(key) {}

        virtual ~IServiceHolder_() = default;

        virtual void* Get() const NOEXCEPT = 0;
    };

    using UServiceHolder_ = TUniquePtr<IServiceHolder_>;

    template <typename _Interface>
    static CONSTEXPR FServiceKey_ ServiceKey() NOEXCEPT {
        return Meta::type_id<_Interface>::value;
    }

    template <typename T>
    static void* ServicePtr_(T& ref) NOEXCEPT { return (&ref); }
    template <typename T>
    static void* ServicePtr_(const TRefPtr<T>& ptr) NOEXCEPT { return ptr.get(); }
    template <typename T>
    static void* ServicePtr_(const TUniquePtr<T>& ptr) NOEXCEPT { return ptr.get(); }

    template <typename T>
    struct TServiceHolder_ final : IServiceHolder_ {
        mutable T Service;

        template <typename... _Args>
        TServiceHolder_(FServiceKey_ key, _Args&&... args)
        :    IServiceHolder_(key)
        ,    Service(std::forward<_Args>(args)...)
        {}

        virtual void* Get() const NOEXCEPT override final {
            return ServicePtr_(Service);
        }
    };

    template <typename T>
    struct TServiceHolder_<T*> final : IServiceHolder_ {
        T* pService;

        TServiceHolder_(FServiceKey_ key, T* p)
        :   IServiceHolder_(key)
        ,   pService(p) {
            Assert(p);
            IF_CONSTEXPR(IsRefCountable<T>::value) {
                AddSafeRef(pService);
            }
        }

        ~TServiceHolder_() {
            IF_CONSTEXPR(IsRefCountable<T>::value) {
                RemoveSafeRef(pService);
            }
        }

        virtual void* Get() const NOEXCEPT override final {
            return pService;
        }
    };

    void* GetService_(FServiceKey_ key) const NOEXCEPT;

    void AddService_(FServiceKey_ key, UServiceHolder_&& rholder);
    void RemoveService_(FServiceKey_ key);

    FModularServices* _parent;
    FLATMAP_INSITU(Internal, FServiceKey_, UServiceHolder_, 5) _services;
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
    STATIC_ASSERT(std::is_base_of_v<_Interface, _Service>);
    CONSTEXPR const FServiceKey_ key = ServiceKey<_Interface>();
    AddService_(key, MakeUnique<TServiceHolder_<_Service>>(key, std::move(rservice)) );
}
//----------------------------------------------------------------------------
template <typename _Interface>
void FModularServices::Remove() {
    CONSTEXPR const FServiceKey_ key = ServiceKey<_Interface>();
    RemoveService_(key);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
