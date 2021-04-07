#pragma once

#include "Vulkan/VulkanCommon.h"

#include "RHI/FrameGraph.h"
#include "RHI/FrameTask.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IVulkanFrameTask : public IFrameTask {
public:
    using FDependencies = TFixedSizeStack<PVulkanFrameTask, MaxTaskDependencies>;
    using FProcessFunc = void (*)(void* visitor, const void* data);

    u32 VisitorId() const { return _visitorId; }
    void SetVisitorId(u32 value) { _visitorId = value; }

    EVulkanExecutionOrder ExecOrder() const { return _execOrder; }
    void SetExecOrder(EVulkanExecutionOrder value) { _execOrder = value; }

    TMemoryView<const PVulkanFrameTask> Inputs() const { return _inputs.MakeView(); }
    TMemoryView<const PVulkanFrameTask> Outputs() const { return _outputs.MakeView(); }

    void Attach(const PVulkanFrameTask& output) {
        Assert(output);
        Add_AssertUnique(_outputs, output);
    }

    void Process(void* visitor) const {
        Assert(visitor);
        _process(visitor, this);
    }

#ifdef USE_PPE_RHIDEBUG
    FStringView TaskName() const { return _taskName; }
    const FRgba8u DebugColor() const { return _debugColor; }
#endif

protected:
    IVulkanFrameTask() = default;

    template <typename T>
    IVulkanFrameTask(const details::TFrameTaskDesc<T>& desc, FProcessFunc process)
    :   _process(process)
#ifdef USE_PPE_RHIDEBUG
    ,   _taskName(desc.TaskName), _debugColor(desc.DebugColor)
#endif
    {
        Assert_NoAssume(_process);
        for (const PFrameTask& dep : desc.Dependencies) {
            Assert(dep);
            Emplace_Back(_inputs, checked_cast<IVulkanFrameTask*>(dep.get()));
        }
    }

    FProcessFunc _process{ nullptr };

    u32 _visitorId{ 0 };
    EVulkanExecutionOrder _execOrder{ EVulkanExecutionOrder::Initial };

    FDependencies _inputs;
    FDependencies _outputs;

#ifdef USE_PPE_RHIDEBUG
    FTaskName _taskName;
    FRgba8u _debugColor{ FRgba8u::One };
#endif
};
//----------------------------------------------------------------------------
// FSubmitRenderPass:
//----------------------------------------------------------------------------
template <>
class PPE_RHIVULKAN_API TVulkanFrameTask<FSubmitRenderPass> final : public IVulkanFrameTask {
public:
    TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FSubmitRenderPass& desc, FProcessFunc process);

    bool Valid() const { return (!!_logicalPass); }

    bool IsSubpass() const { return (!!_prevSubpass); }
    bool IsLastPass() const { return (!_nextSubpass); }

    FVulkanLogicalRenderPass* LogicalPass() const { return _logicalPass; }

    TVulkanFrameTask* PrevSubpass() const { return _prevSubpass; }
    TVulkanFrameTask* NextSubpass() const { return _nextSubpass; }

private:
    FVulkanLogicalRenderPass* _logicalPass;
    TVulkanFrameTask* _prevSubpass{ nullptr };
    TVulkanFrameTask* _nextSubpass{ nullptr };
};
//----------------------------------------------------------------------------
// FDispatchCompute:
//----------------------------------------------------------------------------
template <>
class PPE_RHIVULKAN_API TVulkanFrameTask<FDispatchCompute> final : public IVulkanFrameTask {
public:
    const SCVulkanComputePipeline Pipeline;
    const FPushConstantDatas PushConstants;

    const FDispatchCompute::FComputeCommands Commands;
    const Meta::TOptional<uint3> LocalGroupSize;

#if USE_PPE_RHIDEBUG
    EShaderDebugIndex DebugModeIndex{ Default };
#endif

    TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FDispatchCompute& desc, FProcessFunc process);

    bool Valid() const { return (!!Pipeline); }

    const FVulkanPipelineResourceSet& Resource() const { return _resources; }

private:
    FVulkanPipelineResourceSet _resources;
};
//----------------------------------------------------------------------------
// FDispatchComputeIndirect:
//----------------------------------------------------------------------------
template <>
class PPE_RHIVULKAN_API TVulkanFrameTask<FDispatchComputeIndirect> final : public IVulkanFrameTask {
public:
    const SCVulkanComputePipeline Pipeline;
    const FPushConstantDatas PushConstants;

    const FDispatchComputeIndirect::FComputeCommands Commands;
    const Meta::TOptional<uint3> LocalGroupSize;

    const FVulkanLocalBuffer* const IndirectBuffer;

#if USE_PPE_RHIDEBUG
    EShaderDebugIndex DebugModeIndex{ Default };
#endif

    TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FDispatchComputeIndirect& desc, FProcessFunc process);

    bool Valid() const { return (!!Pipeline); }

    const FVulkanPipelineResourceSet& Resource() const { return _resources; }

    private:
    FVulkanPipelineResourceSet _resources;
};
//----------------------------------------------------------------------------
// FCopyBuffer:
//----------------------------------------------------------------------------
template <>
class PPE_RHIVULKAN_API TVulkanFrameTask<FCopyBuffer> final : public IVulkanFrameTask {
public:
    using FRegion = FCopyBuffer::FRegion;
    using FRegions = FCopyBuffer::FRegions;

    const FVulkanLocalBuffer* const SrcBuffer;
    const FVulkanLocalBuffer* const DstBuffer;
    const FRegions Regions;

    TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FCopyBuffer& desc, FProcessFunc process);

    bool Valid() const { return (!!SrcBuffer && !!DstBuffer && not Regions.empty()); }
};
//----------------------------------------------------------------------------
// FCopyImage:
//----------------------------------------------------------------------------
template <>
class PPE_RHIVULKAN_API TVulkanFrameTask<FCopyImage> final : public IVulkanFrameTask {
    public:
    using FRegion = FCopyImage::FRegion;
    using FRegions = FCopyImage::FRegions;

    const FVulkanLocalImage* const SrcImage;
    const VkImageLayout SrcLayout;
    const FVulkanLocalImage* const DstImage;
    const VkImageLayout DstLayout;
    const FRegions Regions;

    TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FCopyImage& desc, FProcessFunc process);

    bool Valid() const { return (!!SrcImage && !!DstImage && not Regions.empty()); }
};
//----------------------------------------------------------------------------
// FCopyBufferToImage:
//----------------------------------------------------------------------------
template <>
class PPE_RHIVULKAN_API TVulkanFrameTask<FCopyBufferToImage> final : public IVulkanFrameTask {
    public:
    using FRegion = FCopyBufferToImage::FRegion;
    using FRegions = FCopyBufferToImage::FRegions;

    const FVulkanLocalBuffer* const SrcBuffer;
    const FVulkanLocalImage* const DstImage;
    const VkImageLayout DstLayout;
    const FRegions Regions;

    TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FCopyBufferToImage& desc, FProcessFunc process);

    bool Valid() const { return (!!SrcBuffer && !!DstImage && not Regions.empty()); }
};
//----------------------------------------------------------------------------
// FCopyImageToBuffer:
//----------------------------------------------------------------------------
template <>
class PPE_RHIVULKAN_API TVulkanFrameTask<FCopyImageToBuffer> final : public IVulkanFrameTask {
    public:
    using FRegion = FCopyImageToBuffer::FRegion;
    using FRegions = FCopyImageToBuffer::FRegions;

    const FVulkanLocalImage* const SrcImage;
    const VkImageLayout SrcLayout;
    const FVulkanLocalBuffer* const DstBuffer;
    const FRegions Regions;

    TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FCopyImageToBuffer& desc, FProcessFunc process);

    bool Valid() const { return (!!SrcImage && !!DstBuffer && not Regions.empty()); }
};
//----------------------------------------------------------------------------
// FBlitImage:
//----------------------------------------------------------------------------
template <>
class PPE_RHIVULKAN_API TVulkanFrameTask<FBlitImage> final : public IVulkanFrameTask {
public:
    using FRegion = FBlitImage::FRegion;
    using FRegions = FBlitImage::FRegions;

    const FVulkanLocalImage* const SrcImage;
    const VkImageLayout SrcLayout;
    const FVulkanLocalImage* const DstImage;
    const VkImageLayout DstLayout;
    const VkFilter Filter;
    const FRegions Regions;

    TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FBlitImage& desc, FProcessFunc process);

    bool Valid() const { return (!!SrcImage && !!DstImage && not Regions.empty()); }
};
//----------------------------------------------------------------------------
// FGenerateMipmaps:
//----------------------------------------------------------------------------
template <>
class PPE_RHIVULKAN_API TVulkanFrameTask<FGenerateMipmaps> final : public IVulkanFrameTask {
public:
    const FVulkanLocalImage* const Image;
    const u32 BaseLevel;
    const u32 LevelCount;

    TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FBlitImage& desc, FProcessFunc process);

    bool Valid() const { return (!!Image && LevelCount > 0); }
};
//----------------------------------------------------------------------------
// FResolveImage:
//----------------------------------------------------------------------------
template <>
class PPE_RHIVULKAN_API TVulkanFrameTask<FResolveImage> final : public IVulkanFrameTask {
public:
    using FRegion = FResolveImage::FRegion;
    using FRegions = FResolveImage::FRegions;

    const FVulkanLocalImage* const SrcImage;
    const VkImageLayout SrcLayout;
    const FVulkanLocalImage* const DstImage;
    const VkImageLayout DstLayout;
    const FRegions Regions;

    TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FResolveImage& desc, FProcessFunc process);

    bool Valid() const { return (!!SrcImage && !!DstImage && not Regions.empty()); }
};
//----------------------------------------------------------------------------
// FFillBuffer:
//----------------------------------------------------------------------------
template <>
class PPE_RHIVULKAN_API TVulkanFrameTask<FFillBuffer> final : public IVulkanFrameTask {
public:
    const FVulkanLocalBuffer* const DstBuffer;
    const VkDeviceSize DstOffset;
    const VkDeviceSize Size;
    const u32 Pattern;

    TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FFillBuffer& desc, FProcessFunc process);

    bool Valid() const { return (!!DstBuffer && Size > 0); }
};
//----------------------------------------------------------------------------
// FClearColorImage:
//----------------------------------------------------------------------------
template <>
class PPE_RHIVULKAN_API TVulkanFrameTask<FClearColorImage> final : public IVulkanFrameTask {
public:
    using FRange = FClearColorImage::FRange;
    using FRanges = FClearColorImage::FRanges;

    const FVulkanLocalImage* const DstImage;
    const VkImageLayout DstLayout;
    const FRanges Ranges;
    const VkClearColorValue ClearValue;

    TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FClearColorImage& desc, FProcessFunc process);

    bool Valid() const { return (!!DstImage && not Ranges.empty()); }
};
//----------------------------------------------------------------------------
// FClearDepthStencilImage:
//----------------------------------------------------------------------------
template <>
class PPE_RHIVULKAN_API TVulkanFrameTask<FClearDepthStencilImage> final : public IVulkanFrameTask {
public:
    using FRange = FClearDepthStencilImage::FRange;
    using FRanges = FClearDepthStencilImage::FRanges;

    const FVulkanLocalImage* const DstImage;
    const VkImageLayout DstLayout;
    const FRanges Ranges;
    const VkClearDepthStencilValue ClearValue;

    TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FClearDepthStencilImage& desc, FProcessFunc process);

    bool Valid() const { return (!!DstImage && not Ranges.empty()); }
};
//----------------------------------------------------------------------------
// FUpdateBuffer:
//----------------------------------------------------------------------------
template <>
class PPE_RHIVULKAN_API TVulkanFrameTask<FUpdateBuffer> final : public IVulkanFrameTask {
public:
    struct FRegion {
        void* DataPtr{ nullptr };
        VkDeviceSize DataSize{ 0 };
        VkDeviceSize BufferOffset{ 0 };
    };

    const FVulkanLocalBuffer* const DstBuffer;
    const TMemoryView<const FRegion> Regions;

    TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FUpdateBuffer& desc, FProcessFunc process);

    bool Valid() const { return (!!DstBuffer && not Regions.empty()); }
};
//----------------------------------------------------------------------------
// FPresent:
//----------------------------------------------------------------------------
template <>
class PPE_RHIVULKAN_API TVulkanFrameTask<FPresent> final : public IVulkanFrameTask {
public:
    const SCVulkanSwapchain Swapchain;
    const FVulkanLocalImage* const SrcImage;
    const FImageLayer Layer;
    const FMipmapLevel Mipmap;

    TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FPresent& desc, FProcessFunc process);

    bool Valid() const { return (!!Swapchain && !!SrcImage); }
};
//----------------------------------------------------------------------------
// FCustomTask:
//----------------------------------------------------------------------------
template <>
class PPE_RHIVULKAN_API TVulkanFrameTask<FCustomTask> final : public IVulkanFrameTask {
public:
    using FCallback = FCustomTask::FCallback;
    using FImages = TMemoryView<const TPair<const FVulkanLocalImage*, EResourceState>>;
    using FBuffers = TMemoryView<const TPair<const FVulkanLocalBuffer*, EResourceState>>;

    const FCallback Callback;
    const FImages Images;
    const FBuffers Buffers;

    TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FCustomTask& desc, FProcessFunc process);

    bool Valid() const { return (!!Callback); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
