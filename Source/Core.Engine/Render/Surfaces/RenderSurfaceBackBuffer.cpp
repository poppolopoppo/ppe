#include "stdafx.h"

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
SINGLETON_POOL_ALLOCATED_DEF(RenderSurfaceBackBuffer, );
//----------------------------------------------------------------------------
RenderSurfaceBackBuffer::RenderSurfaceBackBuffer(String&& name, Flags selected)
:   AbstractRenderSurface(std::move(name))
,   _selected(selected) {
    Assert(size_t(selected) != 0); // at least one or nothing to do :p
}
//----------------------------------------------------------------------------
RenderSurfaceBackBuffer::~RenderSurfaceBackBuffer() {}
//----------------------------------------------------------------------------
void RenderSurfaceBackBuffer::CreateResources_(
    Graphics::IDeviceAPIEncapsulator *device,
    Graphics::PCRenderTarget& pRenderTarget,
    Graphics::PCDepthStencil& pDepthStencil ) {
    Assert(!pRenderTarget);
    Assert(!pDepthStencil);

    if (RenderSurfaceBackBuffer::RenderTarget & _selected)
        pRenderTarget = device->BackBufferRenderTarget();

    if (RenderSurfaceBackBuffer::DepthStencil & _selected)
        pDepthStencil = device->BackBufferDepthStencil();
}
//----------------------------------------------------------------------------
void RenderSurfaceBackBuffer::DestroyResources_(
    Graphics::IDeviceAPIEncapsulator *device,
    Graphics::PCRenderTarget& pRenderTarget,
    Graphics::PCDepthStencil& pDepthStencil ) {
    if (RenderSurfaceBackBuffer::RenderTarget & _selected) {
        Assert(device->BackBufferRenderTarget() == pRenderTarget);
        pRenderTarget.reset(nullptr);
    }

    if (RenderSurfaceBackBuffer::DepthStencil & _selected) {
        Assert(device->BackBufferDepthStencil() == pDepthStencil);
        pDepthStencil.reset(nullptr);
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
