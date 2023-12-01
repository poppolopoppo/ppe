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
    using FPendingResourceBarriers = HASHMAP_SLAB(RHICommand, const void*, FCommitBarrierFunc);

    using FBufferCopyRegions = TFixedSizeStack<VkBufferCopy, MaxCopyRegions>;
    using FImageCopyRegions = TFixedSizeStack<VkImageCopy, MaxCopyRegions>;
    using FBufferImageCopyRegions = TFixedSizeStack<VkBufferImageCopy, MaxCopyRegions>;
    using FBlitRegions = TFixedSizeStack<VkImageBlit, MaxBlitRegions>;
    using FResolveRegions = TFixedSizeStack<VkImageResolve, MaxResolveRegions>;
    using FImageClearRanges = TFixedSizeStack<VkImageSubresourceRange, MaxClearRanges>;

    using FStencilValue = FDrawDynamicStates::FStencilReference;

    FVulkanTaskProcessor(const SVulkanCommandBuffer& workerCmd, VkCommandBuffer vkCommandBuffer) NOEXCEPT;
    ~FVulkanTaskProcessor();

    void Visit(const FVulkanSubmitRenderPassTask& task);
    void Visit(const FVulkanDispatchComputeTask& task);
    void Visit(const FVulkanDispatchComputeIndirectTask& task);
    void Visit(const FVulkanCopyBufferTask& task);
    void Visit(const FVulkanCopyImageTask& task);
    void Visit(const FVulkanCopyBufferToImageTask& task);
    void Visit(const FVulkanCopyImageToBufferTask& task);
    void Visit(const FVulkanBlitImageTask& task);
    void Visit(const FVulkanResolveImageTask& task);
    void Visit(const FVulkanGenerateMipmapsTask& task);
    void Visit(const FVulkanFillBufferTask& task);
    void Visit(const FVulkanClearColorImageTask& task);
    void Visit(const FVulkanClearDepthStencilImageTask& task);
    void Visit(const FVulkanUpdateBufferTask& task);
    void Visit(const FVulkanPresentTask& task);
    void Visit(const FVulkanUpdateRayTracingShaderTableTask& task);
    void Visit(const FVulkanBuildRayTracingGeometryTask& task);
    void Visit(const FVulkanBuildRayTracingSceneTask& task);
    void Visit(const FVulkanTraceRaysTask& task);
    void Visit(const FVulkanCustomTaskTask& task);

    static void Visit1_DrawVertices(void* visitor, void* data);
    static void Visit2_DrawVertices(void* visitor, void* data);
    static void Visit1_DrawIndexed(void* visitor, void* data);
    static void Visit2_DrawIndexed(void* visitor, void* data);
    static void Visit1_DrawMeshes(void* visitor, void* data);
    static void Visit2_DrawMeshes(void* visitor, void* data);
    static void Visit1_DrawVerticesIndirect(void* visitor, void* data);
    static void Visit2_DrawVerticesIndirect(void* visitor, void* data);
    static void Visit1_DrawIndexedIndirect(void* visitor, void* data);
    static void Visit2_DrawIndexedIndirect(void* visitor, void* data);
    static void Visit1_DrawMeshesIndirect(void* visitor, void* data);
    static void Visit2_DrawMeshesIndirect(void* visitor, void* data);
    static void Visit1_DrawVerticesIndirectCount(void* visitor, void* data);
    static void Visit2_DrawVerticesIndirectCount(void* visitor, void* data);
    static void Visit1_DrawIndexedIndirectCount(void* visitor, void* data);
    static void Visit2_DrawIndexedIndirectCount(void* visitor, void* data);
    static void Visit1_DrawMeshesIndirectCount(void* visitor, void* data);
    static void Visit2_DrawMeshesIndirectCount(void* visitor, void* data);
    static void Visit1_CustomDraw(void* visitor, void* data);
    static void Visit2_CustomDraw(void* visitor, void* data);

    void Run(const PVulkanFrameTask&);

private:
    template <u32 _Uid>
    NODISCARD const auto* ToLocal_(details::TResourceId<_Uid>) const;
    template <u32 _Uid>
    NODISCARD const auto* Resource_(details::TResourceId<_Uid>) const;

    void CommitBarriers_();

    void AddRenderTargetBarriers_(const FVulkanLogicalRenderPass& rp, const FDrawTaskBarriers& info);
    void SetShadingRateImage_(VkImageView* pView, const FVulkanLogicalRenderPass& rp);
    void BeginRenderPass_(const TVulkanFrameTask<FSubmitRenderPass>& task);
    void BeginSubpass_(const TVulkanFrameTask<FSubmitRenderPass>& task);
    NODISCARD bool CreateRenderPass_(TMemoryView<FVulkanLogicalRenderPass* const> passes ARGS_IF_RHIDEBUG(FConstChar debugName));

    void ExtractDescriptorSets_(FVulkanDescriptorSets* pDescriptorSets, const FVulkanPipelineLayout& layout, TMemoryView<u32> dynamicOffsets, TMemoryView<const FVulkanPipelineResourceSet::FResource> resources);
    void BindPipelineResources_(const FVulkanPipelineLayout& layout, TMemoryView<u32> dynamicOffsets, TMemoryView<const FVulkanPipelineResourceSet::FResource> resources, VkPipelineBindPoint bindPoint ARGS_IF_RHIDEBUG(EShaderDebugIndex debugModeIndex));
    NODISCARD bool BindPipeline_(FVulkanPipelineLayout const** pPplnLayout, const FVulkanLogicalRenderPass& rp, const details::FVulkanBaseDrawVerticesTask& task);
    NODISCARD bool BindPipeline_(FVulkanPipelineLayout const** pPplnLayout, const FVulkanLogicalRenderPass& rp, const details::FVulkanBaseDrawMeshesTask& task);
    NODISCARD bool BindPipeline_(FVulkanPipelineLayout const** pPplnLayout, const FVulkanComputePipeline& compute, const Meta::TOptional<uint3>& localSize, VkPipelineCreateFlags flags ARGS_IF_RHIDEBUG(EShaderDebugIndex debugModeIndex));
    void BindPipelinePerPassStates_(const FVulkanLogicalRenderPass& rp, VkPipeline vkPipeline);

    void PushContants_(const FVulkanPipelineLayout& layout, const FPushConstantDatas& pushConstants);
    void SetScissor_(const FVulkanLogicalRenderPass& rp, const TMemoryView<const FRectangleU>& rects);
    void SetDynamicStates_(const FDrawDynamicStates& states);
    void BindIndexBuffer_(VkBuffer indexBuffer, VkDeviceSize indexOffset, VkIndexType indexType);
    void BindShadingRateImage_(VkImageView view);

    void AddImage_(const FVulkanLocalImage* pLocalImage, EResourceState state, VkImageLayout layout, const FImageViewDesc& desc);
    void AddImage_(const FVulkanLocalImage* pLocalImage, EResourceState state, VkImageLayout layout, const VkImageSubresourceLayers& subRes);
    void AddImage_(const FVulkanLocalImage* pLocalImage, EResourceState state, VkImageLayout layout, const VkImageSubresourceRange& subRes);
    void AddImageState_(const FVulkanLocalImage* pLocalImage, FImageState&& rstate);

    void AddBuffer_(const FVulkanLocalBuffer* pLocalBuffer, EResourceState state, VkDeviceSize offset, VkDeviceSize size);
    void AddBuffer_(const FVulkanLocalBuffer* pLocalBuffer, EResourceState state, const VkBufferImageCopy& region, const FVulkanLocalImage* pLocalImage);
    void AddBufferState_(const FVulkanLocalBuffer* pLocalBuffer, FBufferState&& rstate);

    void AddRTScene_(const FVulkanRayTracingLocalScene* scene, EResourceState state);
    void AddRTGeometry_(const FVulkanRayTracingLocalGeometry* pLocalGeom, EResourceState state);

#if USE_PPE_RHIDEBUG
    using FStatistics = FFrameStatistics::FRendering;

    FLinearColor _debugColor;

    template <typename _Functor>
    void EditStatistics_(_Functor&& rendering) const;

    void CmdDebugMarker_(FConstChar text, const FColor& color) const;
    void CmdDebugMarker_(FConstChar text, const FLinearColor& color) const;
    void CmdPushDebugGroup_(FConstChar text, const FColor& color) const;
    void CmdPushDebugGroup_(FConstChar text, const FLinearColor& color) const;
    void CmdPopDebugGroup_() const;
#endif

    SVulkanCommandBuffer _workerCmd;
    const VkCommandBuffer _vkCommandBuffer;

    PVulkanFrameTask _currentTask{ nullptr };
    bool _enableDebugUtils              : 1;
    bool _isDefaultScissor              : 1;
    bool _perPassStatesUpdated          : 1;
    const bool _enableDispatchBase      : 1;
    const bool _enableDrawIndirectCount : 1;
    const bool _enableMeshShaderNV      : 1;
    const bool _enableRayTracingNV      : 1;
    const bool _enableRayTracingKHR     : 1;

    const u32 _maxDrawIndirectCount;
    const u32 _maxDrawMeshTaskCount;

    FPendingResourceBarriers _pendingResourceBarriers;

    FPipelineState _graphicsPipeline;
    FPipelineState _computePipeline;
    FPipelineState _rayTracingPipeline;

    VkBuffer _indexBuffer{ VK_NULL_HANDLE };
    VkDeviceSize _indexBufferOffset{ UMax };
    VkIndexType _indexType{ VK_INDEX_TYPE_MAX_ENUM };

    VkImageView _shadingRateImage{ VK_NULL_HANDLE };

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
