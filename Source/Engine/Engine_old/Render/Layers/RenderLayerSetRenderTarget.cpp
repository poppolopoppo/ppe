#include "stdafx.h"

#include "RenderLayerSetRenderTarget.h"

#include "Core.Graphics/Device/DeviceAPI.h"
#include "Core.Graphics/Device/Texture/DepthStencil.h"
#include "Core.Graphics/Device/Texture/RenderTarget.h"

#include "Core/Allocator/Alloca.h"
#include "Core/Allocator/PoolAllocator-impl.h"
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
static FString RenderLayerSetRenderTargetName_(FAbstractRenderSurface *surface) {
#ifdef WITH_CORE_ENGINE_RENDERLAYER_DEBUGNAME
    return StringFormat("SetRenderTarget_{0}", surface->Name());
#else
    return FString();
#endif
}
//----------------------------------------------------------------------------
static FString RenderLayerSetRenderTargetName_(const TMemoryView<const PAbstractRenderSurface>& surfaces) {
#ifdef WITH_CORE_ENGINE_RENDERLAYER_DEBUGNAME
    STACKLOCAL_OCSTRSTREAM(oss, 2048);
    oss << "SetRenderTargets";
    for (const PAbstractRenderSurface& surface : surfaces)
        oss << "_" << surface->Name();
    return ToString(oss.MakeView());
#else
    return FString();
#endif
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_TAGGED_DEF(Engine, FRenderLayerSetRenderTarget, );
//----------------------------------------------------------------------------
FRenderLayerSetRenderTarget::FRenderLayerSetRenderTarget(FAbstractRenderSurface *surface)
:   FAbstractRenderLayer(RenderLayerSetRenderTargetName_(surface))
,   _count(1) {
    Assert(surface);

    _surfaces[0] = surface;
}
//----------------------------------------------------------------------------
FRenderLayerSetRenderTarget::FRenderLayerSetRenderTarget(const TMemoryView<const PAbstractRenderSurface>& surfaces)
:   FAbstractRenderLayer(RenderLayerSetRenderTargetName_(surfaces))
,   _count(surfaces.size()) {
    Assert(!surfaces.empty());
    Assert(surfaces.size() < MaxSurface); // hard coded value from directx11 macro D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT

    for (size_t i = 0; i < _count; ++i) {
        _surfaces[i] = surfaces[i];
        Assert(_surfaces[i]);
    }
}
//----------------------------------------------------------------------------
FRenderLayerSetRenderTarget::FRenderLayerSetRenderTarget(const TMemoryView<const PAbstractRenderSurface>& surfaces, const PAbstractRenderSurface& depthStencil)
:   FAbstractRenderLayer(RenderLayerSetRenderTargetName_(surfaces))
,   _count(surfaces.size() + 1) {
    Assert(!surfaces.empty());
    Assert(depthStencil);
    Assert(surfaces.size() + 1 < MaxSurface); // hard coded value from directx11 macro D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT

    for (size_t i = 0; i < _count - 1; ++i) {
        _surfaces[i] = surfaces[i];
        Assert(_surfaces[i]);
    }

    _surfaces[_count - 1] = depthStencil;
}
//----------------------------------------------------------------------------
FRenderLayerSetRenderTarget::~FRenderLayerSetRenderTarget() {}
//----------------------------------------------------------------------------
void FRenderLayerSetRenderTarget::PrepareImpl_(
    Graphics::IDeviceAPIEncapsulator *device,
    FMaterialDatabase * /* materialDatabase */,
    const FRenderTree * /* renderTree */,
    FVariabilitySeed  * /* seeds */) {
    for (size_t i = 0; i < _count; ++i)
        _surfaces[i]->Prepare(device, _surfaceLocks[i]);
}
//----------------------------------------------------------------------------
void FRenderLayerSetRenderTarget::RenderImpl_(Graphics::IDeviceAPIContext *context) {
    if (1 == _count) {
        // only one render target

        const Graphics::FRenderTarget *rt = nullptr;
        const Graphics::FDepthStencil *ds = nullptr;
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

        STACKLOCAL_POD_STACK(Graphics::FRenderTargetBinding, actualRTs, _count);
        const Graphics::FDepthStencil *actualDS = nullptr;

        for (size_t i = 0; i < _count; ++i) {
            const Graphics::FRenderTarget *rt = nullptr;
            const Graphics::FDepthStencil *ds = nullptr;
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
void FRenderLayerSetRenderTarget::DestroyImpl_(Graphics::IDeviceAPIEncapsulator *device, const FRenderTree * /* renderTree */) {
    for (size_t i = 0; i < _count; ++i)
        _surfaces[i]->Destroy(device, _surfaceLocks[i]);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
