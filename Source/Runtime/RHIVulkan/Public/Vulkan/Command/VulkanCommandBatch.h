#pragma once

#include "Vulkan/VulkanCommon.h"

#include "Vulkan/Debugger/VulkanLocalDebugger.h"

#include "RHI/CommandBatch.h"
#include "RHI/CommandBuffer.h"
#include "RHI/FrameGraph.h"
#include "RHI/RayTracingTask.h"

#include "Container/Appendable.h"
#include "Container/HashMap.h"
#include "Container/Stack.h"
#include "Container/TupleVector.h"
#include "IO/StringBuilder.h"
#include "Misc/Function.h"

#include <atomic>

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanCommandBatch final : public ICommandBatch {
    friend class FVulkanCommandBuffer;

public:
    STATIC_CONST_INTEGRAL(u32, MaxBatchItems, 8);
    STATIC_CONST_INTEGRAL(u32, MaxBufferParts, 3);
    STATIC_CONST_INTEGRAL(u32, MaxImageParts, 4);
    STATIC_CONST_INTEGRAL(u32, MaxDependencies, 16);
    STATIC_CONST_INTEGRAL(u32, MaxSwapchains, 8);
    STATIC_CONST_INTEGRAL(u32, MaxRegions, 32);

    using FCommandBuffers = TUPLEVECTOR_INSITU(RHIBatch, MaxBatchItems, VkCommandBuffer, FVulkanCommandPool*);
    using FDependencies = TFixedSizeStack<SVulkanCommandBatch, MaxDependencies>;
    using FSignalSemaphores = TFixedSizeStack<VkSemaphore, MaxBatchItems>;
    using FSwapchains = TFixedSizeStack<const FVulkanSwapchain*, MaxSwapchains>;
    using FWaitSemaphores = TUPLEVECTOR_INSITU(RHIBatch, MaxBatchItems, VkSemaphore, VkPipelineStageFlags);

    using FResourceArray = VECTORINSITU(RHICommand, TPair<VkObjectType COMMA FVulkanExternalObject>, 3);
    using FResourceMap = HASHMAP(RHICommand, FResourceHandle, u32);

    enum class EState : u32 {
        Uninitialized = 0,
        Initial,
        Recording,  // build command buffers
        Baked,      // command buffers built, all data locked
        Ready,      // all dependencies in 'Ready', 'Submitted' or 'Complete' states
        Submitted,  // commands was submitted to the GPU
        Complete,   // commands complete execution on the GPU
    };

    struct FStagingBuffer {
        FStagingBufferIndex Index{ Default };
        FRawBufferID BufferId;
        FRawMemoryID MemoryId;
        size_t Capacity{ 0 };
        size_t Size{ 0 };

        VkDeviceMemory DeviceMemory{ VK_NULL_HANDLE };
        ubyte* MappedPtr{ nullptr };
        size_t MemoryOffset{ 0 }; // can be used to flush memory ranges
        bool IsCoherent{ false };

        FStagingBuffer() = default;
        CONSTEXPR FStagingBuffer(FStagingBufferIndex index, FRawBufferID bufferId, FRawMemoryID memoryId, size_t capacity)
        :   Index(index), BufferId(bufferId), MemoryId(memoryId), Capacity(capacity)
        {}

        bool Empty() const { return (0 == Size); }
        bool Full() const { return (Capacity == Size); }
    };

    struct FStagingDataRange {
        FStagingBuffer const* Buffer{ nullptr };
        size_t Offset{ 0 };
        size_t Size{ 0 };

        FRawMemory MakeView() const NOEXCEPT {
            return { static_cast<u8*>(Buffer->MappedPtr) + Offset, Size };
        }
    };

    struct FOnBufferDataLoadedEvent {
        using FDataParts = TFixedSizeStack<FStagingDataRange, MaxBufferParts>;
        using FCallback = FReadBuffer::FCallback;

        FCallback Callback;
        FDataParts Parts;
        size_t TotalSize{ 0 };

        FOnBufferDataLoadedEvent() = default;
        FOnBufferDataLoadedEvent(FCallback&& rcallback, size_t size) NOEXCEPT
        :   Callback(std::move(rcallback)), TotalSize(size)
        {}
        FOnBufferDataLoadedEvent(const FCallback& callback, size_t size) NOEXCEPT
        :   Callback(callback), TotalSize(size)
        {}
    };

    struct FOnImageDataLoadedEvent {
        using FDataParts = TFixedSizeStack<FStagingDataRange, MaxImageParts>;
        using FCallback = FReadImage::FCallback;

        FCallback Callback;
        FDataParts Parts;
        size_t TotalSize{ 0 };
        uint3 ImageSize{ 0 };
        size_t RowPitch{ 0 };
        size_t SlicePitch{ 0 };
        EPixelFormat Format{ Default };
        EImageAspect Aspect{ EImageAspect::Color };

        FOnImageDataLoadedEvent() = default;
        FOnImageDataLoadedEvent(
            FCallback&& rcallback, size_t size, const uint3& imageSize,
            size_t rowPitch, size_t slicePitch, EPixelFormat fmt, EImageAspect aspect ) NOEXCEPT
        :   Callback(std::move(rcallback)), TotalSize(size), ImageSize(imageSize)
        ,   RowPitch(rowPitch), SlicePitch(slicePitch), Format(fmt), Aspect(aspect)
        {}
        FOnImageDataLoadedEvent(
            const FCallback& callback, size_t size, const uint3& imageSize,
            size_t rowPitch, size_t slicePitch, EPixelFormat fmt, EImageAspect aspect ) NOEXCEPT
        :   Callback(callback), TotalSize(size), ImageSize(imageSize)
        ,   RowPitch(rowPitch), SlicePitch(slicePitch), Format(fmt), Aspect(aspect)
        {}
    };

    using FStagingBuffers = TFixedSizeStack<FStagingBuffer, 8>;

    struct FInternalData {
        EQueueType QueueType{ Default };
        FDependencies Dependencies;
        bool SubmitImmediately{ false };
        bool SupportsQuery{ false };
        bool NeedQueueSync{ false };

        // command batch
        struct {
            FCommandBuffers Commands;
            FSignalSemaphores SignalSemaphores;
            FWaitSemaphores WaitSemaphores;
        }   Batch;

        // staging buffer
        struct {
            FStagingBuffers HostToDevice;
            FStagingBuffers DeviceToHost;
            VECTORINSITU(RHICommand, FOnBufferDataLoadedEvent, 1) OnBufferLoadedEvents;
            VECTORINSITU(RHICommand, FOnImageDataLoadedEvent, 1) OnImageLoadedEvents;
        }   Staging;

        // resources
        FResourceArray ReadyToDelete;
        FResourceMap ResourcesToRelease;
        FSwapchains Swapchains;
    };

#if USE_PPE_RHIDEBUG
    using FShaderModules = TFixedSizeStack<PVulkanShaderModule, 8>;
    using FDebugBatchGraph = FVulkanLocalDebugger::FBatchGraph;

    struct FDebugStorageBuffer {
        FBufferID ShaderTraceBuffer;
        FBufferID ReadBackBuffer;
        size_t Capacity{ 0 };
        size_t Size{ 0 };
        VkPipelineStageFlags Stages{ Default };
    };

    struct FDebugMode {
        FShaderModules Modules;
        VkDescriptorSet DescriptorSet{ VK_NULL_HANDLE };
        size_t Offset{ 0 };
        size_t Size{ 0 };
        u32 StorageBufferIndex{ UMax };
        EShaderDebugMode Mode{ Default };
        EShaderStages Stages{ Default };
        FTaskName TaskName;
        uint4 Payload{ 0 };
    };

    using FDebugStorageBuffers = VECTORINSITU(RHIDebug, FDebugStorageBuffer, 1);
    using FDebugModes = VECTORINSITU(RHIDebug, FDebugMode, 1);
    using FDebugDescriptorKey = TPair<FRawBufferID, FRawDescriptorSetLayoutID>;
    using FDebugDescriptorCache = HASHMAP(RHIDebug, FDebugDescriptorKey, FVulkanDescriptorSet);
#endif

    FVulkanCommandBatch(const SVulkanFrameGraph& fg, u32 indexInPool);
    virtual ~FVulkanCommandBatch() override;

    u32 IndexInPool() const { return _indexInPool; }

    EState State() const { return _state.load(std::memory_order_relaxed); }
    FVulkanSubmitted* Submitted() const { return _submitted.load(std::memory_order_relaxed); }
    EQueueUsage QueueUsage() const NOEXCEPT;

    auto Read() const { return _data.LockShared(); }

    void Construct(EQueueType type, TMemoryView<const SCommandBatch> dependsOn);

    bool OnBegin(const FCommandBufferDesc& desc);
    void OnBeforeRecording(VkCommandBuffer cmd);
    void OnAfterRecording(VkCommandBuffer cmd);
    bool OnBaked(FResourceMap& resources);
    bool OnReadyToSubmit();
    bool OnBeforeSubmit(VkSubmitInfo* pSubmit);
    bool OnAfterSubmit(TAppendable<const FVulkanSwapchain*> swapchains, FVulkanSubmitted* submitted);
    bool OnComplete(ARG0_IF_RHIDEBUG(FFrameStatistics* pStats, FVulkanDebugger& debugger, const FShaderDebugCallback& callback));

    void SignalSemaphore(VkSemaphore vkSemaphore);
    void WaitSemaphore(VkSemaphore vkSemaphore, VkPipelineStageFlags stages);
    void PushCommandToFront(FVulkanCommandPool* pPool, VkCommandBuffer vkCmdBuffer);
    void PushCommandToBack(FVulkanCommandPool* pPool, VkCommandBuffer vkCmdBuffer);
    void DependsOn(const SVulkanCommandBatch& other);
    void DestroyPostponed(VkObjectType type, FVulkanExternalObject handle);

    // ICommandBatch

    virtual void OnStrongRefCountReachZero() NOEXCEPT override;

    // staging buffer

    NODISCARD bool StageWrite(FStagingBlock* pStaging, size_t* pOutSize,
        const size_t srcRequiredSize, const size_t blockAlign, const size_t offsetAlign, const size_t dstMinSize );

    NODISCARD bool AddPendingLoad(FRawBufferID* pDstBuffer, FStagingDataRange* pRange, size_t srcOffset, size_t srcTotalSize);
    NODISCARD bool AddPendingLoad(FRawBufferID* pDstBuffer, FStagingDataRange* pRange, size_t srcOffset, size_t srcTotalSize, size_t srcPitch);

    void AddDataLoadedEvent(FOnImageDataLoadedEvent&& revent);
    void AddDataLoadedEvent(FOnBufferDataLoadedEvent&& revent);

#if USE_PPE_RHIDEBUG
    // shader debugger

    const FVulkanDebugName& DebugName() const { return _debugName; }

    void SetShaderModuleForDebug(EShaderDebugIndex id, const PVulkanShaderModule& module);

    NODISCARD bool FindModeInfoForDebug(EShaderDebugMode* pMode, EShaderStages* pStages, EShaderDebugIndex id) const;
    NODISCARD bool FindDescriptorSetForDebug(u32* pBinding, VkDescriptorSet* pSet, u32* pDynamicOffset, EShaderDebugIndex id) const;
    NODISCARD bool FindShaderTimemapForDebug(FRawBufferID* pBuf, size_t* pOffset, size_t* pSize, uint2* pDim, EShaderDebugIndex id) const;

    STATIC_CONST_INTEGRAL(size_t, DebugBufferSize, 8 * 1024 * 1024);

    EShaderDebugIndex AppendShaderForDebug(TMemoryView<const FRectangleU>& regions, const FTaskName& name, const FGraphicsShaderDebugMode& mode, size_t size = DebugBufferSize);
    EShaderDebugIndex AppendShaderForDebug(const FTaskName& name, const FComputeShaderDebugMode& mode, size_t size = DebugBufferSize);
    EShaderDebugIndex AppendShaderForDebug(const FTaskName& name, const FRayTracingShaderDebugMode& mode, size_t size = DebugBufferSize);

    EShaderDebugIndex AppendTimemapForDebug(const uint2& dim, EShaderStages stages);

#endif

private:
    void SetState_(EState from, EState to);

    static void ReleaseResources_(FVulkanResourceManager& resources, FInternalData& data);
    static void ReleaseVulkanObjects_(const FVulkanDevice& device, FInternalData& data);
    static void FinalizeCommands_(FInternalData& data);

    FStagingBuffer* FindOrAddStagingBuffer_(
        FStagingBuffers *pStagingBuffers, size_t stagingSize, EBufferUsage usage,
        size_t srcRequiredSize, size_t blockAlign, size_t offsetAlign, size_t dstMinSize ) const;

    NODISCARD static bool MapMemory_(FVulkanResourceManager& resources, FStagingBuffer& staging);
    static void FinalizeStagingBuffers_(const FVulkanDevice& device, FVulkanResourceManager& resources, FInternalData& data);

    const SVulkanFrameGraph _frameGraph;
    const u32 _indexInPool;

    std::atomic<EState> _state{ EState::Uninitialized };
    std::atomic<FVulkanSubmitted*> _submitted{ nullptr };

    TRHIThreadSafe<FInternalData> _data;

#if USE_PPE_RHIDEBUG
    FVulkanDebugName _debugName;
    mutable FFrameStatistics _statistics;

    // shader debugger
    struct {
        FDebugStorageBuffers Buffers;
        FDebugModes Modes;
        FDebugDescriptorCache DescriptorCache;
        size_t BufferAlign{ 0 };
        const size_t BufferSize{ 64*1024*1024 };
    }   _shaderDebugger;

    // frame debugger
    struct {
        FStringBuilder DebugDump;
        FDebugBatchGraph DebugGraph;
    }   _frameDebugger;

    void BeginShaderDebugger_(VkCommandBuffer cmd);
    void EndShaderDebugger_(VkCommandBuffer cmd);

    NODISCARD bool AllocStorageForDebug_(FDebugMode& debugMode, size_t size);
    NODISCARD bool AllocDescriptorSetForDebug_(VkDescriptorSet* pDescSet, EShaderDebugMode debugMode, EShaderStages stages, FRawBufferID storageBuffer, size_t size, const FConstChar debugName);

    using FDebugStrings = VECTORINSITU(RHIDebug, FString, 8);

    void ParseDebugOutput_(const FShaderDebugCallback& callback);
    NODISCARD bool ParseDebugOutput2_(FDebugStrings* pDump, const FShaderDebugCallback& callback, const FDebugMode& dbg) const;

#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
