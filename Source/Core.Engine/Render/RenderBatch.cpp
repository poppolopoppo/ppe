#include "stdafx.h"

#include "RenderBatch.h"

#include <algorithm>

#include "Core.Graphics/Device/DeviceAPIEncapsulator.h"
#include "Core.Graphics/Device/DeviceDiagnostics.h"
#include "Core.Graphics/Device/Geometry/IndexBuffer.h"
#include "Core.Graphics/Device/Geometry/PrimitiveType.h"
#include "Core.Graphics/Device/Geometry/VertexBuffer.h"

#include "Effect/Effect.h"
#include "Effect/MaterialEffect.h"
#include "Material/Material.h"
#include "Material/MaterialVariability.h"
#include "RenderCommand.h"
#include "RenderTree.h"
#include "Scene/Scene.h"
#include "Texture/TextureCache.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
struct RenderCommandPtrLess : public std::binary_function<const RenderCommand *, const RenderCommand *, bool> {
    bool operator ()(const RenderCommand *lhs, const RenderCommand *rhs) const {
        return lhs->operator <(*rhs);
    }
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
RenderBatch::RenderBatch() {}
//----------------------------------------------------------------------------
RenderBatch::~RenderBatch() {
    Assert(_commands.empty());
}
//----------------------------------------------------------------------------
void RenderBatch::Add(const RenderCommand *pcommand) {
    Assert(pcommand);
    Assert(pcommand->Batch == nullptr);

    const_cast<RenderCommand *>(pcommand)->Batch = this;
    Insert_AssertUnique(_commands, pcommand);
}
//----------------------------------------------------------------------------
void RenderBatch::Remove(const RenderCommand *pcommand) {
    Assert(pcommand);
    Assert(pcommand->Batch == this);

    Remove_AssertExists(_commands, pcommand);
    const_cast<RenderCommand *>(pcommand)->Batch = nullptr;
}
//----------------------------------------------------------------------------
void RenderBatch::Prepare(
    Graphics::IDeviceAPIEncapsulator *device,
    const RenderTree *renderTree,
    VariabilitySeed *seeds) {
    if (_commands.empty())
        return;

    std::sort(_commands.begin(), _commands.end(), RenderCommandPtrLess());

    const Scene *scene = renderTree->Scene();
    TextureCache *const TextureCache = renderTree->TextureCache();

    const RenderCommand *pred = nullptr;
    for (const RenderCommand *pcommand : _commands) {
        seeds[size_t(MaterialVariability::Batch)].Next();

        Assert(pcommand->Batch == this);

        if (!pred || pred->MaterialEffect != pcommand->MaterialEffect) {
            if (!pcommand->Ready) {
                pcommand->MaterialEffect->Create(device, renderTree->Scene());
                pcommand->Ready = true;
            }

            seeds[size_t(MaterialVariability::Material)].Next();
            pcommand->MaterialEffect->Prepare(device, scene, seeds);
        }

        pred = pcommand;
    }
}
//----------------------------------------------------------------------------
void RenderBatch::Render(Graphics::IDeviceAPIContextEncapsulator *context) {
    const Graphics::AbstractDeviceAPIEncapsulator *encapsulator = context->Encapsulator();

    const RenderCommand *pred = nullptr;
    for (const RenderCommand *pcommand : _commands) {
        Assert(pcommand->Ready);
        Assert(pcommand->Batch == this);

        GRAPHICS_DIAGNOSTICS_BEGINEVENT(encapsulator, pcommand->MaterialEffect->Material()->Name().cstr());

        if (!pred || pred->MaterialEffect->Effect() != pcommand->MaterialEffect->Effect()) {
            GRAPHICS_DIAGNOSTICS_SETMARKER(encapsulator, pcommand->MaterialEffect->Effect()->ResourceName());
            pcommand->MaterialEffect->Effect()->Set(context);
        }

        if (!pred || pred->Vertices != pcommand->Vertices) {
            GRAPHICS_DIAGNOSTICS_SETMARKER(encapsulator, pcommand->Vertices->ResourceName());
            context->SetVertexBuffer(pcommand->Vertices);
        }

        if (!pred || pred->Indices != pcommand->Indices) {
            GRAPHICS_DIAGNOSTICS_SETMARKER(encapsulator, pcommand->Indices->ResourceName());
            context->SetIndexBuffer(pcommand->Indices);
        }

        if (!pred || pred->MaterialEffect != pcommand->MaterialEffect) {
            GRAPHICS_DIAGNOSTICS_SETMARKER(encapsulator, pcommand->MaterialEffect->Material()->Name().cstr());
            pcommand->MaterialEffect->Set(context);
        }

        context->DrawIndexedPrimitives( Graphics::PrimitiveType(pcommand->PrimitiveType),
                                        pcommand->BaseVertex,
                                        pcommand->StartIndex,
                                        pcommand->PrimitiveCount );

        GRAPHICS_DIAGNOSTICS_ENDEVENT(encapsulator);

        pred = pcommand;
    }
}
//----------------------------------------------------------------------------
void RenderBatch::Destroy(Graphics::IDeviceAPIEncapsulator *device) {
    for (const RenderCommand *pcommand : _commands) {
        Assert(pcommand->Batch == this);

        if (pcommand->Ready) {
            pcommand->MaterialEffect->Destroy(device);
            pcommand->Ready = false;
        }
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
