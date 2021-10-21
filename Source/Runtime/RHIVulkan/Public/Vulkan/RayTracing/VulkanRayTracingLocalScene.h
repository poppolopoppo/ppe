#pragma once

#include "Vulkan/VulkanCommon.h"

#include "Vulkan/RayTracing/VulkanRayTracingScene.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanRayTracingLocalScene final : Meta::FNonCopyableNorMovable {
public:
    struct FSceneState {
        EResourceState State;
        PVulkanFrameTask Task;
    };

    struct FSceneAccess {
        VkPipelineStageFlags Stages{ 0 };
        VkAccessFlags Access{ 0 };
        EVulkanExecutionOrder Index{ EVulkanExecutionOrder::Initial };
        bool IsReadable : 1;
        bool IsWritable : 1;

        FSceneAccess() NOEXCEPT : IsReadable(false), IsWritable(false) {}

        bool Valid() const { return !!(IsReadable | IsWritable); }
    };

    using FAccessRecords = TFixedSizeStack<FSceneAccess, 16>;
    using FInstance = FVulkanRayTracingScene::FInstance;

    FVulkanRayTracingLocalScene() = default;
    ~FVulkanRayTracingLocalScene();

    NODISCARD bool Construct(const FVulkanRayTracingScene* sceneData);
    void TearDown();

    void AddPendingState(const FSceneState& state) const;
    void CommitBarrier(FVulkanBarrierManager& barriers ARGS_IF_RHIDEBUG(FVulkanLocalDebugger* debuggerIFP = Default)) const;
    void ResetState(EVulkanExecutionOrder index, FVulkanBarrierManager& barriers ARGS_IF_RHIDEBUG(FVulkanLocalDebugger* debuggerIFP = Default));

    const FVulkanRayTracingScene* GlobalData() const { return _rtScene; }
    auto InternalData() const { return _rtScene->Read(); }

    VkAccelerationStructureNV Handle() const { return _rtScene->Handle(); }
    ERayTracingBuildFlags Flags() const { return _rtScene->Flags(); }
    u32 MaxInstanceCount() const { return _rtScene->MaxInstanceCount(); }

#if USE_PPE_RHIDEBUG
    const FVulkanDebugName& DebugName() const { Assert(_rtScene); return _rtScene->DebugName(); }
#endif

private:
    const FVulkanRayTracingScene* _rtScene;

    mutable FSceneAccess _pendingAccesses;
    mutable FSceneAccess _accessForReadWrite;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
