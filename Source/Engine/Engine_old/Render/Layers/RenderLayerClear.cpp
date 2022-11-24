// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "RenderLayerClear.h"

#include "Core.Graphics/Device/DeviceAPI.h"
#include "Core.Graphics/Device/Texture/DepthStencil.h"
#include "Core.Graphics/Device/Texture/RenderTarget.h"

#include "Core/Allocator/Alloca.h"
#include "Core/Allocator/PoolAllocator-impl.h"
#include "Core/IO/Format.h"
#include "Core/IO/String.h"

#include "Render/Surfaces/AbstractRenderSurface.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static FString RenderLayerClearName_(FAbstractRenderSurface *surface) {
#ifdef WITH_CORE_ENGINE_RENDERLAYER_DEBUGNAME
    return StringFormat("Clear_{0}", surface->Name());
#else
    return FString();
#endif
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_TAGGED_DEF(Engine, FRenderLayerClear, );
//----------------------------------------------------------------------------
FRenderLayerClear::FRenderLayerClear(
    FAbstractRenderSurface *surface,
    const ColorRGBAF& color,
    Graphics::EClearOptions options /* = Graphics::EClearOptions::FDepthStencil */,
    float depth /* = 1.0f */,
    u8 stencil /* = 0 */ )
:   FAbstractRenderLayer(RenderLayerClearName_(surface))
,   _surface(surface)
,   _color(color)
,   _options(options)
,   _depth(depth)
,   _stencil(stencil) {
    Assert(surface);
}
//----------------------------------------------------------------------------
FRenderLayerClear::~FRenderLayerClear() {
    Assert(!_surfaceLock);
}
//----------------------------------------------------------------------------
void FRenderLayerClear::PrepareImpl_(
    Graphics::IDeviceAPIEncapsulator *device, 
    FMaterialDatabase * /* materialDatabase */, 
    const FRenderTree * /* renderTree */, 
    FVariabilitySeed * /* seeds */) {
    _surface->Prepare(device, _surfaceLock);
}
//----------------------------------------------------------------------------
void FRenderLayerClear::RenderImpl_(Graphics::IDeviceAPIContext *context) {
    Assert(_surfaceLock);

    const Graphics::FRenderTarget *rt = nullptr;
    const Graphics::FDepthStencil *ds = nullptr;
    _surfaceLock->Acquire(&rt, &ds);

    if (rt && !Meta::HasFlag(_options, Graphics::EClearOptions::NotRenderTarget) )
        context->Clear(rt, _color);

    if (ds && Graphics::EClearOptions::None != _options)
        context->Clear(ds, _options, _depth, _stencil);
}
//----------------------------------------------------------------------------
void FRenderLayerClear::DestroyImpl_(Graphics::IDeviceAPIEncapsulator *device, const FRenderTree * /* renderTree */) {
    Assert(_surfaceLock);

    _surface->Destroy(device, _surfaceLock);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
