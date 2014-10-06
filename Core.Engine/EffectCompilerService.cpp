#include "stdafx.h"

#include "EffectCompilerService.h"

#include "Core/Guid.h"

#include "DeviceEncapsulatorService.h"
#include "IServiceProvider.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ENGINESERVICE_GUID_DEF(IEffectCompilerService);
//----------------------------------------------------------------------------
DefaultEffectCompilerService::DefaultEffectCompilerService()
:   IEffectCompilerService(ENGINESERVICE_CONSTRUCT(IEffectCompilerService)) {}
//----------------------------------------------------------------------------
DefaultEffectCompilerService::~DefaultEffectCompilerService() {}
//----------------------------------------------------------------------------
Engine::EffectCompiler *DefaultEffectCompilerService::EffectCompiler() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(ServiceAvailable());

    return &_effectCompiler;
}
//----------------------------------------------------------------------------
const Engine::EffectCompiler *DefaultEffectCompilerService::EffectCompiler() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(ServiceAvailable());

    return &_effectCompiler;
}
//----------------------------------------------------------------------------
void DefaultEffectCompilerService::Start(IServiceProvider *provider, const Guid& guid) {
    IEffectCompilerService::Start(provider, guid);

    ENGINESERVICE_PROVIDE(IDeviceEncapsulatorService, deviceService, provider);

    _effectCompiler.Start(deviceService->DeviceEncapsulator()->Device());
}
//----------------------------------------------------------------------------
void DefaultEffectCompilerService::Shutdown(IServiceProvider *provider, const Guid& guid) {
    IEffectCompilerService::Shutdown(provider, guid);

    ENGINESERVICE_PROVIDE(IDeviceEncapsulatorService, deviceService, provider);

    _effectCompiler.Shutdown(deviceService->DeviceEncapsulator()->Device());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
