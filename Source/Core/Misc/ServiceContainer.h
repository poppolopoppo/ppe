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

    class IService {
    public:
        IService() {}
        virtual ~IService() {}

        IService(const IService& ) = delete;
        IService& operator =(const IService& ) = delete;

        virtual ServiceId Id() const = 0;
    };
    typedef UniquePtr<const IService> UCService;

    template <typename _Interface>
    static constexpr ServiceId StaticServiceId() {
        return Meta::TypeHash<_Interface>::value();
    }

    template <typename _Interface>
    class Service : public IService {
    public:
        template <typename T>
        Service(T* pimpl) : _pimpl(pimpl) { Assert(_pimpl); }
        ~Service() { Assert(nullptr != _pimpl); }

        _Interface* Pimpl() const { return _pimpl; }

        virtual ServiceId Id() const override {
            return StaticServiceId<_Interface>();
        }

    private:
        _Interface* _pimpl;
    };

    typedef VECTORINSITU(Internal, UCService, 8) services_type;

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

    WRITESCOPELOCK(_barrierRW);
    Assert(nullptr != service);
    Assert(_services.end() == Find_(StaticServiceId<_Interface>()));

    LOG(Info, L"[Service] Register <{0}> with <{1}> (id={2:x})",
        typeid(_Interface).name(), typeid(T).name(), hash_t(StaticServiceId<_Interface>()) );

    _services.emplace_back(new Service<_Interface>(service));
}
//----------------------------------------------------------------------------
template <typename _Interface, typename T>
void ServiceContainer::Unregister(T* service) {
    STATIC_ASSERT(std::is_base_of<_Interface, T>::value);

    WRITESCOPELOCK(_barrierRW);
    Assert(nullptr != service);

    LOG(Info, L"[Service] Unregister <{0}> with <{1}> (id={2})",
        typeid(_Interface).name(), typeid(T).name(), hash_t(StaticServiceId<_Interface>()) );

    const auto it = Find_(StaticServiceId<_Interface>());
    Assert(_services.end() != it);
    Assert(nullptr != dynamic_cast<const Service<_Interface>*>(it->get()));
    Assert(service == static_cast<const Service<_Interface>*>(it->get())->Pimpl());

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
    READSCOPELOCK(_barrierRW);

    const auto it = Find_(StaticServiceId<_Interface>());

    if (_services.end() == it) {
        LOG(Warning, L"[Service] Unknown service <{0}> ! (id={1})",
            typeid(_Interface).name(), hash_t(StaticServiceId<_Interface>()) );

        return nullptr;
    }
    else {
        const auto* service = checked_cast<const Service<_Interface>*>(it->get());
        return service->Pimpl();
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
