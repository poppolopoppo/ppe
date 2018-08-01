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
class FServiceContainer : public IServiceProvider {
public:
    FServiceContainer();
    virtual ~FServiceContainer();

    virtual void RegisterService(const FGuid& guid, IService *service) override;
    virtual void UnregisterService(const FGuid& guid, IService *service) override;

    virtual IService *RequestService(const FGuid& guid) override;

    virtual void Start() override;
    virtual void Shutdown() override;

private:
    ASSOCIATIVE_VECTOR(TService, FGuid, PService) _services;
#ifdef WITH_CORE_ENGINE_SERVICE_DEBUG
    bool _started;
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
