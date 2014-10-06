#pragma once

#include "Engine.h"

#include "AbstractRenderLayer.h"

#include "Core.Graphics/DeviceAPIEncapsulator.h"

#include "Core/PoolAllocator.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(AbstractRenderSurface);
FWD_REFPTR(RenderSurfaceLock);
struct VariabilitySeed;
//----------------------------------------------------------------------------
class RenderLayerClear : public AbstractRenderLayer {
public:
    explicit RenderLayerClear(  AbstractRenderSurface *surface,
                                const ColorRGBAF& color,
                                Graphics::ClearOptions options = Graphics::ClearOptions::DepthStencil,
                                float depth = 1.0f,
                                u8 stencil = 0 );
    virtual ~RenderLayerClear();

    const AbstractRenderSurface *Surface() const { return _surface; }

    const ColorRGBAF& Color() const { return _color; }
    Graphics::ClearOptions Options() const { return _options; }
    float Depth() const { return _depth; }
    u8 Stencil() const { return _stencil; }

    SINGLETON_POOL_ALLOCATED_DECL(RenderLayerClear);

protected:
    virtual void PrepareImpl_(Graphics::IDeviceAPIEncapsulator *device, const RenderTree *renderTree, VariabilitySeed *seeds) override;
    virtual void RenderImpl_(Graphics::IDeviceAPIContextEncapsulator *context) override;
    virtual void DestroyImpl_(Graphics::IDeviceAPIEncapsulator *device, const RenderTree *renderTree) override;

private:
    PAbstractRenderSurface _surface;

    ColorRGBAF _color;
    Graphics::ClearOptions _options;
    float _depth;
    u8 _stencil;

    PRenderSurfaceLock _surfaceLock;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
