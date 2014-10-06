#pragma once

#include "Engine.h"

#include "AbstractRenderLayer.h"

#include "MaterialVariability.h"

#include "Core/PoolAllocator.h"
#include "Core/ScalarRectangle.h"

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
class IDeviceAPIContextEncapsulator;
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

    SINGLETON_POOL_ALLOCATED_DECL(RenderLayerDrawRect);

protected:
    virtual void PrepareImpl_(Graphics::IDeviceAPIEncapsulator *device, const RenderTree *renderTree, VariabilitySeed *seeds) override;
    virtual void RenderImpl_(Graphics::IDeviceAPIContextEncapsulator *context) override;
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
