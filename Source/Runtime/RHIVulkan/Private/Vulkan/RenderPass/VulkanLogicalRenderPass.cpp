
#include "stdafx.h"

#include "Vulkan/RenderPass/VulkanLogicalRenderPass.h"

#include "Meta/Functor.h"
#include "RHI/RenderPassDesc.h"
#include "Vulkan/Command/VulkanCommandBuffer.h"

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
    Meta::Visit(in,
        [=](const FRgba32f& value) { FPlatformMemory::Memcpy(pout->color.float32, value.data, sizeof(value.data)); },
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
    Assert_NoAssume(_drawTasks.empty());
}
#endif
//----------------------------------------------------------------------------
bool FVulkanLogicalRenderPass::Construct(FVulkanCommandBuffer& cmd, const FRenderPassDesc& desc) {
    Assert_NoAssume(not _allocator.Heap.valid());
    Assert_NoAssume(_drawTasks.empty());

    const bool enableShadingRateImage = desc.ShadingRate.ImageId.Valid();

    _allocator = FSlabAllocator{ cmd.Read()->MainAllocator };
    //_localHeap.Heap->SetSlabSize(4_KiB); // #TODO: move from heap to allocator
    _drawTasks = VECTOR_SLAB(IVulkanDrawTask*){ _allocator };

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
            LOG_CHECK(RHI, sharedImg->Desc.Samples == *samples)
        else
            samples = sharedImg->Desc.Samples;

        // add resource state flags

        if (desc.Area.Left() == 0 and desc.Area.Width() == checked_cast<i32>(sharedImg->Width()) and
            desc.Area.Top() == 0 and desc.Area.Height() == checked_cast<i32>(sharedImg->Height()) ) {

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
            palette.shadingRatePaletteEntryCount = Max(checked_cast<u32>(src.ShadingRate.size()), 1u);
            palette.pShadingRatePaletteEntries = &GShadingRateDefaultEntry;

            if (not src.ShadingRate.empty()) {
                const auto entries = _allocator.Heap->AllocateT<VkShadingRatePaletteEntryNV>(
                    checked_cast<u32>(src.ShadingRate.size()) );

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
        scissor.offset.x = desc.Area.Left();
        scissor.offset.y = desc.Area.Top();
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
void FVulkanLogicalRenderPass::TearDown(const FVulkanResourceManager& resources) {
    AssertMessage(L"render pass was not submitted", _isSubmitted);

    _drawTasks.clear();

}
//----------------------------------------------------------------------------
void FVulkanLogicalRenderPass::SetRenderPass(FRawRenderPassID renderPass, u32 subpass, FRawFramebufferID framebuffer, u32 depthIndex) {

}
//----------------------------------------------------------------------------
bool FVulkanLogicalRenderPass::ShadingRateImage(const FVulkanLocalImage** pimage, FImageViewDesc* pdesc) const {

}
//----------------------------------------------------------------------------
bool FVulkanLogicalRenderPass::Submit(FVulkanCommandBuffer& cmd, TMemoryView<const TPair<FRawImageID, EResourceState>> images, TMemoryView<const TPair<FRawBufferID, EResourceState>> buffers) {

}
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
void FVulkanLogicalRenderPass::SetShaderDebugIndex(EShaderDebugIndex id) {

}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
