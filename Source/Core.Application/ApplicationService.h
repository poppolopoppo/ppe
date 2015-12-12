#pragma once

#include "Core.Application/Application.h"

#include "Core/Diagnostic/Logger.h"
#include "Core/Container/Vector.h"
#include "Core/Memory/UniquePtr.h"
#include "Core/Meta/StronglyTyped.h"
#include "Core/Meta/ThreadResource.h"
#include "Core/Meta/TypeHash.h"

namespace Core {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
CORE_STRONGLYTYPED_NUMERIC_DEF(size_t, ServiceId);
//----------------------------------------------------------------------------
class IApplicationService {
public:
    virtual ~IApplicationService() {}
    virtual ServiceId ServiceId() const = 0;
};
//----------------------------------------------------------------------------
class ApplicationServiceContainer : Meta::ThreadResource {
public:
    template <typename _Interface, typename _Concrete>
    void Add(_Concrete& service) {
        LOG(Info, L"[Application][Service] Start <{0}> with <{1}>", typeid(_Interface).name(), typeid(_Concrete).name());
        //STATIC_ASSERT(std::is_assignable<_Interface*, _Concrete*>::value);
        THIS_THREADRESOURCE_CHECKACCESS();
        provider_type provider(new ApplicationServiceProvider_<_Interface>(service));
        const ServiceId serviceId(ApplicationServiceProvider_<_Interface>::Id());
        Assert(_providers.end() == std::find_if(_providers.begin(), _providers.end(), [=](const provider_type& p) { return serviceId == p->ServiceId(); }));
        _providers.emplace_back(std::move(provider));
    }

    template <typename _Interface, typename _Concrete>
    void Remove(_Concrete& service) {
        LOG(Info, L"[Application][Service] Shutdown <{0}> with <{1}>", typeid(_Interface).name(), typeid(_Concrete).name());
        THIS_THREADRESOURCE_CHECKACCESS();
        const ServiceId serviceId(ApplicationServiceProvider_<_Interface>::Id());
        const auto it = std::find_if(_providers.begin(), _providers.end(), [=](const provider_type& p) { return serviceId == p->ServiceId(); });
        Assert(_providers.end() != it);
        auto const provider = checked_cast<const ApplicationServiceProvider_<_Interface>* >(it->get());
        Assert(provider->Service() == &service);
        _providers.erase(it);
    }

    template <typename _Interface>
    _Interface* Get() const {
        THIS_THREADRESOURCE_CHECKACCESS();
        const ServiceId serviceId(ApplicationServiceProvider_<_Interface>::Id());
        const auto it = std::find_if(_providers.begin(), _providers.end(), [=](const provider_type& p) { return serviceId == p->ServiceId(); });
        if (_providers.end() == it) {
            LOG(Warning, L"[Application][Service] Unknown service <{0}> !", typeid(_Interface).name());
            return nullptr;
        }
        return checked_cast<const ApplicationServiceProvider_<_Interface>* >(it->get())->Service();
    }

private:
    template <typename _Interface>
    class ApplicationServiceProvider_ : public IApplicationService {
    public:
        static constexpr size_t Id() { return Meta::TypeHash<_Interface>::value(); }

        ApplicationServiceProvider_(_Interface& service) : _service(service) {}

        _Interface* Service() const { return (&_service); }
        virtual Application::ServiceId ServiceId() const override { return Application::ServiceId(Id()); }

    private:
        _Interface& _service;
    };

    typedef UniquePtr<const IApplicationService> provider_type;
    VECTOR(Application, provider_type) _providers;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core
