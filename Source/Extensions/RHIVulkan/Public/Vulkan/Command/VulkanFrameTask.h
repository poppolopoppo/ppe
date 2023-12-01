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

    u32 VisitorId{ 0 };
    EVulkanExecutionOrder ExecutionOrder{ EVulkanExecutionOrder::Initial };

    TMemoryView<const PVulkanFrameTask> Inputs() const { return _inputs; }
    TMemoryView<const PVulkanFrameTask> Outputs() const { return _outputs; }

    void Attach(FVulkanCommandBuffer& cmd, const PVulkanFrameTask& output);

    void Process(void* visitor) const {
        Assert(visitor);
        _process(visitor, this);
    }

    static void CopyDescriptorSets(
        FVulkanPipelineResourceSet* outResources,
        FVulkanLogicalRenderPass* pRenderPass,
        FVulkanCommandBuffer& cmd,
        const FPipelineResourceSet& input ) NOEXCEPT;

protected:
    IVulkanFrameTask() NOEXCEPT;
    ~IVulkanFrameTask();

    template <typename T>
    IVulkanFrameTask(FVulkanCommandBuffer& cmd, const details::TFrameTaskDesc<T>& desc, FProcessFunc process) NOEXCEPT;

    FProcessFunc _process{ nullptr };

    TMemoryView<const PVulkanFrameTask> _inputs;
    TMemoryView<PVulkanFrameTask> _outputs;

public:
#if USE_PPE_RHIDEBUG
    FConstChar TaskName;
    FColor DebugColor{ FColor::PaperWhite() };

    virtual FVulkanFrameTaskRef DebugRef() const NOEXCEPT = 0;
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

    TPtrRef<FVulkanLogicalRenderPass> LogicalPass;

    bool Valid() const { return (!!LogicalPass); }

    bool IsSubpass() const { return (!!_prevSubpass); }
    bool IsLastPass() const { return (!_nextSubpass); }

    TVulkanFrameTask* PrevSubpass() const { return _prevSubpass; }
    TVulkanFrameTask* NextSubpass() const { return _nextSubpass; }

#if USE_PPE_RHIDEBUG
    virtual FVulkanFrameTaskRef DebugRef() const NOEXCEPT override final { return this; }
#endif

private:
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

    const Meta::TOptional<uint3> LocalGroupSize;

    TMemoryView<const FPushConstantData> PushConstants;
    TMemoryView<const FDispatchCompute::FComputeCommand> Commands;

    using FResource = FVulkanPipelineResourceSet::FResource;
    TMemoryView<u32> DynamicOffsets;
    TMemoryView<const FResource> Resources;

#if USE_PPE_RHIDEBUG
    EShaderDebugIndex DebugModeIndex{ Default };
    virtual FVulkanFrameTaskRef DebugRef() const NOEXCEPT override final { return this; }
#endif

    TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FDispatchCompute& desc, FProcessFunc process) NOEXCEPT;
    ~TVulkanFrameTask();

    bool Valid() const { return (!!Pipeline); }
};
//----------------------------------------------------------------------------
// FDispatchComputeIndirect:
//----------------------------------------------------------------------------
template <>
class PPE_RHIVULKAN_API TVulkanFrameTask<FDispatchComputeIndirect> final : public IVulkanFrameTask {
public:
    const TPtrRef<const FVulkanComputePipeline> Pipeline;

    const TPtrRef<const FVulkanLocalBuffer> IndirectBuffer;

    const Meta::TOptional<uint3> LocalGroupSize;

    TMemoryView<const FPushConstantData> PushConstants;
    TMemoryView<const FDispatchComputeIndirect::FComputeCommand> Commands;

    using FResource = FVulkanPipelineResourceSet::FResource;
    TMemoryView<u32> DynamicOffsets;
    TMemoryView<const FResource> Resources;

#if USE_PPE_RHIDEBUG
    EShaderDebugIndex DebugModeIndex{ Default };
    virtual FVulkanFrameTaskRef DebugRef() const NOEXCEPT override final { return this; }
#endif

    TVulkanFrameTask(FVulkanCommandBuffer& cmd, const FDispatchComputeIndirect& desc, FProcessFunc process) NOEXCEPT;
    ~TVulkanFrameTask();

    bool Valid() const { return (!!Pipeline && !!IndirectBuffer); }
};
//----------------------------------------------------------------------------
// FCopyBuffer:
//----------------------------------------------------------------------------
template <>
class PPE_RHIVULKAN_API TVulkanFrameTask<FCopyBuffer> final : public IVulkanFrameTask {
public:
    using FRegion = FCopyBuffer::FRegion;

    const TPtrRef<const FVulkanLocalBuffer> SrcBuffer;
    const TPtrRef<const FVulkanLocalBuffer> DstBuffer;

    TMemoryView<const FRegion> Regions;

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

    const TPtrRef<const FVulkanLocalImage> SrcImage;
    const TPtrRef<const FVulkanLocalImage> DstImage;

    const VkImageLayout SrcLayout;
    const VkImageLayout DstLayout;

    TMemoryView<const FRegion> Regions;

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

    const TPtrRef<const FVulkanLocalBuffer> SrcBuffer;
    const TPtrRef<const FVulkanLocalImage> DstImage;

    const VkImageLayout DstLayout;
    TMemoryView<const FRegion> Regions;

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

    const TPtrRef<const FVulkanLocalImage> SrcImage;
    const TPtrRef<const FVulkanLocalBuffer> DstBuffer;

    const VkImageLayout SrcLayout;

    TMemoryView<const FRegion> Regions;

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

    const TPtrRef<const FVulkanLocalImage> SrcImage;
    const TPtrRef<const FVulkanLocalImage> DstImage;

    const VkImageLayout SrcLayout;
    const VkImageLayout DstLayout;

    const VkFilter Filter;
    TMemoryView<const FRegion> Regions;

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
    const TPtrRef<const FVulkanLocalImage> Image;
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

    const TPtrRef<const FVulkanLocalImage> SrcImage;
    const TPtrRef<const FVulkanLocalImage> DstImage;

    const VkImageLayout SrcLayout;
    const VkImageLayout DstLayout;

    TMemoryView<const FRegion> Regions;

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
    const TPtrRef<const FVulkanLocalBuffer> DstBuffer;
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

    const TPtrRef<const FVulkanLocalImage> DstImage;
    const VkImageLayout DstLayout;
    const VkClearColorValue ClearValue;

    TMemoryView<const FRange> Ranges;

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

    const TPtrRef<const FVulkanLocalImage> DstImage;
    const VkImageLayout DstLayout;
    const VkClearDepthStencilValue ClearValue;

    TMemoryView<const FRange> Ranges;

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

    const TPtrRef<const FVulkanLocalBuffer> DstBuffer;
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
    const TPtrRef<const FVulkanSwapchain> Swapchain;
    const TPtrRef<const FVulkanLocalImage> SrcImage;
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
    using FImage = TPair<TPtrRef<const FVulkanLocalImage>, EResourceState>;
    using FBuffer = TPair<TPtrRef<const FVulkanLocalBuffer>, EResourceState>;

    const FCallback Callback;
    TMemoryView<const FImage> Images;
    TMemoryView<const FBuffer> Buffers;

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
