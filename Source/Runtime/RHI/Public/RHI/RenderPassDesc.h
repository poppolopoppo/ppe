#pragma once

#include "RHI_fwd.h"

#include "RHI/ImageDesc.h"
#include "RHI/PipelineResources.h"
#include "RHI/RenderState.h"
#include "RHI/ResourceEnums.h"
#include "RHI/ResourceId.h"

#include "Color/Color.h"
#include "Container/Array.h"
#include "Container/Stack.h"
#include "Maths/ScalarRectangle.h"
#include "Meta/Optional.h"

#include <variant>

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FRenderTarget {
    using FClearValue = std::variant<
        FLinearColor, FRgba32u, FRgba32i,
        FDepthValue, FStencilValue, FDepthStencilValue >;

    FRawImageID ImageId; // may be image module in initial state (created by CreateRenderTarget or other)
    Meta::TOptional<FImageViewDesc> Desc; // may be used to specialize level, layer, different format, layout, ...
    FClearValue ClearValue{ FRgba32u(0) }; // default is black color
    EAttachmentLoadOp LoadOp{ EAttachmentLoadOp::Load };
    EAttachmentStoreOp StoreOp{ EAttachmentStoreOp::Store };
};
//----------------------------------------------------------------------------
struct FRenderViewport {
    using FShadingRatePalette = TFixedSizeStack<EShadingRatePalette, u32(EShadingRatePalette::_Count)>;

    FRectangleF Rect;
    float MinDepth{ 0.0f };
    float MaxDepth{ 1.0f };
    FShadingRatePalette ShadingRate;
};
PPE_ASSUME_TYPE_AS_POD(FRenderViewport)
//----------------------------------------------------------------------------
struct FRenderPassDesc {
    using FClearValue = FRenderTarget::FClearValue;

    struct FShadingRate {
        FRawImageID ImageId;
        FImageLayer Layer;
        FMipmapLevel Mipmap;
        // #TODO: coarse sample order
    };

    using FRenderTargets = TStaticArray<FRenderTarget, MaxColorBuffers + 1/* DepthStencil */>;
    using FViewports = TFixedSizeStack<FRenderViewport, MaxViewports>;

    FBlendState Blend;
    FDepthBufferState Depth;
    FStencilBufferState Stencil;
    FRasterizationState Rasterization;
    FMultisampleState Multisample;

    FShadingRate ShadingRate;
    FRenderTargets RenderTargets;
    FViewports Viewports;
    FRectangleU Area;

    FPipelineResourceSet PerPassResources;

    FRenderPassDesc() = default;
    explicit FRenderPassDesc(const FRectangleU& area) : Area(area) { Assert(Area.HasPositiveExtentsStrict()); }
    explicit FRenderPassDesc(const uint2& extent) : FRenderPassDesc(FRectangleU{ extent }) {}

    FRenderPassDesc& AddTarget(ERenderTargetID id, FRawImageID image);
    FRenderPassDesc& AddTarget(ERenderTargetID id, FRawImageID image, EAttachmentLoadOp loadOp, EAttachmentStoreOp storeOp);
    FRenderPassDesc& AddTarget(ERenderTargetID id, FRawImageID image, FClearValue&& clearValue, EAttachmentStoreOp storeOp);
    FRenderPassDesc& AddTarget(ERenderTargetID id, FRawImageID image, const FImageViewDesc& desc, EAttachmentLoadOp loadOp, EAttachmentStoreOp storeOp);
    FRenderPassDesc& AddTarget(ERenderTargetID id, FRawImageID image, const FImageViewDesc& desc, FClearValue&& clearValue, EAttachmentStoreOp storeOp);

    FRenderPassDesc& AddViewport(const uint2& extent, float minDepth = 0.0f, float maxDepth = 1.0f, TMemoryView<const EShadingRatePalette> shadingRate = Default);
    FRenderPassDesc& AddViewport(const float2& extent, float minDepth = 0.0f, float maxDepth = 1.0f, TMemoryView<const EShadingRatePalette> shadingRate = Default);
    FRenderPassDesc& AddViewport(const FRectangleF& rect, float minDepth = 0.0f, float maxDepth = 1.0f, TMemoryView<const EShadingRatePalette> shadingRate = Default);

    FRenderPassDesc& AddColorBuffer(ERenderTargetID id, EColorMask colorMask = EColorMask::RGBA);
    FRenderPassDesc& AddColorBuffer(ERenderTargetID id, EBlendFactor srcBlendFactor, EBlendFactor dstBlendFactor, EBlendOp blendOp, EColorMask colorMask = EColorMask::RGBA);
    FRenderPassDesc& AddColorBuffer(ERenderTargetID id,
        EBlendFactor srcBlendFactorColor, EBlendFactor srcBlendFactorAlpha,
        EBlendFactor dstBlendFactorColor, EBlendFactor dstBlendFactorAlpha,
        EBlendOp blendOpColor, EBlendOp blendOpAlpha,
        EColorMask colorMask = EColorMask::RGBA );

    FRenderPassDesc& AddResources(const FDescriptorSetID& id, const PCPipelineResources& res);

    // Blend
    FRenderPassDesc& SetLogicOp(ELogicOp logicOp) { Blend.LogicOp = logicOp; return (*this); }
    FRenderPassDesc& SetBlendColor(const FRgba32f& color) { Blend.BlendColor = color; return (*this); }

    // Depth
    FRenderPassDesc& SetDepthTestEnabled(bool value) { Depth.EnableDepthTest = value; return (*this); }
    FRenderPassDesc& SetDepthWriteEnabled(bool value) { Depth.EnableDepthWrite = value; return (*this); }
    FRenderPassDesc& SetDepthCompareOp(ECompareOp value) { Depth.CompareOp = value; return (*this); }
    FRenderPassDesc& SetDepthBounds(float min, float max) { Depth.Bounds = float2(min, max); return (*this); }
    FRenderPassDesc& SetDepthBoundsEnabled(bool value) { Depth.EnableBounds = value; return (*this); }

    // Stencil
    FRenderPassDesc& SetStencilTestEnabled(bool value) { Stencil.EnabledStencilTests = value; return (*this); }

    FRenderPassDesc& SetStencilFrontFaceFailOp(EStencilOp value) { Stencil.Front.FailOp = value; return (*this); }
    FRenderPassDesc& SetStencilFrontFaceDepthFailOp(EStencilOp value) { Stencil.Front.DepthFailOp = value; return (*this); }
    FRenderPassDesc& SetStencilFrontFacePassOp(EStencilOp value) { Stencil.Front.PassOp = value; return (*this); }
    FRenderPassDesc& SetStencilFrontFaceCompareOp(ECompareOp value) { Stencil.Front.CompareOp = value; return (*this); }
    FRenderPassDesc& SetStencilFrontFaceReference(u8 value) { Stencil.Front.Reference = FStencilValue(value); return (*this); }
    FRenderPassDesc& SetStencilFrontFaceWriteMask(u8 value) { Stencil.Front.WriteMask = FStencilValue(value); return (*this); }
    FRenderPassDesc& SetStencilFrontFaceCompareMask(u8 value) { Stencil.Front.CompareMask = FStencilValue(value); return (*this); }

    FRenderPassDesc& SetStencilBackFaceFailOp(EStencilOp value) { Stencil.Back.FailOp = value; return (*this); }
    FRenderPassDesc& SetStencilBackFaceDepthFailOp(EStencilOp value) { Stencil.Back.DepthFailOp = value; return (*this); }
    FRenderPassDesc& SetStencilBackFacePassOp(EStencilOp value) { Stencil.Back.PassOp = value; return (*this); }
    FRenderPassDesc& SetStencilBackFaceCompareOp(ECompareOp value) { Stencil.Back.CompareOp = value; return (*this); }
    FRenderPassDesc& SetStencilBackFaceReference(u8 value) { Stencil.Back.Reference = FStencilValue(value); return (*this); }
    FRenderPassDesc& SetStencilBackFaceWriteMask(u8 value) { Stencil.Back.WriteMask = FStencilValue(value); return (*this); }
    FRenderPassDesc& SetStencilBackFaceCompareMask(u8 value) { Stencil.Back.CompareMask = FStencilValue(value); return (*this); }

    // Rasterization
    FRenderPassDesc& SetPolygonMode(EPolygonMode value) { Rasterization.PolygonMode = value; return (*this); }
    FRenderPassDesc& SetLineWidth(float value) { Rasterization.LineWidth = value; return (*this); }
    FRenderPassDesc& SetDepthBiasConstFactor(float value) { Rasterization.DepthBiasConstantFactor = value; return (*this); }
    FRenderPassDesc& SetDepthBiasClamp(float value) { Rasterization.DepthBiasClamp = value; return (*this); }
    FRenderPassDesc& SetDepthBiasSlopeFactor(float value) { Rasterization.DepthBiasSlopeFactor = value; return (*this); }
    FRenderPassDesc& SetDepthBias(float constFactor, float clamp, float slopeFactor) { Rasterization.SetDepthBias(constFactor, clamp, slopeFactor); return (*this); }
    FRenderPassDesc& SetDepthBiasEnabled(bool value) { Rasterization.EnableDepthBias = value; return (*this); }
    FRenderPassDesc& SetDepthClampEnabled(bool value) { Rasterization.EnableDepthClamp = value; return (*this); }
    FRenderPassDesc& SetRasterizerDiscard(bool value) { Rasterization.EnableDiscard = value; return (*this); }
    FRenderPassDesc& SetCullMode(ECullMode value) { Rasterization.CullMode = value; return (*this); }
    FRenderPassDesc& SetFrontFaceCCW(bool value) { Rasterization.EnableFrontFaceCCW = value; return (*this); }

    // Multisample
    FRenderPassDesc& SetSampleMask(TMemoryView<const u32> value);
    FRenderPassDesc& SetMultiSamples(FMultiSamples value) { Multisample.Samples = value; return (*this); }
    FRenderPassDesc& SetMultiSamples(FMultiSamples value, TMemoryView<const u32> mask);
    FRenderPassDesc& SetMinSampleShading(float value) { Multisample.MinSampleShading = value; return (*this); }
    FRenderPassDesc& SetSampleShadingEnabled(bool value) { Multisample.EnableSampleShading = value; return (*this); }
    FRenderPassDesc& SetAlphaToCoverageEnabled(bool value) { Multisample.EnableAlphaToCoverage = value; return (*this); }
    FRenderPassDesc& SetAlphaToOneEnabled(bool value) { Multisample.EnableAlphaToOne = value; return (*this); }

    FRenderPassDesc& SetShadingRateImage(FRawImageID image, FImageLayer layer = Default, FMipmapLevel level = Default);
};
//----------------------------------------------------------------------------
inline FRenderPassDesc& FRenderPassDesc::AddTarget(ERenderTargetID id, FRawImageID image) {
    return AddTarget(id, image, EAttachmentLoadOp::Load, EAttachmentStoreOp::Store);
}
//----------------------------------------------------------------------------
inline FRenderPassDesc& FRenderPassDesc::AddTarget(ERenderTargetID id, FRawImageID image, EAttachmentLoadOp loadOp, EAttachmentStoreOp storeOp) {
    Assert(image);
    Assert(loadOp != EAttachmentLoadOp::Clear);
    FRenderTarget rt;
    rt.ImageId = image;
    rt.LoadOp = loadOp;
    rt.StoreOp = storeOp;
    RenderTargets[u32(id)] = std::move(rt);
    return (*this);
}
//----------------------------------------------------------------------------
inline FRenderPassDesc& FRenderPassDesc::AddTarget(ERenderTargetID id, FRawImageID image, FClearValue&& clearValue, EAttachmentStoreOp storeOp) {
    Assert(image);
    FRenderTarget rt;
    rt.ImageId = image;
    rt.ClearValue = std::move(clearValue);
    rt.LoadOp = EAttachmentLoadOp::Clear;
    rt.StoreOp = storeOp;
    RenderTargets[u32(id)] = std::move(rt);
    return (*this);
}
//----------------------------------------------------------------------------
inline FRenderPassDesc& FRenderPassDesc::AddTarget(ERenderTargetID id, FRawImageID image, const FImageViewDesc& desc, EAttachmentLoadOp loadOp, EAttachmentStoreOp storeOp) {
    Assert(image);
    Assert(loadOp != EAttachmentLoadOp::Clear);
    FRenderTarget rt;
    rt.ImageId = image;
    rt.Desc = desc;
    rt.LoadOp = loadOp;
    rt.StoreOp = storeOp;
    RenderTargets[u32(id)] = std::move(rt);
    return (*this);
}
//----------------------------------------------------------------------------
inline FRenderPassDesc& FRenderPassDesc::AddTarget(ERenderTargetID id, FRawImageID image, const FImageViewDesc& desc, FClearValue&& clearValue, EAttachmentStoreOp storeOp) {
    Assert(image);
    FRenderTarget rt;
    rt.ImageId = image;
    rt.Desc = desc;
    rt.ClearValue = std::move(clearValue);
    rt.LoadOp = EAttachmentLoadOp::Clear;
    rt.StoreOp = storeOp;
    RenderTargets[u32(id)] = std::move(rt);
    return (*this);
}
//----------------------------------------------------------------------------
inline FRenderPassDesc& FRenderPassDesc::AddViewport(const uint2& extent, float minDepth, float maxDepth, TMemoryView<const EShadingRatePalette> shadingRate) {
    return AddViewport(FRectangleF(float2::Zero, float2(extent)), minDepth, maxDepth, shadingRate);
}
//----------------------------------------------------------------------------
inline FRenderPassDesc& FRenderPassDesc::AddViewport(const float2& extent, float minDepth, float maxDepth, TMemoryView<const EShadingRatePalette> shadingRate) {
    return AddViewport(FRectangleF(float2::Zero, extent), minDepth, maxDepth, shadingRate);
}
//----------------------------------------------------------------------------
inline FRenderPassDesc& FRenderPassDesc::AddViewport(const FRectangleF& rect, float minDepth, float maxDepth, TMemoryView<const EShadingRatePalette> shadingRate) {
    Assert(rect.HasPositiveExtentsStrict());
    Viewports.Push(rect, minDepth, maxDepth, shadingRate);
    return (*this);
}
//----------------------------------------------------------------------------
inline FRenderPassDesc& FRenderPassDesc::AddColorBuffer(ERenderTargetID id, EColorMask colorMask) {
    FColorBufferState cb;
    cb.ColorMask = colorMask;
    Blend.Buffers[u32(id)] = std::move(cb);
    return (*this);
}
//----------------------------------------------------------------------------
inline FRenderPassDesc& FRenderPassDesc::AddColorBuffer(ERenderTargetID id, EBlendFactor srcBlendFactor, EBlendFactor dstBlendFactor, EBlendOp blendOp, EColorMask colorMask) {
    return AddColorBuffer(id, srcBlendFactor, srcBlendFactor, dstBlendFactor, dstBlendFactor, blendOp, blendOp, colorMask);
}
//----------------------------------------------------------------------------
inline FRenderPassDesc& FRenderPassDesc::AddColorBuffer(ERenderTargetID id,
    EBlendFactor srcBlendFactorColor, EBlendFactor srcBlendFactorAlpha,
    EBlendFactor dstBlendFactorColor, EBlendFactor dstBlendFactorAlpha,
    EBlendOp blendOpColor, EBlendOp blendOpAlpha,
    EColorMask colorMask ) {
    FColorBufferState cb;
    cb.SrcBlendFactor = { srcBlendFactorColor, srcBlendFactorAlpha };
    cb.DstBlendFactor = { dstBlendFactorColor, dstBlendFactorAlpha };
    cb.BlendOp = { blendOpColor, blendOpAlpha };
    cb.ColorMask = colorMask;
    Blend.Buffers[u32(id)] = std::move(cb);
    return (*this);
}
//----------------------------------------------------------------------------
inline FRenderPassDesc& FRenderPassDesc::AddResources(const FDescriptorSetID& id, const PCPipelineResources& res) {
    Assert(id);
    Assert(res);
    PerPassResources.Add(id) = res;
    return (*this);
}
//----------------------------------------------------------------------------
inline FRenderPassDesc& FRenderPassDesc::SetSampleMask(TMemoryView<const u32> value) {
    Assert(value.size() <= lengthof(Multisample.SampleMask.data));
    forrange(i, 0, lengthof(Multisample.SampleMask.data)) {
        Multisample.SampleMask[i] = (i < value.size() ? value[i] : 0u);
    }
    return (*this);
}
//----------------------------------------------------------------------------
inline FRenderPassDesc& FRenderPassDesc::SetShadingRateImage(FRawImageID image, FImageLayer layer, FMipmapLevel level) {
    Assert(image);
    ShadingRate.ImageId = image;
    ShadingRate.Layer = layer;
    ShadingRate.Mipmap = level;
    return (*this);
}
//----------------------------------------------------------------------------
inline FRenderPassDesc& FRenderPassDesc::SetMultiSamples(FMultiSamples value, TMemoryView<const u32> mask) {
    Assert(mask.size() == ((*value + 31) / 32));
    return SetMultiSamples(value).SetSampleMask(mask);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
