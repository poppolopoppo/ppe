// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "TextureCacheService.h"

#include "Core/Meta/Guid.h"

#include "DeviceEncapsulatorService.h"
#include "IServiceProvider.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ENGINESERVICE_GUID_DEF(ITextureCacheService);
//----------------------------------------------------------------------------
FDefaultTextureCacheService::FDefaultTextureCacheService(size_t capacityInBytes)
:   ITextureCacheService(ENGINESERVICE_CONSTRUCT(ITextureCacheService))
,   _textureCache(capacityInBytes) {}
//----------------------------------------------------------------------------
FDefaultTextureCacheService::~FDefaultTextureCacheService() {}
//----------------------------------------------------------------------------
Engine::FTextureCache *FDefaultTextureCacheService::FTextureCache() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(ServiceAvailable());

    return &_textureCache;
}
//----------------------------------------------------------------------------
const Engine::FTextureCache *FDefaultTextureCacheService::FTextureCache() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(ServiceAvailable());

    return &_textureCache;
}
//----------------------------------------------------------------------------
void FDefaultTextureCacheService::Start(IServiceProvider *provider, const FGuid& guid) {
    ITextureCacheService::Start(provider, guid);

    ENGINESERVICE_PROVIDE(IDeviceEncapsulatorService, deviceService, provider);

    _textureCache.Start(deviceService->DeviceEncapsulator()->Device());
}
//----------------------------------------------------------------------------
void FDefaultTextureCacheService::Shutdown(IServiceProvider *provider, const FGuid& guid) {
    ITextureCacheService::Shutdown(provider, guid);

    ENGINESERVICE_PROVIDE(IDeviceEncapsulatorService, deviceService, provider);

    _textureCache.Shutdown(deviceService->DeviceEncapsulator()->Device());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
