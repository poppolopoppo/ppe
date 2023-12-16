#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Effect/SharedConstantBufferFactory.h"
#include "Core.Engine/Service/IService.h"
#include "Core.Engine/Service/Service_fwd.h"

namespace Core {
struct FGuid;
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ISharedConstantBufferFactoryService : public IService {
protected:
    ISharedConstantBufferFactoryService(const char *serviceName, int servicePriority)
    :   IService(serviceName, servicePriority) {}
public:
    virtual ~ISharedConstantBufferFactoryService() {}

    virtual Engine::FSharedConstantBufferFactory *FSharedConstantBufferFactory() = 0;
    virtual const Engine::FSharedConstantBufferFactory *FSharedConstantBufferFactory() const = 0;

    ENGINESERVICE_GUID_DECL(ISharedConstantBufferFactoryService);
};
//----------------------------------------------------------------------------
class FDefaultSharedConstantBufferFactoryService : public ISharedConstantBufferFactoryService {
public:
    FDefaultSharedConstantBufferFactoryService();
    virtual ~FDefaultSharedConstantBufferFactoryService();

    virtual Engine::FSharedConstantBufferFactory *FSharedConstantBufferFactory() override;
    virtual const Engine::FSharedConstantBufferFactory *FSharedConstantBufferFactory() const override;

    virtual void Start(IServiceProvider *provider, const FGuid& guid) override;
    virtual void Shutdown(IServiceProvider *provider, const FGuid& guid) override;

private:
    Engine::FSharedConstantBufferFactory _sharedConstantBufferFactory;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
