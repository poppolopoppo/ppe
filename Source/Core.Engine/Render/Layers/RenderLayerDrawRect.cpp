#include "stdafx.h"

#include "RenderLayerDrawRect.h"

#include "Core.Graphics/Device/DeviceAPIEncapsulator.h"
#include "Core.Graphics/Device/Geometry/PrimitiveType.h"
#include "Core.Graphics/Device/Geometry/VertexBuffer.h"
#include "Core.Graphics/Device/Geometry/VertexDeclaration.h"

#include "Core/Allocator/Alloca.h"
#include "Core/Allocator/PoolAllocator-impl.h"
#include "Core/IO/Format.h"
#include "Core/IO/String.h"

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
static String RenderLayerDrawRectName_(const MaterialEffect *material, const RectangleF& viewport) {
#ifdef WITH_CORE_ENGINE_RENDERLAYER_DEBUGNAME
    return StringFormat("DrawRect_#{0}__{1:#f2}_{2:#f2}_{3:#f2}_{4:#f2}",
                        material->Material()->Name(),
                        viewport.Left(), viewport.Top(),
                        viewport.Width(), viewport.Height() );
#else
    return String();
#endif
}
//----------------------------------------------------------------------------
static void CreateRectVertices_(
    Graphics::PVertexBuffer& vertices,
    Graphics::IDeviceAPIEncapsulator *device,
    const MaterialEffect *material,
    const RectangleF& viewport ) {
    Assert(!vertices);

    static const size_t vertexCount = 4; // triangle strip
    const float4 positions0[vertexCount] = {
        {viewport.Left(),   viewport.Top(),     1, 1 },
        {viewport.Left(),   viewport.Bottom(),  1, 1 },
        {viewport.Right(),  viewport.Top(),     1, 1 },
        {viewport.Right(),  viewport.Bottom(),  1, 1 },
    };
    const float4 texcoords0[vertexCount] = {
        { 0, 1, 0, 0 },
        { 0, 0, 0, 0 },
        { 1, 1, 0, 0 },
        { 1, 0, 0, 0 },
    };

    const Graphics::VertexDeclaration *vertexDeclaration = material->Effect()->VertexDeclaration();
    vertices = new Graphics::VertexBuffer(
        vertexDeclaration,
        vertexCount,
        Graphics::BufferMode::None,
        Graphics::BufferUsage::Default );
    vertices->Freeze();

    const auto vertexData = MALLOCA_VIEW(u8, vertexDeclaration->SizeInBytes() * vertexCount);
    memset(vertexData.Pointer(), 0, vertexData.SizeInBytes());

    GenericVertex vertex(vertexDeclaration);
    vertex.SetDestination(vertexData);

    const GenericVertex::SubPart position2f_0 = vertex.Position2f(0);
    const GenericVertex::SubPart position3f_0 = vertex.Position3f(0);
    const GenericVertex::SubPart position4f_0 = vertex.Position4f(0);

    const GenericVertex::SubPart texcoord2f_0 = vertex.TexCoord2f(0);
    const GenericVertex::SubPart texcoord3f_0 = vertex.TexCoord3f(0);
    const GenericVertex::SubPart texcoord4f_0 = vertex.TexCoord4f(0);

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
SINGLETON_POOL_ALLOCATED_DEF(RenderLayerDrawRect, );
//----------------------------------------------------------------------------
RenderLayerDrawRect::RenderLayerDrawRect(Engine::MaterialEffect *materialEffect)
:   RenderLayerDrawRect(materialEffect, RectangleF(-1.0f, -1.0f, 2.0f, 2.0f)) {}
//----------------------------------------------------------------------------
RenderLayerDrawRect::RenderLayerDrawRect(Engine::MaterialEffect *materialEffect, const RectangleF& viewport)
:   AbstractRenderLayer(RenderLayerDrawRectName_(materialEffect, viewport))
,   _materialEffect(materialEffect)
,   _viewport(viewport) {
    Assert(materialEffect);
    Assert(viewport.HasPositiveExtentsStrict());
    Assert(_effectVariability.Value == VariabilitySeed::Invalid);
}
//----------------------------------------------------------------------------
RenderLayerDrawRect::~RenderLayerDrawRect() {
    Assert(!_vertices);
}
//----------------------------------------------------------------------------
void RenderLayerDrawRect::PrepareImpl_(Graphics::IDeviceAPIEncapsulator *device, const RenderTree *renderTree, VariabilitySeed *seeds) {
    const VariabilitySeed variability = renderTree->EffectCompiler()->Variability();

    if (!_vertices) {
        CreateRectVertices_(_vertices, device, _materialEffect, _viewport);
        _materialEffect->Create(device, renderTree->Scene());

        _effectVariability = variability;
    }

    if (variability != _effectVariability &&
        _effectVariability.Value != VariabilitySeed::Invalid ) {
        LOG(Information, L"[RenderLayerDrawRect] Invalidate render rect in layer \"{0}\" ({1}>{2})",
            Name().c_str(), _effectVariability.Value, variability.Value );

        _materialEffect->Destroy(device);
        _materialEffect->Create(device, renderTree->Scene());

        _effectVariability = variability;
    }

    seeds[size_t(MaterialVariability::Material)].Next();
    seeds[size_t(MaterialVariability::Batch)].Next();

    _materialEffect->Prepare(device, renderTree->Scene(), seeds);
}
//----------------------------------------------------------------------------
void RenderLayerDrawRect::RenderImpl_(Graphics::IDeviceAPIContextEncapsulator *context) {
    Assert(_vertices);

    _materialEffect->Effect()->Set(context);
    _materialEffect->Set(context);

    context->SetVertexBuffer(_vertices);
    context->DrawPrimitives(Graphics::PrimitiveType::TriangleStrip, 0, 2);
}
//----------------------------------------------------------------------------
void RenderLayerDrawRect::DestroyImpl_(Graphics::IDeviceAPIEncapsulator *device, const RenderTree *renderTree) {
    Assert(_vertices);

    _vertices->Destroy(device);
    RemoveRef_AssertReachZero(_vertices);

    _materialEffect->Destroy(device);
    _effectVariability.Value = VariabilitySeed::Invalid;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
