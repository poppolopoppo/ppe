
#include "stdafx.h"

#include "Vulkan/Command/VulkanFrameTask.h"

#include "Vulkan/Command/VulkanCommandBuffer.h"
#include "Vulkan/Command/VulkanRaytracingTask.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static VkClearColorValue VKClearColorValue_(const FClearColorImage::FClearColor& clear) {
    VkClearColorValue dst{};

    Meta::Visit(clear,
        [&dst](const FLinearColor& src) {
            STATIC_ASSERT(sizeof(src) == sizeof(dst.float32));
            FPlatformMemory::Memcpy(dst.float32, &src, sizeof(src));
        },
        [&dst](const FRgba32u& src) {
            STATIC_ASSERT(sizeof(src) == sizeof(dst.uint32));
            FPlatformMemory::Memcpy(dst.uint32, &src, sizeof(src));
        },
        [&dst](const FRgba32i& src) {
            STATIC_ASSERT(sizeof(src) == sizeof(dst.int32));
            FPlatformMemory::Memcpy(dst.int32, &src, sizeof(src));
        });

    return dst;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
IVulkanFrameTask::IVulkanFrameTask(const details::TFrameTaskDesc<T>& desc, FProcessFunc process) NOEXCEPT
:   _process(process)
#if USE_PPE_RHIDEBUG
,   _taskName(desc.TaskName)
,   _debugColor(desc.DebugColor)
#endif
{
    Assert_NoAssume(_process);

    _inputs.Append(desc.Dependencies.MakeView().Map([](auto dep) NOEXCEPT {
        Assert(dep);
        return checked_cast<IVulkanFrameTask*>(dep.get());
    }));
}
//----------------------------------------------------------------------------
void IVulkanFrameTask::CopyDescriptorSets(
    FVulkanPipelineResourceSet* outResources,
    FVulkanLogicalRenderPass* pRenderPass,
    FVulkanCommandBuffer& cmd,
    const FPipelineResourceSet& input ) {
    Assert(outResources);

    for (const TPair<FDescriptorSetID, PCPipelineResources>& src : input) {
        const TMemoryView<const u32> dynamicOffsets = src.second->DynamicOffsets();

        outResources->Resources.Push(
            src.first,
            cmd.CreateDescriptorSet(*src.second),
            checked_cast<u32>(outResources->DynamicOffsets.size()),
            checked_cast<u32>(dynamicOffsets.size()) );

        outResources->DynamicOffsets.Append(dynamicOffsets);
    }

    if (not pRenderPass)
        return;

    const u32 baseOffset = checked_cast<u32>(outResources->DynamicOffsets.size());

    for(const FVulkanPipelineResourceSet::FResource& src : pRenderPass->PerPassResources().Resources) {
        Assert_NoAssume(not input.GetIFP(src.DescriptorSetId));

        outResources->Resources.Push(
            src.DescriptorSetId,
            src.PipelineResources,
            src.DynamicOffsetIndex + baseOffset,
            src.DynamicOffsetCount );
    }

    outResources->DynamicOffsets.Append(pRenderPass->PerPassResources().DynamicOffsets.MakeView());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Submit
//----------------------------------------------------------------------------
TVulkanFrameTask<FSubmitRenderPass>::TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FSubmitRenderPass& desc, FProcessFunc process)
:   IVulkanFrameTask(desc, process)
,   _logicalPass(cmd.ToLocal(desc.RenderPassId)) {

    if (_logicalPass)
        VerifyRelease( _logicalPass->Submit(cmd, desc.Images.MakeView(), desc.Buffers.MakeView()) );
}
//----------------------------------------------------------------------------
// Compute
//----------------------------------------------------------------------------
TVulkanFrameTask<FDispatchCompute>::TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FDispatchCompute& desc, FProcessFunc process)
:   IVulkanFrameTask(desc, process)
,   Pipeline(cmd.AcquireTransient(desc.Pipeline))
,   PushConstants(desc.PushConstants)
,   Commands(desc.Commands)
,   LocalGroupSize(desc.LocalGroupSize) {

    CopyDescriptorSets(&_resources, nullptr, cmd, desc.Resources);

#if USE_PPE_RHIDEBUG
    if (desc.DebugMode.Mode != Default)
        DebugModeIndex = cmd.Batch()->AppendShaderForDebug(desc.TaskName, desc.DebugMode);
#endif
}
//----------------------------------------------------------------------------
TVulkanFrameTask<FDispatchComputeIndirect>::TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FDispatchComputeIndirect& desc, FProcessFunc process)
:   IVulkanFrameTask(desc, process)
,   Pipeline(cmd.AcquireTransient(desc.Pipeline))
,   PushConstants(desc.PushConstants)
,   Commands(desc.Commands)
,   LocalGroupSize(desc.LocalGroupSize)
,   IndirectBuffer(cmd.ToLocal(desc.IndirectBuffer)) {
    Assert_NoAssume(IndirectBuffer);
    Assert_NoAssume(IndirectBuffer->Read()->Desc.Usage & EBufferUsage::Indirect);

    CopyDescriptorSets(&_resources, nullptr, cmd, desc.Resources);

#if USE_PPE_RHIDEBUG
    if (desc.DebugMode.Mode != Default)
        DebugModeIndex = cmd.Batch()->AppendShaderForDebug(desc.TaskName, desc.DebugMode);
#endif
}
//----------------------------------------------------------------------------
// Copy
//----------------------------------------------------------------------------
TVulkanFrameTask<FCopyBuffer>::TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FCopyBuffer& desc, FProcessFunc process)
:   IVulkanFrameTask(desc, process)
,   SrcBuffer(cmd.ToLocal(desc.SrcBuffer))
,   DstBuffer(cmd.ToLocal(desc.DstBuffer))
,   Regions(desc.Regions) {
    Assert_NoAssume(SrcBuffer and SrcBuffer->Read()->Desc.Usage & EBufferUsage::TransferSrc);
    Assert_NoAssume(DstBuffer and DstBuffer->Read()->Desc.Usage & EBufferUsage::TransferDst);
}
//----------------------------------------------------------------------------
TVulkanFrameTask<FCopyImage>::TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FCopyImage& desc, FProcessFunc process)
:   IVulkanFrameTask(desc, process)
,   SrcImage(cmd.ToLocal(desc.SrcImage))
,   SrcLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
,   DstImage(cmd.ToLocal(desc.DstImage))
,   DstLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
,   Regions(desc.Regions) {
    Assert_NoAssume(SrcImage and SrcImage->Read()->Desc.Usage & EImageUsage::TransferSrc);
    Assert_NoAssume(DstImage and DstImage->Read()->Desc.Usage & EImageUsage::TransferDst);
}
//----------------------------------------------------------------------------
TVulkanFrameTask<FCopyBufferToImage>::TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FCopyBufferToImage& desc, FProcessFunc process)
:   IVulkanFrameTask(desc, process)
,   SrcBuffer(cmd.ToLocal(desc.SrcBuffer))
,   DstImage(cmd.ToLocal(desc.DstImage))
,   DstLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
,   Regions(desc.Regions) {
    Assert_NoAssume(SrcBuffer and SrcBuffer->Read()->Desc.Usage & EBufferUsage::TransferSrc);
    Assert_NoAssume(DstImage and DstImage->Read()->Desc.Usage & EImageUsage::TransferDst);
}
//----------------------------------------------------------------------------
TVulkanFrameTask<FCopyImageToBuffer>::TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FCopyImageToBuffer& desc, FProcessFunc process)
:   IVulkanFrameTask(desc, process)
,   SrcImage(cmd.ToLocal(desc.SrcImage))
,   SrcLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
,   DstBuffer(cmd.ToLocal(desc.DstBuffer))
,   Regions(desc.Regions) {
    Assert_NoAssume(SrcImage and SrcImage->Read()->Desc.Usage & EImageUsage::TransferSrc);
    Assert_NoAssume(DstBuffer and DstBuffer->Read()->Desc.Usage & EBufferUsage::TransferDst);
}
//----------------------------------------------------------------------------
// Blit
//----------------------------------------------------------------------------
TVulkanFrameTask<FBlitImage>::TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FBlitImage& desc, FProcessFunc process)
:   IVulkanFrameTask(desc, process)
,   SrcImage(cmd.ToLocal(desc.SrcImage))
,   SrcLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
,   DstImage(cmd.ToLocal(desc.DstImage))
,   DstLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
,   Filter(VkCast(desc.Filter))
,   Regions(desc.Regions) {
    Assert_NoAssume(SrcImage and SrcImage->Read()->Desc.Usage & EImageUsage::TransferSrc);
    Assert_NoAssume(DstImage and DstImage->Read()->Desc.Usage & EImageUsage::TransferDst);
}
//----------------------------------------------------------------------------
// GenerateMipMaps
//----------------------------------------------------------------------------
TVulkanFrameTask<FGenerateMipmaps>::TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FGenerateMipmaps& desc, FProcessFunc process)
:   IVulkanFrameTask(desc, process)
,   Image(cmd.ToLocal(desc.Image))
,   BaseMipLevel(*desc.BaseLevel)
,   LevelCount(desc.LevelCount)
,   BaseLayer(*desc.BaseLayer)
,   LayerCount(desc.LayerCount) {
    Assert_NoAssume(Image and Image->Read()->Desc.Usage & (EImageUsage::TransferSrc + EImageUsage::TransferDst));
}
//----------------------------------------------------------------------------
// ResolveImage
//----------------------------------------------------------------------------
TVulkanFrameTask<FResolveImage>::TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FResolveImage& desc, FProcessFunc process)
:   IVulkanFrameTask(desc, process)
,   SrcImage(cmd.ToLocal(desc.SrcImage))
,   SrcLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
,   DstImage(cmd.ToLocal(desc.DstImage))
,   DstLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
,   Regions(desc.Regions) {
    Assert_NoAssume(SrcImage and SrcImage->Read()->Desc.Usage & EImageUsage::TransferSrc);
    Assert_NoAssume(DstImage and DstImage->Read()->Desc.Usage & EImageUsage::TransferDst);
}
//----------------------------------------------------------------------------
// FillBuffer
//----------------------------------------------------------------------------
TVulkanFrameTask<FFillBuffer>::TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FFillBuffer& desc, FProcessFunc process)
:   IVulkanFrameTask(desc, process)
,   DstBuffer(cmd.ToLocal(desc.DstBuffer))
,   DstOffset(checked_cast<VkDeviceSize>(desc.DstOffset))
,   Size(checked_cast<VkDeviceSize>(desc.Size))
,   Pattern(desc.Pattern) {
    Assert_NoAssume(DstBuffer and DstBuffer->Read()->Desc.Usage & EBufferUsage::TransferDst);
}
//----------------------------------------------------------------------------
// Clear
//----------------------------------------------------------------------------
TVulkanFrameTask<FClearColorImage>::TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FClearColorImage& desc, FProcessFunc process)
:   IVulkanFrameTask(desc, process)
,   DstImage(cmd.ToLocal(desc.DstImage))
,   DstLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
,   Ranges(desc.Ranges)
,   ClearValue(VKClearColorValue_(desc.ClearColor)) {
    Assert_NoAssume(DstImage and DstImage->Read()->Desc.Usage & EImageUsage::TransferDst);
}
//----------------------------------------------------------------------------
TVulkanFrameTask<FClearDepthStencilImage>::TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FClearDepthStencilImage& desc, FProcessFunc process)
:   IVulkanFrameTask(desc, process)
,   DstImage(cmd.ToLocal(desc.DstImage))
,   DstLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
,   Ranges(desc.Ranges)
,   ClearValue{ desc.ClearDepth, desc.ClearStencil } {
    Assert_NoAssume(DstImage and DstImage->Read()->Desc.Usage & EImageUsage::TransferDst);
}
//----------------------------------------------------------------------------
// UpdateBuffer
//----------------------------------------------------------------------------
TVulkanFrameTask<FUpdateBuffer>::TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FUpdateBuffer& desc, FProcessFunc process)
:   IVulkanFrameTask(desc, process)
,   DstBuffer(cmd.ToLocal(desc.DstBuffer))
,   Regions(cmd.Allocator().AllocateT<FRegion>(desc.Regions.size())) {
    Assert_NoAssume(DstBuffer and DstBuffer->Read()->Desc.Usage & EBufferUsage::TransferDst);

    forrange(i, 0, checked_cast<u32>(Regions.size())) {
        const FUpdateBuffer::FRegion& src = desc.Regions[i];
        FRegion& dst = const_cast<FRegion&>(Regions[i]);

        dst.DataPtr = cmd.Allocator().Allocate(src.Data.SizeInBytes());
        dst.DataSize = checked_cast<VkDeviceSize>(src.Data.SizeInBytes());
        dst.BufferOffset = checked_cast<VkDeviceSize>(src.Offset);

        FPlatformMemory::Memcpy(dst.DataPtr, src.Data.data(), src.Data.SizeInBytes());
    }
}
//----------------------------------------------------------------------------
// Present
//----------------------------------------------------------------------------
TVulkanFrameTask<FPresent>::TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FPresent& desc, FProcessFunc process)
:   IVulkanFrameTask(desc, process)
,   Swapchain(cmd.AcquireTransient(desc.Swapchain))
,   SrcImage(cmd.ToLocal(desc.SrcImage))
,   Layer(desc.Layer)
,   Mipmap(desc.Mipmap) {
    Assert_NoAssume(SrcImage and SrcImage->Read()->Desc.Usage & EImageUsage::TransferSrc);
}
//----------------------------------------------------------------------------
// Custom
//----------------------------------------------------------------------------
TVulkanFrameTask<FCustomTask>::TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FCustomTask& desc, FProcessFunc process)
:   IVulkanFrameTask(desc, process)
,   Callback(desc.Callback)
,   Images(cmd.Allocator().AllocateCopyT(desc.Images.MakeView(), [&cmd](auto src) {
        return TPair<const FVulkanLocalImage*, EResourceState>(cmd.ToLocal(src.first), src.second);
    }))
,   Buffers(cmd.Allocator().AllocateCopyT(desc.Buffers.MakeView(), [&cmd](auto src) {
        return TPair<const FVulkanLocalBuffer*, EResourceState>(cmd.ToLocal(src.first), src.second);
    })) {

}
//----------------------------------------------------------------------------
// RayTracing
//----------------------------------------------------------------------------
TVulkanFrameTask<FUpdateRayTracingShaderTable>::TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FUpdateRayTracingShaderTable& desc, FProcessFunc process)
:   IVulkanFrameTask(desc, process)
,   Pipeline(desc.Pipeline)
,   RTScene(cmd.ToLocal(desc.Scene))
,   ShaderTable(const_cast<FVulkanRTShaderTable*>(cmd.AcquireTransient(desc.ShaderTable)))
,   ShaderGroups(cmd.Allocator().AllocateCopyT(desc.ShaderGroups.MakeView()))
,   RayGenShader(desc.RayGenShader)
,   MaxRecursionDepth(desc.MaxRecursionDepth) {

}
//----------------------------------------------------------------------------
TVulkanFrameTask<FBuildRayTracingGeometry>::TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FBuildRayTracingGeometry& desc, FProcessFunc process)
:   IVulkanFrameTask(desc, process)
,   UsableBuffers(cmd.Allocator()) {
    Unused(cmd); // Initialized in FVulkanCommandBuffer::Task(const FBuildRayTracingGeometry& task)
}
//----------------------------------------------------------------------------
TVulkanFrameTask<FBuildRayTracingScene>::TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FBuildRayTracingScene& desc, FProcessFunc process)
:   IVulkanFrameTask(desc, process) {
    Unused(cmd); // Initialized in FVulkanCommandBuffer::Task(const FBuildRayTracingScene& task)
}
//----------------------------------------------------------------------------
TVulkanFrameTask<FTraceRays>::TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FTraceRays& desc, FProcessFunc process)
:   IVulkanFrameTask(desc, process)
,   ShaderTable(cmd.AcquireTransient(desc.ShaderTable))
,   PushConstants(desc.PushConstants)
,   GroupCount(Max(desc.GroupCount, 1u)) {
    Assert_NoAssume(ShaderTable);

    CopyDescriptorSets(&_resources, nullptr, cmd, desc.Resources);

#if USE_PPE_RHIDEBUG
    if (desc.DebugMode.Mode != Default)
        DebugModeIndex = cmd.Batch()->AppendShaderForDebug(desc.TaskName, desc.DebugMode);
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
