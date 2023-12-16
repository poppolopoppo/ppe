// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "RenderCommand.h"

#include <algorithm>

#include "Core.Graphics/Device/DeviceAPI.h"
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
#include "Scene/Scene.h"

#include "RenderBatch.h"
#include "RenderTree.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static FRenderCommandRegistration *AcquireRenderCommandForOnePass_(
    FEffectCompiler *effectCompiler,
    FAbstractRenderLayer *baseRenderLayer, 
    const FMaterial *material,
    const FEffectDescriptor *effectDescriptor,
    const Graphics::IndexBuffer *indices,
    const Graphics::FVertexBuffer *vertices,
    Graphics::EPrimitiveType primitiveType,
    size_t baseVertex,
    size_t startIndex,
    size_t primitiveCount ) {
    Assert(effectDescriptor);

    const Graphics::FVertexDeclaration *vertexDeclaration = vertices->VertexDeclaration();

    const PMaterialEffect materialEffect = effectCompiler->CreateMaterialEffect(effectDescriptor, vertexDeclaration, material);

    AddRef(materialEffect);
    AddRef(indices);
    AddRef(vertices);

    FRenderCommandCriteria criteria;
    criteria.SetMaterialEffect(materialEffect);
    criteria.Indices = indices;
    criteria.Vertices = vertices;
    criteria.Effect = materialEffect->Effect();
    Assert(!criteria.Ready());

    FRenderCommandParams params;
    params.BaseVertex = checked_cast<u32>(baseVertex);
    params.StartIndex = checked_cast<u32>(startIndex);
    params.SetPrimitiveCount(checked_cast<u32>(primitiveCount));
    params.SetPrimitiveType(primitiveType);

    FRenderCommandRegistration *const pcommand = new FRenderCommandRegistration {
        nullptr,    /* will be bet set by render batch */
        nullptr     /* next command */ };

    FAbstractRenderLayer *const renderLayer = baseRenderLayer->NextLayer(effectDescriptor->RenderLayerOffset());
    AssertRelease(renderLayer);

    FRenderBatch *const renderBatch = renderLayer->RenderBatchIFP();
    AssertRelease(renderBatch);

    renderBatch->Add(pcommand, criteria, params);

    return pcommand;
}
//----------------------------------------------------------------------------
static void ReleaseRenderCommandForOnePass_(
    const FRenderCommandRegistration *pcommand,
    Graphics::IDeviceAPIEncapsulator *device ) {
    Assert(pcommand->Batch);

    const FRenderCommandCriteria criteria = pcommand->Batch->Remove(pcommand);
    Assert(1 == criteria.MaterialEffect()->RefCount());

    if (criteria.Ready())
        criteria.MaterialEffect()->Destroy(device);

    RemoveRef(criteria.Vertices);
    RemoveRef(criteria.Indices);
    RemoveRef(criteria.MaterialEffect());
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_TAGGED_DEF(Engine, FRenderCommandRegistration, );
//----------------------------------------------------------------------------
bool AcquireRenderCommand(
    URenderCommand& pOutCommand,
    FRenderTree *renderTree,
    const char *renderLayerName,
    const FMaterial *material,
    const Graphics::IndexBuffer *indices,
    const Graphics::FVertexBuffer *vertices,
    Graphics::EPrimitiveType primitiveType,
    size_t baseVertex,
    size_t startIndex,
    size_t primitiveCount ) {
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
    AssertRelease(renderTree->Scene()->Status() <= ESceneStatus::BeforePrepare);

    PAbstractRenderLayer renderLayer;
    if (!renderTree->TryGet(renderLayerName, renderLayer)) {
        AssertNotImplemented();
        return false;
    }

    if (!renderLayer->Enabled())
        return false;

    const FScene *scene = renderTree->Scene();
    const FMaterialDatabase *materialDatabase = scene->MaterialDatabase();

    PCEffectPasses effectPasses;
    if (!materialDatabase->TryGetEffect(material->Name(), effectPasses)) {
        AssertNotImplemented();
        return false;
    }
    Assert(effectPasses);

    const FEffectDescriptor *effectDescriptors[FMultiPassEffectDescriptor::MaxPassCount];
    const size_t effectDescriptorsCount = effectPasses->FillEffectPasses(effectDescriptors);
    if (0 == effectDescriptorsCount) {
        AssertNotReached();
        return false;
    }

    FRenderCommandRegistration *prevcmd = nullptr;
    forrange(i, 0, effectDescriptorsCount) {
        FRenderCommandRegistration *const passcmd = AcquireRenderCommandForOnePass_(
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
void ReleaseRenderCommand(  URenderCommand& pcommand,
                            Graphics::IDeviceAPIEncapsulator *device ) {
    Assert(pcommand);
    Assert(device);

    const FRenderCommandRegistration *nextcmd = pcommand->Next;
    while (nextcmd) {
        const FRenderCommandRegistration *nextnextcmd = nextcmd->Next;
        ReleaseRenderCommandForOnePass_(nextcmd, device);
        checked_delete(nextcmd);
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
