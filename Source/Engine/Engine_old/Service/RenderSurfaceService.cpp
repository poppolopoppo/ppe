#include "stdafx.h"

#include "RenderSurfaceService.h"

#include "Core/Meta/Guid.h"

#include "DeviceEncapsulatorService.h"
#include "IServiceProvider.h"

#define CORE_RENDERSURFACE_VIRTUALDIRW L"VirtualData:/Surfaces/"
#define CORE_RENDERSURFACE_RENDERTARGETBASENAMEW L"RT"
#define CORE_RENDERSURFACE_DEPTHSTENCILBASENAMEW L"DS"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ENGINESERVICE_GUID_DEF(IRenderSurfaceService);
//----------------------------------------------------------------------------
FDefaultRenderSurfaceService::FDefaultRenderSurfaceService()
:   IRenderSurfaceService(ENGINESERVICE_CONSTRUCT(IRenderSurfaceService))
,   _manager(CORE_RENDERSURFACE_VIRTUALDIRW, CORE_RENDERSURFACE_RENDERTARGETBASENAMEW, CORE_RENDERSURFACE_DEPTHSTENCILBASENAMEW){}
//----------------------------------------------------------------------------
FDefaultRenderSurfaceService::~FDefaultRenderSurfaceService() {}
//----------------------------------------------------------------------------
Engine::FRenderSurfaceManager *FDefaultRenderSurfaceService::Manager() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(ServiceAvailable());

    return &_manager;
}
//----------------------------------------------------------------------------
const Engine::FRenderSurfaceManager *FDefaultRenderSurfaceService::Manager() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(ServiceAvailable());

    return &_manager;
}
//----------------------------------------------------------------------------
void FDefaultRenderSurfaceService::Start(IServiceProvider *provider, const FGuid& guid) {
    IRenderSurfaceService::Start(provider, guid);

    ENGINESERVICE_PROVIDE(IDeviceEncapsulatorService, deviceService, provider);

    _manager.Start(deviceService->DeviceEncapsulator()->Device());
}
//----------------------------------------------------------------------------
void FDefaultRenderSurfaceService::Shutdown(IServiceProvider *provider, const FGuid& guid) {
    IRenderSurfaceService::Shutdown(provider, guid);

    ENGINESERVICE_PROVIDE(IDeviceEncapsulatorService, deviceService, provider);

    _manager.Shutdown(deviceService->DeviceEncapsulator()->Device());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
