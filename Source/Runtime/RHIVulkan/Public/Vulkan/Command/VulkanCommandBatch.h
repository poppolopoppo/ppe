#pragma once

#include "Vulkan/VulkanCommon.h"

#include "RHI/CommandBatch.h"
#include "RHI/CommandBuffer.h"
#include "RHI/FrameGraph.h"
#include "RHI/RayTracingTask.h"

#include "Container/Appendable.h"
#include "Container/HashMap.h"
#include "Container/Stack.h"
#include "Misc/Function.h"
#include "Thread/ThreadSafe.h"

#include <atomic>

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EStagingBufferIndex : u32 { Unknown = ~0u };
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanCommandBatch final : public ICommandBatch {
public:
    STATIC_CONST_INTEGRAL(u32, MaxBatchItems, 8);
    STATIC_CONST_INTEGRAL(u32, MaxBufferParts, 3);
    STATIC_CONST_INTEGRAL(u32, MaxImageParts, 4);
    STATIC_CONST_INTEGRAL(u32, MaxDependencies, 16);
    STATIC_CONST_INTEGRAL(u32, MaxSwapchains, 8);

    using FCommandBuffers = TFixedSizeStack<TPair<VkCommandBuffer, SCVulkanCommandPool>, MaxBatchItems>;
    using FDependencies = TFixedSizeStack<PVulkanCommandBatch, MaxDependencies>;
    using FSignalSemaphores = TFixedSizeStack<VkSemaphore, MaxBatchItems>;
    using FSwapchains = TFixedSizeStack<PVulkanSwapchain, MaxSwapchains>;
    using FWaitSemaphores = TFixedSizeStack<TPair<VkSemaphore, VkPipelineStageFlags>, MaxBatchItems>;

    using FResourceArray = VECTORINSITU(RHICommand, TPair<VkObjectType COMMA uintptr_t>, 3);
    using FResourceMap = HASHMAP(RHICommand, FResourceHandle, u32);

    enum class EState : u32 {
        Initial,
        Recording,  // build command buffers
        Backed,     // command buffers built, all data locked
        Ready,      // all dependencies in 'Ready', 'Submitted' or 'Complete' states
        Submitted,  // commands was submitted to the GPU
        Complete,   // commands complete execution on the GPU
    };

    struct FInternalData {
        EQueueType QueueType{ Default };
        FDependencies Dependencies;
        bool SubmitImmediately{ false };
        bool SupportsQuery{ false };
        bool NeedQueueSync{ false };
    };

    struct FStagingBuffer {
        EStagingBufferIndex Index{ Default };
        FRawBufferID BufferId;
        FRawMemoryID MemoryId;
        u32 Capacity{ 0 };
        u32 Size{ 0 };

        VkDeviceMemory DeviceMemory{ VK_NULL_HANDLE };
        void* MappedPtr{ nullptr };
        u32 MemoryOffset : 31;
        u32 IsCoherent : 1;

        CONSTEXPR FStagingBuffer() : MemoryOffset(0), IsCoherent(0) {}
        CONSTEXPR FStagingBuffer(EStagingBufferIndex index, FRawBufferID bufferId, FRawMemoryID memoryId, u32 capacity)
        :   Index(index), BufferId(bufferId), MemoryId(memoryId), Capacity(capacity)
        {}

        bool Empty() const { return (0 == Size); }
        bool Full() const { return (Capacity == Size); }
    };

    struct FStagingDataRange {
        FStagingBuffer const* Buffer;
        u32 Offset;
        u32 Size;
    };

    struct FOnBufferDataLoadedEvent {
        using FDataParts = TFixedSizeStack<FStagingDataRange, MaxBufferParts>;
        using FCallback = FReadBuffer::FCallback;

        FCallback Callback;
        FDataParts Parts;
        u32 TotalSize{ 0 };

        FOnBufferDataLoadedEvent() = default;
        FOnBufferDataLoadedEvent(FCallback&& rcallback, u32 size) NOEXCEPT
        :   Callback(std::move(rcallback)), TotalSize(size)
        {}
    };

    struct FOnImageDataLoadedEvent {
        using FDataParts = TFixedSizeStack<FStagingDataRange, MaxImageParts>;
        using FCallback = FReadImage::FCallback;

        FCallback Callback;
        FDataParts Parts;
        u32 TotalSize{ 0 };
        int3 ImageSize{ 0 };
        u32 RowPitch{ 0 };
        u32 SlicePitch{ 0 };
        EPixelFormat Format{ Default };
        EImageAspect Aspect{ EImageAspect::Color };

        FOnImageDataLoadedEvent() = default;
        FOnImageDataLoadedEvent(
            FCallback&& rcallback, u32 size, const uint3& imageSize,
            u32 rowPitch, u32 slicePitch, EPixelFormat fmt, EImageAspect aspect ) NOEXCEPT
        :   Callback(std::move(rcallback)), TotalSize(size), ImageSize(imageSize)
        ,   RowPitch(rowPitch), SlicePitch(slicePitch), Format(fmt), Aspect(aspect)
        {}
    };

#if USE_PPE_RHIDEBUG
    using FShaderModules = TFixedSizeStack<PShaderModule, 8>;
    using FDebugBatchGraph = FVulkanLocalDebugger::FBatchGraph;

    struct FDebugStorageBuffer {
        FBufferID ShaderTraceBuffer;
        FBufferID ReadBackBuffer;
        u32 Capacity{ 0 };
        u32 Size{ 0 };
        VkPipelineStageFlags Stages{ Default };
    };

    struct FDebugMode {
        FShaderModules Modules;
        VkDescriptorSet DescriptorSet{ VK_NULL_HANDLE };
        u32 Offset{ 0 };
        u32 Size{ 0 };
        u32 StorageBufferIndex{ UMax };
        EShaderDebugMode Mode{ Default };
        EShaderStages Stages{ Default };
        FTaskName TaskName;
        uint4 Payload{ 0 };
    };

    using FDebugStorageBuffers = VECTOR(RHIDebug, FDebugStorageBuffer);
    using FDebugModes = VECTOR(RHIDebug, FDebugMode);
    using FDebugDescriptorKey = TPair<FRawBufferID, FRawDescriptorSetLayoutID>;;
    using FDebugDescriptorCache = HASHMAP(RHIDebug, FDebugDescriptorKey, FVulkanDescriptorSets);
#endif

    FVulkanCommandBatch(FVulkanFrameGraph& fg, u32 indexInPool);
    ~FVulkanCommandBatch();

    u32 IndexInPool() const { return _indexInPool; }

    EState State() const { return _state.load(std::memory_order_relaxed); }
    FVulkanSubmitted* Submitted() const { return _submitted.load(std::memory_order_relaxed); }

    auto Read() const { return _data.LockShared(); }

    void Create(EQueueType type, TMemoryView<const FCommandBufferBatch> dependsOn);

    bool OnStart(const FCommandBufferDesc& desc);
    void OnBeforeRecording(VkCommandBuffer cmd);
    void OnAfterRecording(VkCommandBuffer cmd);
    bool OnBaked(FResourceMap& resources);
    bool OnReadyToSubmit();
    bool OnBeforeSubmit(VkSubmitInfo* psubmit);
    bool OnAfterSubmit(TAppendable<SCVulkanSwapchain> swapchains, FVulkanSubmitted* submitted);
    bool OnComplete(FFrameStatistics* pstats, FVulkanDebugger& debugger, FShaderDebugCallback&& callback);

    void SignalSemaphore(VkSemaphore semaphore);
    void WaitSemaphore(VkSemaphore semaphore, VkPipelineStageFlags stages);
    void PushCommandToFront(VkCommandBuffer vkCmdBuffer, const FVulkanCommandPool* pool);
    void PushCommandToBack(VkCommandBuffer vkCmdBuffer, const FVulkanCommandPool* pool);
    void DependsOn(FVulkanCommandBatch* other);
    void DestroyPostponed(VkObjectType type, uintptr_t handle);

    // ICommandBatch

    virtual void TearDown() override;

    // staging buffer

    bool StageWrite(FRawBufferID* pDstBuffer, u32* pDstOffset, u32* pOutSize, void** pMappedPtr,
        const u32 srcRequiredSize, const u32 blockAlign, const u32 offsetAlign, const u32 dstMinSize );

    bool AddPendingLoad(FRawBufferID* pDstBuffer, FStagingDataRange* pRange, u32 srcOffset, u32 srcTotalSize);
    bool AddPendingLoad(FRawBufferID* pDstBuffer, FStagingDataRange* pRange, u32 srcOffset, u32 srcTotalSize, u32 srcPitch);

    bool AddDataLoadedEvent(FOnImageDataLoadedEvent&& revent);
    bool AddDataLoadedEvent(FOnBufferDataLoadedEvent&& revent);


#if USE_PPE_RHIDEBUG
    // shader debugger

    bool SetShaderModuleForDebug(EShaderDebugIndex id, PShaderModule&& rmodule);

    bool FindModeInfoForDebug(EShaderDebugMode* pmode, EShaderStages* pstages, EShaderDebugIndex id) const;
    bool FindDescriptorSetForDebug(u32* pbinding, VkDescriptorSet* pset, uint2* pdim, EShaderDebugIndex id) const;
    bool FindShaderStampForDebug(FRawBufferID* pbuf, u32* poffset, u32* psize, uint2* pdim, EShaderDebugIndex id) const;

    STATIC_CONST_INTEGRAL(u32, DebugBufferSize, 8 * 1024 * 1024);

    EShaderDebugIndex AppendShaderForDebug(TMemoryView<const FRectangleI>& regions, const FTaskName& name, const FGraphicsShaderDebugMode& mode, u32 size = DebugBufferSize);
    EShaderDebugIndex AppendShaderForDebug(const FTaskName& name, const FComputeShaderDebugMode& mode, u32 size = DebugBufferSize);
    EShaderDebugIndex AppendShaderForDebug(const FTaskName& name, const FRayTracingShaderDebugMode& mode, u32 size = DebugBufferSize);

    EShaderDebugIndex AppendTimemap(const uint2& dim, EShaderStages stages);

#endif

private:
    void SetState_(EState from, EState to);
    void ReleaseResources_();
    void ReleaseVulkanObjects_();
    void FinalizeCommands_();

    void AddPendingLoad_(FRawBufferID* pDstBuffer, FStagingDataRange* pRange,
        u32 srcRequiredSize, u32 blockAlign, u32 offsetAlign, u32 dstMinSize );

    bool MapMemory_(FStagingBuffer& staging) const;
    void FinalizeStagingBuffers_(const FVulkanDevice& device);

    const SVulkanFrameGraph _frameGraph;
    const u32 _indexInPool;

    std::atomic<EState> _state{ EState::Initial };
    std::atomic<FVulkanSubmitted*> _submitted{ nullptr };

    TRHIThreadSafe<FInternalData> _data;

    // command batch
    struct {
        FCommandBuffers Commands;
        FSignalSemaphores SignalSemaphores;
        FWaitSemaphores WaitSemaphores;
    }   _batch;

    // staging buffer
    struct {
        TFixedSizeStack<FStagingBuffer, 8> HostToDevice;
        TFixedSizeStack<FStagingBuffer, 8> DeviceToHost;
        VECTORINSITU(RHICommand, FOnBufferDataLoadedEvent, 1) OnBufferLoadedEvents;
        VECTORINSITU(RHICommand, FOnImageDataLoadedEvent, 1) OnImageLoadedEvents;
    }   _staging;

    // resources
    FResourceArray _readyToDelete;
    FResourceMap _resourcesToRelease;
    FSwapchains _swapchains;

#if USE_PPE_RHIDEBUG

    // shader debugger
    struct {
        FDebugStorageBuffers Buffers;
        FDebugModes Modes;
        FDebugDescriptorCache DescriptorCache;
        u32 BufferAlign{ 0 };
        const u32 BufferSize{ 64*1024*1024 };
    }   _shaderDebugger;

    // frame debugger
    struct {
        FString DebugDump;
        FDebugBatchGraph DebugGraph;
        FFrameStatistics Statistics;
    }   _frameDebugger;

    void BeginShaderDebugger_(VkCommandBuffer cmd);
    void EndShaderDebugger_(VkCommandBuffer cmd);

    bool AllocStorage_(FDebugMode& debugMode, u32 size);
    bool AllocDescriptorSet_(VkDescriptorSet* pDescSet, EShaderDebugMode debugMode, EShaderStages stages, FRawBufferID storageBuffer, u32 size);

    void ParseDebugOutput_(const FShaderDebugCallback& debug);
    bool ParseDebugOutput2_(TAppendable<FString> outp, const FShaderDebugCallback& debug, const FDebugMode& dbg) const;

#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
