#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Render/Surfaces/AbstractRenderSurface.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/IO/String.h"
#include "Core/Maths/ScalarVector.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(RenderSurfaceRelative);
class FRenderSurfaceRelative : public FAbstractRenderSurface {
public:
    FRenderSurfaceRelative(  FString&& name, const float2& ratio, // relative to backbuffer
                            const Graphics::FSurfaceFormat *renderTargetFormatIFN,
                            const Graphics::FSurfaceFormat *depthStencilFormatIFN = nullptr);
    FRenderSurfaceRelative(  FString&& name, const float2& ratio, FAbstractRenderSurface *relative,
                            const Graphics::FSurfaceFormat *renderTargetFormatIFN,
                            const Graphics::FSurfaceFormat *depthStencilFormatIFN = nullptr );
    virtual ~FRenderSurfaceRelative();

    const float2& Ratio() const { return _ratio; }
    const FAbstractRenderSurface *Relative() const { return _relative.get(); }

    const Graphics::FSurfaceFormat *RenderTargetFormat() const { return _renderTargetFormat; }
    const Graphics::FSurfaceFormat *DepthStencilFormat() const { return _depthStencilFormat; }

    SINGLETON_POOL_ALLOCATED_DECL();

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

    const Graphics::FSurfaceFormat *_renderTargetFormat;
    const Graphics::FSurfaceFormat *_depthStencilFormat;

    PRenderSurfaceLock _relativeLock;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
