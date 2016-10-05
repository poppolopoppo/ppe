#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Service/IService.h"
#include "Core.Engine/Service/Service_fwd.h"
#include "Core.Engine/Texture/TextureCache.h"

namespace Core {
struct FGuid;
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ITextureCacheService : public IService {
protected:
    ITextureCacheService(const char *serviceName, int servicePriority)
    :   IService(serviceName, servicePriority) {}
public:
    virtual ~ITextureCacheService() {}

    virtual Engine::FTextureCache *FTextureCache() = 0;
    virtual const Engine::FTextureCache *FTextureCache() const = 0;

    ENGINESERVICE_GUID_DECL(ITextureCacheService);
};
//----------------------------------------------------------------------------
class FDefaultTextureCacheService : public ITextureCacheService {
public:
    explicit FDefaultTextureCacheService(size_t capacityInBytes);
    virtual ~FDefaultTextureCacheService();

    virtual Engine::FTextureCache *FTextureCache() override;
    virtual const Engine::FTextureCache *FTextureCache() const override;

    virtual void Start(IServiceProvider *provider, const FGuid& guid) override;
    virtual void Shutdown(IServiceProvider *provider, const FGuid& guid) override;

private:
    Engine::FTextureCache _textureCache;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
