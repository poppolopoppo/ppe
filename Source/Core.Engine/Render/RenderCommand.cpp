#include "stdafx.h"

#include "RenderBatch.h"

#include <algorithm>

#include "Core.Graphics/Device/DeviceAPIEncapsulator.h"
#include "Core.Graphics/Device/Geometry/IndexBuffer.h"
#include "Core.Graphics/Device/Geometry/PrimitiveType.h"
#include "Core.Graphics/Device/Geometry/VertexBuffer.h"

#include "Core/Allocator/PoolAllocator-impl.h"

#include "Effect/Effect.h"
#include "Effect/EffectCompiler.h"
#include "Effect/EffectDescriptor.h"
#include "Effect/MaterialEffect.h"
#include "Effect/MultiPassEffectDescriptor.h"
#include "Layers/AbstractRenderLayer.h"
#include "Material/Material.h"
#include "RenderTree.h"
#include "Scene/Scene.h"

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
namespace {
//----------------------------------------------------------------------------
static RenderCommand *AcquireRenderCommandForOnePass_(
    EffectCompiler *effectCompiler,
    AbstractRenderLayer *baseRenderLayer, 
    const Material *material,
    const EffectDescriptor *effectDescriptor,
    const Graphics::IndexBuffer *indices,
    const Graphics::VertexBuffer *vertices,
    Graphics::PrimitiveType primitiveType,
    size_t baseVertex,
    size_t startIndex,
    size_t primitiveCount ) {
    Assert(effectDescriptor);

    const Graphics::VertexDeclaration *vertexDeclaration = vertices->VertexDeclaration();

    const PMaterialEffect materialEffect = effectCompiler->CreateMaterialEffect(effectDescriptor, vertexDeclaration, material);

    RenderCommand *const pcommand = new RenderCommand {
        nullptr, // will be bet set by render batch
        materialEffect,
        indices, vertices,
        checked_cast<u32>(baseVertex),
        checked_cast<u32>(startIndex),
        checked_cast<u32>(primitiveCount),
        u32(primitiveType),
        false /* not ready */,
        nullptr /* next command */ };

    // manually ref counting since this is a pod
    AddRef(pcommand->MaterialEffect);
    AddRef(pcommand->Indices);
    AddRef(pcommand->Vertices);

    AbstractRenderLayer *const renderLayer = baseRenderLayer->NextLayer(effectDescriptor->RenderLayerOffset());
    AssertRelease(renderLayer);

    RenderBatch *const renderBatch = renderLayer->RenderBatchIFP();
    AssertRelease(renderBatch);

    renderBatch->Add(pcommand);

    return pcommand;
}
//----------------------------------------------------------------------------
static void ReleaseRenderCommandForOnePass_(
    const RenderCommand *pcommand,
    Graphics::IDeviceAPIEncapsulator *device ) {
    Assert(pcommand->Batch);

    if (pcommand->Ready) {
        pcommand->MaterialEffect->Destroy(device);
        pcommand->Ready = false;
    }

    pcommand->Batch->Remove(pcommand);

    // manually ref counting since this is a pod
    RemoveRef(pcommand->MaterialEffect);
    RemoveRef(pcommand->Indices);
    RemoveRef(pcommand->Vertices);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool AcquireRenderCommand(
    UniquePtr<const RenderCommand>& pOutCommand,
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
    Assert(!pOutCommand); // client must control lifetime
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

    const Scene *scene = renderTree->Scene();
    const MaterialDatabase *materialDatabase = scene->MaterialDatabase();

    PCEffectPasses effectPasses;
    if (!materialDatabase->TryGetEffect(material->Name(), effectPasses)) {
        AssertNotImplemented();
        return false;
    }
    Assert(effectPasses);

    const EffectDescriptor *effectDescriptors[MultiPassEffectDescriptor::MaxPassCount];
    const size_t effectDescriptorsCount = effectPasses->FillEffectPasses(effectDescriptors);
    if (0 == effectDescriptorsCount) {
        AssertNotReached();
        return false;
    }

    RenderCommand *prevcmd = nullptr;
    forrange(i, 0, effectDescriptorsCount) {
        RenderCommand *const passcmd = AcquireRenderCommandForOnePass_(
            renderTree->EffectCompiler(),
            renderLayer,
            material,
            effectDescriptors[i],
            indices,
            vertices,
            primitiveType,
            baseVertex,
            startIndex,
            primitiveCount );
        Assert(passcmd);

        if (nullptr == prevcmd)
            pOutCommand.reset(passcmd);
        else
            prevcmd->Next = passcmd;

        prevcmd = passcmd;
    }

    Assert(pOutCommand);
    return true;
}
//----------------------------------------------------------------------------
void ReleaseRenderCommand(
    UniquePtr<const RenderCommand>& pcommand,
    Graphics::IDeviceAPIEncapsulator *device ) {
    Assert(pcommand);
    Assert(1 == pcommand->MaterialEffect->RefCount());
    Assert(device);

    const RenderCommand *nextcmd = pcommand->Next;
    while (nextcmd) {
        const RenderCommand *nextnextcmd = nextcmd->Next;
        ReleaseRenderCommandForOnePass_(nextcmd, device);
        delete nextcmd;
        nextcmd = nextnextcmd;
    }

    ReleaseRenderCommandForOnePass_(pcommand.get(), device);
    pcommand.reset(nullptr);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
