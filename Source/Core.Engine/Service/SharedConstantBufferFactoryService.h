#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Effect/SharedConstantBufferFactory.h"
#include "Core.Engine/Service/IService.h"
#include "Core.Engine/Service/Service_fwd.h"

namespace Core {
struct Guid;
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

    virtual Engine::SharedConstantBufferFactory *SharedConstantBufferFactory() = 0;
    virtual const Engine::SharedConstantBufferFactory *SharedConstantBufferFactory() const = 0;

    ENGINESERVICE_GUID_DECL(ISharedConstantBufferFactoryService);
};
//----------------------------------------------------------------------------
class DefaultSharedConstantBufferFactoryService : public ISharedConstantBufferFactoryService {
public:
    DefaultSharedConstantBufferFactoryService();
    virtual ~DefaultSharedConstantBufferFactoryService();

    virtual Engine::SharedConstantBufferFactory *SharedConstantBufferFactory() override;
    virtual const Engine::SharedConstantBufferFactory *SharedConstantBufferFactory() const override;

    virtual void Start(IServiceProvider *provider, const Guid& guid) override;
    virtual void Shutdown(IServiceProvider *provider, const Guid& guid) override;

private:
    Engine::SharedConstantBufferFactory _sharedConstantBufferFactory;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
