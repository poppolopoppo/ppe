#include "stdafx.h"

#include "RenderSurface.h"

#include "Core.Graphics/DepthStencil.h"
#include "Core.Graphics/RenderTarget.h"
#include "Core.Graphics/SurfaceFormat.h"

#include "Core/PoolAllocator-impl.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_DEF(RenderSurface, );
//----------------------------------------------------------------------------
RenderSurface::RenderSurface(
    String&& name, size_t width, size_t height,
    const Graphics::SurfaceFormat *renderTargetFormatIFN,
    const Graphics::SurfaceFormat *depthStencilFormatIFN )
:   AbstractRenderSurface(std::move(name))
,   _width(width)
,   _height(height)
,   _renderTargetFormat(renderTargetFormatIFN)
,   _depthStencilFormat(depthStencilFormatIFN) {
    Assert(width > 0);
    Assert(height > 0);
    Assert( renderTargetFormatIFN || // at least one or nothing to do :p
            depthStencilFormatIFN );
    Assert( !depthStencilFormatIFN ||
            depthStencilFormatIFN->IsDepth() );
}
//----------------------------------------------------------------------------
RenderSurface::~RenderSurface() {}
//----------------------------------------------------------------------------
void RenderSurface::CreateResources_(
    Graphics::IDeviceAPIEncapsulator *device,
    Graphics::PCRenderTarget& pRenderTarget,
    Graphics::PCDepthStencil& pDepthStencil ) {
    Assert(!pRenderTarget);
    Assert(!pDepthStencil);

    Graphics::PRenderTarget rt;
    Graphics::PDepthStencil ds;

    if (_renderTargetFormat) {
        rt = new Graphics::RenderTarget(_width, _height, _renderTargetFormat);
        rt->Freeze();
        rt->Create(device);
    }

    if (_depthStencilFormat) {
        ds = new Graphics::DepthStencil(_width, _height, _depthStencilFormat);
        ds->Freeze();
        ds->Create(device);
    }

    pRenderTarget = rt;
    pDepthStencil = ds;
}
//----------------------------------------------------------------------------
void RenderSurface::DestroyResources_(
    Graphics::IDeviceAPIEncapsulator *device,
    Graphics::PCRenderTarget& pRenderTarget,
    Graphics::PCDepthStencil& pDepthStencil ) {
    if (_renderTargetFormat) {
        const_cast<Graphics::RenderTarget *>(pRenderTarget.get());
        RemoveRef_AssertReachZero(pRenderTarget);
    }

    if (_depthStencilFormat) {
        const_cast<Graphics::DepthStencil *>(pDepthStencil.get());
        RemoveRef_AssertReachZero(pDepthStencil);
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
