#include "stdafx.h"

#include "RenderLayerDrawRect.h"

#include "Core.Graphics/Device/DeviceAPI.h"
#include "Core.Graphics/Device/Geometry/PrimitiveType.h"
#include "Core.Graphics/Device/Geometry/VertexBuffer.h"
#include "Core.Graphics/Device/Geometry/VertexDeclaration.h"

#include "Core/Allocator/Alloca.h"
#include "Core/Allocator/PoolAllocator-impl.h"
#include "Core/IO/Format.h"
#include "Core/IO/String.h"

#include "Camera/CameraModel.h"
#include "Effect/Effect.h"
#include "Effect/EffectCompiler.h"
#include "Effect/MaterialEffect.h"
#include "Material/Material.h"
#include "Material/MaterialVariability.h"
#include "Mesh/Geometry/GenericVertex.h"
#include "Mesh/Geometry/GenericVertexExport.h"
#include "Mesh/Geometry/GeometricPrimitives.h"
#include "Render/RenderTree.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static FString RenderLayerDrawRectName_(const FMaterialEffect *material, const RectangleF& viewport) {
#ifdef WITH_CORE_ENGINE_RENDERLAYER_DEBUGNAME
    return StringFormat("DrawRect_#{0}__{1:#f2}_{2:#f2}_{3:#f2}_{4:#f2}",
                        material->Material()->Name(),
                        viewport.Left(), viewport.Top(),
                        viewport.Width(), viewport.Height() );
#else
    return FString();
#endif
}
//----------------------------------------------------------------------------
static void CreateRectVertices_(
    Graphics::PVertexBuffer& vertices,
    Graphics::IDeviceAPIEncapsulator *device,
    const FMaterialEffect *material,
    const RectangleF& viewport ) {
    Assert(!vertices);

    static const size_t vertexCount = 4; // triangle strip
    const float4 positions0[vertexCount] = {
        {viewport.Left(),   viewport.Top(),     float(ECameraRay::LeftBottom),   1 },
        {viewport.Left(),   viewport.Bottom(),  float(ECameraRay::LeftTop),      1 },
        {viewport.Right(),  viewport.Top(),     float(ECameraRay::RightBottom),  1 },
        {viewport.Right(),  viewport.Bottom(),  float(ECameraRay::RightTop),     1 },
    };
    const float4 texcoords0[vertexCount] = {
        { 0, 1, 0, 0 },
        { 0, 0, 0, 0 },
        { 1, 1, 0, 0 },
        { 1, 0, 0, 0 },
    };

    const Graphics::FVertexDeclaration *vertexDeclaration = material->Effect()->VertexDeclaration().get();
    vertices = new Graphics::FVertexBuffer(
        vertexDeclaration,
        vertexCount,
        Graphics::EBufferMode::None,
        Graphics::EBufferUsage::Default );
    vertices->Freeze();

    const auto vertexData = MALLOCA_VIEW(u8, vertexDeclaration->SizeInBytes() * vertexCount);
    memset(vertexData.Pointer(), 0, vertexData.SizeInBytes());

    FGenericVertex vertex(vertexDeclaration);
    vertex.SetDestination(vertexData);

    const FGenericVertex::FSubPart position2f_0 = vertex.Position2f(0);
    const FGenericVertex::FSubPart position3f_0 = vertex.Position3f(0);
    const FGenericVertex::FSubPart position4f_0 = vertex.Position4f(0);

    const FGenericVertex::FSubPart texcoord2f_0 = vertex.TexCoord2f(0);
    const FGenericVertex::FSubPart texcoord3f_0 = vertex.TexCoord3f(0);
    const FGenericVertex::FSubPart texcoord4f_0 = vertex.TexCoord4f(0);

    AssertRelease(  position2f_0 || position3f_0 || position4f_0 ||
                    texcoord2f_0 || texcoord3f_0 || texcoord4f_0 );

    for (size_t i = 0; i < vertexCount; ++i) {
        const float4& pos = positions0[i];
        const float4& uv  = texcoords0[i];

        if (position4f_0)
            position4f_0.WriteValue(vertex, pos);
        else if (position3f_0)
            position3f_0.WriteValue(vertex, pos.xyz());
        else if (position2f_0)
            position2f_0.WriteValue(vertex, pos.xy());

        if (texcoord4f_0)
            texcoord4f_0.WriteValue(vertex, uv);
        else if (texcoord3f_0)
            texcoord3f_0.WriteValue(vertex, uv.xyz());
        else if (texcoord2f_0)
            texcoord2f_0.WriteValue(vertex, uv.xy());

        vertex.NextVertex();
    }

    vertices->Create(device, vertexData.Cast<const u8>());
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_TAGGED_DEF(Engine, FRenderLayerDrawRect, );
//----------------------------------------------------------------------------
FRenderLayerDrawRect::FRenderLayerDrawRect(Engine::FMaterialEffect *materialEffect)
:   FRenderLayerDrawRect(materialEffect, RectangleF(-1.0f, -1.0f, 2.0f, 2.0f)) {}
//----------------------------------------------------------------------------
FRenderLayerDrawRect::FRenderLayerDrawRect(Engine::FMaterialEffect *materialEffect, const RectangleF& viewport)
:   FAbstractRenderLayer(RenderLayerDrawRectName_(materialEffect, viewport))
,   _materialEffect(materialEffect)
,   _viewport(viewport) {
    Assert(materialEffect);
    Assert(viewport.HasPositiveExtentsStrict());
    Assert(_effectVariability.Value == FVariabilitySeed::Invalid);
}
//----------------------------------------------------------------------------
FRenderLayerDrawRect::~FRenderLayerDrawRect() {
    Assert(!_vertices);
}
//----------------------------------------------------------------------------
void FRenderLayerDrawRect::PrepareImpl_(Graphics::IDeviceAPIEncapsulator *device, FMaterialDatabase *materialDatabase, const FRenderTree *renderTree, FVariabilitySeed *seeds) {
    const FVariabilitySeed variability = renderTree->EffectCompiler()->Variability();

    if (!_vertices) {
        CreateRectVertices_(_vertices, device, _materialEffect.get(), _viewport);
        _materialEffect->Create(device, materialDatabase, renderTree->Scene());

        _effectVariability = variability;
    }

    if (variability != _effectVariability &&
        _effectVariability.Value != FVariabilitySeed::Invalid ) {
        LOG(Info, L"[FRenderLayerDrawRect] Invalidate render rect in layer \"{0}\" ({1}>{2})",
            FName().c_str(), _effectVariability.Value, variability.Value );

        _materialEffect->Destroy(device);
        _materialEffect->Create(device, materialDatabase, renderTree->Scene());

        _effectVariability = variability;
    }

    seeds[size_t(EMaterialVariability::FMaterial)].Next();
    seeds[size_t(EMaterialVariability::Batch)].Next();

    _materialEffect->Prepare(device, renderTree->Scene(), seeds);
}
//----------------------------------------------------------------------------
void FRenderLayerDrawRect::RenderImpl_(Graphics::IDeviceAPIContext *context) {
    Assert(_vertices);

    _materialEffect->Effect()->Set(context);
    _materialEffect->Set(context);

    context->SetVertexBuffer(_vertices.get());
    context->DrawPrimitives(Graphics::EPrimitiveType::TriangleStrip, 0, 2);
}
//----------------------------------------------------------------------------
void FRenderLayerDrawRect::DestroyImpl_(Graphics::IDeviceAPIEncapsulator *device, const FRenderTree * /* renderTree */) {
    Assert(_vertices);

    _vertices->Destroy(device);
    RemoveRef_AssertReachZero(_vertices);

    _materialEffect->Destroy(device);
    _effectVariability.Value = FVariabilitySeed::Invalid;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
