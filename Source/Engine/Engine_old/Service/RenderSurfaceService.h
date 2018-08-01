#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Render/RenderSurfaceManager.h"
#include "Core.Engine/Service/IService.h"
#include "Core.Engine/Service/Service_fwd.h"

namespace Core {
struct FGuid;
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

    virtual Engine::FRenderSurfaceManager *Manager() = 0;
    virtual const Engine::FRenderSurfaceManager *Manager() const = 0;

    ENGINESERVICE_GUID_DECL(IRenderSurfaceService);
};
//----------------------------------------------------------------------------
class FDefaultRenderSurfaceService: public IRenderSurfaceService {
public:
    FDefaultRenderSurfaceService();
    virtual ~FDefaultRenderSurfaceService();

    virtual Engine::FRenderSurfaceManager *Manager() override;
    virtual const Engine::FRenderSurfaceManager *Manager() const override;

    virtual void Start(IServiceProvider *provider, const FGuid& guid) override;
    virtual void Shutdown(IServiceProvider *provider, const FGuid& guid) override;

private:
    Engine::FRenderSurfaceManager _manager;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
