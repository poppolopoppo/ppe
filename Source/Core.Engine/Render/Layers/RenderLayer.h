#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Render/Layers/AbstractRenderLayer.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Memory/RefPtr.h"

#include "Core.Engine/Material/MaterialVariability.h"
#include "Core.Engine/Render/RenderBatch.h"

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
class IDeviceAPIContext;
FWD_REFPTR(IndexBuffer);
FWD_REFPTR(VertexBuffer);
enum class EPrimitiveType;
}

namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(MaterialEffect);
struct FVariabilitySeed;
//----------------------------------------------------------------------------
class FRenderLayer : public FAbstractRenderLayer {
public:
    explicit FRenderLayer(FString&& name);
    virtual ~FRenderLayer();

    const FRenderBatch& Batch() const { return _batch; }

    SINGLETON_POOL_ALLOCATED_DECL();

protected:
    virtual FRenderBatch *RenderBatchIFPImpl_() override { return &_batch; }
    virtual const FRenderBatch *RenderBatchIFPImpl_() const override { return &_batch; }

    virtual void PrepareImpl_(Graphics::IDeviceAPIEncapsulator *device, FMaterialDatabase *materialDatabase, const FRenderTree *renderTree, FVariabilitySeed *seeds) override;
    virtual void RenderImpl_(Graphics::IDeviceAPIContext *context) override;
    virtual void DestroyImpl_(Graphics::IDeviceAPIEncapsulator *device, const FRenderTree *renderTree) override;

private:
    FRenderBatch _batch;
    FVariabilitySeed _effectVariability;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
