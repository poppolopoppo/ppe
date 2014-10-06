#include "stdafx.h"

#include "RenderSurfaceProxy.h"

#include "Core.Graphics/DepthStencil.h"
#include "Core.Graphics/RenderTarget.h"
#include "Core.Graphics/SurfaceFormat.h"

#include "Core/PoolAllocator-impl.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_DEF(RenderSurfaceProxy, );
//----------------------------------------------------------------------------
RenderSurfaceProxy::RenderSurfaceProxy(
    String&& name,
    AbstractRenderSurface *renderTargetSurfaceIFN,
    AbstractRenderSurface *depthStencilSurfaceIFN )
:   AbstractRenderSurface(std::move(name))
,   _renderTargetSurface(renderTargetSurfaceIFN)
,   _depthStencilSurface(depthStencilSurfaceIFN) {
    Assert( renderTargetSurfaceIFN || // at least one or nothing to do :p
            depthStencilSurfaceIFN );
}
//----------------------------------------------------------------------------
RenderSurfaceProxy::~RenderSurfaceProxy() {}
//----------------------------------------------------------------------------
void RenderSurfaceProxy::CreateResources_(
    Graphics::IDeviceAPIEncapsulator *device,
    Graphics::PCRenderTarget& pRenderTarget,
    Graphics::PCDepthStencil& pDepthStencil ) {
    Assert(!pRenderTarget);
    Assert(!pDepthStencil);

    const Graphics::RenderTarget *proxyRT = nullptr;
    const Graphics::DepthStencil *proxyDS = nullptr;

    if (_renderTargetSurface) {
        _renderTargetSurface->Prepare(device, _renderTargetLock);

        const Graphics::RenderTarget *rt = nullptr;
        const Graphics::DepthStencil *ds = nullptr;
        _renderTargetLock->Acquire(&rt, &ds);

        Assert(rt);
        proxyRT = rt;
    }

    if (_depthStencilSurface) {
        _depthStencilSurface->Prepare(device, _depthStencilLock);

        const Graphics::RenderTarget *rt = nullptr;
        const Graphics::DepthStencil *ds = nullptr;
        _depthStencilLock->Acquire(&rt, &ds);

        Assert(ds);
        proxyDS = ds;
    }
}
//----------------------------------------------------------------------------
void RenderSurfaceProxy::DestroyResources_(
    Graphics::IDeviceAPIEncapsulator *device,
    Graphics::PCRenderTarget& pRenderTarget,
    Graphics::PCDepthStencil& pDepthStencil ) {
    if (_renderTargetSurface) {
        Assert(pRenderTarget);
        pRenderTarget.reset(nullptr);
        _renderTargetSurface->Destroy(device, _renderTargetLock);
    }

    if (_depthStencilSurface) {
        Assert(pDepthStencil);
        pDepthStencil.reset(nullptr);
        _depthStencilSurface->Destroy(device, _depthStencilLock);
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
