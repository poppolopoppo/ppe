#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Service/IService.h"
#include "Core.Engine/Service/Service_fwd.h"
#include "Core.Engine/Texture/TextureCache.h"

namespace Core {
struct Guid;
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

    virtual Engine::TextureCache *TextureCache() = 0;
    virtual const Engine::TextureCache *TextureCache() const = 0;

    ENGINESERVICE_GUID_DECL(ITextureCacheService);
};
//----------------------------------------------------------------------------
class DefaultTextureCacheService : public ITextureCacheService {
public:
    explicit DefaultTextureCacheService(size_t capacityInBytes);
    virtual ~DefaultTextureCacheService();

    virtual Engine::TextureCache *TextureCache() override;
    virtual const Engine::TextureCache *TextureCache() const override;

    virtual void Start(IServiceProvider *provider, const Guid& guid) override;
    virtual void Shutdown(IServiceProvider *provider, const Guid& guid) override;

private:
    Engine::TextureCache _textureCache;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
