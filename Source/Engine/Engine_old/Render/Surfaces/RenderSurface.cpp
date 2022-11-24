// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "RenderSurface.h"

#include "Core.Graphics/Device/Texture/DepthStencil.h"
#include "Core.Graphics/Device/Texture/RenderTarget.h"
#include "Core.Graphics/Device/Texture/SurfaceFormat.h"

#include "Core/Allocator/PoolAllocator-impl.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_TAGGED_DEF(Engine, FRenderSurface, );
//----------------------------------------------------------------------------
FRenderSurface::FRenderSurface(
    FString&& name, size_t width, size_t height,
    const Graphics::FSurfaceFormat *renderTargetFormatIFN,
    const Graphics::FSurfaceFormat *depthStencilFormatIFN )
:   FAbstractRenderSurface(std::move(name))
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
FRenderSurface::~FRenderSurface() {}
//----------------------------------------------------------------------------
void FRenderSurface::CreateResources_(
    Graphics::IDeviceAPIEncapsulator *device,
    Graphics::PCRenderTarget& pRenderTarget,
    Graphics::PCDepthStencil& pDepthStencil ) {
    Assert(!pRenderTarget);
    Assert(!pDepthStencil);

    Graphics::PRenderTarget rt;
    Graphics::PDepthStencil ds;

    if (_renderTargetFormat) {
        rt = new Graphics::FRenderTarget(_width, _height, _renderTargetFormat);
        rt->Freeze();
        rt->Create(device);
    }

    if (_depthStencilFormat) {
        ds = new Graphics::FDepthStencil(_width, _height, _depthStencilFormat);
        ds->Freeze();
        ds->Create(device);
    }

    pRenderTarget = rt;
    pDepthStencil = ds;
}
//----------------------------------------------------------------------------
void FRenderSurface::DestroyResources_(
    Graphics::IDeviceAPIEncapsulator *device,
    Graphics::PCRenderTarget& pRenderTarget,
    Graphics::PCDepthStencil& pDepthStencil ) {
    if (_renderTargetFormat) {
        const_cast<Graphics::FRenderTarget *>(pRenderTarget.get());
        RemoveRef_AssertReachZero(pRenderTarget);
    }

    if (_depthStencilFormat) {
        const_cast<Graphics::FDepthStencil *>(pDepthStencil.get());
        RemoveRef_AssertReachZero(pDepthStencil);
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
