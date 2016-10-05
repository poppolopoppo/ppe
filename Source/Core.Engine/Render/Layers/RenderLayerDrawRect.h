#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Render/Layers/AbstractRenderLayer.h"

#include "Core.Engine/Material/MaterialVariability.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Maths/ScalarRectangle.h"

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
class FRenderTree;
struct FVariabilitySeed;
//----------------------------------------------------------------------------
class FRenderLayerDrawRect : public FAbstractRenderLayer {
public:
    explicit FRenderLayerDrawRect(Engine::FMaterialEffect *materialEffect);
    FRenderLayerDrawRect(Engine::FMaterialEffect *materialEffect, const RectangleF& viewport);
    virtual ~FRenderLayerDrawRect();

    const Engine::FMaterialEffect *FMaterialEffect() const { return _materialEffect.get(); }
    const RectangleF& Viewport() const { return _viewport; }

    SINGLETON_POOL_ALLOCATED_DECL();

protected:
    virtual void PrepareImpl_(Graphics::IDeviceAPIEncapsulator *device, FMaterialDatabase *materialDatabase, const FRenderTree *renderTree, FVariabilitySeed *seeds) override;
    virtual void RenderImpl_(Graphics::IDeviceAPIContext *context) override;
    virtual void DestroyImpl_(Graphics::IDeviceAPIEncapsulator *device, const FRenderTree *renderTree) override;

private:
    PMaterialEffect _materialEffect;
    RectangleF _viewport;
    Graphics::PVertexBuffer _vertices;
    FVariabilitySeed _effectVariability;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
