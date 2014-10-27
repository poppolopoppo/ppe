#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Render/Surfaces/AbstractRenderSurface.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/IO/String.h"
#include "Core/Maths/Geometry/ScalarVector.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(RenderSurfaceRelative);
class RenderSurfaceRelative : public AbstractRenderSurface {
public:
    RenderSurfaceRelative(  String&& name, const float2& ratio, // relative to backbuffer
                            const Graphics::SurfaceFormat *renderTargetFormatIFN,
                            const Graphics::SurfaceFormat *depthStencilFormatIFN = nullptr);
    RenderSurfaceRelative(  String&& name, const float2& ratio, AbstractRenderSurface *relative,
                            const Graphics::SurfaceFormat *renderTargetFormatIFN,
                            const Graphics::SurfaceFormat *depthStencilFormatIFN = nullptr );
    virtual ~RenderSurfaceRelative();

    const float2& Ratio() const { return _ratio; }
    const AbstractRenderSurface *Relative() const { return _relative.get(); }

    const Graphics::SurfaceFormat *RenderTargetFormat() const { return _renderTargetFormat; }
    const Graphics::SurfaceFormat *DepthStencilFormat() const { return _depthStencilFormat; }

    SINGLETON_POOL_ALLOCATED_DECL(RenderSurfaceRelative);

protected:
    virtual void CreateResources_(  Graphics::IDeviceAPIEncapsulator *device,
                                    Graphics::PCRenderTarget& pRenderTarget,
                                    Graphics::PCDepthStencil& pDepthStencil ) override;

    virtual void DestroyResources_( Graphics::IDeviceAPIEncapsulator *device,
                                    Graphics::PCRenderTarget& pRenderTarget,
                                    Graphics::PCDepthStencil& pDepthStencil ) override;

private:
    float2 _ratio;
    PAbstractRenderSurface _relative;

    const Graphics::SurfaceFormat *_renderTargetFormat;
    const Graphics::SurfaceFormat *_depthStencilFormat;

    PRenderSurfaceLock _relativeLock;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
