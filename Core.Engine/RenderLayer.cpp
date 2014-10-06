#include "stdafx.h"

#include "RenderLayer.h"

#include "Core/Logger.h"
#include "Core/PoolAllocator-impl.h"

#include "EffectCompiler.h"
#include "RenderTree.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_DEF(RenderLayer, );
//----------------------------------------------------------------------------
RenderLayer::RenderLayer(String&& name)
:   AbstractRenderLayer(std::move(name), true/* enabled */, true/* exported */) {
    Assert(!Name().empty());
    Assert(_effectVariability.Value == VariabilitySeed::Invalid);
}
//----------------------------------------------------------------------------
RenderLayer::~RenderLayer() {
    Assert(_effectVariability.Value == VariabilitySeed::Invalid);
}
//----------------------------------------------------------------------------
void RenderLayer::PrepareImpl_(Graphics::IDeviceAPIEncapsulator *device, const RenderTree *renderTree, VariabilitySeed *seeds) {
    const VariabilitySeed variability = renderTree->EffectCompiler()->Variability();
    if (variability != _effectVariability &&
        _effectVariability.Value != VariabilitySeed::Invalid ) {
        LOG(Information, L"[RenderLayer] Invalidate render batch in layer \"{0}\" ({1}>{2})",
            Name().c_str(), _effectVariability.Value, variability.Value );

        _batch.Destroy(device);
        _batch.Prepare(device, renderTree, seeds); // prepare is done twice to ensure it has been called at least once before
    }
    else {
        _batch.Prepare(device, renderTree, seeds);
    }

    _effectVariability = variability;
}
//----------------------------------------------------------------------------
void RenderLayer::RenderImpl_(Graphics::IDeviceAPIContextEncapsulator *context) {
    _batch.Render(context);
}
//----------------------------------------------------------------------------
void RenderLayer::DestroyImpl_(Graphics::IDeviceAPIEncapsulator *device, const RenderTree *renderTree) {
    _batch.Destroy(device);
    _effectVariability.Value = VariabilitySeed::Invalid;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
