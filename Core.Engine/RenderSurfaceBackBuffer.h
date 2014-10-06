#pragma once

#include "Engine.h"

#include "AbstractRenderSurface.h"

#include "Core/PoolAllocator.h"
#include "Core/RefPtr.h"
#include "Core/String.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(RenderSurfaceBackBuffer);
class RenderSurfaceBackBuffer : public AbstractRenderSurface {
public:
    enum Flags {
        RenderTarget = 1,
        DepthStencil = 2,
        RenderTarget_DepthStencil = 3,
    };

    RenderSurfaceBackBuffer(String&& name, Flags selected);
    virtual ~RenderSurfaceBackBuffer();

    Flags Selected() const { return _selected; }

    SINGLETON_POOL_ALLOCATED_DECL(RenderSurfaceBackBuffer);

protected:
    virtual void CreateResources_(  Graphics::IDeviceAPIEncapsulator *device,
                                    Graphics::PCRenderTarget& pRenderTarget,
                                    Graphics::PCDepthStencil& pDepthStencil ) override;

    virtual void DestroyResources_( Graphics::IDeviceAPIEncapsulator *device,
                                    Graphics::PCRenderTarget& pRenderTarget,
                                    Graphics::PCDepthStencil& pDepthStencil ) override;

private:
    Flags _selected;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
