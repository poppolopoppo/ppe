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
AbstractRenderLayer::AbstractRenderLayer(
    String&& name,
    bool enabled /* = true */,
    bool exported /* = false */,
    AbstractRenderLayer *next /* = nullptr */ )
:   _name(std::move(name)) {
    _nextWFlags.Reset(next, enabled, exported);
}
//----------------------------------------------------------------------------
AbstractRenderLayer::~AbstractRenderLayer() {}
//----------------------------------------------------------------------------
void AbstractRenderLayer::SetName(String&& name) {
    _name = std::move(name);
}
//----------------------------------------------------------------------------
void AbstractRenderLayer::SetEnabled(bool value) {
    _nextWFlags.SetFlag0(value);
}
//----------------------------------------------------------------------------
void AbstractRenderLayer::SetNext(AbstractRenderLayer *layer) {
    _nextWFlags.Set(layer);
}
//----------------------------------------------------------------------------
AbstractRenderLayer *AbstractRenderLayer::NextLayer(size_t offset) const {
    if (0 == offset)
        return const_cast<AbstractRenderLayer *>(this);

    AbstractRenderLayer *const next = Next();
    Assert(next);

    return next->NextLayer(offset - 1);
}
//----------------------------------------------------------------------------
RenderBatch *AbstractRenderLayer::RenderBatchIFP() {
    // TODO : check that this is called before prepare !
    return RenderBatchIFPImpl_();
}
//----------------------------------------------------------------------------
const RenderBatch *AbstractRenderLayer::RenderBatchIFP() const {
    // TODO : check that this is called before prepare !
    return RenderBatchIFPImpl_();
}
//----------------------------------------------------------------------------
void AbstractRenderLayer::Prepare(Graphics::IDeviceAPIEncapsulator *device, MaterialDatabase *materialDatabase, const RenderTree *renderTree, VariabilitySeed *seeds) {
    PrepareImpl_(device, materialDatabase, renderTree, seeds);
}
//----------------------------------------------------------------------------
void AbstractRenderLayer::Render(Graphics::IDeviceAPIContextEncapsulator *context) {
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
void AbstractRenderLayer::Destroy(Graphics::IDeviceAPIEncapsulator *device, const RenderTree *renderTree) {
    DestroyImpl_(device, renderTree);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
