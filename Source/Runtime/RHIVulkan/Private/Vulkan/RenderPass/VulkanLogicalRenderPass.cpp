
#include "stdafx.h"

#include "Vulkan/RenderPass/VulkanLogicalRenderPass.h"

#include "Vulkan/Command/VulkanCommandBuffer.h"

#include "RHI/RenderPassDesc.h"

#include "Diagnostic/Logger.h"
#include "Meta/Functor.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
#ifdef VK_NV_shading_rate_image
static const VkShadingRatePaletteEntryNV GShadingRateDefaultEntry{
    VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_PIXEL_NV
};
#endif
//----------------------------------------------------------------------------
static void SetVulkanClearValue_(VkClearValue* pout, const FRenderPassDesc::FClearValue& in) {
    STATIC_ASSERT(sizeof(float[4]) == sizeof(FLinearColor));
    Meta::Visit(in,
        [=](const FLinearColor& value) { FPlatformMemory::Memcpy(pout->color.float32, &value, sizeof(value)); },
        [=](const FRgba32u& value) { FPlatformMemory::Memcpy(pout->color.uint32, value.data, sizeof(value.data)); },
        [=](const FRgba32i& value) { FPlatformMemory::Memcpy(pout->color.int32, value.data, sizeof(value.data)); },
        [=](const FDepthValue& value) { pout->depthStencil = { value.Value, 0 }; },
        [=](const FStencilValue& value) { pout->depthStencil = { 0, value.Value }; },
        [=](const FDepthStencilValue& value) { pout->depthStencil = { value.Depth, value.Stencil }; }
    );
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
FVulkanLogicalRenderPass::~FVulkanLogicalRenderPass() {
    Assert_NoAssume(not _drawTasks.Available);
}
#endif
//----------------------------------------------------------------------------
bool FVulkanLogicalRenderPass::Construct(FVulkanCommandBuffer& cmd, const FRenderPassDesc& desc) {
    Assert_NoAssume(not _allocator.Available);
    Assert_NoAssume(not _drawTasks.Available);

    const bool enableShadingRateImage = desc.ShadingRate.ImageId.Valid();

    _allocator.Construct(TSlabAllocator{ cmd.Write()->MainAllocator });
    _allocator->SetSlabSize(4_KiB);

    _drawTasks.Construct(TSlabAllocator{ *_allocator });

    _area = desc.Area;
    _blendState = desc.Blend;
    _depthState = desc.Depth;
    _multisampleState = desc.Multisample;
    _rasterizationState = desc.Rasterization;
    _stencilState = desc.Stencil;

    Meta::TOptional<FMultiSamples> samples;

    // copy descriptor sets

    for (const auto& src : desc.PerPassResources) {
        const TMemoryView<const u32> offsets = src.second->DynamicOffsets();

        _perPassResources.Resources.Push(
            src.first,
            cmd.CreateDescriptorSet(*src.second),
            checked_cast<u32>(_perPassResources.DynamicOffsets.size()),
            checked_cast<u32>(offsets.size()) );

        _perPassResources.DynamicOffsets.Append(offsets);
    }

    // copy render targets

    forrange(i, 0, desc.RenderTargets.size()) {
        const FRenderTarget& src = desc.RenderTargets[i];
        if (not src.ImageId.Valid())
            continue;

        FColorTarget dst;
        dst.LocalImage = cmd.ToLocal(src.ImageId);
        LOG_CHECK(RHI, nullptr != dst.LocalImage);

        const auto sharedImg = dst.LocalImage->Read();

        if (src.Desc.has_value()) {
            dst.Desc = *src.Desc;
            dst.Desc.Validate(sharedImg->Desc);
        }
        else {
            dst.Desc = FImageViewDesc{ sharedImg->Desc };
        }

        dst.ImageId = src.ImageId;
        dst.Samples = VkCast(sharedImg->Desc.Samples);
        dst.LoadOp = VkCast(src.LoadOp);
        dst.StoreOp = VkCast(src.StoreOp);
        dst.State = EResourceState::Unknown;
        dst.Index = checked_cast<u32>(i);
        dst.ImageHash = hash_tuple(dst.ImageId, dst.Desc);

        SetVulkanClearValue_(&_clearValues[i], src.ClearValue);

        // validate image view description

        if (dst.Desc.Format == EPixelFormat::Unknown)
            dst.Desc.Format = sharedImg->Desc.Format;

        if (samples.has_value())
            LOG_CHECK(RHI, sharedImg->Desc.Samples == *samples);
        else
            samples = sharedImg->Desc.Samples;

        // add resource state flags

        if (desc.Area.Left() == 0 and desc.Area.Width() == sharedImg->Width() and
            desc.Area.Top() == 0 and desc.Area.Height() == sharedImg->Height() ) {

            if (src.LoadOp == EAttachmentLoadOp::Clear or
                src.LoadOp == EAttachmentLoadOp::Invalidate )
                dst.State |= EResourceState::InvalidateBefore;

            if (src.StoreOp == EAttachmentStoreOp::Invalidate)
                dst.State |= EResourceState::InvalidateAfter;
        }

        // add color or depth-stencil render target

        if (EPixelFormat_HasDepthOrStencil(dst.Desc.Format)) {
            Assert_NoAssume(static_cast<ERenderTargetID>(i) == ERenderTargetID::DepthStencil);
            Assert_NoAssume(sharedImg->Desc.Usage & EImageUsage::DepthStencilAttachment);

            dst.State |= EResourceState::DepthStencilAttachmentReadWrite; // #TODO: add support for other layouts

            _depthStencilTarget = FDepthStencilTarget{ dst };
        }
        else {
            Assert_NoAssume(sharedImg->Desc.Usage ^ (EImageUsage::ColorAttachment|EImageUsage::ColorAttachmentBlend));

            dst.State |= EResourceState::ColorAttachmentReadWrite; // #TODO: remove 'Read' state if blending disabled or 'loadOp' is 'Clear'

            _colorTargets.Push(dst);
        }
    }

    // validate image samples

    if (samples.has_value()) {
        if (_multisampleState.Samples != *samples) {
            _multisampleState.Samples = *samples;
            _multisampleState.SampleMask = uint4{ UMax };
        }
    }

    // create viewports and default scissors

    for (const FRenderViewport& src  : desc.Viewports) {
        Assert_NoAssume(
            src.Rect.Left() >= static_cast<float>(desc.Area.Left()) and
            src.Rect.Right() <= static_cast<float>(desc.Area.Right()) );
        Assert_NoAssume(
            src.Rect.Top() >= static_cast<float>(desc.Area.Top()) and
            src.Rect.Bottom() <= static_cast<float>(desc.Area.Bottom()) );

        VkViewport dst;
        dst.x = src.Rect.Left();
        dst.y = src.Rect.Top();
        dst.width = src.Rect.Width();
        dst.height = src.Rect.Height();
        dst.minDepth = src.MinDepth;
        dst.maxDepth = src.MaxDepth;
        _viewports.Push(dst);

        VkRect2D scissor;
        scissor.offset.x = FPlatformMaths::RoundToInt(src.Rect.Left());
        scissor.offset.y = FPlatformMaths::RoundToInt(src.Rect.Top());
        scissor.extent.width = FPlatformMaths::RoundToInt(src.Rect.Width());
        scissor.extent.height = FPlatformMaths::RoundToInt(src.Rect.Height());
        _scissors.Push(scissor);

#ifdef VK_NV_shading_rate_image
        if (enableShadingRateImage) {
            VkShadingRatePaletteNV palette{};
            palette.shadingRatePaletteEntryCount = Max(checked_cast<u32>(src.ShadingRate.size()), 1_u32);
            palette.pShadingRatePaletteEntries = &GShadingRateDefaultEntry;

            if (not src.ShadingRate.empty()) {
                const auto entries = _allocator->AllocateT<VkShadingRatePaletteEntryNV>(
                    checked_cast<u32>(src.ShadingRate.size()) );
                palette.pShadingRatePaletteEntries = entries.data();

                forrange(i, 0, src.ShadingRate.size())
                    entries[i] = VkCast(src.ShadingRate[i]);
            }

            _shadingRatePalette.Push(palette);
        }
#endif
    }

    // create default viewport

    if (desc.Viewports.empty()) {
        VkViewport dst;
        dst.x = static_cast<float>(desc.Area.Left());
        dst.y = static_cast<float>(desc.Area.Top());
        dst.width = static_cast<float>(desc.Area.Width());
        dst.height = static_cast<float>(desc.Area.Height());
        dst.minDepth = 0.0f;
        dst.maxDepth = 1.0f;
        _viewports.Push(dst);

        VkRect2D scissor;
        scissor.offset.x = checked_cast<int32_t>(desc.Area.Left());
        scissor.offset.y = checked_cast<int32_t>(desc.Area.Top());
        scissor.extent.width = desc.Area.Width();
        scissor.extent.height = desc.Area.Height();
        _scissors.Push(scissor);
    }

    // set shading rate image
#ifdef VK_NV_shading_rate_image
    if (enableShadingRateImage) {
        _shadingRateImage = cmd.ToLocal(desc.ShadingRate.ImageId);
        _shadingRateImageLayer = desc.ShadingRate.Layer;
        _shadingRateImageLevel = desc.ShadingRate.Mipmap;
    }
#endif

    return true;
}
//----------------------------------------------------------------------------
void FVulkanLogicalRenderPass::TearDown(const FVulkanResourceManager& ) {
    AssertMessage(L"render pass was not submitted", _isSubmitted);

    Assert_NoAssume(_allocator.Available);
    Assert_NoAssume(_drawTasks.Available);

    // skip destruction and forget about allocated memory: everything will be discarded with the next line
    _drawTasks.Discard();
    _allocator->DiscardAll();
    _allocator.Destroy();

    _shadingRateImage = nullptr;
}
//----------------------------------------------------------------------------
void FVulkanLogicalRenderPass::SetRenderPass(FRawRenderPassID renderPass, u32 subpass, FRawFramebufferID framebuffer, u32 depthIndex) {
    Assert(renderPass);
    Assert(framebuffer);

    _framebufferId = framebuffer;
    _renderPassId = renderPass;
    _subpassIndex = subpass;

    if (depthIndex < _clearValues.size()) {
        _clearValues[depthIndex] = _clearValues[_depthStencilTarget.Index];
        _depthStencilTarget.Index = depthIndex;
    }
}
//----------------------------------------------------------------------------
bool FVulkanLogicalRenderPass::ShadingRateImage(const FVulkanLocalImage** outImage, FImageViewDesc* outDesc) const {
#ifdef VK_NV_shading_rate_image
    if (not _shadingRateImage)
        return false;

    Assert(outImage);
    Assert(outDesc);

    *outImage = _shadingRateImage;

    outDesc->View = EImageView_2D;
    outDesc->Format = EPixelFormat::R8u;
    outDesc->BaseLevel = _shadingRateImageLevel;
    outDesc->BaseLayer = _shadingRateImageLayer;
    outDesc->AspectMask = EImageAspect::Color;

    return true;

#else
    PP_FOREACH_ARGS(UNUSED, pImage, pDesc);
    return false;

#endif
}
//----------------------------------------------------------------------------
bool FVulkanLogicalRenderPass::Submit(
    FVulkanCommandBuffer& cmd,
    const TMemoryView<const TPair<FRawImageID, EResourceState>>& images,
    const TMemoryView<const TPair<FRawBufferID, EResourceState>>& buffers ) {
    LOG_CHECK(RHI, not _isSubmitted);

    if (not images.empty()) {
        Assert_NoAssume(_mutableImages.empty());
        _mutableImages = _allocator->AllocateT<FMutableImages::value_type>(images.size());

        forrange(i, 0, images.size())
            Meta::Construct(&_mutableImages[i], cmd.ToLocal(images[i].first), images[i].second);
    }

    if (not buffers.empty()) {
        Assert_NoAssume(_mutableBuffers.empty());
        _mutableBuffers = _allocator->AllocateT<FMutableBuffers::value_type>(buffers.size());

        forrange(i, 0, buffers.size())
            Meta::Construct(&_mutableBuffers[i], cmd.ToLocal(buffers[i].first), buffers[i].second);
    }

    _isSubmitted = true;
    return true;
}
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
void FVulkanLogicalRenderPass::SetShaderDebugIndex(EShaderDebugIndex id) {
    for (IVulkanDrawTask* pTask : *_drawTasks)
        pTask->SetDebugModeIndex(id);
}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
