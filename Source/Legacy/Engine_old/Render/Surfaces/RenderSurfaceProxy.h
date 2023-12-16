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
class FRenderSurfaceProxy : public FAbstractRenderSurface {
public:
    FRenderSurfaceProxy( FString&& name,
                        FAbstractRenderSurface *renderTargetSurfaceIFN,
                        FAbstractRenderSurface *depthStencilSurfaceIFN );
    virtual ~FRenderSurfaceProxy();

    const FAbstractRenderSurface *RenderTargetSurface() const { return _renderTargetSurface; }
    const FAbstractRenderSurface *DepthStencilSurface() const { return _depthStencilSurface; }

    SINGLETON_POOL_ALLOCATED_DECL();

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
