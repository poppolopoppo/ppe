#pragma once

#include "Vulkan/VulkanCommon.h"

#include "Vulkan/Buffer/VulkanLocalBuffer.h"
#include "Vulkan/Command/VulkanDrawTask.h"
#include "Vulkan/Image/VulkanLocalImage.h"
#include "Vulkan/RayTracing/VulkanRayTracingLocalGeometry.h"
#include "Vulkan/RayTracing/VulkanRayTracingLocalScene.h"

#include "Allocator/SlabAllocator.h"
#include "Container/HashMap.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanTaskProcessor final : public FVulkanDeviceFunctions {
public:
    class FDrawContext;
    class FDrawTaskBarriers;
    class FDrawTaskCommands;
    class FPipelineResourceBarriers;

    using FBufferRange = FVulkanLocalBuffer::FBufferRange;
    using FImageRange = FVulkanLocalImage::FImageRange;
    using FBufferState = FVulkanLocalBuffer::FBufferState;
    using FImageState = FVulkanLocalImage::FImageState;
    using FRTSceneState = FVulkanRayTracingLocalScene::FSceneState;
    using FRTGeometryState = FVulkanRayTracingLocalGeometry::FGeometryState;

    struct FPipelineState {
        VkPipeline Pipeline{ VK_NULL_HANDLE };
    };

    using FCommitBarrierFunc = void (*)(const void*, FVulkanBarrierManager& ARGS_IF_RHIDEBUG(FVulkanLocalDebugger*));
    using FPendingResourceBarriers = HASHMAP_SLAB(const void*, FCommitBarrierFunc);

    using FBufferCopyRegions = TFixedSizeStack<VkBufferCopy, MaxCopyRegions>;
    using FImageCopyRegions = TFixedSizeStack<VkImageCopy, MaxCopyRegions>;
    using FBufferImageCopyRegions = TFixedSizeStack<VkBufferImageCopy, MaxCopyRegions>;
    using FBlitRegions = TFixedSizeStack<VkImageBlit, MaxBlitRegions>;
    using FResolveRegions = TFixedSizeStack<VkImageResolve, MaxResolveRegions>;
    using FImageClearRanges = TFixedSizeStack<VkImageSubresourceRange, MaxClearRanges>;

    using FStencilValue = FDrawDynamicStates::FStencilReference;

#if USE_PPE_RHIDEBUG
    using FStatistics = FFrameStatistics::FRendering;
#endif

    FVulkanTaskProcessor(const SVulkanCommandBuffer& fgThread, VkCommandBuffer vkCmdBuffer) NOEXCEPT;
    ~FVulkanTaskProcessor();

    void Visit(const FVulkanSubmitRenderPassTask&);
    void Visit(const FVulkanDispatchComputeTask&);
    void Visit(const FVulkanDispatchComputeIndirectTask&);
    void Visit(const FVulkanCopyBufferTask&);
    void Visit(const FVulkanCopyImageTask&);
    void Visit(const FVulkanCopyBufferToImageTask&);
    void Visit(const FVulkanCopyImageToBufferTask&);
    void Visit(const FVulkanBlitImageTask&);
    void Visit(const FVulkanResolveImageTask&);
    void Visit(const FVulkanGenerateMipmapsTask&);
    void Visit(const FVulkanFillBufferTask&);
    void Visit(const FVulkanClearColorImageTask&);
    void Visit(const FVulkanClearDepthStencilImageTask&);
    void Visit(const FVulkanUpdateBufferTask&);
    void Visit(const FVulkanPresentTask&);
    void Visit(const FVulkanUpdateRayTracingShaderTableTask&);
    void Visit(const FVulkanBuildRayTracingGeometryTask&);
    void Visit(const FVulkanBuildRayTracingSceneTask&);
    void Visit(const FVulkanTraceRaysTask&);
    void Visit(const FVulkanCustomTaskTask&);

    static void Visit1_DrawVertices(void *, void *);
    static void Visit2_DrawVertices(void *, void *);
    static void Visit1_DrawIndexed(void *, void *);
    static void Visit2_DrawIndexed(void *, void *);
    static void Visit1_DrawMeshes(void *, void *);
    static void Visit2_DrawMeshes(void *, void *);
    static void Visit1_DrawVerticesIndirect(void *, void *);
    static void Visit2_DrawVerticesIndirect(void *, void *);
    static void Visit1_DrawIndexedIndirect(void *, void *);
    static void Visit2_DrawIndexedIndirect(void *, void *);
    static void Visit1_DrawMeshesIndirect(void *, void *);
    static void Visit2_DrawMeshesIndirect(void *, void *);
    static void Visit1_DrawVerticesIndirectCount(void *, void *);
    static void Visit2_DrawVerticesIndirectCount(void *, void *);
    static void Visit1_DrawIndexedIndirectCount(void *, void *);
    static void Visit2_DrawIndexedIndirectCount(void *, void *);
    static void Visit1_DrawMeshesIndirectCount(void *, void *);
    static void Visit2_DrawMeshesIndirectCount(void *, void *);
    static void Visit1_CustomDraw(void *, void *);
    static void Visit2_CustomDraw(void *, void *);

    void Run(const PVulkanFrameTask&);

private:
    template <u32 _Uid>
    NODISCARD const auto* ToLocal_(details::TResourceId<_Uid>) const;
    template <u32 _Uid>
    NODISCARD const auto* Resource_(details::TResourceId<_Uid>) const;

    void CommitBarriers_();

    void AddRenderTargetBarriers_(const FVulkanLogicalRenderPass& rp, const FDrawTaskBarriers& info);
    void SetShadingRateImage_(VkImageView* pview, const FVulkanLogicalRenderPass& rp);
    void BeginRenderPass_(const TVulkanFrameTask<FSubmitRenderPass>& task);
    void BeginSubpass_(const TVulkanFrameTask<FSubmitRenderPass>& task);
    bool CreateRenderPass_(TMemoryView<FVulkanLogicalRenderPass* const> passes);

    void ExtractDescriptorSets_(FVulkanDescriptorSets* pdsets, const FVulkanPipelineLayout& layout, const FVulkanPipelineResourceSet& resourceSet);
    void BindPipelineResources_(const FVulkanPipelineLayout& layout, const FVulkanPipelineResourceSet& resourceSet, VkPipelineBindPoint bindPoint ARGS_IF_RHIDEBUG(EShaderDebugIndex debugModeIndex));
    bool BindPipeline_(FVulkanPipelineLayout* ppplnLayout, const FVulkanLogicalRenderPass& rp, const details::FVulkanBaseDrawVerticesTask& task);
    bool BindPipeline_(FVulkanPipelineLayout* ppplnLayout, const FVulkanLogicalRenderPass& rp, const details::FVulkanBaseDrawMeshesTask& task);
    bool BindPipeline_(FVulkanPipelineLayout* ppplnLayout, const FVulkanComputePipeline& compute, const Meta::TOptional<uint3>& localSize, VkPipelineCreateFlags flags ARGS_IF_RHIDEBUG(EShaderDebugIndex debugModeIndex));
    void BindPipeline2_(const FVulkanLogicalRenderPass& rp, VkPipeline vkPipeline);

    void PushContants_(const FVulkanPipelineLayout& layout, const FPushConstantDatas& data);
    void SetScissor_(const FVulkanLogicalRenderPass& rp, const TMemoryView<const FRectangleI>& rects);
    void SetDynamicStates_(const FDrawDynamicStates& states);
    void BindIndexBuffer_(VkBuffer indexBuf, VkDeviceSize indexOffset, VkIndexType indexType);
    void BindShadingRateImage_(VkImageView view);
    void ResetDrawContext_();

    void AddImage_(const FVulkanLocalImage* img, EResourceState state, VkImageLayout layout, const FImageViewDesc& desc);
    void AddImage_(const FVulkanLocalImage* img, EResourceState state, VkImageLayout layout, const VkImageSubresourceLayers& subRes);
    void AddImage_(const FVulkanLocalImage* img, EResourceState state, VkImageLayout layout, const VkImageSubresourceRange& subRes);
    void AddImageState_(const FVulkanLocalImage* img, const FImageState& state);

    void AddBuffer_(const FVulkanLocalBuffer* buf, EResourceState state, VkDeviceSize offset, VkDeviceSize size);
    void AddBuffer_(const FVulkanLocalBuffer* buf, EResourceState state, const VkBufferImageCopy& region, const FVulkanLocalImage* img);
    void AddBufferState_(const FVulkanLocalBuffer* buf, const FBufferState& state);

    void AddRTScene_(const FVulkanRayTracingLocalScene* scene, EResourceState state);
    void AddRTGeometry_(const FVulkanRayTracingLocalGeometry* geom, EResourceState state);

#if USE_PPE_RHIDEBUG
    void CmdDebugMarker_(FStringView text) const;
    void CmdPushDebugGroup_(FStringView text) const;
    void CmdPopDebugGroup_() const;

    FStatistics& Stats() const;
#endif

    SVulkanCommandBuffer _fgThread;
    const VkCommandBuffer _vkCmdBuffer;

    PVulkanFrameTask _currentTask{ nullptr };
    bool _enableDebugUtils : 1;
    bool _isDefaultScissor : 1;
    bool _perPassStatesUpdated : 1;

    FPendingResourceBarriers _pendingResourceBarriers;

    FPipelineState _graphicsPipeline;
    FPipelineState _computePipeline;
    FPipelineState _rayTracingPipeline;

    VkBuffer _indexBuffer{ VK_NULL_HANDLE };
    VkDeviceSize _indexBufferOffset{ UMax };
    VkIndexType _indexType{ VK_INDEX_TYPE_MAX_ENUM };

    VkImageView _shadingRateImage{ VK_NULL_HANDLE };

#if USE_PPE_RHIDEBUG
    static constexpr float DebugColor_[4] = {  1, 1, 1, 1 };
#endif

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
