#pragma once

#include "Engine.h"

#include "AbstractRenderLayer.h"

#include "Core/PoolAllocator.h"
#include "Core/RefPtr.h"

#include "MaterialVariability.h"
#include "RenderBatch.h"

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
class IDeviceAPIContextEncapsulator;
FWD_REFPTR(IndexBuffer);
FWD_REFPTR(VertexBuffer);
enum class PrimitiveType;
}

namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(MaterialEffect);
struct VariabilitySeed;
//----------------------------------------------------------------------------
class RenderLayer : public AbstractRenderLayer {
public:
    explicit RenderLayer(String&& name);
    virtual ~RenderLayer();

    const RenderBatch& Batch() const { return _batch; }

    SINGLETON_POOL_ALLOCATED_DECL(RenderLayer);

protected:
    virtual RenderBatch *RenderBatchIFPImpl_() override { return &_batch; }
    virtual const RenderBatch *RenderBatchIFPImpl_() const override { return &_batch; }

    virtual void PrepareImpl_(Graphics::IDeviceAPIEncapsulator *device, const RenderTree *renderTree, VariabilitySeed *seeds) override;
    virtual void RenderImpl_(Graphics::IDeviceAPIContextEncapsulator *context) override;
    virtual void DestroyImpl_(Graphics::IDeviceAPIEncapsulator *device, const RenderTree *renderTree) override;

private:
    RenderBatch _batch;
    VariabilitySeed _effectVariability;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
