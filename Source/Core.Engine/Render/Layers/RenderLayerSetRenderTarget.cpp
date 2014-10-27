#include "stdafx.h"

#include "RenderLayerSetRenderTarget.h"

#include "Core.Graphics/Device/DeviceAPIEncapsulator.h"
#include "Core.Graphics/Device/Texture/DepthStencil.h"
#include "Core.Graphics/Device/Texture/RenderTarget.h"

#include "Core/Allocator/Alloca.h"
#include "Core/Allocator/PoolAllocator.h"
#include "Core/IO/Format.h"
#include "Core/IO/Stream.h"
#include "Core/IO/String.h"
#include "Core/Memory/MemoryStack.h"

#include "Render/Surfaces/AbstractRenderSurface.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static String RenderLayerSetRenderTargetName_(AbstractRenderSurface *surface) {
#ifdef WITH_CORE_ENGINE_RENDERLAYER_DEBUGNAME
    return StringFormat("SetRenderTarget_{0}", surface->Name());
#else
    return String();
#endif
}
//----------------------------------------------------------------------------
static String RenderLayerSetRenderTargetName_(const MemoryView<PAbstractRenderSurface>& surfaces) {
#ifdef WITH_CORE_ENGINE_RENDERLAYER_DEBUGNAME
    char buffer[2048];
    {
        OCStrStream oss(buffer);
        oss << "SetRenderTargets";
        for (const PAbstractRenderSurface& surface : surfaces)
            oss << "_" << surface->Name();
    }
    return String(buffer);
#else
    return String();
#endif
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_DEF(RenderLayerSetRenderTarget, );
//----------------------------------------------------------------------------
RenderLayerSetRenderTarget::RenderLayerSetRenderTarget(AbstractRenderSurface *surface)
:   AbstractRenderLayer(RenderLayerSetRenderTargetName_(surface))
,   _count(1) {
    Assert(surface);

    _surfaces[0] = surface;
}
//----------------------------------------------------------------------------
RenderLayerSetRenderTarget::RenderLayerSetRenderTarget(const MemoryView<PAbstractRenderSurface>& surfaces)
:   AbstractRenderLayer(RenderLayerSetRenderTargetName_(surfaces))
,   _count(surfaces.size()) {
    Assert(!surfaces.empty());
    Assert(surfaces.size() < MaxSurface); // hard coded value from directx11 macro D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT

    for (size_t i = 0; i < _count; ++i) {
        _surfaces[i] = surfaces[i];
        Assert(_surfaces[i]);
    }
}
//----------------------------------------------------------------------------
RenderLayerSetRenderTarget::~RenderLayerSetRenderTarget() {}
//----------------------------------------------------------------------------
void RenderLayerSetRenderTarget::PrepareImpl_(Graphics::IDeviceAPIEncapsulator *device, const RenderTree *renderTree, VariabilitySeed *seeds) {
    for (size_t i = 0; i < _count; ++i)
        _surfaces[i]->Prepare(device, _surfaceLocks[i]);
}
//----------------------------------------------------------------------------
void RenderLayerSetRenderTarget::RenderImpl_(Graphics::IDeviceAPIContextEncapsulator *context) {
    if (1 == _count) {
        // only one render target

        const Graphics::RenderTarget *rt = nullptr;
        const Graphics::DepthStencil *ds = nullptr;
        _surfaceLocks[0]->Acquire(&rt, &ds);
        _surfaceLocks[0]->Unbind(context);

        const ViewportF viewport = (rt)
            ? ViewportF(0.0f, 0.0f, float(rt->Width()), float(rt->Height()), 0.0f, 1.0f)
            : ViewportF(0.0f, 0.0f, float(ds->Width()), float(ds->Height()), 0.0f, 1.0f);

        context->SetRenderTarget(rt, ds);
        context->SetViewport(viewport);
    }
    else {
        // Multiple Render Target (MRT)
        Assert(_count > 0);

        MALLOCA_STACK(Graphics::RenderTargetBinding, actualRTs, _count);
        const Graphics::DepthStencil *actualDS = nullptr;

        for (size_t i = 0; i < _count; ++i) {
            const Graphics::RenderTarget *rt = nullptr;
            const Graphics::DepthStencil *ds = nullptr;
            _surfaceLocks[i]->Acquire(&rt, &ds);
            _surfaceLocks[i]->Unbind(context);

            if (rt)
                actualRTs.PushPOD({rt, actualRTs.size()});

            if (ds && !actualDS)
                actualDS = ds;
        }

        const ViewportF viewport = (actualRTs.size())
            ? ViewportF(0.0f, 0.0f, float(actualRTs[0].RT->Width()), float(actualRTs[0].RT->Height()), 0.0f, 1.0f)
            : ViewportF(0.0f, 0.0f, float(actualDS->Width()), float(actualDS->Height()), 0.0f, 1.0f);

        context->SetRenderTargets(actualRTs, actualDS);
        context->SetViewport(viewport);
    }
}
//----------------------------------------------------------------------------
void RenderLayerSetRenderTarget::DestroyImpl_(Graphics::IDeviceAPIEncapsulator *device, const RenderTree *renderTree) {
    for (size_t i = 0; i < _count; ++i)
        _surfaces[i]->Destroy(device, _surfaceLocks[i]);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
