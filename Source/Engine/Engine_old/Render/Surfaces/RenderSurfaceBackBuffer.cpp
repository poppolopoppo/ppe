// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "RenderSurfaceBackBuffer.h"

#include "Core.Graphics/Device/Texture/DepthStencil.h"
#include "Core.Graphics/Device/Texture/RenderTarget.h"
#include "Core.Graphics/Device/Texture/SurfaceFormat.h"

#include "Core/Allocator/PoolAllocator-impl.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_TAGGED_DEF(Engine, FRenderSurfaceBackBuffer, );
//----------------------------------------------------------------------------
FRenderSurfaceBackBuffer::FRenderSurfaceBackBuffer(FString&& name, EFlags selected)
:   FAbstractRenderSurface(std::move(name))
,   _selected(selected) {
    Assert(size_t(selected) != 0); // at least one or nothing to do :p
}
//----------------------------------------------------------------------------
FRenderSurfaceBackBuffer::~FRenderSurfaceBackBuffer() {}
//----------------------------------------------------------------------------
void FRenderSurfaceBackBuffer::CreateResources_(
    Graphics::IDeviceAPIEncapsulator *device,
    Graphics::PCRenderTarget& pRenderTarget,
    Graphics::PCDepthStencil& pDepthStencil ) {
    Assert(!pRenderTarget);
    Assert(!pDepthStencil);

    if (FRenderSurfaceBackBuffer::FRenderTarget & _selected)
        pRenderTarget = device->BackBufferRenderTarget();

    if (FRenderSurfaceBackBuffer::FDepthStencil & _selected)
        pDepthStencil = device->BackBufferDepthStencil();
}
//----------------------------------------------------------------------------
void FRenderSurfaceBackBuffer::DestroyResources_(
    Graphics::IDeviceAPIEncapsulator *device,
    Graphics::PCRenderTarget& pRenderTarget,
    Graphics::PCDepthStencil& pDepthStencil ) {
    if (FRenderSurfaceBackBuffer::FRenderTarget & _selected) {
        Assert(device->BackBufferRenderTarget() == pRenderTarget);
        pRenderTarget.reset(nullptr);
    }

    if (FRenderSurfaceBackBuffer::FDepthStencil & _selected) {
        Assert(device->BackBufferDepthStencil() == pDepthStencil);
        pDepthStencil.reset(nullptr);
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
