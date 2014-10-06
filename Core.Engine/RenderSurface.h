#pragma once

#include "Engine.h"

#include "AbstractRenderSurface.h"

#include "Core/PoolAllocator.h"
#include "Core/RefPtr.h"
#include "Core/ScalarVector.h"
#include "Core/String.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(RenderSurface);
class RenderSurface : public AbstractRenderSurface {
public:
    RenderSurface(  String&& name, size_t width, size_t height,
                    const Graphics::SurfaceFormat *renderTargetFormatIFN,
                    const Graphics::SurfaceFormat *depthStencilFormatIFN );
    virtual ~RenderSurface();

    size_t Width() const { return _width; }
    size_t Height() const { return _height; }

    const Graphics::SurfaceFormat *RenderTargetFormat() const { return _renderTargetFormat; }
    const Graphics::SurfaceFormat *DepthStencilFormat() const { return _depthStencilFormat; }

    SINGLETON_POOL_ALLOCATED_DECL(RenderSurface);

protected:
    virtual void CreateResources_(  Graphics::IDeviceAPIEncapsulator *device,
                                    Graphics::PCRenderTarget& pRenderTarget,
                                    Graphics::PCDepthStencil& pDepthStencil ) override;

    virtual void DestroyResources_( Graphics::IDeviceAPIEncapsulator *device,
                                    Graphics::PCRenderTarget& pRenderTarget,
                                    Graphics::PCDepthStencil& pDepthStencil ) override;

private:
    size_t _width;
    size_t _height;

    const Graphics::SurfaceFormat *_renderTargetFormat;
    const Graphics::SurfaceFormat *_depthStencilFormat;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
