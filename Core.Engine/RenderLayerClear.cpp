#include "stdafx.h"

#include "RenderLayerClear.h"

#include "Core.Graphics/DepthStencil.h"
#include "Core.Graphics/DeviceAPIEncapsulator.h"
#include "Core.Graphics/RenderTarget.h"

#include "Core/Alloca.h"
#include "Core/Format.h"
#include "Core/PoolAllocator-impl.h"
#include "Core/String.h"

#include "AbstractRenderSurface.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static String RenderLayerClearName_(AbstractRenderSurface *surface) {
#ifdef WITH_CORE_ENGINE_RENDERLAYER_DEBUGNAME
    return StringFormat("Clear_{0}", surface->Name());
#else
    return String();
#endif
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_DEF(RenderLayerClear, );
//----------------------------------------------------------------------------
RenderLayerClear::RenderLayerClear(
    AbstractRenderSurface *surface,
    const ColorRGBAF& color,
    Graphics::ClearOptions options /* = Graphics::ClearOptions::DepthStencil */,
    float depth /* = 1.0f */,
    u8 stencil /* = 0 */ )
:   AbstractRenderLayer(RenderLayerClearName_(surface))
,   _surface(surface)
,   _color(color)
,   _options(options)
,   _depth(depth)
,   _stencil(stencil) {
    Assert(surface);
}
//----------------------------------------------------------------------------
RenderLayerClear::~RenderLayerClear() {
    Assert(!_surfaceLock);
}
//----------------------------------------------------------------------------
void RenderLayerClear::PrepareImpl_(Graphics::IDeviceAPIEncapsulator *device, const RenderTree *renderTree, VariabilitySeed *seeds) {
    _surface->Prepare(device, _surfaceLock);
}
//----------------------------------------------------------------------------
void RenderLayerClear::RenderImpl_(Graphics::IDeviceAPIContextEncapsulator *context) {
    Assert(_surfaceLock);

    const Graphics::RenderTarget *rt = nullptr;
    const Graphics::DepthStencil *ds = nullptr;
    _surfaceLock->Acquire(&rt, &ds);

    if (rt && !(size_t(Graphics::ClearOptions::NotRenderTarget) & size_t(_options)) )
        context->Clear(rt, _color);

    if (ds && Graphics::ClearOptions::None != _options)
        context->Clear(ds, _options, _depth, _stencil);
}
//----------------------------------------------------------------------------
void RenderLayerClear::DestroyImpl_(Graphics::IDeviceAPIEncapsulator *device, const RenderTree *renderTree) {
    Assert(_surfaceLock);

    _surface->Destroy(device, _surfaceLock);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
