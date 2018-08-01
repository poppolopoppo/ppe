#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Render/Layers/AbstractRenderLayer.h"

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
class FRenderLayerSetRenderTarget : public FAbstractRenderLayer {
public:
    explicit FRenderLayerSetRenderTarget(FAbstractRenderSurface *surface);
    explicit FRenderLayerSetRenderTarget(const TMemoryView<const PAbstractRenderSurface>& surfaces);
    FRenderLayerSetRenderTarget(const TMemoryView<const PAbstractRenderSurface>& surfaces, const PAbstractRenderSurface& depthStencil);
    virtual ~FRenderLayerSetRenderTarget();

    SINGLETON_POOL_ALLOCATED_DECL();

protected:
    virtual void PrepareImpl_(Graphics::IDeviceAPIEncapsulator *device, FMaterialDatabase *materialDatabase, const FRenderTree *renderTree, FVariabilitySeed *seeds) override;
    virtual void RenderImpl_(Graphics::IDeviceAPIContext *context) override;
    virtual void DestroyImpl_(Graphics::IDeviceAPIEncapsulator *device, const FRenderTree *renderTree) override;

private:
    STATIC_CONST_INTEGRAL(size_t, MaxSurface, 8);

    size_t _count;

    PAbstractRenderSurface _surfaces[MaxSurface];
    PRenderSurfaceLock _surfaceLocks[MaxSurface];
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
