#pragma once

#include "Core/Core.h"

#include "Core/Container/FlatMap.h"
#include "Core/Diagnostic/Logger.h"
#include "Core/Memory/UniquePtr.h"
#include "Core/Meta/TypeHash.h"
#include "Core/Thread/ReadWriteLock.h"

#include <type_traits>

namespace Core {
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
        return Meta::TTypeHash<_Interface>::value();
    }

    class TService {
    public:
        TService() : TService(0, nullptr, FStringView()) {}
        TService(FServiceId id, void* pimpl, const FStringView& name)
            : _id(id), _pimpl(pimpl), _name(name) {}

		FServiceId Id() const { return _id; }
        void* Pimpl() const { return _pimpl; }
        const FStringView& Name() const { return _name; }

    private:
		FServiceId _id;
        void* _pimpl;
        FStringView _name;
    };

    typedef FLAT_MAPINSITU(Internal, FServiceId, TService, 8) services_type;

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
        MakeStringView(typeid(_Interface).name(), Meta::noinit_tag{})
#endif
	};

    WRITESCOPELOCK(_barrierRW);

    _Interface* const pimpl = service; // important before casting to (void*)
	_services.Emplace_AssertUnique(serviceId, serviceId, (void*)pimpl, serviceName);

    LOG(Info, L"[Service] Register <{0}> with <{1}> (id={2:x})",
        serviceName, typeid(T).name(), hash_t(serviceId));
}
//----------------------------------------------------------------------------
template <typename _Interface, typename T>
void FServiceContainer::Unregister(T* service) {
    STATIC_ASSERT(std::is_base_of<_Interface, T>::value);
    Assert(nullptr != service);

    const FServiceId serviceId = StaticServiceId<_Interface>();

    WRITESCOPELOCK(_barrierRW);

    LOG(Info, L"[Service] Unregister <{0}> with <{1}> (id={2})",
        typeid(_Interface).name(), typeid(T).name(), hash_t(serviceId) );

#ifdef WITH_CORE_ASSERT
    _Interface* const pimpl = service; // important before casting to (void*)
#endif

    if (not _services.Erase(serviceId))
		AssertNotReached(); // service should have beed registered !
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
        LOG(Warning, L"[Service] Unknown service <{0}> ! (id={1})",
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
} //!namespace Core
