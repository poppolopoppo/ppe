#pragma once

#include "Vulkan/VulkanCommon.h"

#if USE_PPE_RHIDEBUG

#include "Vulkan/Buffer/VulkanLocalBuffer.h"
#include "Vulkan/Image/VulkanLocalImage.h"
#include "Vulkan/RayTracing/VulkanRayTracingLocalGeometry.h"
#include "Vulkan/RayTracing/VulkanRayTracingLocalScene.h"

#include "Container/HashMap.h"
#include "Container/StringHashSet.h"
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

    using FTaskMap = VECTOR(RHIDebug, FTaskInfo);

    struct FBatchGraph {
        FString Body;
        FString FirstNode;
        FString LastNode;
    };

    FVulkanLocalDebugger();

    void Begin(EDebugFlags flags);
    void End(
        FStringBuilder* pdump,
        FBatchGraph* pgraph,
        const FVulkanFrameGraph& fg,
        FStringView name, u32 cmdBufferUID,
        TMemoryView<const SVulkanCommandBatch> dependencies );

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
    FString TaskName_(EVulkanExecutionOrder idx) const;
    static FString TaskName_(const PVulkanFrameTask& task);
    static FStringLiteral QueueName_(const FVulkanDevice& device, u32 queueFamilyIndex);

    struct FRawTextDump_;
    struct FGraphVizDump_;

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

#endif //!USE_PPE_RHIDEBUG
