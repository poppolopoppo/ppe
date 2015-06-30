#include "stdafx.h"

#include "SharedConstantBufferFactoryService.h"

#include "Core/Meta/Guid.h"

#include "DeviceEncapsulatorService.h"
#include "IServiceProvider.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ENGINESERVICE_GUID_DEF(ISharedConstantBufferFactoryService);
//----------------------------------------------------------------------------
DefaultSharedConstantBufferFactoryService::DefaultSharedConstantBufferFactoryService()
:   ISharedConstantBufferFactoryService(ENGINESERVICE_CONSTRUCT(ISharedConstantBufferFactoryService)) {}
//----------------------------------------------------------------------------
DefaultSharedConstantBufferFactoryService::~DefaultSharedConstantBufferFactoryService() {}
//----------------------------------------------------------------------------
Engine::SharedConstantBufferFactory *DefaultSharedConstantBufferFactoryService::SharedConstantBufferFactory() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(ServiceAvailable());

    return &_sharedConstantBufferFactory;
}
//----------------------------------------------------------------------------
const Engine::SharedConstantBufferFactory *DefaultSharedConstantBufferFactoryService::SharedConstantBufferFactory() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(ServiceAvailable());

    return &_sharedConstantBufferFactory;
}
//----------------------------------------------------------------------------
void DefaultSharedConstantBufferFactoryService::Start(IServiceProvider *provider, const Guid& guid) {
    ISharedConstantBufferFactoryService::Start(provider, guid);

    ENGINESERVICE_PROVIDE(IDeviceEncapsulatorService, deviceService, provider);

    _sharedConstantBufferFactory.Start(deviceService->DeviceEncapsulator()->Device());
}
//----------------------------------------------------------------------------
void DefaultSharedConstantBufferFactoryService::Shutdown(IServiceProvider *provider, const Guid& guid) {
    ISharedConstantBufferFactoryService::Shutdown(provider, guid);

    ENGINESERVICE_PROVIDE(IDeviceEncapsulatorService, deviceService, provider);

    _sharedConstantBufferFactory.Shutdown(deviceService->DeviceEncapsulator()->Device());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
