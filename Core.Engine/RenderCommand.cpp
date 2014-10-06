#include "stdafx.h"

#include "RenderBatch.h"

#include <algorithm>

#include "Core.Graphics/DeviceAPIEncapsulator.h"
#include "Core.Graphics/IndexBuffer.h"
#include "Core.Graphics/PrimitiveType.h"
#include "Core.Graphics/VertexBuffer.h"

#include "Core/PoolAllocator-impl.h"

#include "AbstractRenderLayer.h"
#include "Effect.h"
#include "EffectDescriptor.h"
#include "EffectCompiler.h"
#include "Material.h"
#include "MaterialEffect.h"
#include "RenderTree.h"
#include "Scene.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_DEF(RenderCommand, );
//----------------------------------------------------------------------------
bool RenderCommand::operator ==(const RenderCommand& other) const {
    return  BaseVertex == other.BaseVertex &&
            StartIndex == other.StartIndex &&
            PrimitiveCount == other.PrimitiveCount &&
            PrimitiveType == other.PrimitiveType &&
            Vertices == other.Vertices &&
            Indices == other.Indices &&
            MaterialEffect == other.MaterialEffect;
}
//----------------------------------------------------------------------------
bool RenderCommand::operator <(const RenderCommand& other) const {
#define RENDERCOMMAND_FIELDSORT(_GET) \
    if ((_GET) < (other._GET)) \
        return true; \
    else if ((_GET) > (other._GET)) \
        return false

    RENDERCOMMAND_FIELDSORT(MaterialEffect->Effect());
    RENDERCOMMAND_FIELDSORT(Vertices);
    RENDERCOMMAND_FIELDSORT(Indices);
    RENDERCOMMAND_FIELDSORT(MaterialEffect);
    RENDERCOMMAND_FIELDSORT(BaseVertex);
    RENDERCOMMAND_FIELDSORT(StartIndex);
    RENDERCOMMAND_FIELDSORT(PrimitiveCount);

    return size_t(PrimitiveType) < size_t(other.PrimitiveType);

#undef RENDERCOMMAND_FIELDSORT
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool AcquireRenderCommand(
    UniquePtr<const RenderCommand>& pcommand,
    RenderTree *renderTree,
    const char *renderLayerName,
    const Material *material,
    const Graphics::IndexBuffer *indices,
    const Graphics::VertexBuffer *vertices,
    Graphics::PrimitiveType primitiveType,
    size_t baseVertex,
    size_t startIndex,
    size_t primitiveCount
    ) {
    Assert(!pcommand); // client must control lifetime
    Assert(renderTree);
    Assert(renderLayerName);
    Assert(material);
    Assert(indices);
    Assert(vertices);

    //
    // It is not allowed to request rendering too late in the frame !
    //
    // Why ?
    // -----
    // If added after Prepare() the command won't be sorted correctly and
    // will mess up the rendering of other commands.
    //
    renderTree->OwnedByThisThread();
    AssertRelease(renderTree->Scene()->Status() <= SceneStatus::BeforePrepare);

    PAbstractRenderLayer renderLayer;
    if (!renderTree->TryGet(renderLayerName, renderLayer)) {
        AssertNotImplemented();
        return false;
    }

    if (!renderLayer->Enabled())
        return false;

    RenderBatch *renderBatch = nullptr;
    if (!(renderBatch = renderLayer->RenderBatchIFP()) ) {
        AssertNotImplemented();
        return false;
    }

    const Scene *scene = renderTree->Scene();
    const MaterialDatabase *materialDatabase = scene->MaterialDatabase();

    PEffectDescriptor effectDescriptor;
    if (!materialDatabase->TryGetEffect(material->Name(), effectDescriptor)) {
        AssertNotImplemented();
        return false;
    }

    const PMaterialEffect materialEffect = renderTree->EffectCompiler()->CreateMaterialEffect(
        effectDescriptor, vertices->VertexDeclaration(), material);

    pcommand.reset(new RenderCommand {
        nullptr, // will be bet set by render batch
        materialEffect,
        indices, vertices,
        checked_cast<u32>(baseVertex),
        checked_cast<u32>(startIndex),
        checked_cast<u32>(primitiveCount),
        u32(primitiveType),
        false /* not ready */});

    // manually ref counting since this is a pod
    AddRef(pcommand->MaterialEffect);
    AddRef(pcommand->Indices);
    AddRef(pcommand->Vertices);

    renderBatch->Add(pcommand.get());

    return true;
}
//----------------------------------------------------------------------------
void ReleaseRenderCommand(
    UniquePtr<const RenderCommand>& pcommand,
    Graphics::IDeviceAPIEncapsulator *device ) {
    Assert(pcommand);
    Assert(pcommand->Batch);
    Assert(1 == pcommand->MaterialEffect->RefCount());
    Assert(device);

    if (pcommand->Ready) {
        pcommand->MaterialEffect->Destroy(device);
        pcommand->Ready = false;
    }

    pcommand->Batch->Remove(pcommand.get());

    // manually ref counting since this is a pod
    RemoveRef(pcommand->MaterialEffect);
    RemoveRef(pcommand->Indices);
    RemoveRef(pcommand->Vertices);

    pcommand.reset(nullptr);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
