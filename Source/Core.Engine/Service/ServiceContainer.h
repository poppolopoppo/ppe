#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Container/AssociativeVector.h"
#include "Core/Meta/Guid.h"

#include "Core.Engine/Service/IService.h"
#include "Core.Engine/Service/IServiceProvider.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ServiceContainer : public IServiceProvider {
public:
    ServiceContainer();
    virtual ~ServiceContainer();

    virtual void RegisterService(const Guid& guid, IService *service) override;
    virtual void UnregisterService(const Guid& guid, IService *service) override;

    virtual IService *RequestService(const Guid& guid) override;

    virtual void Start() override;
    virtual void Shutdown() override;

private:
    ASSOCIATIVE_VECTOR(Service, Guid, PService) _services;
#ifdef WITH_CORE_ENGINE_SERVICE_DEBUG
    bool _started;
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
