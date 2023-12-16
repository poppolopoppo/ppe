#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Render/Layers/AbstractRenderLayer.h"

#include "Core.Graphics/Device/DeviceAPI.h"

#include "Core/Allocator/PoolAllocator.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(AbstractRenderSurface);
FWD_REFPTR(RenderSurfaceLock);
struct FVariabilitySeed;
//----------------------------------------------------------------------------
class FRenderLayerClear : public FAbstractRenderLayer {
public:
    explicit FRenderLayerClear(  FAbstractRenderSurface *surface,
                                const ColorRGBAF& color,
                                Graphics::EClearOptions options = Graphics::EClearOptions::FDepthStencil,
                                float depth = 1.0f,
                                u8 stencil = 0 );
    virtual ~FRenderLayerClear();

    const FAbstractRenderSurface *Surface() const { return _surface; }

    const ColorRGBAF& Color() const { return _color; }
    Graphics::EClearOptions Options() const { return _options; }
    float Depth() const { return _depth; }
    u8 Stencil() const { return _stencil; }

    SINGLETON_POOL_ALLOCATED_DECL();

protected:
    virtual void PrepareImpl_(Graphics::IDeviceAPIEncapsulator *device, FMaterialDatabase *materialDatabase, const FRenderTree *renderTree, FVariabilitySeed *seeds) override;
    virtual void RenderImpl_(Graphics::IDeviceAPIContext *context) override;
    virtual void DestroyImpl_(Graphics::IDeviceAPIEncapsulator *device, const FRenderTree *renderTree) override;

private:
    PAbstractRenderSurface _surface;

    ColorRGBAF _color;
    Graphics::EClearOptions _options;
    float _depth;
    u8 _stencil;

    PRenderSurfaceLock _surfaceLock;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
