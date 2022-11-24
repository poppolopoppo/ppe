// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

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
FDefaultSharedConstantBufferFactoryService::FDefaultSharedConstantBufferFactoryService()
:   ISharedConstantBufferFactoryService(ENGINESERVICE_CONSTRUCT(ISharedConstantBufferFactoryService)) {}
//----------------------------------------------------------------------------
FDefaultSharedConstantBufferFactoryService::~FDefaultSharedConstantBufferFactoryService() {}
//----------------------------------------------------------------------------
Engine::FSharedConstantBufferFactory *FDefaultSharedConstantBufferFactoryService::FSharedConstantBufferFactory() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(ServiceAvailable());

    return &_sharedConstantBufferFactory;
}
//----------------------------------------------------------------------------
const Engine::FSharedConstantBufferFactory *FDefaultSharedConstantBufferFactoryService::FSharedConstantBufferFactory() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(ServiceAvailable());

    return &_sharedConstantBufferFactory;
}
//----------------------------------------------------------------------------
void FDefaultSharedConstantBufferFactoryService::Start(IServiceProvider *provider, const FGuid& guid) {
    ISharedConstantBufferFactoryService::Start(provider, guid);

    ENGINESERVICE_PROVIDE(IDeviceEncapsulatorService, deviceService, provider);

    _sharedConstantBufferFactory.Start(deviceService->DeviceEncapsulator()->Device());
}
//----------------------------------------------------------------------------
void FDefaultSharedConstantBufferFactoryService::Shutdown(IServiceProvider *provider, const FGuid& guid) {
    ISharedConstantBufferFactoryService::Shutdown(provider, guid);

    ENGINESERVICE_PROVIDE(IDeviceEncapsulatorService, deviceService, provider);

    _sharedConstantBufferFactory.Shutdown(deviceService->DeviceEncapsulator()->Device());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
