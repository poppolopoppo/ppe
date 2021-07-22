#pragma once

#include "Vulkan/VulkanCommon.h"

#include "Vulkan/Command/VulkanBarrierManager.h"
#include "Vulkan/Command/VulkanCommandBatch.h"
#include "Vulkan/Command/VulkanCommandPool.h"
#include "Vulkan/Command/VulkanTaskGraph.h"
#include "Vulkan/Command/VulkanTaskProcessor.h"
#include "Vulkan/Instance/VulkanResourceManager.h"
#include "Vulkan/Pipeline/VulkanPipelineCache.h"
#include "Vulkan/RenderPass/VulkanLogicalRenderPass.h"

#include "RHI/CommandBuffer.h"

#include "Allocator/SlabHeap.h"
#include "Allocator/SlabAllocator.h"
#include "Container/Array.h"
#include "Container/Vector.h"
#include "Memory/MemoryPool.h"

#include <atomic>

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FVulkanExternalCommandBatch {
    EVulkanQueueFamily QueueFamily{ Default };
    TMemoryView<const VkCommandBuffer> Commands;
    TMemoryView<const VkSemaphore> SignalSemaphores;
    TMemoryView<const TPair<VkSemaphore, VkPipelineStageFlags>> WaitSemaphores;
};
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanCommandBuffer final : public ICommandBuffer {
public:
    enum class EState {
        Initial,
        Recording,
        Compiling,
    };

    using FAllocator = FPoolingSlabHeap;
    using FTransientTaskArray = VECTOR_SLAB(PVulkanFrameTask);
    using FTaskGraph = TVulkanTaskGraph<FVulkanTaskProcessor>;

    using FResourceMap = FVulkanCommandBatch::FResourceMap;
    using FStagingBuffer = FVulkanCommandBatch::FStagingBuffer;

    STATIC_CONST_INTEGRAL(u32, MaxBufferParts, FVulkanCommandBatch::MaxBufferParts);
    STATIC_CONST_INTEGRAL(u32, MaxImageParts, FVulkanCommandBatch::MaxImageParts);
    STATIC_CONST_INTEGRAL(u32, MinBufferPart, 4_KiB);

    using FPerQueuePool = ARRAYINSITU(RHICommand, FVulkanCommandPool, u32(EQueueType::_Count));
    using FResourceIndex = FVulkanResourceManager::FIndex;

    template <typename T, size_t _ChunkSize, size_t _MaxChunks>
    using TPool = TTypedMemoryPool< TResourceProxy<T>, _ChunkSize, _MaxChunks, ALLOCATOR(RHICommand) >;

    using FLogicalRenderPasses = TPool<FVulkanLogicalRenderPass, 1u << 10, 16>;

    template <typename T, typename _GlobalPool, size_t _MaxChunks = 16>
    struct TLocalPool {
        TPool<T, _GlobalPool::MaxSize / _MaxChunks,  _MaxChunks> Pool;
        TStaticArray<FResourceIndex, _GlobalPool::MaxSize> ToLocal;
        u32 MaxLocalIndex{ 0 };
        u32 MaxGlobalIndex{ 0 };
    };

    using FLocalImages = TLocalPool<FVulkanLocalImage, FVulkanResourceManager::FImagePool>;
    using FLocalBuffers = TLocalPool<FVulkanLocalBuffer, FVulkanResourceManager::FBufferPool>;
    using FLocalRTScenes = TLocalPool<FVulkanRayTracingLocalScene, FVulkanResourceManager::FRTScenePool>;
    using FLocalRTGeometries = TLocalPool<FVulkanRayTracingLocalGeometry, FVulkanResourceManager::FRTGeometryPool>;

    struct FInternalData {
        SLABHEAP_POOLED(RHICommand) MainAllocator;
        FTaskGraph TaskGraph;
        SVulkanCommandBatch Batch;
        EState State{ EState::Initial };
        EVulkanQueueFamily QueueIndex{ Default };

        FVulkanBarrierManager BarrierManager;
        FVulkanPipelineCache PipelineCache;

        struct {
            FResourceMap ResourceMap;
            FLocalImages Images;
            FLocalBuffers Buffers;
            FLocalRTScenes RTScenes;
            FLocalRTGeometries RTGeometries;
            FLogicalRenderPasses LogicalRenderPasses;
            u32 LogicalRenderPassCount{ 0 };
        }   RM;

        FPerQueuePool PerQueue; // #TODO: use global command pool manager to minimize memory usage

#if USE_PPE_RHIDEBUG
        FVulkanDebugName DebugName;
        TUniquePtr<FVulkanLocalDebugger> Debugger;
        bool DebugFullBarriers{ false };
        bool DebugQueueSync{ false };

        struct {
            EShaderDebugIndex TimemapIndex{ Default };
            EShaderStages TimemapStages{ Default };
        }   ShaderDbg;
#endif

    };

    FVulkanCommandBuffer(const SVulkanFrameGraph& fg, u32 indexInPool);
    virtual ~FVulkanCommandBuffer() override;

    const FVulkanDevice& Device() const NOEXCEPT;
    FVulkanResourceManager& ResourceManager() const NOEXCEPT;
    FVulkanMemoryManager& MemoryManager() const { return ResourceManager().MemoryManager(); }
    u32 IndexInPool() const { return _indexInPool; }

    auto Read() const { return _data.LockShared(); }
    auto Write() { return _data.LockExclusive(); }

    FAllocator& Allocator() { return Write()->MainAllocator; }
    SVulkanCommandBatch Batch() const { return Read()->Batch; }
    EVulkanQueueFamily QueueFamily() const { return Read()->QueueIndex; }
#if USE_PPE_RHIDEBUG
    const FVulkanDebugName& DebugName() const { return Read()->Batch->DebugName(); }
    FVulkanLocalDebugger* Debugger() const { return Read()->Debugger.get(); }

    template <typename _Functor>
    auto EditStatistics(_Functor&& stats) {
        return stats(Write()->Batch->_statistics);
    }
#endif

    NODISCARD bool Begin(const FCommandBufferDesc& desc, const SVulkanCommandBatch& batch, const PVulkanDeviceQueue& queue);
    NODISCARD bool Execute();

    void SignalSemaphore(VkSemaphore semaphore);
    void WaitSemaphore(VkSemaphore semaphore, VkPipelineStageFlags stage);

    // Resource management

    template <u32 _Uid>
    NODISCARD auto* AcquireTransient(details::TResourceId<_Uid> id);
    template <u32 _Uid>
    void ReleaseResource(details::TResourceId<_Uid> id);

    FVulkanLogicalRenderPass* ToLocal(FLogicalPassID id);
    FVulkanLocalBuffer* ToLocal(FRawBufferID id);
    FVulkanLocalImage* ToLocal(FRawImageID id);
    FVulkanRayTracingLocalScene* ToLocal(FRawRTSceneID id);
    FVulkanRayTracingLocalGeometry* ToLocal(FRawRTGeometryID id);

    const FVulkanPipelineResources* CreateDescriptorSet(const FPipelineResources& desc);

    bool ExternalCommands(const FVulkanExternalCommandBatch& info);

    // ICommandBuffer

    virtual SFrameGraph FrameGraph() const NOEXCEPT override;
    virtual FRawImageID SwapchainImage(FRawSwapchainID swapchainId) override;
    NODISCARD virtual bool DependsOn(const FCommandBufferBatch& cmd) override;
    NODISCARD virtual bool StagingAlloc(FStagingBlock* pStaging, size_t size, size_t align) override;

    virtual void AcquireImage(FRawImageID id, bool makeMutable, bool invalidate) override;
    virtual void AcquireBuffer(FRawBufferID id, bool makeMutable) override;

    virtual PFrameTask Task(const FSubmitRenderPass& task) override;
    virtual PFrameTask Task(const FDispatchCompute& task) override;
    virtual PFrameTask Task(const FDispatchComputeIndirect& task) override;
    virtual PFrameTask Task(const FCopyBuffer& task) override;
    virtual PFrameTask Task(const FCopyImage& task) override;
    virtual PFrameTask Task(const FCopyBufferToImage& task) override;
    virtual PFrameTask Task(const FCopyImageToBuffer& task) override;
    virtual PFrameTask Task(const FBlitImage& task) override;
    virtual PFrameTask Task(const FResolveImage& task) override;
    virtual PFrameTask Task(const FGenerateMipmaps& task) override;
    virtual PFrameTask Task(const FFillBuffer& task) override;
    virtual PFrameTask Task(const FClearColorImage& task) override;
    virtual PFrameTask Task(const FClearDepthStencilImage& task) override;
    virtual PFrameTask Task(const FUpdateBuffer& task) override;
    virtual PFrameTask Task(const FUpdateImage& task) override;
    virtual PFrameTask Task(const FReadBuffer& task) override;
    virtual PFrameTask Task(const FReadImage& task) override;
    virtual PFrameTask Task(const FPresent& task) override;
    virtual PFrameTask Task(const FUpdateRayTracingShaderTable& task) override;
    virtual PFrameTask Task(const FBuildRayTracingGeometry& task) override;
    virtual PFrameTask Task(const FBuildRayTracingScene& task) override;
    virtual PFrameTask Task(const FTraceRays& task) override;
    virtual PFrameTask Task(const FCustomTask& task) override;

    virtual FLogicalPassID CreateRenderPass(const FRenderPassDesc& desc) override;

    virtual void Task(FLogicalPassID renderPass, const FDrawVertices& draw) override;
    virtual void Task(FLogicalPassID renderPass, const FDrawIndexed& draw) override;
    virtual void Task(FLogicalPassID renderPass, const FDrawVerticesIndirect& draw) override;
    virtual void Task(FLogicalPassID renderPass, const FDrawIndexedIndirect& draw) override;
    virtual void Task(FLogicalPassID renderPass, const FDrawVerticesIndirectCount& draw) override;
    virtual void Task(FLogicalPassID renderPass, const FDrawIndexedIndirectCount& draw) override;
    virtual void Task(FLogicalPassID renderPass, const FDrawMeshes& draw) override;
    virtual void Task(FLogicalPassID renderPass, const FDrawMeshesIndirect& draw) override;
    virtual void Task(FLogicalPassID renderPass, const FDrawMeshesIndirectCount& draw) override;
    virtual void Task(FLogicalPassID renderPass, const FCustomDraw& draw) override;

#if USE_PPE_RHIDEBUG
    NODISCARD virtual bool BeginShaderTimeMap(const uint2& dim, EShaderStages stages = EShaderStages::All) override;
    NODISCARD virtual PFrameTask EndShaderTimeMap(FRawImageID dstImage, FImageLayer layer = Default, FMipmapLevel level = Default, TMemoryView<PFrameTask> dependsOn = Default) override;
#endif

private:
    const SVulkanFrameGraph _frameGraph;
    const u32 _indexInPool; // index in FVulkanFrameGraph::_cmdBufferPool

    TRHIThreadSafe<FInternalData> _data{};

    // staging buffer

    template <typename T>
    NODISCARD bool StagingAlloc_(FInternalData& data, const FVulkanLocalBuffer** pBuffer, VkDeviceSize* pOffset, T** pData, size_t count);
    NODISCARD bool StagingStore_(FInternalData& data, const FVulkanLocalBuffer** pBuffer, VkDeviceSize* pOffset, const void* srcData, u32 dataSize, u32 offsetAlign);
    NODISCARD static bool StorePartialData_(FInternalData& data, FStagingBlock* pDstStaging, u32* pOutSize, FRawMemoryConst srcData, u32 srcOffset);
    NODISCARD static bool StagingImageStore_(FInternalData& data, FStagingBlock* pDstStaging, u32* pOutSize, FRawMemoryConst srcData, u32 srcOffset, u32 srcPitch, u32 srcTotalSize);

    NODISCARD PFrameTask MakeUpdateBufferTask_(FInternalData& data, const FUpdateBuffer& task);
    NODISCARD PFrameTask MakeUpdateImageTask_(FInternalData& data, const FUpdateImage& task);
    NODISCARD PFrameTask MakeReadBufferTask_(FInternalData& data, const FReadBuffer& task);
    NODISCARD PFrameTask MakeReadImageTask_(FInternalData& data, const FReadImage& task);

    // task processor

    void AfterCompilation_(FInternalData& data);
    bool BuildCommandBuffers_(FInternalData& data);
    bool ProcessTasks_(FInternalData& data, VkCommandBuffer cmd);

    // resource manager

    template <u32 _Uid, typename _Resource, typename _MainPool, size_t _MaxChunks>
    NODISCARD _Resource* ToLocal_(
        details::TResourceId<_Uid> id,
        TLocalPool<_Resource, _MainPool, _MaxChunks>& localResources
        ARGS_IF_RHIDEBUG(FWStringView debugMessage));

    void FlushLocalResourceStates_(FInternalData& data, EVulkanExecutionOrder, FVulkanBarrierManager& ARGS_IF_RHIDEBUG(FVulkanLocalDebugger*));
    void ResetLocalRemapping_(FInternalData& data);

};
//----------------------------------------------------------------------------
template <u32 _Uid>
auto* FVulkanCommandBuffer::AcquireTransient(details::TResourceId<_Uid> id) {
    auto[it, inserted] = Write()->RM.ResourceMap.insert(MakePair(id.Pack(), 1));
    return ResourceManager().ResourceDataIFP(id, inserted);
}
//----------------------------------------------------------------------------
template <u32 _Uid>
void FVulkanCommandBuffer::ReleaseResource(details::TResourceId<_Uid> id) {
    ++Write()->RM.ResourceMap.insert(MakePair(id.Pack(), 0)).first->second;
}
//----------------------------------------------------------------------------
inline const FVulkanPipelineResources* FVulkanCommandBuffer::CreateDescriptorSet(const FPipelineResources& desc) {
    return ResourceManager().CreateDescriptorSet(desc, Write()->RM.ResourceMap);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
