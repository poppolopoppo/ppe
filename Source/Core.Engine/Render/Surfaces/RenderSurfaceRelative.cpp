#include "stdafx.h"

#include "RenderSurfaceRelative.h"

#include "Core.Graphics/Device/Texture/DepthStencil.h"
#include "Core.Graphics/Device/Texture/RenderTarget.h"
#include "Core.Graphics/Device/Texture/SurfaceFormat.h"

#include "Core/Allocator/PoolAllocator-impl.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_TAGGED_DEF(Engine, RenderSurfaceRelative, );
//----------------------------------------------------------------------------
RenderSurfaceRelative::RenderSurfaceRelative(
    String&& name, const float2& ratio,
    const Graphics::SurfaceFormat *renderTargetFormatIFN,
    const Graphics::SurfaceFormat *depthStencilFormatIFN )
:   RenderSurfaceRelative(std::move(name), ratio, nullptr, renderTargetFormatIFN, depthStencilFormatIFN) {}
//----------------------------------------------------------------------------
RenderSurfaceRelative::RenderSurfaceRelative(
    String&& name, const float2& ratio, AbstractRenderSurface *relative,
    const Graphics::SurfaceFormat *renderTargetFormatIFN,
    const Graphics::SurfaceFormat *depthStencilFormatIFN )
:   AbstractRenderSurface(std::move(name))
,   _ratio(ratio)
,   _relative(relative)
,   _renderTargetFormat(renderTargetFormatIFN)
,   _depthStencilFormat(depthStencilFormatIFN) {
    Assert(ratio.AllGreaterThan(float2::Zero()));
    Assert( renderTargetFormatIFN || // at least one or nothing to do :p
            depthStencilFormatIFN );
    Assert( !depthStencilFormatIFN ||
            depthStencilFormatIFN->IsDepth() );
}
//----------------------------------------------------------------------------
RenderSurfaceRelative::~RenderSurfaceRelative() {
    Assert(!_relativeLock);
}
//----------------------------------------------------------------------------
void RenderSurfaceRelative::CreateResources_(
    Graphics::IDeviceAPIEncapsulator *device,
    Graphics::PCRenderTarget& pRenderTarget,
    Graphics::PCDepthStencil& pDepthStencil ) {
    Assert(!pRenderTarget);
    Assert(!pDepthStencil);
    Assert(!_relativeLock);

    Graphics::PRenderTarget rt;
    Graphics::PDepthStencil ds;

    const Graphics::RenderTarget *relativeRT = nullptr;
    const Graphics::DepthStencil *relativeDS = nullptr;

    if (_relative) {
        _relative->Prepare(device, _relativeLock);
        _relativeLock->Acquire(&relativeRT, &relativeDS);
    }
    else {
        relativeRT = device->BackBufferRenderTarget();
        relativeDS = device->BackBufferDepthStencil();
    }

    const Graphics::Texture2D *relativeTex2D = relativeRT;
    if (!relativeTex2D)
        relativeTex2D = relativeDS;
    Assert(relativeTex2D);

    const size_t relativeWidth = size_t(std::round(_ratio.x() * relativeTex2D->Width()) );
    const size_t relativeHeight = size_t(std::round(_ratio.y() * relativeTex2D->Height()) );
    Assert(relativeWidth > 0);
    Assert(relativeHeight > 0);

    if (_renderTargetFormat) {
        rt = new Graphics::RenderTarget(relativeWidth, relativeHeight, _renderTargetFormat);
        rt->Freeze();
        rt->Create(device);
    }

    if (_depthStencilFormat) {
        ds = new Graphics::DepthStencil(relativeWidth, relativeHeight, _depthStencilFormat);
        ds->Freeze();
        ds->Create(device);
    }

    pRenderTarget = rt;
    pDepthStencil = ds;
}
//----------------------------------------------------------------------------
void RenderSurfaceRelative::DestroyResources_(
    Graphics::IDeviceAPIEncapsulator *device,
    Graphics::PCRenderTarget& pRenderTarget,
    Graphics::PCDepthStencil& pDepthStencil ) {
    Assert(_relativeLock || !_relative);

    if (_relative)
        _relative->Destroy(device, _relativeLock);

    if (_renderTargetFormat) {
        const_cast<Graphics::RenderTarget *>(pRenderTarget.get())->Destroy(device);
        RemoveRef_AssertReachZero(pRenderTarget);
    }

    if (_depthStencilFormat) {
        const_cast<Graphics::DepthStencil *>(pDepthStencil.get())->Destroy(device);
        RemoveRef_AssertReachZero(pDepthStencil);
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
