#pragma once

#include "Core.h"

#include "Container/FlatMap.h"
#include "Diagnostic/Logger.h"
#include "IO/StringView.h"
#include "Memory/UniquePtr.h"
#include "Meta/TypeHash.h"
#include "Thread/ReadWriteLock.h"

#include <type_traits>

namespace PPE {
EXTERN_LOG_CATEGORY(PPE_CORE_API, Services)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FServiceContainer {
public:
    FServiceContainer();
    ~FServiceContainer();

    FServiceContainer(const FServiceContainer& ) = delete;
    FServiceContainer& operator =(const FServiceContainer& ) = delete;

    size_t size() const;
    bool empty() const { return (0 == size()); }

    template <typename _Interface, typename T>
    void Register(T* service);
    template <typename _Interface, typename T>
    void Unregister(T* service);

    template <typename _Interface>
    _Interface* Get() const;
    template <typename _Interface>
    _Interface* GetIFP() const;

    template <typename _Interface, typename T, typename... _Args>
    void Create(TUniquePtr<T>& service, _Args&&... args) {
        Assert(nullptr == service);
        service.reset(new T(std::forward<_Args>(args)...));
        Register<_Interface>(service.get());
    }

    template <typename _Interface, typename T>
    void Destroy(TUniquePtr<T>& service) {
        Assert(nullptr != service);
        Unregister<_Interface>(service.get());
        service.reset();
    }

private:
    typedef size_t FServiceId;

    template <typename _Interface>
    static constexpr FServiceId  StaticServiceId() {
        return Meta::type_id<_Interface>::value;
    }

    class FServiceHolder {
    public:
        FServiceHolder() : FServiceHolder(0, nullptr, FStringView()) {}
        FServiceHolder(FServiceId id, void* pimpl, const FStringView& name)
            : _id(id), _pimpl(pimpl), _name(name) {}

        FServiceId Id() const { return _id; }
        void* Pimpl() const { return _pimpl; }
        const FStringView& Name() const { return _name; }

    private:
        FServiceId _id;
        void* _pimpl;
        FStringView _name;
    };

    typedef FLATMAP_INSITU(Internal, FServiceId, FServiceHolder, 8) services_type;

    FReadWriteLock _barrierRW;
    services_type _services;
};
//----------------------------------------------------------------------------
inline size_t FServiceContainer::size() const {
    READSCOPELOCK(_barrierRW);
    return _services.size();
}
//----------------------------------------------------------------------------
template <typename _Interface, typename T>
void FServiceContainer::Register(T* service) {
    STATIC_ASSERT(std::is_base_of<_Interface, T>::value);
    Assert(nullptr != service);

    const FServiceId serviceId = StaticServiceId<_Interface>();

    const FStringView serviceName{
#ifdef USE_DEBUG_LOGGER
        MakeCStringView(typeid(_Interface).name())
#endif
    };

    WRITESCOPELOCK(_barrierRW);

    _Interface* const pimpl = service; // important before casting to (void*)
    _services.Emplace_AssertUnique(serviceId, serviceId, (void*)pimpl, serviceName);

    LOG(Services, Info, L"[Service] Register <{0}> with <{1}> (id={2:x})",
        serviceName, typeid(T).name(), hash_t(serviceId));
}
//----------------------------------------------------------------------------
template <typename _Interface, typename T>
void FServiceContainer::Unregister(T* service) {
    STATIC_ASSERT(std::is_base_of<_Interface, T>::value);
    Assert(nullptr != service);

    const FServiceId serviceId = StaticServiceId<_Interface>();

    WRITESCOPELOCK(_barrierRW);

    LOG(Services, Info, L"[Service] Unregister <{0}> with <{1}> (id={2})",
        typeid(_Interface).name(), typeid(T).name(), hash_t(serviceId) );

#ifdef WITH_PPE_ASSERT
    const auto it = _services.Find(serviceId);
    if (_services.end() == it)
        AssertNotReached();

    _Interface* const pimpl = service; // important before casting to (void*)
    Assert(it->second.Pimpl() == pimpl);

    _services.Erase(it);
#else
    if (not _services.Erase(serviceId))
        AssertNotReached(); // service should have been already registered !
#endif
}
//----------------------------------------------------------------------------
template <typename _Interface>
_Interface* FServiceContainer::Get() const {
    _Interface* const service = GetIFP<_Interface>();
    AssertRelease(nullptr != service);
    return service;
}
//----------------------------------------------------------------------------
template <typename _Interface>
_Interface* FServiceContainer::GetIFP() const {
    const FServiceId serviceId = StaticServiceId<_Interface>();

    READSCOPELOCK(_barrierRW);

    const auto it = _services.Find(serviceId);

    if (_services.end() == it) {
        LOG(Services, Warning, L"[Service] Unknown service <{0}> ! (id={1})",
            typeid(_Interface).name(), hash_t(serviceId) );

        return nullptr;
    }
    else {
        Assert(it->second.Pimpl());
        return static_cast<_Interface*>(it->second.Pimpl());
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
