#pragma once

#include "Vulkan/VulkanCommon.h"

#include "Vulkan/Buffer/VulkanLocalBuffer.h"
#include "Vulkan/Image/VulkanLocalImage.h"
#include "Vulkan/RayTracing/VulkanRayTracingLocalGeometry.h"
#include "Vulkan/RayTracing/VulkanRayTracingLocalScene.h"

#include "Container/HashMap.h"
#include "Container/StringHashSet.h"
#include "Container/SparseArray.h"
#include "Container/Vector.h"

#include <variant>

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanLocalDebugger final {
public:
    template <typename _Barrier>
    struct TBarrier {
        _Barrier Info{ };
        EVulkanExecutionOrder SrcIndex{ EVulkanExecutionOrder::Initial };
        EVulkanExecutionOrder DstIndex{ EVulkanExecutionOrder::Initial };
        VkPipelineStageFlags SrcStageMask{ 0 };
        VkPipelineStageFlags DstStageMask{ 0 };
        VkDependencyFlags DependencyFlags{ 0 };
    };

    template <typename _Barrier>
    struct TResourceInfo {
        VECTORINSITU(RHIDebug, TBarrier<_Barrier>, 2) Barriers;
    };

    using FImageInfo = TResourceInfo<VkImageMemoryBarrier>;
    using FBufferInfo = TResourceInfo<VkBufferMemoryBarrier>;
    using FRTSceneInfo = TResourceInfo<VkMemoryBarrier>;
    using FRTGeometryInfo = TResourceInfo<VkMemoryBarrier>;

    using FImageUsage = TPair<const FVulkanImage*, FVulkanLocalImage::FImageState>;
    using FBufferUsage = TPair<const FVulkanBuffer*, FVulkanLocalBuffer::FBufferState>;
    using FRTSceneUsage = TPair<const FVulkanRayTracingScene*, FVulkanRayTracingLocalScene::FSceneState>;
    using FRTGeometryUsage = TPair<const FVulkanRayTracingGeometry*, FVulkanRayTracingLocalGeometry::FGeometryState>;

    using FResourceUsage = std::variant< FImageUsage, FBufferUsage, FRTSceneUsage, FRTGeometryUsage >;

    using FImageResources = HASHMAP(RHIDebug, const FVulkanImage*, FImageInfo);
    using FBufferResources = HASHMAP(RHIDebug, const FVulkanBuffer*, FBufferInfo);
    using FRTSceneResources = HASHMAP(RHIDebug, const FVulkanRayTracingScene*, FRTSceneInfo);
    using FRTGeometryResources = HASHMAP(RHIDebug, const FVulkanRayTracingGeometry*, FRTGeometryInfo);

    struct FTaskInfo {
        mutable FString AnyNode;
        PVulkanFrameTask Task{ nullptr };
        VECTORINSITU(RHIDebug, FResourceUsage, 3) Resources;

        FTaskInfo() = default;
        explicit FTaskInfo(PVulkanFrameTask task) : Task{ task } {}
    };

    using FTaskMap = SPARSEARRAY(RHIDebug, FTaskInfo);

    struct FBatchGraph {
        FString Body;
        FString FirstNode;
        FString LastNode;
    };

    FVulkanLocalDebugger();

    void Begin(EDebugFlags flags);
    void End(FString* pdump, FBatchGraph* pgraph, FStringView name, u32 cmdBufferUID);

    void AddBarrier(
        const FVulkanBuffer* buffer,
        EVulkanExecutionOrder srcIndex,
        EVulkanExecutionOrder dstIndex,
        VkPipelineStageFlags srcStageMask,
        VkPipelineStageFlags dstStageMask,
        VkDependencyFlags dependencyFlags,
        const VkBufferMemoryBarrier& barrier );
    void AddBarrier(
        const FVulkanImage* image,
        EVulkanExecutionOrder srcIndex,
        EVulkanExecutionOrder dstIndex,
        VkPipelineStageFlags srcStageMask,
        VkPipelineStageFlags dstStageMask,
        VkDependencyFlags dependencyFlags,
        const VkImageMemoryBarrier& barrier );
    void AddBarrier(
        const FVulkanRayTracingScene* rtScene,
        EVulkanExecutionOrder srcIndex,
        EVulkanExecutionOrder dstIndex,
        VkPipelineStageFlags srcStageMask,
        VkPipelineStageFlags dstStageMask,
        VkDependencyFlags dependencyFlags,
        const VkMemoryBarrier& barrier );
    void AddBarrier(
        const FVulkanRayTracingGeometry* rtGeometry,
        EVulkanExecutionOrder srcIndex,
        EVulkanExecutionOrder dstIndex,
        VkPipelineStageFlags srcStageMask,
        VkPipelineStageFlags dstStageMask,
        VkDependencyFlags dependencyFlags,
        const VkMemoryBarrier& barrier );

    void AddHostReadAccess(const FVulkanBuffer* buffer, u32 offset, u32 size);
    void AddHostWriteAccess(const FVulkanBuffer* buffer, u32 offset, u32 size);

    void AddUsage(const FVulkanBuffer* buffer, const FVulkanLocalBuffer::FBufferState& state);
    void AddUsage(const FVulkanImage* image, const FVulkanLocalImage::FImageState& state);
    void AddUsage(const FVulkanRayTracingScene* rtScene, const FVulkanRayTracingLocalScene::FSceneState& state);
    void AddUsage(const FVulkanRayTracingGeometry* rtGeometry, const FVulkanRayTracingLocalGeometry::FGeometryState& state);

    void AddTask(PVulkanFrameTask task);

private:
    PVulkanFrameTask Task_(EVulkanExecutionOrder idx) const;
    FStringView TaskName_(EVulkanExecutionOrder idx) const;
    FStringView TaskName_(PVulkanFrameTask task) const;

    void DumpFrame_(FStringBuilder* pout, FStringView name) const;

    void DumpImages_(FStringBuilder* pout) const;
    void DumpImageInfo_(FStringBuilder* pout, const FVulkanImage* image, const FImageInfo& info) const;

    void DumpBuffers_(FStringBuilder* pout) const;
    void DumpBufferInfo_(FStringBuilder* pout, const FVulkanBuffer* buffer, const FBufferInfo& info) const;

    void DumpQueue_(FStringBuilder* pout, const FTaskMap& tasks) const;
    void DumpResourceUsage_(FStringBuilder* pout, const TMemoryView<const FResourceUsage>& usages) const;
    void DumpTaskData_(FStringBuilder* pout, PVulkanFrameTask task) const;

    void DumpTaskInfo_(FStringBuilder* pout, const FVulkanSubmitRenderPassTask& task) const;
    void DumpTaskInfo_(FStringBuilder* pout, const FVulkanDispatchComputeTask& task) const;
    void DumpTaskInfo_(FStringBuilder* pout, const FVulkanDispatchComputeIndirectTask& task) const;
    void DumpTaskInfo_(FStringBuilder* pout, const FVulkanCopyBufferTask& task) const;
    void DumpTaskInfo_(FStringBuilder* pout, const FVulkanCopyImageTask& task) const;
    void DumpTaskInfo_(FStringBuilder* pout, const FVulkanCopyBufferToImageTask& task) const;
    void DumpTaskInfo_(FStringBuilder* pout, const FVulkanCopyImageToBufferTask& task) const;
    void DumpTaskInfo_(FStringBuilder* pout, const FVulkanBlitImageTask& task) const;
    void DumpTaskInfo_(FStringBuilder* pout, const FVulkanResolveImageTask& task) const;
    void DumpTaskInfo_(FStringBuilder* pout, const FVulkanGenerateMipmapsTask& task) const;
    void DumpTaskInfo_(FStringBuilder* pout, const FVulkanFillBufferTask& task) const;
    void DumpTaskInfo_(FStringBuilder* pout, const FVulkanClearColorImageTask& task) const;
    void DumpTaskInfo_(FStringBuilder* pout, const FVulkanClearDepthStencilImageTask& task) const;
    void DumpTaskInfo_(FStringBuilder* pout, const FVulkanUpdateBufferTask& task) const;
    void DumpTaskInfo_(FStringBuilder* pout, const FVulkanUpdateImageTask& task) const;
    void DumpTaskInfo_(FStringBuilder* pout, const FVulkanReadBufferTask& task) const;
    void DumpTaskInfo_(FStringBuilder* pout, const FVulkanReadImageTask& task) const;
    void DumpTaskInfo_(FStringBuilder* pout, const FVulkanPresentTask& task) const;
    void DumpTaskInfo_(FStringBuilder* pout, const FVulkanCustomTaskTask& task) const;
    void DumpTaskInfo_(FStringBuilder* pout, const FVulkanUpdateRayTracingShaderTableTask& task) const;
    void DumpTaskInfo_(FStringBuilder* pout, const FVulkanBuildRayTracingGeometryTask& task) const;
    void DumpTaskInfo_(FStringBuilder* pout, const FVulkanBuildRayTracingSceneTask& task) const;
    void DumpTaskInfo_(FStringBuilder* pout, const FVulkanTraceRaysTask& task) const;

    FTaskMap _tasks;
    FImageResources _images;
    FBufferResources _buffers;
    FRTSceneResources _rtScenes;
    FRTGeometryResources _rtGeometries;
    mutable STRINGVIEW_HASHSET_MEMOIZE(RHIDebug, ECase::Sensitive) _existingNodes;

    FString _subBatchUID;
    u32 _counter{ 0 };
    EDebugFlags _flags{ Default };
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
