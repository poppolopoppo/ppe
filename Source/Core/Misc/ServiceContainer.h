#pragma once

#include "Core/Core.h"

#include "Core/Container/Vector.h"
#include "Core/Diagnostic/Logger.h"
#include "Core/Memory/UniquePtr.h"
#include "Core/Meta/TypeHash.h"
#include "Core/Thread/ReadWriteLock.h"

#include <type_traits>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ServiceContainer {
public:
    ServiceContainer();
    ~ServiceContainer();

    ServiceContainer(const ServiceContainer& ) = delete;
    ServiceContainer& operator =(const ServiceContainer& ) = delete;

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
    void Create(UniquePtr<T>& service, _Args&&... args) {
        Assert(nullptr == service);
        service.reset(new T(std::forward<_Args>(args)...));
        Register<_Interface>(service.get());
    }

    template <typename _Interface, typename T>
    void Destroy(UniquePtr<T>& service) {
        Assert(nullptr != service);
        Unregister<_Interface>(service.get());
        service.reset();
    }

private:
    typedef size_t ServiceId;

    template <typename _Interface>
    static constexpr ServiceId  StaticServiceId() {
        return Meta::TypeHash<_Interface>::value();
    }

    class Service {
    public:
        Service() : Service(0, nullptr, nullptr) {}
        Service(ServiceId id, void* pimpl, const char*name)
            : _id(id), _pimpl(pimpl), _name(name) {}

        ServiceId Id() const { return _id; }
        void* Pimpl() const { return _pimpl; }
        const char* Name() const { return _name; }

    private:
        ServiceId _id;
        void* _pimpl;
        const char* _name;
    };

    typedef VECTORINSITU(Internal, Service, 8) services_type;

    services_type::const_iterator Find_(ServiceId serviceId) const;

    ReadWriteLock _barrierRW;
    services_type _services;
};
//----------------------------------------------------------------------------
inline size_t ServiceContainer::size() const {
    READSCOPELOCK(_barrierRW);
    return _services.size();
}
//----------------------------------------------------------------------------
template <typename _Interface, typename T>
void ServiceContainer::Register(T* service) {
    STATIC_ASSERT(std::is_base_of<_Interface, T>::value);

    const ServiceId serviceId = StaticServiceId<_Interface>();

    WRITESCOPELOCK(_barrierRW);
    Assert(nullptr != service);
    Assert(_services.end() == Find_(serviceId));

    const char* name(
#ifdef USE_DEBUG_LOGGER
        typeid(_Interface).name()
#else
        nullptr
#endif
    );

    LOG(Info, L"[Service] Register <{0}> with <{1}> (id={2:x})",
        name, typeid(T).name(), hash_t(serviceId) );

    _Interface* const pimpl = service; // important before casting to (void*)

    _services.emplace_back(serviceId, (void*)pimpl, name);
}
//----------------------------------------------------------------------------
template <typename _Interface, typename T>
void ServiceContainer::Unregister(T* service) {
    STATIC_ASSERT(std::is_base_of<_Interface, T>::value);

    const ServiceId serviceId = StaticServiceId<_Interface>();

    WRITESCOPELOCK(_barrierRW);
    Assert(nullptr != service);

    LOG(Info, L"[Service] Unregister <{0}> with <{1}> (id={2})",
        typeid(_Interface).name(), typeid(T).name(), hash_t(serviceId) );

#ifdef WITH_CORE_ASSERT
    _Interface* const pimpl = service; // important before casting to (void*)
#endif

    const auto it = Find_(serviceId);
    Assert(_services.end() != it);
    Assert((void*)pimpl == it->Pimpl());

    _services.erase_DontPreserveOrder(it);
}
//----------------------------------------------------------------------------
template <typename _Interface>
_Interface* ServiceContainer::Get() const {
    _Interface* const service = GetIFP<_Interface>();
    AssertRelease(nullptr != service);
    return service;
}
//----------------------------------------------------------------------------
template <typename _Interface>
_Interface* ServiceContainer::GetIFP() const {
    const ServiceId serviceId = StaticServiceId<_Interface>();

    READSCOPELOCK(_barrierRW);

    const auto it = Find_(serviceId);

    if (_services.end() == it) {
        LOG(Warning, L"[Service] Unknown service <{0}> ! (id={1})",
            typeid(_Interface).name(), hash_t(serviceId) );

        return nullptr;
    }
    else {
        Assert(it->Pimpl());
        return static_cast<_Interface*>(it->Pimpl());
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
