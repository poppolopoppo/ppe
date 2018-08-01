#include "stdafx.h"

#include "EffectCompilerService.h"

#include "Core/Meta/Guid.h"

#include "DeviceEncapsulatorService.h"
#include "SharedConstantBufferFactoryService.h"
#include "IServiceProvider.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ENGINESERVICE_GUID_DEF(IEffectCompilerService);
//----------------------------------------------------------------------------
FDefaultEffectCompilerService::FDefaultEffectCompilerService()
:   IEffectCompilerService(ENGINESERVICE_CONSTRUCT(IEffectCompilerService)) {}
//----------------------------------------------------------------------------
FDefaultEffectCompilerService::~FDefaultEffectCompilerService() {}
//----------------------------------------------------------------------------
Engine::FEffectCompiler *FDefaultEffectCompilerService::FEffectCompiler() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(ServiceAvailable());

    return &_effectCompiler;
}
//----------------------------------------------------------------------------
const Engine::FEffectCompiler *FDefaultEffectCompilerService::FEffectCompiler() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(ServiceAvailable());

    return &_effectCompiler;
}
//----------------------------------------------------------------------------
void FDefaultEffectCompilerService::Start(IServiceProvider *provider, const FGuid& guid) {
    IEffectCompilerService::Start(provider, guid);

    ENGINESERVICE_PROVIDE(IDeviceEncapsulatorService, deviceService, provider);
    ENGINESERVICE_PROVIDE(ISharedConstantBufferFactoryService, sharedConstantBufferFactoryService, provider);

    _effectCompiler.Start(  deviceService->DeviceEncapsulator()->Device(),
                            sharedConstantBufferFactoryService->SharedConstantBufferFactory() );
}
//----------------------------------------------------------------------------
void FDefaultEffectCompilerService::Shutdown(IServiceProvider *provider, const FGuid& guid) {
    IEffectCompilerService::Shutdown(provider, guid);

    ENGINESERVICE_PROVIDE(IDeviceEncapsulatorService, deviceService, provider);
    ENGINESERVICE_PROVIDE(ISharedConstantBufferFactoryService, sharedConstantBufferFactoryService, provider);

    _effectCompiler.Shutdown(   deviceService->DeviceEncapsulator()->Device(),
                                sharedConstantBufferFactoryService->SharedConstantBufferFactory() );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
