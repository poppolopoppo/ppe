#pragma once

#include "Engine.h"

#include "IService.h"
#include "Service_fwd.h"
#include "RenderSurfaceManager.h"

namespace Core {
struct Guid;
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IRenderSurfaceService : public IService {
protected:
    IRenderSurfaceService(const char *serviceName, int servicePriority)
    :   IService(serviceName, servicePriority) {}
public:
    virtual ~IRenderSurfaceService() {}

    virtual Engine::RenderSurfaceManager *Manager() = 0;
    virtual const Engine::RenderSurfaceManager *Manager() const = 0;

    ENGINESERVICE_GUID_DECL(IRenderSurfaceService);
};
//----------------------------------------------------------------------------
class DefaultRenderSurfaceService: public IRenderSurfaceService {
public:
    DefaultRenderSurfaceService();
    virtual ~DefaultRenderSurfaceService();

    virtual Engine::RenderSurfaceManager *Manager() override;
    virtual const Engine::RenderSurfaceManager *Manager() const override;

    virtual void Start(IServiceProvider *provider, const Guid& guid) override;
    virtual void Shutdown(IServiceProvider *provider, const Guid& guid) override;

private:
    Engine::RenderSurfaceManager _manager;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
