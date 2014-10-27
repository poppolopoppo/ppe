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
struct VariabilitySeed;
//----------------------------------------------------------------------------
class RenderLayerSetRenderTarget : public AbstractRenderLayer {
public:
    explicit RenderLayerSetRenderTarget(AbstractRenderSurface *surface);
    explicit RenderLayerSetRenderTarget(const MemoryView<PAbstractRenderSurface>& surfaces);
    virtual ~RenderLayerSetRenderTarget();

    SINGLETON_POOL_ALLOCATED_DECL(RenderLayerSetRenderTarget);

protected:
    virtual void PrepareImpl_(Graphics::IDeviceAPIEncapsulator *device, const RenderTree *renderTree, VariabilitySeed *seeds) override;
    virtual void RenderImpl_(Graphics::IDeviceAPIContextEncapsulator *context) override;
    virtual void DestroyImpl_(Graphics::IDeviceAPIEncapsulator *device, const RenderTree *renderTree) override;

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
