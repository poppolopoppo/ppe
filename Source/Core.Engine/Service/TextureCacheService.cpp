#include "stdafx.h"

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
DefaultTextureCacheService::DefaultTextureCacheService(size_t capacityInBytes)
:   ITextureCacheService(ENGINESERVICE_CONSTRUCT(ITextureCacheService))
,   _textureCache(capacityInBytes) {}
//----------------------------------------------------------------------------
DefaultTextureCacheService::~DefaultTextureCacheService() {}
//----------------------------------------------------------------------------
Engine::TextureCache *DefaultTextureCacheService::TextureCache() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(ServiceAvailable());

    return &_textureCache;
}
//----------------------------------------------------------------------------
const Engine::TextureCache *DefaultTextureCacheService::TextureCache() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(ServiceAvailable());

    return &_textureCache;
}
//----------------------------------------------------------------------------
void DefaultTextureCacheService::Start(IServiceProvider *provider, const Guid& guid) {
    ITextureCacheService::Start(provider, guid);

    ENGINESERVICE_PROVIDE(IDeviceEncapsulatorService, deviceService, provider);

    _textureCache.Start(deviceService->DeviceEncapsulator()->Device());
}
//----------------------------------------------------------------------------
void DefaultTextureCacheService::Shutdown(IServiceProvider *provider, const Guid& guid) {
    ITextureCacheService::Shutdown(provider, guid);

    ENGINESERVICE_PROVIDE(IDeviceEncapsulatorService, deviceService, provider);

    _textureCache.Shutdown(deviceService->DeviceEncapsulator()->Device());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
