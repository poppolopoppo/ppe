#include "stdafx.h"

#include "AbstractRenderLayer.h"

#include "Scene/Scene.h"

#include "Core.Graphics/Device/DeviceAPI.h"
#include "Core.Graphics/Device/DeviceDiagnostics.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FAbstractRenderLayer::FAbstractRenderLayer(
    FString&& name,
    bool enabled /* = true */,
    bool exported /* = false */,
    FAbstractRenderLayer *next /* = nullptr */ )
:   _name(std::move(name)) {
    _nextWFlags.Reset(next, enabled, exported);
}
//----------------------------------------------------------------------------
FAbstractRenderLayer::~FAbstractRenderLayer() {}
//----------------------------------------------------------------------------
void FAbstractRenderLayer::SetName(FString&& name) {
    _name = std::move(name);
}
//----------------------------------------------------------------------------
void FAbstractRenderLayer::SetEnabled(bool value) {
    _nextWFlags.SetFlag0(value);
}
//----------------------------------------------------------------------------
void FAbstractRenderLayer::SetNext(FAbstractRenderLayer *layer) {
    _nextWFlags.Set(layer);
}
//----------------------------------------------------------------------------
FAbstractRenderLayer *FAbstractRenderLayer::NextLayer(size_t offset) const {
    if (0 == offset)
        return const_cast<FAbstractRenderLayer *>(this);

    FAbstractRenderLayer *const next = Next();
    Assert(next);

    return next->NextLayer(offset - 1);
}
//----------------------------------------------------------------------------
FRenderBatch *FAbstractRenderLayer::RenderBatchIFP() {
    // TODO : check that this is called before prepare !
    return RenderBatchIFPImpl_();
}
//----------------------------------------------------------------------------
const FRenderBatch *FAbstractRenderLayer::RenderBatchIFP() const {
    // TODO : check that this is called before prepare !
    return RenderBatchIFPImpl_();
}
//----------------------------------------------------------------------------
void FAbstractRenderLayer::Prepare(Graphics::IDeviceAPIEncapsulator *device, FMaterialDatabase *materialDatabase, const FRenderTree *renderTree, FVariabilitySeed *seeds) {
    PrepareImpl_(device, materialDatabase, renderTree, seeds);
}
//----------------------------------------------------------------------------
void FAbstractRenderLayer::Render(Graphics::IDeviceAPIContext *context) {
    if (Enabled()) {
        GRAPHICS_DIAGNOSTICS_BEGINEVENT(context->Diagnostics(), _name.c_str());

        RenderImpl_(context);

        GRAPHICS_DIAGNOSTICS_ENDEVENT(context->Diagnostics());
    }
    else {
        GRAPHICS_DIAGNOSTICS_SETMARKER(context->Diagnostics(), _name.c_str());
    }
}
//----------------------------------------------------------------------------
void FAbstractRenderLayer::Destroy(Graphics::IDeviceAPIEncapsulator *device, const FRenderTree *renderTree) {
    DestroyImpl_(device, renderTree);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
