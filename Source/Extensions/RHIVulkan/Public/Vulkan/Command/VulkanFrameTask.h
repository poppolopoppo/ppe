#pragma once

#include "Vulkan/VulkanCommon.h"

#include "Vulkan/Pipeline/VulkanComputePipeline.h"

#include "RHI/FrameGraph.h"
#include "RHI/FrameTask.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
using FVulkanFrameTaskRef = std::variant<
    const FVulkanSubmitRenderPassTask*,
    const FVulkanDispatchComputeTask*,
    const FVulkanDispatchComputeIndirectTask*,
    const FVulkanCopyBufferTask*,
    const FVulkanCopyImageTask*,
    const FVulkanCopyBufferToImageTask*,
    const FVulkanCopyImageToBufferTask*,
    const FVulkanBlitImageTask*,
    const FVulkanResolveImageTask*,
    const FVulkanGenerateMipmapsTask*,
    const FVulkanFillBufferTask*,
    const FVulkanClearColorImageTask*,
    const FVulkanClearDepthStencilImageTask*,
    const FVulkanUpdateBufferTask*,
    // const FVulkanUpdateImageTask*,
    // const FVulkanReadBufferTask*,
    // const FVulkanReadImageTask*,
    const FVulkanPresentTask*,
    const FVulkanCustomTaskTask*,
    const FVulkanUpdateRayTracingShaderTableTask*,
    const FVulkanBuildRayTracingGeometryTask*,
    const FVulkanBuildRayTracingSceneTask*,
    const FVulkanTraceRaysTask*
>;
#endif
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API IVulkanFrameTask : public IFrameTask {
public:
    using FDependencies = TFixedSizeStack<PVulkanFrameTask, MaxTaskDependencies>;
    using FProcessFunc = void (*)(void* visitor, const void* data);

    u32 VisitorId() const { return _visitorId; }
    void SetVisitorId(u32 value) { _visitorId = value; }

    EVulkanExecutionOrder ExecutionOrder() const { return _executionOrder; }
    void SetExecutionOrder(EVulkanExecutionOrder value) { _executionOrder = value; }

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

#if USE_PPE_RHIDEBUG
    const FTaskName& TaskName() const { return _taskName; }
    const FLinearColor& DebugColor() const { return _debugColor; }
    virtual FVulkanFrameTaskRef DebugRef() const NOEXCEPT = 0;
#endif

    static void CopyDescriptorSets(
        FVulkanPipelineResourceSet* outResources,
        FVulkanLogicalRenderPass* pRenderPass,
        FVulkanCommandBuffer& cmd,
        const FPipelineResourceSet& input );

protected:
    IVulkanFrameTask() NOEXCEPT;
    ~IVulkanFrameTask();

    template <typename T>
    IVulkanFrameTask(const details::TFrameTaskDesc<T>& desc, FProcessFunc process) NOEXCEPT;

    FProcessFunc _process{ nullptr };

    u32 _visitorId{ 0 };
    EVulkanExecutionOrder _executionOrder{ EVulkanExecutionOrder::Initial };

    FDependencies _inputs;
    FDependencies _outputs;

#if USE_PPE_RHIDEBUG
    FTaskName _taskName;
    FLinearColor _debugColor{ FLinearColor::PaperWhite };
#endif
};
//----------------------------------------------------------------------------
// FSubmitRenderPass:
//----------------------------------------------------------------------------
template <>
class PPE_RHIVULKAN_API TVulkanFrameTask<FSubmitRenderPass> final : public IVulkanFrameTask {
public:
    TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FSubmitRenderPass& desc, FProcessFunc process) NOEXCEPT;
    ~TVulkanFrameTask();

    bool Valid() const { return (!!_logicalPass); }

    bool IsSubpass() const { return (!!_prevSubpass); }
    bool IsLastPass() const { return (!_nextSubpass); }

    FVulkanLogicalRenderPass* LogicalPass() const { return _logicalPass; }

    TVulkanFrameTask* PrevSubpass() const { return _prevSubpass; }
    TVulkanFrameTask* NextSubpass() const { return _nextSubpass; }

#if USE_PPE_RHIDEBUG
    virtual FVulkanFrameTaskRef DebugRef() const NOEXCEPT override final { return this; }
#endif

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
    const TPtrRef<const FVulkanComputePipeline> Pipeline;
    const FPushConstantDatas PushConstants;

    const FDispatchCompute::FComputeCommands Commands;
    const Meta::TOptional<uint3> LocalGroupSize;

#if USE_PPE_RHIDEBUG
    EShaderDebugIndex DebugModeIndex{ Default };
    virtual FVulkanFrameTaskRef DebugRef() const NOEXCEPT override final { return this; }
#endif

    TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FDispatchCompute& desc, FProcessFunc process) NOEXCEPT;
    ~TVulkanFrameTask();

    bool Valid() const { return (!!Pipeline); }

    const FVulkanPipelineResourceSet& Resources() const { return _resources; }

private:
    FVulkanPipelineResourceSet _resources;
};
//----------------------------------------------------------------------------
// FDispatchComputeIndirect:
//----------------------------------------------------------------------------
template <>
class PPE_RHIVULKAN_API TVulkanFrameTask<FDispatchComputeIndirect> final : public IVulkanFrameTask {
public:
    const TPtrRef<const FVulkanComputePipeline> Pipeline;
    const FPushConstantDatas PushConstants;

    const FDispatchComputeIndirect::FComputeCommands Commands;
    const Meta::TOptional<uint3> LocalGroupSize;

    const FVulkanLocalBuffer* const IndirectBuffer;

#if USE_PPE_RHIDEBUG
    EShaderDebugIndex DebugModeIndex{ Default };
    virtual FVulkanFrameTaskRef DebugRef() const NOEXCEPT override final { return this; }
#endif

    TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FDispatchComputeIndirect& desc, FProcessFunc process) NOEXCEPT;
    ~TVulkanFrameTask();

    bool Valid() const { return (!!Pipeline && !!IndirectBuffer); }

    const FVulkanPipelineResourceSet& Resources() const { return _resources; }

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

    TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FCopyBuffer& desc, FProcessFunc process) NOEXCEPT;
    ~TVulkanFrameTask();

    bool Valid() const { return (!!SrcBuffer && !!DstBuffer && not Regions.empty()); }

#if USE_PPE_RHIDEBUG
    virtual FVulkanFrameTaskRef DebugRef() const NOEXCEPT override final { return this; }
#endif
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

    TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FCopyImage& desc, FProcessFunc process) NOEXCEPT;
    ~TVulkanFrameTask();

    bool Valid() const { return (!!SrcImage && !!DstImage && not Regions.empty()); }

#if USE_PPE_RHIDEBUG
    virtual FVulkanFrameTaskRef DebugRef() const NOEXCEPT override final { return this; }
#endif
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

    TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FCopyBufferToImage& desc, FProcessFunc process) NOEXCEPT;
    ~TVulkanFrameTask();

    bool Valid() const { return (!!SrcBuffer && !!DstImage && not Regions.empty()); }

#if USE_PPE_RHIDEBUG
    virtual FVulkanFrameTaskRef DebugRef() const NOEXCEPT override final { return this; }
#endif
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

    TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FCopyImageToBuffer& desc, FProcessFunc process) NOEXCEPT;
    ~TVulkanFrameTask();

    bool Valid() const { return (!!SrcImage && !!DstBuffer && not Regions.empty()); }

#if USE_PPE_RHIDEBUG
    virtual FVulkanFrameTaskRef DebugRef() const NOEXCEPT override final { return this; }
#endif
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

    TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FBlitImage& desc, FProcessFunc process) NOEXCEPT;
    ~TVulkanFrameTask();

    bool Valid() const { return (!!SrcImage && !!DstImage && not Regions.empty()); }

#if USE_PPE_RHIDEBUG
    virtual FVulkanFrameTaskRef DebugRef() const NOEXCEPT override final { return this; }
#endif
};
//----------------------------------------------------------------------------
// FGenerateMipmaps:
//----------------------------------------------------------------------------
template <>
class PPE_RHIVULKAN_API TVulkanFrameTask<FGenerateMipmaps> final : public IVulkanFrameTask {
public:
    const FVulkanLocalImage* const Image;
    const u32 BaseMipLevel;
    const u32 LevelCount;
    const u32 BaseLayer;
    const u32 LayerCount;

    TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FGenerateMipmaps& desc, FProcessFunc process) NOEXCEPT;
    ~TVulkanFrameTask();

    bool Valid() const { return (!!Image && LevelCount > 0); }

#if USE_PPE_RHIDEBUG
    virtual FVulkanFrameTaskRef DebugRef() const NOEXCEPT override final { return this; }
#endif
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

    TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FResolveImage& desc, FProcessFunc process) NOEXCEPT;
    ~TVulkanFrameTask();

    bool Valid() const { return (!!SrcImage && !!DstImage && not Regions.empty()); }

#if USE_PPE_RHIDEBUG
    virtual FVulkanFrameTaskRef DebugRef() const NOEXCEPT override final { return this; }
#endif
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

    TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FFillBuffer& desc, FProcessFunc process) NOEXCEPT;
    ~TVulkanFrameTask();

    bool Valid() const { return (!!DstBuffer && Size > 0); }

#if USE_PPE_RHIDEBUG
    virtual FVulkanFrameTaskRef DebugRef() const NOEXCEPT override final { return this; }
#endif
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

    TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FClearColorImage& desc, FProcessFunc process) NOEXCEPT;
    ~TVulkanFrameTask();

    bool Valid() const { return (!!DstImage && not Ranges.empty()); }

#if USE_PPE_RHIDEBUG
    virtual FVulkanFrameTaskRef DebugRef() const NOEXCEPT override final { return this; }
#endif
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

    TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FClearDepthStencilImage& desc, FProcessFunc process) NOEXCEPT;
    ~TVulkanFrameTask();

    bool Valid() const { return (!!DstImage && not Ranges.empty()); }

#if USE_PPE_RHIDEBUG
    virtual FVulkanFrameTaskRef DebugRef() const NOEXCEPT override final { return this; }
#endif
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

    TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FUpdateBuffer& desc, FProcessFunc process) NOEXCEPT;
    ~TVulkanFrameTask();

    bool Valid() const { return (!!DstBuffer && not Regions.empty()); }

#if USE_PPE_RHIDEBUG
    virtual FVulkanFrameTaskRef DebugRef() const NOEXCEPT override final { return this; }
#endif
};
//----------------------------------------------------------------------------
// FPresent:
//----------------------------------------------------------------------------
template <>
class PPE_RHIVULKAN_API TVulkanFrameTask<FPresent> final : public IVulkanFrameTask {
public:
    const FVulkanSwapchain* const Swapchain;
    const FVulkanLocalImage* const SrcImage;
    const FImageLayer Layer;
    const FMipmapLevel Mipmap;

    TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FPresent& desc, FProcessFunc process) NOEXCEPT;
    ~TVulkanFrameTask();

    bool Valid() const { return (!!Swapchain && !!SrcImage); }

#if USE_PPE_RHIDEBUG
    virtual FVulkanFrameTaskRef DebugRef() const NOEXCEPT override final { return this; }
#endif
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

    TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FCustomTask& desc, FProcessFunc process) NOEXCEPT;
    ~TVulkanFrameTask();

    bool Valid() const { return (!!Callback); }

#if USE_PPE_RHIDEBUG
    virtual FVulkanFrameTaskRef DebugRef() const NOEXCEPT override final { return this; }
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
