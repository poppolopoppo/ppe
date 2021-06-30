#pragma once

#include "RHIVulkan_fwd.h"

#include "Vulkan/Command/VulkanCommandBatch.h"
#include "Vulkan/Command/VulkanCommandBuffer.h"
#include "Vulkan/Command/VulkanCommandPool.h"
#include "Vulkan/Command/VulkanSubmitted.h"
#include "Vulkan/Command/VulkanTaskGraph.h"
#include "Vulkan/Instance/VulkanDevice.h"
#include "Vulkan/Instance/VulkanResourceManager.h"

#include "RHI/FrameGraph.h"

#include "Container/RingBuffer.h"
#include "Container/Vector.h"
#include "Thread/AtomicPool.h"
#include "Vulkan/Debugger/VulkanDebugger.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanFrameGraph final : public IFrameGraph {
public:
    enum class EState : u32 {
        Initial,
        Initialization,
        Idle,
        Destroyed
    };

    using EBatchState = FVulkanCommandBatch::EState;
    using FPerQueueSemaphore = Meta::TArray<VkSemaphore, static_cast<u32>(EQueueType::_Count)>;

    struct FQueueData {
    // immutable
        PVulkanDeviceQueue Ptr; // pointer to the physical queue
        EQueueType Type{ Default };

    // mutable
        RINGBUFFER(RHIFrameGraph, PVulkanCommandBatch, 32) Pending;
        VECTORINSITU(RHIFrameGraph, FVulkanSubmitted*, 3) Submitted;
        VECTORINSITU(RHIFrameGraph, VkImageMemoryBarrier, 3) ImageBarriers;

        FVulkanCommandPool CommandPool;
        FPerQueueSemaphore Semaphores{};
    };

    using FCommandBufferPool = TAtomicPool<FVulkanCommandBuffer, 4, u32>;
    using FCommandBatchPool = TAtomicPool<FVulkanCommandBatch, 16, u32>;
    using FSubmittedPool = TAtomicPool<FVulkanSubmitted, 8, u32>;
    using FQueueMap = TFixedSizeStack<FQueueData, static_cast<u32>(EQueueType::_Count)>;
    using FFences = VECTOR(RHIFrameGraph, VkFence);
    using FSemaphores = VECTOR(RHIFrameGraph, VkSemaphore);

public:
    explicit FVulkanFrameGraph(const FVulkanDeviceInfo& deviceInfo);
    ~FVulkanFrameGraph() override;

    const FVulkanDevice& Device() const { return _device; }
    FVulkanResourceManager& ResourceManager() { return _resourceManager; }

#if USE_PPE_RHIDEBUG
    VkQueryPool QueryPool() const { return _vkQueryPool; }
#endif

    void RecycleBatch(FVulkanCommandBatch* batch);
    NODISCARD PVulkanDeviceQueue FindQueue(EQueueType queueType) const;

    NODISCARD bool Construct();

    // IFrameGraph

    ETargetRHI TargetRHI() const NOEXCEPT override { return ETargetRHI::Vulkan; }
    void* ExternalDevice() const NOEXCEPT override { return _device.vkDevice(); }

    void TearDown() override;

    void ReleaseMemory() NOEXCEPT override;

    bool AddPipelineCompiler(const PPipelineCompiler& pcompiler) override;
    EQueueUsage AvailableQueues() const NOEXCEPT override { return _queueUsage; }
#if USE_PPE_RHIDEBUG
    bool SetShaderDebugCallback(FShaderDebugCallback&& rcallback) override;
#endif

    NODISCARD FMPipelineID CreatePipeline(FMeshPipelineDesc& desc ARGS_IF_RHIDEBUG(FConstChar debugName)) override;
    NODISCARD FRTPipelineID CreatePipeline(FRayTracingPipelineDesc& desc ARGS_IF_RHIDEBUG(FConstChar debugName)) override;
    NODISCARD FGPipelineID CreatePipeline(FGraphicsPipelineDesc& desc ARGS_IF_RHIDEBUG(FConstChar debugName)) override;
    NODISCARD FCPipelineID CreatePipeline(FComputePipelineDesc& desc ARGS_IF_RHIDEBUG(FConstChar debugName)) override;
    NODISCARD FImageID CreateImage(const FImageDesc& desc, const FMemoryDesc& mem ARGS_IF_RHIDEBUG(FConstChar debugName)) override;
    NODISCARD FImageID CreateImage(const FImageDesc& desc, const FMemoryDesc& mem, EResourceState defaultState ARGS_IF_RHIDEBUG(FConstChar debugName)) override;
    NODISCARD FBufferID CreateBuffer(const FBufferDesc& desc, const FMemoryDesc& mem ARGS_IF_RHIDEBUG(FConstChar debugName)) override;
    NODISCARD FSamplerID CreateSampler(const FSamplerDesc& desc ARGS_IF_RHIDEBUG(FConstChar debugName)) override;
    NODISCARD FSwapchainID CreateSwapchain(const FSwapchainDesc& desc, FRawSwapchainID oldSwapchain ARGS_IF_RHIDEBUG(FConstChar debugName)) override;
    NODISCARD FRTGeometryID CreateRayTracingGeometry(const FRayTracingGeometryDesc& desc, const FMemoryDesc& mem ARGS_IF_RHIDEBUG(FConstChar debugName)) override;
    NODISCARD FRTSceneID CreateRayTracingScene(const FRayTracingSceneDesc& desc, const FMemoryDesc& mem ARGS_IF_RHIDEBUG(FConstChar debugName)) override;
    NODISCARD FRTShaderTableID CreateRayTracingShaderTable(ARG0_IF_RHIDEBUG(const FStringView& name)) override;

    NODISCARD bool InitPipelineResources(FPipelineResources* pResources, FRawGPipelineID pipeline, const FDescriptorSetID& id) const override;
    NODISCARD bool InitPipelineResources(FPipelineResources* pResources, FRawCPipelineID pipeline, const FDescriptorSetID& id) const override;
    NODISCARD bool InitPipelineResources(FPipelineResources* pResources, FRawMPipelineID pipeline, const FDescriptorSetID& id) const override;
    NODISCARD bool InitPipelineResources(FPipelineResources* pResources, FRawRTPipelineID pipeline, const FDescriptorSetID& id) const override;

    NODISCARD bool IsSupported(FRawImageID image, const FImageViewDesc& desc) const NOEXCEPT override;
    NODISCARD bool IsSupported(FRawBufferID buffer, const FBufferViewDesc& desc) const NOEXCEPT override;
    NODISCARD bool IsSupported(const FImageDesc& desc, EMemoryType memType) const NOEXCEPT override;
    NODISCARD bool IsSupported(const FBufferDesc& desc, EMemoryType memType) const NOEXCEPT override;

    NODISCARD bool CachePipelineResources(FPipelineResources& resources) override;

    void ReleaseResource(FPipelineResources& resources) override;
    NODISCARD bool ReleaseResource(FGPipelineID& id) override;
    NODISCARD bool ReleaseResource(FCPipelineID& id) override;
    NODISCARD bool ReleaseResource(FMPipelineID& id) override;
    NODISCARD bool ReleaseResource(FRTPipelineID& id) override;
    NODISCARD bool ReleaseResource(FImageID& id) override;
    NODISCARD bool ReleaseResource(FBufferID& id) override;
    NODISCARD bool ReleaseResource(FSamplerID& id) override;
    NODISCARD bool ReleaseResource(FSwapchainID& id) override;
    NODISCARD bool ReleaseResource(FRTGeometryID& id) override;
    NODISCARD bool ReleaseResource(FRTSceneID& id) override;
    NODISCARD bool ReleaseResource(FRTShaderTableID& id) override;

    NODISCARD const FBufferDesc& Description(FRawBufferID id) const override;
    NODISCARD const FImageDesc& Description(FRawImageID id) const override;

    NODISCARD FImageID CreateImage(
        const FImageDesc& desc,
        FExternalImage externalImage,
        FOnReleaseExternalImage&& onRelease,
        TMemoryView<const u32> queueFamilyIndices,
        EResourceState defaultState
        ARGS_IF_RHIDEBUG(FConstChar debugName)) override;
    NODISCARD FBufferID CreateBuffer(
        const FBufferDesc& desc,
        FExternalBuffer externalBuffer,
        FOnReleaseExternalBuffer&& onRelease,
        TMemoryView<const u32> queueFamilyIndices
        ARGS_IF_RHIDEBUG(FConstChar debugName)) override;

    NODISCARD const void* ExternalDescription(FRawBufferID id) const NOEXCEPT override;
    NODISCARD const void* ExternalDescription(FRawImageID id) const NOEXCEPT override;

    NODISCARD bool IsResourceAlive(FRawGPipelineID id) const NOEXCEPT override { return _resourceManager.IsResourceAlive(id); }
    NODISCARD bool IsResourceAlive(FRawCPipelineID id) const NOEXCEPT override { return _resourceManager.IsResourceAlive(id); }
    NODISCARD bool IsResourceAlive(FRawMPipelineID id) const NOEXCEPT override { return _resourceManager.IsResourceAlive(id); }
    NODISCARD bool IsResourceAlive(FRawRTPipelineID id) const NOEXCEPT override { return _resourceManager.IsResourceAlive(id); }
    NODISCARD bool IsResourceAlive(FRawImageID id) const NOEXCEPT override { return _resourceManager.IsResourceAlive(id); }
    NODISCARD bool IsResourceAlive(FRawBufferID id) const NOEXCEPT override { return _resourceManager.IsResourceAlive(id); }
    NODISCARD bool IsResourceAlive(FRawSwapchainID id) const NOEXCEPT override { return _resourceManager.IsResourceAlive(id); }
    NODISCARD bool IsResourceAlive(FRawRTGeometryID id) const NOEXCEPT override { return _resourceManager.IsResourceAlive(id); }
    NODISCARD bool IsResourceAlive(FRawRTSceneID id) const NOEXCEPT override { return _resourceManager.IsResourceAlive(id); }
    NODISCARD bool IsResourceAlive(FRawRTShaderTableID id) const NOEXCEPT override { return _resourceManager.IsResourceAlive(id); }

    NODISCARD FGPipelineID AcquireResource(FRawGPipelineID id) override { return (_resourceManager.AcquireResource(id) ? FGPipelineID(id) : Default); }
    NODISCARD FCPipelineID AcquireResource(FRawCPipelineID id) override { return (_resourceManager.AcquireResource(id) ? FCPipelineID(id) : Default); }
    NODISCARD FMPipelineID AcquireResource(FRawMPipelineID id) override { return (_resourceManager.AcquireResource(id) ? FMPipelineID(id) : Default); }
    NODISCARD FRTPipelineID AcquireResource(FRawRTPipelineID id) override { return (_resourceManager.AcquireResource(id) ? FRTPipelineID(id) : Default); }
    NODISCARD FImageID AcquireResource(FRawImageID id) override { return (_resourceManager.AcquireResource(id) ? FImageID(id) : Default); }
    NODISCARD FBufferID AcquireResource(FRawBufferID id) override { return (_resourceManager.AcquireResource(id) ? FBufferID(id) : Default); }
    NODISCARD FSwapchainID AcquireResource(FRawSwapchainID id) override { return (_resourceManager.AcquireResource(id) ? FSwapchainID(id) : Default); }
    NODISCARD FRTGeometryID AcquireResource(FRawRTGeometryID id) override { return (_resourceManager.AcquireResource(id) ? FRTGeometryID(id) : Default); }
    NODISCARD FRTSceneID AcquireResource(FRawRTSceneID id) override { return (_resourceManager.AcquireResource(id) ? FRTSceneID(id) : Default); }
    NODISCARD FRTShaderTableID AcquireResource(FRawRTShaderTableID id) override { return (_resourceManager.AcquireResource(id) ? FRTShaderTableID(id) : Default); }

    NODISCARD bool UpdateHostBuffer(FRawBufferID id, size_t offset, size_t size, const void* data) override;
    NODISCARD bool MapBufferRange(FRawBufferID id, size_t offset, size_t& size, void** data) override;

    NODISCARD FCommandBufferBatch Begin(const FCommandBufferDesc& desc, TMemoryView<const FCommandBufferBatch> dependsOn) override;
    NODISCARD bool Execute(FCommandBufferBatch& cmdBatch) override;
    NODISCARD bool Wait(TMemoryView<const FCommandBufferBatch> commands, FNanoseconds timeout) override;
    NODISCARD bool Flush(EQueueUsage queues) override;
    NODISCARD bool WaitIdle() override;

#if USE_PPE_RHIDEBUG
    NODISCARD bool DumpStatistics(FFrameStatistics* pStats) const override;
#endif

private:
    using FCommandBatches = FVulkanSubmitted::FBatches;
    using FPendingSwapchains = TFixedSizeStack<const FVulkanSwapchain*, 16>;
    using FSubmitInfos = TFixedSizeStack<VkSubmitInfo, FVulkanSubmitted::MaxBatches>;
    using FTransientFences = TFixedSizeStack<VkFence, 32>;
    using FTransientSemaphores = FVulkanSubmitted::FSemaphores;
    using FTransientSubmitted = TFixedSizeStack<FVulkanSubmitted*, FVulkanSubmitted::MaxBatches>;

    template <typename _PplnId>
    NODISCARD bool InitPipelineResources_(FPipelineResources* pResources, const _PplnId& pplnId, const FDescriptorSetID& dsId) const;
    template <typename T>
    NODISCARD bool ReleaseResource_(details::TResourceWrappedId<T>& id);

    NODISCARD VkSemaphore CreateSemaphore_();
    void TransitImageLayoutToDefault_(FRawImageID imageId, VkImageLayout initialLayout, u32 queueFamily);

    NODISCARD bool IsInitialized_() const { return (EState::Idle == _state.load(std::memory_order_relaxed)); }
    NODISCARD EState State_() const { return _state.load(std::memory_order_acquire); }
    NODISCARD bool SetState_(EState expected, EState newState) { return _state.compare_exchange_strong(expected, newState,
        std::memory_order_release, std::memory_order_relaxed); }

    NODISCARD bool FlushAll_(EQueueUsage queues, u32 maxIter);
    NODISCARD bool FlushQueue_(EQueueType index, u32 maxIter);

    NODISCARD bool CreateQueue_(EQueueType index, const PVulkanDeviceQueue& queue);
    NODISCARD bool AddGraphicsQueue_();
    NODISCARD bool AddAsyncComputeQueue_();
    NODISCARD bool AddAsyncTransferQueue_();

    NODISCARD bool IsUniqueQueue_(const PVulkanDeviceQueue& queue) const;
    NODISCARD FQueueData& QueueData_(EQueueType index);
    NODISCARD EVulkanQueueFamilyMask QueuesMask_(EQueueUsage types) const;

    std::atomic<EState> _state;

    FVulkanDevice _device;

    FCriticalSection _queueCS;
    FQueueMap _queueMap;
    EQueueUsage _queueUsage;

    FCommandBufferPool _cmdBufferPool;
    FCommandBatchPool _cmdBatchPool;
    FSubmittedPool _submittedPool;

    FVulkanResourceManager _resourceManager;

#if USE_PPE_RHIDEBUG
    FVulkanDebugger _debugger;
    VkQueryPool _vkQueryPool;
    FShaderDebugCallback _shaderDebugCallback;

    mutable FCriticalSection _lastFrameStatsCS;
    mutable FFrameStatistics _lastFrameStats;

    mutable std::atomic<u64> _submittingTime{ 0 };
    mutable std::atomic<u64> _waitingTime{ 0 };
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
