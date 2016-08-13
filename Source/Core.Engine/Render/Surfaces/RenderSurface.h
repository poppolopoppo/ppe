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

    SINGLETON_POOL_ALLOCATED_DECL();

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
