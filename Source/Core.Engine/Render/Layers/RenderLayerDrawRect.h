#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Render/Layers/AbstractRenderLayer.h"

#include "Core.Engine/Material/MaterialVariability.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Maths/Geometry/ScalarRectangle.h"

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
class IDeviceAPIContext;
FWD_REFPTR(VertexBuffer);
}

namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(MaterialEffect);
class RenderTree;
struct VariabilitySeed;
//----------------------------------------------------------------------------
class RenderLayerDrawRect : public AbstractRenderLayer {
public:
    explicit RenderLayerDrawRect(Engine::MaterialEffect *materialEffect);
    RenderLayerDrawRect(Engine::MaterialEffect *materialEffect, const RectangleF& viewport);
    virtual ~RenderLayerDrawRect();

    const Engine::MaterialEffect *MaterialEffect() const { return _materialEffect.get(); }
    const RectangleF& Viewport() const { return _viewport; }

    SINGLETON_POOL_ALLOCATED_DECL();

protected:
    virtual void PrepareImpl_(Graphics::IDeviceAPIEncapsulator *device, MaterialDatabase *materialDatabase, const RenderTree *renderTree, VariabilitySeed *seeds) override;
    virtual void RenderImpl_(Graphics::IDeviceAPIContext *context) override;
    virtual void DestroyImpl_(Graphics::IDeviceAPIEncapsulator *device, const RenderTree *renderTree) override;

private:
    PMaterialEffect _materialEffect;
    RectangleF _viewport;
    Graphics::PVertexBuffer _vertices;
    VariabilitySeed _effectVariability;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
