#include "stdafx.h"

#include "ServiceContainer.h"

#include "Core/Diagnostic/Logger.h"

#include <algorithm>

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
typedef TPair<FGuid, IService *> ServiceEntry;
struct FServiceLessByPriority_ {
    bool operator ()(const ServiceEntry& lhs, const ServiceEntry& rhs) const {
        return lhs.second->ServicePriority() < rhs.second->ServicePriority();
    }
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FServiceContainer::FServiceContainer()
#ifdef WITH_CORE_ENGINE_SERVICE_DEBUG
:   _started(false)
#endif
{}
//----------------------------------------------------------------------------
FServiceContainer::~FServiceContainer() {
    THIS_THREADRESOURCE_CHECKACCESS();

#ifdef WITH_CORE_ENGINE_SERVICE_DEBUG
    Assert(!_started);

    for (TPair<FGuid, PService>& service : _services) {
        LOG(Info, L"[Service] Removing reference for unregistered service <{0}> with guid {1} ...",
            service.second->ServiceName(), service.first );

        if (service.second->RefCount() > 1)
            LOG(Warning, L"[Service] Unregistered service <{0}> will survive ({1} references)",
                service.second->ServiceName(), service.first, service.second->RefCount() - 1 );

        service.second = nullptr;
    }
#endif

    _services.clear();
}
//----------------------------------------------------------------------------
void FServiceContainer::RegisterService(const FGuid& guid, IService *service) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!guid.empty());
    Assert(service);

#ifdef WITH_CORE_ENGINE_SERVICE_DEBUG
    Assert(!_started);

    LOG(Info, L"[Service] Register service <{0}> with guid {1} ...",
        service->ServiceName(), guid );
#endif

    _services.Insert_AssertUnique(guid, service);
}
//----------------------------------------------------------------------------
void FServiceContainer::UnregisterService(const FGuid& guid, IService *service) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!guid.empty());
    Assert(service);

#ifdef WITH_CORE_ENGINE_SERVICE_DEBUG
    Assert(!_started);

    LOG(Info, L"[Service] Unregister service <{0}> with guid {1} ...",
        service->ServiceName(), guid );
#endif

    _services.Remove_AssertExists(guid, service);
}
//----------------------------------------------------------------------------
IService *FServiceContainer::RequestService(const FGuid& guid) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!guid.empty());

    const auto it = _services.Find(guid);
    return (it != _services.end())  ? it->second.get() : nullptr;
}
//----------------------------------------------------------------------------
void FServiceContainer::Start() {
    THIS_THREADRESOURCE_CHECKACCESS();

#ifdef WITH_CORE_ENGINE_SERVICE_DEBUG
    Assert(!_started);
    _started = true;
#endif

    // stable sort services by priority
    std::stable_sort(_services.begin(), _services.end(), FServiceLessByPriority_());

    // straight order when shutting down
    for (const TPair<FGuid, PService>& service : _services)
        service.second->Start(this, service.first);
}
//----------------------------------------------------------------------------
void FServiceContainer::Shutdown() {
    THIS_THREADRESOURCE_CHECKACCESS();

#ifdef WITH_CORE_ENGINE_SERVICE_DEBUG
    Assert(_started);
    _started = false;
#endif

    // reverse order when shutting down
    const auto end = _services.rend();
    for (auto it = _services.rbegin(); it != end; ++it)
        it->second->Shutdown(this, it->first);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
