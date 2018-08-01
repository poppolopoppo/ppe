#include "stdafx.h"

#include "RenderSurfaceProxy.h"

#include "Core.Graphics/Device/Texture/DepthStencil.h"
#include "Core.Graphics/Device/Texture/RenderTarget.h"
#include "Core.Graphics/Device/Texture/SurfaceFormat.h"

#include "Core/Allocator/PoolAllocator-impl.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_TAGGED_DEF(Engine, FRenderSurfaceProxy, );
//----------------------------------------------------------------------------
FRenderSurfaceProxy::FRenderSurfaceProxy(
    FString&& name,
    FAbstractRenderSurface *renderTargetSurfaceIFN,
    FAbstractRenderSurface *depthStencilSurfaceIFN )
:   FAbstractRenderSurface(std::move(name))
,   _renderTargetSurface(renderTargetSurfaceIFN)
,   _depthStencilSurface(depthStencilSurfaceIFN) {
    Assert( renderTargetSurfaceIFN || // at least one or nothing to do :p
            depthStencilSurfaceIFN );
}
//----------------------------------------------------------------------------
FRenderSurfaceProxy::~FRenderSurfaceProxy() {}
//----------------------------------------------------------------------------
void FRenderSurfaceProxy::CreateResources_(
    Graphics::IDeviceAPIEncapsulator *device,
    Graphics::PCRenderTarget& pRenderTarget,
    Graphics::PCDepthStencil& pDepthStencil ) {
    Assert(!pRenderTarget);
    Assert(!pDepthStencil);

    const Graphics::FRenderTarget *proxyRT = nullptr;
    const Graphics::FDepthStencil *proxyDS = nullptr;

    if (_renderTargetSurface) {
        _renderTargetSurface->Prepare(device, _renderTargetLock);

        const Graphics::FRenderTarget *rt = nullptr;
        const Graphics::FDepthStencil *ds = nullptr;
        _renderTargetLock->Acquire(&rt, &ds);

        Assert(rt);
        proxyRT = rt;
    }

    if (_depthStencilSurface) {
        _depthStencilSurface->Prepare(device, _depthStencilLock);

        const Graphics::FRenderTarget *rt = nullptr;
        const Graphics::FDepthStencil *ds = nullptr;
        _depthStencilLock->Acquire(&rt, &ds);

        Assert(ds);
        proxyDS = ds;
    }
}
//----------------------------------------------------------------------------
void FRenderSurfaceProxy::DestroyResources_(
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
