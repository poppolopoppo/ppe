// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

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
IVulkanFrameTask::IVulkanFrameTask() NOEXCEPT = default;
//----------------------------------------------------------------------------
IVulkanFrameTask::~IVulkanFrameTask() = default;
//----------------------------------------------------------------------------
template <typename T>
IVulkanFrameTask::IVulkanFrameTask(FVulkanCommandBuffer& cmd, const details::TFrameTaskDesc<T>& desc, FProcessFunc process) NOEXCEPT
:   _process(process) {
    Assert_NoAssume(_process);

    _inputs = cmd.EmbedCopy(desc.Dependencies.MakeView().Map([](auto dep) NOEXCEPT {
        Assert(dep);
        return MakePtrRef(checked_cast<IVulkanFrameTask*>(dep.get()));
    }));

#if USE_PPE_RHIDEBUG
    TaskName = cmd.EmbedString(desc.TaskName.Str());
    DebugColor = desc.DebugColor.Quantize(EGammaSpace::sRGB);
#endif
}
//----------------------------------------------------------------------------
void IVulkanFrameTask::Attach(FVulkanCommandBuffer& cmd, const PVulkanFrameTask& output) {
    Assert(output);
    Assert_NoAssume(not _outputs.Contains(output));

    _outputs = cmd.Write()->MainAllocator.ReallocateT(_outputs, _outputs.size() + 1);
    _outputs.back() = output;
}
//----------------------------------------------------------------------------
void IVulkanFrameTask::CopyDescriptorSets(
    FVulkanPipelineResourceSet* outResources,
    FVulkanLogicalRenderPass* pRenderPass,
    FVulkanCommandBuffer& cmd,
    const FPipelineResourceSet& input ) NOEXCEPT {
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
TVulkanFrameTask<FSubmitRenderPass>::TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FSubmitRenderPass& desc, FProcessFunc process) NOEXCEPT
:   IVulkanFrameTask(cmd, desc, process)
,   LogicalPass(cmd.ToLocal(desc.RenderPassId)) {

    if (LogicalPass)
        VerifyRelease(LogicalPass->Submit(cmd, desc.Images.MakeView(), desc.Buffers.MakeView()) );
}
//----------------------------------------------------------------------------
TVulkanFrameTask<FSubmitRenderPass>::~TVulkanFrameTask() = default;
//----------------------------------------------------------------------------
// Compute
//----------------------------------------------------------------------------
TVulkanFrameTask<FDispatchCompute>::TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FDispatchCompute& desc, FProcessFunc process) NOEXCEPT
:   IVulkanFrameTask(cmd, desc, process)
,   Pipeline(cmd.AcquireTransient(desc.Pipeline))
,   LocalGroupSize(desc.LocalGroupSize)
,   PushConstants(cmd.EmbedCopy(desc.PushConstants.MakeView()))
,   Commands(cmd.EmbedCopy(desc.Commands.MakeView())) {

    FVulkanPipelineResourceSet resources;
    CopyDescriptorSets(&resources, nullptr, cmd, desc.Resources);

    DynamicOffsets = cmd.EmbedCopy(resources.DynamicOffsets.MakeView());
    Resources = cmd.EmbedCopy(resources.Resources.MakeView());

#if USE_PPE_RHIDEBUG
    if (desc.DebugMode.Mode != Default)
        DebugModeIndex = cmd.Batch()->AppendShaderForDebug(desc.TaskName, desc.DebugMode);
#endif
}
//----------------------------------------------------------------------------
TVulkanFrameTask<FDispatchCompute>::~TVulkanFrameTask() = default;
//----------------------------------------------------------------------------
TVulkanFrameTask<FDispatchComputeIndirect>::TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FDispatchComputeIndirect& desc, FProcessFunc process) NOEXCEPT
:   IVulkanFrameTask(cmd, desc, process)
,   Pipeline(cmd.AcquireTransient(desc.Pipeline))
,   IndirectBuffer(cmd.ToLocal(desc.IndirectBuffer))
,   LocalGroupSize(desc.LocalGroupSize)
,   PushConstants(cmd.EmbedCopy(desc.PushConstants.MakeView()))
,   Commands(cmd.EmbedCopy(desc.Commands.MakeView()))
{
    Assert_NoAssume(IndirectBuffer);
    Assert_NoAssume(IndirectBuffer->Read()->Desc.Usage & EBufferUsage::Indirect);

    FVulkanPipelineResourceSet resources;
    CopyDescriptorSets(&resources, nullptr, cmd, desc.Resources);

    DynamicOffsets = cmd.EmbedCopy(resources.DynamicOffsets.MakeView());
    Resources = cmd.EmbedCopy(resources.Resources.MakeView());

#if USE_PPE_RHIDEBUG
    if (desc.DebugMode.Mode != Default)
        DebugModeIndex = cmd.Batch()->AppendShaderForDebug(desc.TaskName, desc.DebugMode);
#endif
}
//----------------------------------------------------------------------------
TVulkanFrameTask<FDispatchComputeIndirect>::~TVulkanFrameTask() = default;
//----------------------------------------------------------------------------
// Copy
//----------------------------------------------------------------------------
TVulkanFrameTask<FCopyBuffer>::TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FCopyBuffer& desc, FProcessFunc process) NOEXCEPT
:   IVulkanFrameTask(cmd, desc, process)
,   SrcBuffer(cmd.ToLocal(desc.SrcBuffer))
,   DstBuffer(cmd.ToLocal(desc.DstBuffer))
,   Regions(cmd.EmbedCopy(desc.Regions.MakeView())) {
    Assert_NoAssume(SrcBuffer and SrcBuffer->Read()->Desc.Usage & EBufferUsage::TransferSrc);
    Assert_NoAssume(DstBuffer and DstBuffer->Read()->Desc.Usage & EBufferUsage::TransferDst);
}
//----------------------------------------------------------------------------
TVulkanFrameTask<FCopyBuffer>::~TVulkanFrameTask() = default;
//----------------------------------------------------------------------------
// FCopyImage
//----------------------------------------------------------------------------
TVulkanFrameTask<FCopyImage>::TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FCopyImage& desc, FProcessFunc process) NOEXCEPT
:   IVulkanFrameTask(cmd, desc, process)
,   SrcImage(cmd.ToLocal(desc.SrcImage))
,   DstImage(cmd.ToLocal(desc.DstImage))
,   SrcLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
,   DstLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
,   Regions(cmd.EmbedCopy(desc.Regions.MakeView())) {
    Assert_NoAssume(SrcImage and SrcImage->Read()->Desc.Usage & EImageUsage::TransferSrc);
    Assert_NoAssume(DstImage and DstImage->Read()->Desc.Usage & EImageUsage::TransferDst);
}
//----------------------------------------------------------------------------
TVulkanFrameTask<FCopyImage>::~TVulkanFrameTask() = default;
//----------------------------------------------------------------------------
// FCopyBufferToImage
//----------------------------------------------------------------------------
TVulkanFrameTask<FCopyBufferToImage>::TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FCopyBufferToImage& desc, FProcessFunc process) NOEXCEPT
:   IVulkanFrameTask(cmd, desc, process)
,   SrcBuffer(cmd.ToLocal(desc.SrcBuffer))
,   DstImage(cmd.ToLocal(desc.DstImage))
,   DstLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
,   Regions(cmd.EmbedCopy(desc.Regions.MakeView())) {
    Assert_NoAssume(SrcBuffer and SrcBuffer->Read()->Desc.Usage & EBufferUsage::TransferSrc);
    Assert_NoAssume(DstImage and DstImage->Read()->Desc.Usage & EImageUsage::TransferDst);
}
//----------------------------------------------------------------------------
TVulkanFrameTask<FCopyBufferToImage>::~TVulkanFrameTask() = default;
//----------------------------------------------------------------------------
// FCopyImageToBuffer
//----------------------------------------------------------------------------
TVulkanFrameTask<FCopyImageToBuffer>::TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FCopyImageToBuffer& desc, FProcessFunc process) NOEXCEPT
:   IVulkanFrameTask(cmd, desc, process)
,   SrcImage(cmd.ToLocal(desc.SrcImage))
,   DstBuffer(cmd.ToLocal(desc.DstBuffer))
,   SrcLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
,   Regions(cmd.EmbedCopy(desc.Regions.MakeView())) {
    Assert_NoAssume(SrcImage and SrcImage->Read()->Desc.Usage & EImageUsage::TransferSrc);
    Assert_NoAssume(DstBuffer and DstBuffer->Read()->Desc.Usage & EBufferUsage::TransferDst);
}
//----------------------------------------------------------------------------
TVulkanFrameTask<FCopyImageToBuffer>::~TVulkanFrameTask() = default;
//----------------------------------------------------------------------------
// Blit
//----------------------------------------------------------------------------
TVulkanFrameTask<FBlitImage>::TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FBlitImage& desc, FProcessFunc process) NOEXCEPT
:   IVulkanFrameTask(cmd, desc, process)
,   SrcImage(cmd.ToLocal(desc.SrcImage))
,   DstImage(cmd.ToLocal(desc.DstImage))
,   SrcLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
,   DstLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
,   Filter(VkCast(desc.Filter))
,   Regions(cmd.EmbedCopy(desc.Regions.MakeView())) {
    Assert_NoAssume(SrcImage and SrcImage->Read()->Desc.Usage & EImageUsage::TransferSrc);
    Assert_NoAssume(DstImage and DstImage->Read()->Desc.Usage & EImageUsage::TransferDst);
}
//----------------------------------------------------------------------------
TVulkanFrameTask<FBlitImage>::~TVulkanFrameTask() = default;
//----------------------------------------------------------------------------
// GenerateMipMaps
//----------------------------------------------------------------------------
TVulkanFrameTask<FGenerateMipmaps>::TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FGenerateMipmaps& desc, FProcessFunc process) NOEXCEPT
:   IVulkanFrameTask(cmd, desc, process)
,   Image(cmd.ToLocal(desc.Image))
,   BaseMipLevel(*desc.BaseLevel)
,   LevelCount(desc.LevelCount)
,   BaseLayer(*desc.BaseLayer)
,   LayerCount(desc.LayerCount) {
    Assert_NoAssume(Image and Image->Read()->Desc.Usage & (EImageUsage::TransferSrc + EImageUsage::TransferDst));
}
//----------------------------------------------------------------------------
TVulkanFrameTask<FGenerateMipmaps>::~TVulkanFrameTask() = default;
//----------------------------------------------------------------------------
// ResolveImage
//----------------------------------------------------------------------------
TVulkanFrameTask<FResolveImage>::TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FResolveImage& desc, FProcessFunc process) NOEXCEPT
:   IVulkanFrameTask(cmd, desc, process)
,   SrcImage(cmd.ToLocal(desc.SrcImage))
,   DstImage(cmd.ToLocal(desc.DstImage))
,   SrcLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
,   DstLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
,   Regions(cmd.EmbedCopy(desc.Regions.MakeView())) {
    Assert_NoAssume(SrcImage and SrcImage->Read()->Desc.Usage & EImageUsage::TransferSrc);
    Assert_NoAssume(DstImage and DstImage->Read()->Desc.Usage & EImageUsage::TransferDst);
}
//----------------------------------------------------------------------------
TVulkanFrameTask<FResolveImage>::~TVulkanFrameTask() = default;
//----------------------------------------------------------------------------
// FillBuffer
//----------------------------------------------------------------------------
TVulkanFrameTask<FFillBuffer>::TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FFillBuffer& desc, FProcessFunc process) NOEXCEPT
:   IVulkanFrameTask(cmd, desc, process)
,   DstBuffer(cmd.ToLocal(desc.DstBuffer))
,   DstOffset(checked_cast<VkDeviceSize>(desc.DstOffset))
,   Size(checked_cast<VkDeviceSize>(desc.Size))
,   Pattern(desc.Pattern) {
    Assert_NoAssume(DstBuffer and DstBuffer->Read()->Desc.Usage & EBufferUsage::TransferDst);
}
//----------------------------------------------------------------------------
TVulkanFrameTask<FFillBuffer>::~TVulkanFrameTask() = default;
//----------------------------------------------------------------------------
// Clear
//----------------------------------------------------------------------------
TVulkanFrameTask<FClearColorImage>::TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FClearColorImage& desc, FProcessFunc process) NOEXCEPT
:   IVulkanFrameTask(cmd, desc, process)
,   DstImage(cmd.ToLocal(desc.DstImage))
,   DstLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
,   ClearValue(VKClearColorValue_(desc.ClearColor))
,   Ranges(cmd.EmbedCopy(desc.Ranges.MakeView())){
    Assert_NoAssume(DstImage and DstImage->Read()->Desc.Usage & EImageUsage::TransferDst);
}
//----------------------------------------------------------------------------
TVulkanFrameTask<FClearColorImage>::~TVulkanFrameTask() = default;
//----------------------------------------------------------------------------
// FClearDepthStencilImage
//----------------------------------------------------------------------------
TVulkanFrameTask<FClearDepthStencilImage>::TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FClearDepthStencilImage& desc, FProcessFunc process) NOEXCEPT
:   IVulkanFrameTask(cmd, desc, process)
,   DstImage(cmd.ToLocal(desc.DstImage))
,   DstLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
,   ClearValue{ desc.ClearDepth, desc.ClearStencil }
,   Ranges(cmd.EmbedCopy(desc.Ranges.MakeView())) {
    Assert_NoAssume(DstImage and DstImage->Read()->Desc.Usage & EImageUsage::TransferDst);
}
//----------------------------------------------------------------------------
TVulkanFrameTask<FClearDepthStencilImage>::~TVulkanFrameTask() = default;
//----------------------------------------------------------------------------
// UpdateBuffer
//----------------------------------------------------------------------------
TVulkanFrameTask<FUpdateBuffer>::TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FUpdateBuffer& desc, FProcessFunc process) NOEXCEPT
:   IVulkanFrameTask(cmd, desc, process)
,   DstBuffer(cmd.ToLocal(desc.DstBuffer))
,   Regions(cmd.EmbedView<FRegion>(desc.Regions.size())) {
    Assert_NoAssume(DstBuffer and DstBuffer->Read()->Desc.Usage & EBufferUsage::TransferDst);

    forrange(i, 0, checked_cast<u32>(Regions.size())) {
        const FUpdateBuffer::FRegion& src = desc.Regions[i];

        FRegion& dst = const_cast<FRegion&>(Regions[i]);
        dst.DataPtr = cmd.EmbedCopy(src.Data).data();
        dst.DataSize = checked_cast<VkDeviceSize>(src.Data.SizeInBytes());
        dst.BufferOffset = checked_cast<VkDeviceSize>(src.Offset);
    }
}
//----------------------------------------------------------------------------
TVulkanFrameTask<FUpdateBuffer>::~TVulkanFrameTask() = default;
//----------------------------------------------------------------------------
// Present
//----------------------------------------------------------------------------
TVulkanFrameTask<FPresent>::TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FPresent& desc, FProcessFunc process) NOEXCEPT
:   IVulkanFrameTask(cmd, desc, process)
,   Swapchain(cmd.AcquireTransient(desc.Swapchain))
,   SrcImage(cmd.ToLocal(desc.SrcImage))
,   Layer(desc.Layer)
,   Mipmap(desc.Mipmap) {
    Assert_NoAssume(SrcImage and SrcImage->Read()->Desc.Usage & EImageUsage::TransferSrc);
}
//----------------------------------------------------------------------------
TVulkanFrameTask<FPresent>::~TVulkanFrameTask() = default;
//----------------------------------------------------------------------------
// Custom
//----------------------------------------------------------------------------
TVulkanFrameTask<FCustomTask>::TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FCustomTask& desc, FProcessFunc process) NOEXCEPT
:   IVulkanFrameTask(cmd, desc, process)
,   Callback(desc.Callback)
,   Images(cmd.EmbedCopy(desc.Images.MakeView().Map([&cmd](auto src) {
        return FImage(cmd.ToLocal(src.first), src.second);
    })))
,   Buffers(cmd.EmbedCopy(desc.Buffers.MakeView().Map([&cmd](auto src) {
        return FBuffer(cmd.ToLocal(src.first), src.second);
    })))
{}
//----------------------------------------------------------------------------
TVulkanFrameTask<FCustomTask>::~TVulkanFrameTask() = default;
//----------------------------------------------------------------------------
// FUpdateRayTracingShaderTable
//----------------------------------------------------------------------------
TVulkanFrameTask<FUpdateRayTracingShaderTable>::TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FUpdateRayTracingShaderTable& desc, FProcessFunc process) NOEXCEPT
:   IVulkanFrameTask(cmd, desc, process)
,   Pipeline(desc.Pipeline)
,   MaxRecursionDepth(desc.MaxRecursionDepth)
,   RTScene(cmd.ToLocal(desc.Scene))
,   ShaderTable(const_cast<FVulkanRTShaderTable*>(cmd.AcquireTransient(desc.ShaderTable)))
,   ShaderGroups(cmd.EmbedCopy(desc.ShaderGroups.MakeView()))
,   RayGenShader(desc.RayGenShader)
{}
//----------------------------------------------------------------------------
TVulkanFrameTask<FUpdateRayTracingShaderTable>::~TVulkanFrameTask() = default;
//----------------------------------------------------------------------------
// FBuildRayTracingGeometry
//----------------------------------------------------------------------------
TVulkanFrameTask<FBuildRayTracingGeometry>::TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FBuildRayTracingGeometry& desc, FProcessFunc process) NOEXCEPT
:   IVulkanFrameTask(cmd, desc, process)
,   UsableBuffers(cmd.Write()->MainAllocator) {
    Unused(cmd); // Initialized in FVulkanCommandBuffer::Task(const FBuildRayTracingGeometry& task)
}
//----------------------------------------------------------------------------
TVulkanFrameTask<FBuildRayTracingGeometry>::~TVulkanFrameTask() = default;
//----------------------------------------------------------------------------
// FBuildRayTracingScene
//----------------------------------------------------------------------------
TVulkanFrameTask<FBuildRayTracingScene>::TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FBuildRayTracingScene& desc, FProcessFunc process) NOEXCEPT
:   IVulkanFrameTask(cmd, desc, process) {
    Unused(cmd); // Initialized in FVulkanCommandBuffer::Task(const FBuildRayTracingScene& task)
}
//----------------------------------------------------------------------------
TVulkanFrameTask<FBuildRayTracingScene>::~TVulkanFrameTask() = default;
//----------------------------------------------------------------------------
// FTraceRays
//----------------------------------------------------------------------------
TVulkanFrameTask<FTraceRays>::TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FTraceRays& desc, FProcessFunc process) NOEXCEPT
:   IVulkanFrameTask(cmd, desc, process)
,   ShaderTable(cmd.AcquireTransient(desc.ShaderTable))
,   PushConstants(cmd.EmbedCopy(desc.PushConstants.MakeView()))
,   GroupCount(Max(desc.GroupCount, 1u)) {
    Assert_NoAssume(ShaderTable);

    FVulkanPipelineResourceSet resources;
    CopyDescriptorSets(&resources, nullptr, cmd, desc.Resources);

    DynamicOffsets = cmd.EmbedCopy(resources.DynamicOffsets.MakeView());
    Resources = cmd.EmbedCopy(resources.Resources.MakeView());

#if USE_PPE_RHIDEBUG
    if (desc.DebugMode.Mode != Default)
        DebugModeIndex = cmd.Batch()->AppendShaderForDebug(desc.TaskName, desc.DebugMode);
#endif
}
//----------------------------------------------------------------------------
TVulkanFrameTask<FTraceRays>::~TVulkanFrameTask() = default;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
