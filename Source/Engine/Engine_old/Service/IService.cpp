#include "stdafx.h"

#include "IService.h"

#include "Core/Meta/Guid.h"

#ifdef WITH_CORE_ENGINE_SERVICE_DEBUG
#   include "Core/Diagnostic/Logger.h"
#endif

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
IService::IService(const char *serviceName, int servicePriority)
:   _servicePriority(servicePriority)
#ifdef WITH_CORE_ENGINE_SERVICE_DEBUG
,   _serviceName(serviceName)
,   _serviceProvider(nullptr)
#endif
{
#ifdef WITH_CORE_ENGINE_SERVICE_DEBUG
    Assert(!_serviceName.empty());

    LOG(Info, L"[Service] Creating service <{0}> with priority {1} ...",
        _serviceName.c_str(), _servicePriority );
#else
    Assert(!serviceName); // no useless name when no debug
#endif
}
//----------------------------------------------------------------------------
IService::~IService() {
    THIS_THREADRESOURCE_CHECKACCESS();
#ifdef WITH_CORE_ENGINE_SERVICE_DEBUG
    LOG(Info, L"[Service] Destroying service <{0}> with priority {1} ...",
        _serviceName.c_str(), _servicePriority );

    Assert(!_serviceProvider); // service not stopped
#endif
}
//----------------------------------------------------------------------------
void IService::Start(IServiceProvider *provider, const FGuid& guid) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(provider);

#ifdef WITH_CORE_ENGINE_SERVICE_DEBUG
    Assert(!_serviceProvider);
    _serviceProvider = provider;

    LOG(Info, L"[Service] Starting service <{0}> with guid {1} and priority {2} ...",
        _serviceName.c_str(), guid, _servicePriority );
#endif
}
//----------------------------------------------------------------------------
void IService::Shutdown(IServiceProvider *provider, const FGuid& guid) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(provider);

#ifdef WITH_CORE_ENGINE_SERVICE_DEBUG
    Assert(provider == _serviceProvider);
    _serviceProvider = nullptr;

    LOG(Info, L"[Service] Shutting down service <{0}> with guid {1} and priority {2} ...",
        _serviceName.c_str(), guid, _servicePriority );
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
