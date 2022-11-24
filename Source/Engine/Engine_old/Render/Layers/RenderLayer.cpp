// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "RenderLayer.h"

#include "Core/Allocator/PoolAllocator-impl.h"
#include "Core/Diagnostic/Logger.h"

#include "Effect/EffectCompiler.h"
#include "Render/RenderTree.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_TAGGED_DEF(Engine, FRenderLayer, );
//----------------------------------------------------------------------------
FRenderLayer::FRenderLayer(FString&& name)
:   FAbstractRenderLayer(std::move(name), true/* enabled */, true/* exported */) {
    Assert(!FName().empty());
    Assert(_effectVariability.Value == FVariabilitySeed::Invalid);
}
//----------------------------------------------------------------------------
FRenderLayer::~FRenderLayer() {
    Assert(_effectVariability.Value == FVariabilitySeed::Invalid);
}
//----------------------------------------------------------------------------
void FRenderLayer::PrepareImpl_(
    Graphics::IDeviceAPIEncapsulator *device,
    FMaterialDatabase *materialDatabase, const FRenderTree *renderTree, FVariabilitySeed *seeds ) {
    const FVariabilitySeed variability = renderTree->EffectCompiler()->Variability();
    if (variability != _effectVariability &&
        _effectVariability.Value != FVariabilitySeed::Invalid ) {
        LOG(Info, L"[FRenderLayer] Invalidate render batch in layer \"{0}\" ({1}>{2})",
            FName().c_str(), _effectVariability.Value, variability.Value );

        _batch.Destroy(device);
        _batch.Prepare(device, materialDatabase, renderTree, seeds); // prepare is done twice to ensure it has been called at least once before
    }
    else {
        _batch.Prepare(device, materialDatabase, renderTree, seeds);
    }

    _effectVariability = variability;
}
//----------------------------------------------------------------------------
void FRenderLayer::RenderImpl_(Graphics::IDeviceAPIContext *context) {
    _batch.Render(context);
}
//----------------------------------------------------------------------------
void FRenderLayer::DestroyImpl_(Graphics::IDeviceAPIEncapsulator *device, const FRenderTree * /* renderTree */) {
    _batch.Destroy(device);
    _effectVariability.Value = FVariabilitySeed::Invalid;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
