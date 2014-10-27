#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Render/Surfaces/AbstractRenderSurface.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/IO/String.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(RenderSurfaceProxy);
class RenderSurfaceProxy : public AbstractRenderSurface {
public:
    RenderSurfaceProxy( String&& name,
                        AbstractRenderSurface *renderTargetSurfaceIFN,
                        AbstractRenderSurface *depthStencilSurfaceIFN );
    virtual ~RenderSurfaceProxy();

    const AbstractRenderSurface *RenderTargetSurface() const { return _renderTargetSurface; }
    const AbstractRenderSurface *DepthStencilSurface() const { return _depthStencilSurface; }

    SINGLETON_POOL_ALLOCATED_DECL(RenderSurfaceProxy);

protected:
    virtual void CreateResources_(  Graphics::IDeviceAPIEncapsulator *device,
                                    Graphics::PCRenderTarget& pRenderTarget,
                                    Graphics::PCDepthStencil& pDepthStencil ) override;

    virtual void DestroyResources_( Graphics::IDeviceAPIEncapsulator *device,
                                    Graphics::PCRenderTarget& pRenderTarget,
                                    Graphics::PCDepthStencil& pDepthStencil ) override;

private:
    PAbstractRenderSurface _renderTargetSurface;
    PAbstractRenderSurface _depthStencilSurface;

    PRenderSurfaceLock _renderTargetLock;
    PRenderSurfaceLock _depthStencilLock;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
