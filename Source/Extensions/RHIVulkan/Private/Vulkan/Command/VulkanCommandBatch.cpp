﻿// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Vulkan/Command/VulkanCommandBatch.h"

#include "Diagnostic/Logger.h"
#include "Vulkan/Instance/VulkanFrameGraph.h"
#include "Vulkan/Pipeline/VulkanShaderModule.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVulkanCommandBatch::FVulkanCommandBatch(const SVulkanFrameGraph& fg, u32 indexInPool)
:   _frameGraph(fg)
,   _indexInPool(indexInPool) {
    Assert(_frameGraph);
    STATIC_ASSERT(decltype(_state)::is_always_lock_free);

#if USE_PPE_RHIDEBUG
    _shaderDebugger.BufferAlign = checked_cast<size_t>(_frameGraph->Device().Limits().minStorageBufferOffsetAlignment);
    Assert_NoAssume(not EnableShaderDebugging || _frameGraph->Device().Limits().maxBoundDescriptorSets > DebugDescriptorSet);
#endif
}
//----------------------------------------------------------------------------
FVulkanCommandBatch::~FVulkanCommandBatch() {
    Assert_NoAssume(State() == EState::Uninitialized);
}
//----------------------------------------------------------------------------
void FVulkanCommandBatch::Construct(EQueueType type, TMemoryView<const SCommandBatch> dependsOn) {
    Assert_NoAssume(State() == EState::Uninitialized);
    const auto exclusiveData = _data.LockExclusive();

    Assert_NoAssume(exclusiveData->Dependencies.empty());
    Assert_NoAssume(exclusiveData->Batch.Commands.empty());
    Assert_NoAssume(exclusiveData->Batch.SignalSemaphores.empty());
    Assert_NoAssume(exclusiveData->Batch.WaitSemaphores.empty());
    Assert_NoAssume(exclusiveData->Staging.HostToDevice.empty());
    Assert_NoAssume(exclusiveData->Staging.DeviceToHost.empty());
    Assert_NoAssume(exclusiveData->Staging.OnBufferLoadedEvents.empty());
    Assert_NoAssume(exclusiveData->Staging.OnImageLoadedEvents.empty());
    Assert_NoAssume(exclusiveData->ResourcesToRelease.empty());
    Assert_NoAssume(_submitted == nullptr);
#if USE_PPE_RHIDEBUG
    Assert_NoAssume(_shaderDebugger.Buffers.empty());
    Assert_NoAssume(_shaderDebugger.Modes.empty());
#endif

    exclusiveData->QueueType = type;
    if (const PVulkanDeviceQueue queue = _frameGraph->FindQueue(type))
        exclusiveData->SubmitImmediately = !!(queue->FamilyFlags & (
            VK_QUEUE_GRAPHICS_BIT |
            VK_QUEUE_COMPUTE_BIT ));
    else
        exclusiveData->SubmitImmediately = false;

    SetState_(EState::Uninitialized, EState::Initial);

    for (const SCommandBatch& dep : dependsOn) {
        if (SVulkanCommandBatch batch = checked_cast<FVulkanCommandBatch>(dep))
            exclusiveData->Dependencies.Push(std::move(batch));
        else
            AssertNotReached();
    }
}
//----------------------------------------------------------------------------
void FVulkanCommandBatch::TearDown() {
    Assert_NoAssume(State() == EState::Complete);

    const auto exclusiveData = _data.LockExclusive();
    Unused(exclusiveData); // just for locking

    _state.store(EState::Uninitialized, std::memory_order_relaxed);

    _frameGraph->RecycleBatch(this); // put this batch back in cache
}
//----------------------------------------------------------------------------
void FVulkanCommandBatch::OnStrongRefCountReachZero() NOEXCEPT {
    Assert_NoAssume(RefCount() == 0);

    TearDown();
}
//----------------------------------------------------------------------------
void FVulkanCommandBatch::SignalSemaphore(VkSemaphore vkSemaphore) {
    Assert(VK_NULL_HANDLE != vkSemaphore);
    Assert_NoAssume(State() < EState::Submitted);

    const auto exclusiveData = _data.LockExclusive();

    AssertMessage_NoAssume("semaphore already added",
        not exclusiveData->Batch.SignalSemaphores.Contains(vkSemaphore));
    exclusiveData->Batch.SignalSemaphores.Push(vkSemaphore);
}
//----------------------------------------------------------------------------
void FVulkanCommandBatch::WaitSemaphore(VkSemaphore vkSemaphore, VkPipelineStageFlags stages) {
    Assert(VK_NULL_HANDLE != vkSemaphore);
    Assert_NoAssume(State() < EState::Submitted);

    const auto exclusiveData = _data.LockExclusive();

    AssertMessage_NoAssume("semaphore already added",
        not exclusiveData->Batch.WaitSemaphores.MakeView<0>().Contains(vkSemaphore));
    exclusiveData->Batch.WaitSemaphores.emplace_back(vkSemaphore, stages);
}
//----------------------------------------------------------------------------
void FVulkanCommandBatch::PushCommandToFront(FVulkanCommandPool* pPool, VkCommandBuffer vkCmdBuffer) {
    Assert(VK_NULL_HANDLE != vkCmdBuffer);
    Assert(pPool);
    Assert_NoAssume(State() < EState::Submitted);

    const auto exclusiveData = _data.LockExclusive();

    AssertMessage_NoAssume("command buffer already added",
        not exclusiveData->Batch.Commands.MakeView<0>().Contains(vkCmdBuffer));
    exclusiveData->Batch.Commands.insert(exclusiveData->Batch.Commands.begin(), { vkCmdBuffer, pPool });
}
//----------------------------------------------------------------------------
void FVulkanCommandBatch::PushCommandToBack(FVulkanCommandPool* pPool, VkCommandBuffer vkCmdBuffer) {
    Assert(VK_NULL_HANDLE != vkCmdBuffer);
    Assert(pPool);
    Assert_NoAssume(State() < EState::Submitted);

    const auto exclusiveData = _data.LockExclusive();

    AssertMessage_NoAssume("command buffer already added",
        not exclusiveData->Batch.Commands.MakeView<0>().Contains(vkCmdBuffer));
    exclusiveData->Batch.Commands.emplace_back(vkCmdBuffer, pPool);
}
//----------------------------------------------------------------------------
void FVulkanCommandBatch::DependsOn(const SVulkanCommandBatch& other) {
    Assert(other);
    Assert_NoAssume(State() < EState::Baked);

    const auto exclusiveData = _data.LockExclusive();

    Assert_NoAssume(not exclusiveData->Dependencies.Contains(other));
    exclusiveData->Dependencies.Push(other);
}
//----------------------------------------------------------------------------
void FVulkanCommandBatch::DestroyPostponed(VkObjectType type, FVulkanExternalObject handle) {
    Assert(handle);
    Assert_NoAssume(State() < EState::Baked);

    const auto exclusiveData = _data.LockExclusive();

    AssertMessage_NoAssume("resource already registered for postponed destroy",
        exclusiveData->ReadyToDelete.end() == std::find_if(exclusiveData->ReadyToDelete.begin(), exclusiveData->ReadyToDelete.end(), [handle](const auto& it) {
            return (it.second == handle);
        }));
    exclusiveData->ReadyToDelete.emplace_back(type, handle);
}
//----------------------------------------------------------------------------
void FVulkanCommandBatch::SetState_(EState from, EState to) {
    Assert_NoAssume(State() == from);
    Assert_NoAssume(static_cast<u32>(from) < static_cast<u32>(to));
    Unused(from);

    const auto exclusiveData = _data.LockExclusive();
    Unused(exclusiveData); // just for locking

    _state.store(to, std::memory_order_relaxed);
}
//----------------------------------------------------------------------------
EQueueUsage FVulkanCommandBatch::QueueUsage() const noexcept {
    return EQueueType_Usage(Read()->QueueType);
}
//----------------------------------------------------------------------------
bool FVulkanCommandBatch::OnBegin(const FCommandBufferDesc& desc) {
    SetState_(EState::Initial, EState::Recording);

#if USE_PPE_RHIDEBUG
    const auto exclusiveData = _data.LockExclusive();
    exclusiveData->NeedQueueSync = (desc.DebugFlags & EDebugFlags::QueueSync);
    _debugName = desc.Name;
    _statistics.Reset();
#else
    Unused(desc);
#endif

    return true;
}
//----------------------------------------------------------------------------
void FVulkanCommandBatch::OnBeforeRecording(VkCommandBuffer cmd) {
    Assert_NoAssume(VK_NULL_HANDLE != cmd);
    Assert_NoAssume(State() == EState::Recording);

#if USE_PPE_RHIDEBUG
    const auto exclusiveData = _data.LockExclusive();

    if (exclusiveData->SupportsQuery) {
        const FVulkanDevice& device = _frameGraph->Device();
        VkQueryPool queryPool = _frameGraph->QueryPool();
        Assert_NoAssume(VK_NULL_HANDLE != queryPool);

        device.vkCmdResetQueryPool(cmd, queryPool, _indexInPool*2, 2);
        device.vkCmdWriteTimestamp(cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, queryPool, _indexInPool*2);
    }

    BeginShaderDebugger_(cmd);
#else
    Unused(cmd);
#endif
}
//----------------------------------------------------------------------------
void FVulkanCommandBatch::OnAfterRecording(VkCommandBuffer cmd) {
    Assert_NoAssume(VK_NULL_HANDLE != cmd);
    Assert_NoAssume(State() == EState::Recording);

#if USE_PPE_RHIDEBUG
    const auto exclusiveData = _data.LockExclusive();

    EndShaderDebugger_(cmd);

    if (exclusiveData->SupportsQuery) {
        const FVulkanDevice& device = _frameGraph->Device();
        VkQueryPool queryPool = _frameGraph->QueryPool();
        Assert_NoAssume(VK_NULL_HANDLE != queryPool);

        device.vkCmdWriteTimestamp(cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, queryPool, _indexInPool*2 + 1);
    }

#else
    Unused(cmd);
#endif
}
//----------------------------------------------------------------------------
bool FVulkanCommandBatch::OnBaked(FResourceMap& resources) {
    SetState_(EState::Recording, EState::Baked);

    const auto exclusiveData = _data.LockExclusive();
    Unused(exclusiveData); // just for locking

    Assert_NoAssume(exclusiveData->ResourcesToRelease.empty());
    exclusiveData->ResourcesToRelease = std::move(resources);
    return true;
}
//----------------------------------------------------------------------------
bool FVulkanCommandBatch::OnReadyToSubmit() {
    SetState_(EState::Baked, EState::Ready);
    return true;
}
//----------------------------------------------------------------------------
bool FVulkanCommandBatch::OnBeforeSubmit(VkSubmitInfo* pSubmit) {
    Assert(pSubmit);

    const auto exclusiveData = _data.LockExclusive();
    Unused(exclusiveData); // just for locking

    const FVulkanDevice& device = _frameGraph->Device();

    pSubmit->sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    pSubmit->pNext = nullptr;
    pSubmit->pCommandBuffers = exclusiveData->Batch.Commands.MakeView<0>().data();
    pSubmit->commandBufferCount = checked_cast<u32>(exclusiveData->Batch.Commands.size());
    pSubmit->pSignalSemaphores = exclusiveData->Batch.SignalSemaphores.data();
    pSubmit->signalSemaphoreCount = checked_cast<u32>(exclusiveData->Batch.SignalSemaphores.size());
    pSubmit->pWaitSemaphores = exclusiveData->Batch.WaitSemaphores.MakeView<0>().data();
    pSubmit->pWaitDstStageMask = exclusiveData->Batch.WaitSemaphores.MakeView<1>().data();
    pSubmit->waitSemaphoreCount = checked_cast<u32>(exclusiveData->Batch.WaitSemaphores.size());

    // flush mapped memory before submitting

    TFixedSizeStack<VkMappedMemoryRange, MaxRegions> regions;
    for (FStagingBuffer& buf : exclusiveData->Staging.HostToDevice) {
        if (buf.IsCoherent)
            continue;

        if (regions.full()) {
            VK_CALL( device.vkFlushMappedMemoryRanges(device.vkDevice(), checked_cast<u32>(regions.size()), regions.data()) );
            regions.clear();
        }

        VkMappedMemoryRange* const pRegion = regions.Push_Uninitialized();
        pRegion->sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        pRegion->pNext = nullptr;
        pRegion->memory = buf.DeviceMemory;
        pRegion->offset = checked_cast<VkDeviceSize>(buf.MemoryOffset);
        pRegion->size = checked_cast<VkDeviceSize>(buf.Size);
    }

    if (not regions.empty())
        VK_CALL( device.vkFlushMappedMemoryRanges(device.vkDevice(), checked_cast<u32>(regions.size()), regions.data()) );

    return true;
}
//----------------------------------------------------------------------------
bool FVulkanCommandBatch::OnAfterSubmit(TAppendable<const FVulkanSwapchain*> swapchains, FVulkanSubmitted* submitted) {
    Assert(submitted);
    Assert_NoAssume(nullptr == _submitted);
    SetState_(EState::Ready, EState::Submitted);

    const auto exclusiveData = _data.LockExclusive();

    for (const FVulkanSwapchain* sw : exclusiveData->Swapchains)
        swapchains.push_back(sw);

    exclusiveData->Swapchains.clear();
    exclusiveData->Dependencies.clear();
    _submitted = submitted;

    return true;
}
//----------------------------------------------------------------------------
bool FVulkanCommandBatch::OnComplete(ARG0_IF_RHIDEBUG(FFrameStatistics* pStats, FVulkanDebugger& debugger, const FShaderDebugCallback& callback)) {
    Assert(_submitted);
    SetState_(EState::Submitted, EState::Complete);

    const auto exclusiveData = _data.LockExclusive();
    Unused(exclusiveData); // only for locking

    const FVulkanDevice& device = _frameGraph->Device();
    FVulkanResourceManager& resources = _frameGraph->ResourceManager();

    FinalizeCommands_(*exclusiveData);
    ONLY_IF_RHIDEBUG(ParseDebugOutput_(callback));
    FinalizeStagingBuffers_(device, resources, *exclusiveData);
    ReleaseResources_(resources, *exclusiveData);
    ReleaseVulkanObjects_(device, *exclusiveData);

#if USE_PPE_RHIDEBUG
    debugger.AddBatchDump(_debugName, _frameDebugger.DebugDump.ToString());
    debugger.AddBatchGraph(std::move(_frameDebugger.DebugGraph));

    if (exclusiveData->SupportsQuery) {
        VkQueryPool queryPool = _frameGraph->QueryPool();

        u64 queryResults[2];
        VK_CALL( device.vkGetQueryPoolResults(
            device.vkDevice(), queryPool,
            _indexInPool*2, checked_cast<u32>(lengthof(queryResults)),
            sizeof(queryResults), queryResults,
            sizeof(queryResults[0]), VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT) );

        _statistics.Renderer.GpuTime += FNanoseconds{
            static_cast<double>(queryResults[1] - queryResults[0])
        };
    }
#endif

#if USE_PPE_RHIDEBUG
    pStats->Merge(_statistics);
#endif

    _submitted = nullptr;
    return true;
}
//----------------------------------------------------------------------------
void FVulkanCommandBatch::FinalizeCommands_(FInternalData& data) {
    for (const auto it : data.Batch.Commands) {
        if (FVulkanCommandPool* const pPool = std::get<1>(it))
            pPool->RecyclePrimary(std::get<0>(it));
    }

    data.Batch.Commands.clear();
    data.Batch.SignalSemaphores.clear();
    data.Batch.WaitSemaphores.clear();
}
//----------------------------------------------------------------------------
void FVulkanCommandBatch::FinalizeStagingBuffers_(const FVulkanDevice& device, FVulkanResourceManager& resources, FInternalData& data) {
    using T = FBufferView::value_type;
    TFixedSizeStack<VkMappedMemoryRange, MaxRegions> regions;

    // map device-to-host staging buffers
    for (FStagingBuffer& buf : data.Staging.DeviceToHost) {
        // buffer may be recreate on defragmentation pass, so we need to obtain actual pointer *every* frame
        Verify( MapMemory_(resources, buf) );
        if (buf.IsCoherent)
            continue;

        // invalidate non-coherent memory before reading
        if (regions.full()) {
            VK_CALL( device.vkInvalidateMappedMemoryRanges(device.vkDevice(), checked_cast<u32>(regions.size()), regions.data()) );
            regions.clear();
        }

        VkMappedMemoryRange* pRegion = regions.Push_Uninitialized();
        pRegion->sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        pRegion->pNext = nullptr;
        pRegion->memory = buf.DeviceMemory;
        pRegion->offset = checked_cast<VkDeviceSize>(buf.MemoryOffset);
        pRegion->size = checked_cast<VkDeviceSize>(buf.Size);
    }

    if (not regions.empty())
        VK_CALL( device.vkInvalidateMappedMemoryRanges(device.vkDevice(), checked_cast<u32>(regions.size()), regions.data()) );

    // trigger buffer events
    for (const FOnBufferDataLoadedEvent& evt : data.Staging.OnBufferLoadedEvents) {
        TFixedSizeStack<TMemoryView<const T>, MaxBufferParts> dataParts;
        ONLY_IF_ASSERT(size_t totalSize = 0);

        for (const FStagingDataRange& part : evt.Parts) {
            const auto view = part.MakeView().Cast<T>();
            dataParts.Push(view);
            ONLY_IF_ASSERT(totalSize += part.Size);
        }

        Assert_NoAssume(totalSize == evt.TotalSize);
        evt.Callback(FBufferView{ dataParts.MakeView() });
    }
    data.Staging.OnBufferLoadedEvents.clear();

    // trigger image events
    for (const FOnImageDataLoadedEvent& evt : data.Staging.OnImageLoadedEvents) {
        TFixedSizeStack<TMemoryView<const T>, MaxImageParts> dataParts;
        ONLY_IF_ASSERT(size_t totalSize = 0);

        for (const FStagingDataRange& part : evt.Parts) {
            const auto view = part.MakeView().Cast<T>();
            dataParts.Push(view);
            ONLY_IF_ASSERT(totalSize += part.Size);
        }

        Assert_NoAssume(totalSize == evt.TotalSize);
        evt.Callback(FImageView{ dataParts.MakeView(), evt.ImageSize, evt.RowPitch, evt.SlicePitch, evt.Format, evt.Aspect });
    }
    data.Staging.OnImageLoadedEvents.clear();

    // release resources
    for (const FStagingBuffer& buf : data.Staging.HostToDevice)
        resources.ReleaseStagingBuffer(buf.Index);
    data.Staging.HostToDevice.clear();

    for (const FStagingBuffer& buf : data.Staging.DeviceToHost)
        resources.ReleaseStagingBuffer(buf.Index);
    data.Staging.DeviceToHost.clear();
}
//----------------------------------------------------------------------------
bool FVulkanCommandBatch::MapMemory_(FVulkanResourceManager& resources, FStagingBuffer& staging) {
    FVulkanMemoryInfo info;
    if (resources.ResourceData(staging.MemoryId).MemoryInfo(&info, resources.MemoryManager())) {
        staging.MappedPtr = info.MappedPtr;
        staging.MemoryOffset = info.Offset;
        staging.DeviceMemory = info.Memory;
        staging.IsCoherent = !!(info.Flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        return true;
    }

    return false;
}
//----------------------------------------------------------------------------
void FVulkanCommandBatch::ReleaseResources_(FVulkanResourceManager& resources, FInternalData& data) {
    for (const auto& it : data.ResourcesToRelease) {
        it.first.Visit([&resources, count{ it.second }](auto resourceId) {
            resources.ReleaseResource(resourceId, count);
        });
    }
    data.ResourcesToRelease.clear();
}
//----------------------------------------------------------------------------
void FVulkanCommandBatch::ReleaseVulkanObjects_(const FVulkanDevice& device, FInternalData& data) {
    VkDevice const vkDevice = device.vkDevice();
    VkAllocationCallbacks const* const pAllocator = device.vkAllocator();

    for (auto& it : data.ReadyToDelete) {
        switch (it.first) {
        case VK_OBJECT_TYPE_SEMAPHORE: device.vkDestroySemaphore(vkDevice, it.second.Cast<VkSemaphore>(), pAllocator); break;
        case VK_OBJECT_TYPE_FENCE: device.vkDestroyFence(vkDevice, it.second.Cast<VkFence>(), pAllocator); break;
        case VK_OBJECT_TYPE_DEVICE_MEMORY: device.vkFreeMemory(vkDevice, it.second.Cast<VkDeviceMemory>(), pAllocator); break;
        case VK_OBJECT_TYPE_IMAGE: device.vkDestroyImage(vkDevice, it.second.Cast<VkImage>(), pAllocator); break;
        case VK_OBJECT_TYPE_IMAGE_VIEW: device.vkDestroyImageView(vkDevice, it.second.Cast<VkImageView>(), pAllocator); break;
        case VK_OBJECT_TYPE_EVENT: device.vkDestroyEvent(vkDevice, it.second.Cast<VkEvent>(), pAllocator); break;
        case VK_OBJECT_TYPE_QUERY_POOL: device.vkDestroyQueryPool(vkDevice, it.second.Cast<VkQueryPool>(), pAllocator); break;
        case VK_OBJECT_TYPE_BUFFER: device.vkDestroyBuffer(vkDevice, it.second.Cast<VkBuffer>(), pAllocator); break;
        case VK_OBJECT_TYPE_BUFFER_VIEW: device.vkDestroyBufferView(vkDevice, it.second.Cast<VkBufferView>(), pAllocator); break;
        case VK_OBJECT_TYPE_PIPELINE_LAYOUT: device.vkDestroyPipelineLayout(vkDevice, it.second.Cast<VkPipelineLayout>(), pAllocator); break;
        case VK_OBJECT_TYPE_RENDER_PASS: device.vkDestroyRenderPass(vkDevice, it.second.Cast<VkRenderPass>(), pAllocator); break;
        case VK_OBJECT_TYPE_PIPELINE: device.vkDestroyPipeline(vkDevice, it.second.Cast<VkPipeline>(), pAllocator); break;
        case VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT: device.vkDestroyDescriptorSetLayout(vkDevice, it.second.Cast<VkDescriptorSetLayout>(), pAllocator); break;
        case VK_OBJECT_TYPE_SAMPLER: device.vkDestroySampler(vkDevice, it.second.Cast<VkSampler>(), pAllocator); break;
        case VK_OBJECT_TYPE_DESCRIPTOR_POOL: device.vkDestroyDescriptorPool(vkDevice, it.second.Cast<VkDescriptorPool>(), pAllocator); break;
        case VK_OBJECT_TYPE_FRAMEBUFFER: device.vkDestroyFramebuffer(vkDevice, it.second.Cast<VkFramebuffer>(), pAllocator); break;
        case VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION: device.vkDestroySamplerYcbcrConversion(vkDevice, it.second.Cast<VkSamplerYcbcrConversion>(), pAllocator); break;
        case VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE: device.vkDestroyDescriptorUpdateTemplate(vkDevice, it.second.Cast<VkDescriptorUpdateTemplate>(), pAllocator); break;
        case VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR: device.vkDestroyAccelerationStructureKHR(vkDevice, it.second.Cast<VkAccelerationStructureKHR>(), pAllocator); break;
        case VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV: device.vkDestroyAccelerationStructureNV(vkDevice, it.second.Cast<VkAccelerationStructureNV>(), pAllocator); break;
        default: AssertNotImplemented();
        }
    }

    data.ReadyToDelete.clear();
}
//----------------------------------------------------------------------------
FVulkanCommandBatch::FStagingBuffer* FVulkanCommandBatch::FindOrAddStagingBuffer_(
    FStagingBuffers* pStagingBuffers, size_t stagingSize, EBufferUsage usage,
    size_t srcRequiredSize, size_t blockAlign, size_t offsetAlign, size_t dstMinSize ) const {
    Assert(pStagingBuffers);
    Assert(blockAlign > 0 && offsetAlign > 0);
    Assert_NoAssume(dstMinSize == Meta::RoundToPrev(dstMinSize, blockAlign));

    // search in existing
    size_t maxSize{ 0 };
    FStagingBuffer* suitable{ nullptr };
    FStagingBuffer* maxAvailable{ nullptr };
    for (FStagingBuffer& buf : *pStagingBuffers) {
        const size_t off = Meta::RoundToNext(buf.Size, offsetAlign);
        const size_t available = Meta::RoundToPrev(buf.Capacity - off, blockAlign);

        if (available >= srcRequiredSize) {
            suitable = &buf;
            break;
        }

        if (not maxAvailable || available > maxSize) {
            maxAvailable = &buf;
            maxSize = available;
        }
    }

    // no suitable space, try to use max available block
    if (not suitable && maxAvailable && maxSize >= dstMinSize)
        suitable = maxAvailable;

    // else allocate a new buffer
    if (not suitable) {
        Assert(dstMinSize < stagingSize);
        PPE_LOG_CHECK( RHI, not pStagingBuffers->full() );

        FVulkanResourceManager& resources = _frameGraph->ResourceManager();

        FRawBufferID bufferId;
        FStagingBufferIndex stagingBufferIndex;
        PPE_LOG_CHECK( RHI, resources.CreateStagingBuffer(&bufferId, &stagingBufferIndex, usage) );

        const FRawMemoryID memoryId = resources.ResourceData(bufferId).Read()->MemoryId.Get();
        PPE_LOG_CHECK( RHI, memoryId );

        pStagingBuffers->Push(stagingBufferIndex, bufferId, memoryId, stagingSize);
        suitable = pStagingBuffers->Peek();

        VerifyRelease( MapMemory_(resources, *suitable) );
    }

    Assert(suitable);
    return suitable;
}
//----------------------------------------------------------------------------
bool FVulkanCommandBatch::StageWrite(
    FStagingBlock* pStaging, size_t* pOutSize,
    const size_t srcRequiredSize, const size_t blockAlign, const size_t offsetAlign, const size_t dstMinSize ) {
    Assert(pStaging);
    Assert(pOutSize);

    const auto exclusiveData = _data.LockExclusive();

    FStagingBuffer* const pSuitable = FindOrAddStagingBuffer_(
        &exclusiveData->Staging.HostToDevice,
        _frameGraph->ResourceManager().HostWriteBufferSize(),
        EBufferUsage::TransferSrc,
        srcRequiredSize, blockAlign, offsetAlign, dstMinSize );
    if (not pSuitable)
        return false;

    // write data to buffer
    pStaging->RawBufferID = pSuitable->BufferId;
    pStaging->Offset = Meta::RoundToNext(pSuitable->Size, offsetAlign);
    pStaging->Mapped = (pSuitable->MappedPtr + pStaging->Offset);

    *pOutSize = Min(
        Meta::RoundToPrev(pSuitable->Capacity - pStaging->Offset, blockAlign),
        srcRequiredSize );

    pSuitable->Size = (pStaging->Offset + *pOutSize);
    return true;
}
//----------------------------------------------------------------------------
bool FVulkanCommandBatch::AddPendingLoad(FRawBufferID* pDstBuffer, FStagingDataRange* pRange, size_t srcOffset, size_t srcTotalSize) {
    return AddPendingLoad(pDstBuffer, pRange, srcOffset, srcTotalSize, 1);
}
//----------------------------------------------------------------------------
bool FVulkanCommandBatch::AddPendingLoad(FRawBufferID* pDstBuffer, FStagingDataRange* pRange, size_t srcOffset, size_t srcTotalSize, size_t srcPitch) {
    Assert(pDstBuffer);
    Assert(pRange);

    const auto exclusiveData = _data.LockExclusive();

    const size_t srcRequiredSize = srcTotalSize - srcOffset;
    const size_t blockAlign = srcPitch;
    const size_t offsetAlign = 16;
    const size_t dstMinSize = Max( (srcTotalSize + MaxBufferParts - 1) / MaxBufferParts, srcPitch ); // skip blocks less than 1/N of data size

    FStagingBuffer* const pSuitable = FindOrAddStagingBuffer_(
        &exclusiveData->Staging.DeviceToHost,
        _frameGraph->ResourceManager().HostReadBufferSize(),
        EBufferUsage::TransferDst,
        srcRequiredSize, blockAlign, offsetAlign, dstMinSize );
    if (not pSuitable)
        return false;

    // #TODO: make immutable because read after write happens after waiting for fences, and it implicitly make changes visible to the host

    // write data to buffer
    pRange->Buffer = pSuitable;
    pRange->Offset = Meta::RoundToNext(pSuitable->Size, offsetAlign);
    pRange->Size = Min( Meta::RoundToPrev(pSuitable->Capacity - pRange->Offset, blockAlign), srcRequiredSize );
    *pDstBuffer = pSuitable->BufferId;

    pSuitable->Size = (pRange->Offset + pRange->Size);
    return true;
}
//----------------------------------------------------------------------------
void FVulkanCommandBatch::AddDataLoadedEvent(FOnBufferDataLoadedEvent&& revent) {
    Assert(revent.Callback);
    Assert_NoAssume(not revent.Parts.empty());

    const auto exclusiveData = _data.LockExclusive();

    exclusiveData->Staging.OnBufferLoadedEvents.push_back(std::move(revent));
}
//----------------------------------------------------------------------------
void FVulkanCommandBatch::AddDataLoadedEvent(FOnImageDataLoadedEvent&& revent) {
    Assert(revent.Callback);
    Assert_NoAssume(not revent.Parts.empty());

    const auto exclusiveData = _data.LockExclusive();

    exclusiveData->Staging.OnImageLoadedEvents.push_back(std::move(revent));
}
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
void FVulkanCommandBatch::BeginShaderDebugger_(VkCommandBuffer cmd) {
    Assert(cmd);
    if (_shaderDebugger.Buffers.empty())
        return;

    const FVulkanDevice& device = _frameGraph->Device();
    FVulkanResourceManager& resources = _frameGraph->ResourceManager();

    auto addBarriers = [&device, &resources, cmd, this](VkAccessFlags srcAccess, VkAccessFlags dstAccess, VkPipelineStageFlags dstStage) {
        TFixedSizeStack<VkBufferMemoryBarrier, 16> barriers;
        VkPipelineStageFlags dstStageFlags = 0;

        for (FDebugStorageBuffer& sb : _shaderDebugger.Buffers) {
            VkBufferMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
            barrier.srcAccessMask = srcAccess;
            barrier.dstAccessMask = dstAccess;
            barrier.buffer = resources.ResourceData(sb.ShaderTraceBuffer.Get()).Handle();
            barrier.offset = 0;
            barrier.size = static_cast<VkDeviceSize>(sb.Size);
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            Assert_NoAssume(VK_NULL_HANDLE != barrier.buffer);

            dstStageFlags |= sb.Stages;
            barriers.Push(barrier);

            if (barriers.full()) {
                device.vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
                    dstStageFlags | dstStage, 0,
                    0, nullptr,
                    checked_cast<u32>(barriers.size()), barriers.data(),
                    0 , nullptr );
                barriers.clear();
                dstStageFlags = 0;
            }
        }

        if (barriers.size()) {
            device.vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
                dstStageFlags | dstStage, 0,
                0, nullptr,
                checked_cast<u32>(barriers.size()), barriers.data(),
                0 , nullptr );
        }
    };

    addBarriers(0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

    // clear all buffers to zero
    for (FDebugStorageBuffer& sb : _shaderDebugger.Buffers) {
        VkBuffer buf = resources.ResourceData(sb.ShaderTraceBuffer).Handle();
        Assert(VK_NULL_HANDLE != buf);

        device.vkCmdFillBuffer( cmd, buf, 0, checked_cast<VkDeviceSize>(sb.Size), 0 );
    }

    addBarriers(VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

    // copy data
    for (FDebugMode& dbg : _shaderDebugger.Modes) {
        VkBuffer buf = resources.ResourceData(_shaderDebugger.Buffers[dbg.StorageBufferIndex].ShaderTraceBuffer).Handle();
        Assert(VK_NULL_HANDLE != buf);
        const u32 size = resources.DebugShaderStorageSize(dbg.Stages, dbg.Mode);
        Assert_NoAssume( size <= sizeof(dbg.Payload) );

        device.vkCmdUpdateBuffer( cmd, buf, static_cast<VkDeviceSize>(dbg.Offset), static_cast<VkDeviceSize>(size), &dbg.Payload );
    }

    addBarriers(VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT, 0);
}
#endif
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
void FVulkanCommandBatch::EndShaderDebugger_(VkCommandBuffer cmd) {
    Assert(cmd);
    if (_shaderDebugger.Buffers.empty())
        return;

    const FVulkanDevice& device = _frameGraph->Device();
    FVulkanResourceManager& resources = _frameGraph->ResourceManager();

    // copy to staging buffer
    for (FDebugStorageBuffer& sb : _shaderDebugger.Buffers) {
        VkBuffer srcBuf = resources.ResourceData(sb.ShaderTraceBuffer).Handle();
        VkBuffer dstBuf = resources.ResourceData(sb.ReadBackBuffer).Handle();
        Assert(VK_NULL_HANDLE != srcBuf);
        Assert(VK_NULL_HANDLE != dstBuf);

        VkBufferMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.buffer = srcBuf;
        barrier.offset = 0;
        barrier.size = checked_cast<VkDeviceSize>(sb.Size);
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        device.vkCmdPipelineBarrier( cmd,
            sb.Stages, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
            0, nullptr,
            1, &barrier,
            0, nullptr );

        VkBufferCopy region{};
        region.size = checked_cast<VkDeviceSize>(sb.Size);

        device.vkCmdCopyBuffer( cmd, srcBuf, dstBuf, 1, &region );
    }
}
#endif
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
void FVulkanCommandBatch::SetShaderModuleForDebug(EShaderDebugIndex id, const PVulkanShaderModule& module) {
    Assert(module);

    const auto exclusiveData = _data.LockExclusive();
    Unused(exclusiveData); // only for locking

    FDebugMode& dbg = _shaderDebugger.Modes[static_cast<u32>(id)];
    dbg.Modules.Push(module);
}
#endif
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
bool FVulkanCommandBatch::FindModeInfoForDebug(EShaderDebugMode* pMode, EShaderStages* pStages, EShaderDebugIndex id) const {
    Assert(pMode);
    Assert(pStages);

    const auto sharedData = _data.LockShared();
    Unused(sharedData); // only for locking

    const FDebugMode& dbg = _shaderDebugger.Modes[static_cast<u32>(id)];
    *pMode = dbg.Mode;
    *pStages = dbg.Stages;

    return true;
}
#endif
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
bool FVulkanCommandBatch::FindDescriptorSetForDebug(u32* pBinding, VkDescriptorSet* pSet, u32* pDynamicOffset, EShaderDebugIndex id) const {
    Assert(pBinding);
    Assert(pSet);
    Assert(pDynamicOffset);

    const auto sharedData = _data.LockShared();
    Unused(sharedData); // only for locking

    const FDebugMode& dbg = _shaderDebugger.Modes[static_cast<u32>(id)];
    *pBinding = DebugDescriptorSet;
    *pSet = dbg.DescriptorSet;
    *pDynamicOffset = checked_cast<u32>(dbg.Offset);

    return true;
}
#endif
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
bool FVulkanCommandBatch::FindShaderTimemapForDebug(FRawBufferID* pBuf, size_t* pOffset, size_t* pSize, uint2* pDim, EShaderDebugIndex id) const {
    Assert(pBuf);
    Assert(pOffset);
    Assert(pSize);
    Assert(pDim);

    const auto sharedData = _data.LockShared();
    Unused(sharedData); // only for locking

    const FDebugMode& dbg = _shaderDebugger.Modes[static_cast<u32>(id)];
    *pBuf = _shaderDebugger.Buffers[dbg.StorageBufferIndex].ShaderTraceBuffer.Get();
    *pOffset = dbg.Offset;
    *pSize = dbg.Size;
    *pDim = dbg.Payload.zw;

    return true;
}
#endif
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
EShaderDebugIndex FVulkanCommandBatch::AppendShaderForDebug(TMemoryView<const FRectangle2u>& regions, const FTaskName& name, const FGraphicsShaderDebugMode& mode, size_t size) {
    Assert_NoAssume(EnableShaderDebugging);
    Assert_NoAssume(not name.empty());

    Unused(regions);

    const auto sharedData = _data.LockShared();
    Unused(sharedData); // only for locking

    FDebugMode dbg;
    dbg.TaskName = name;
    dbg.Mode = mode.Mode;
    dbg.Stages = mode.Stages;
    dbg.Payload = uint4(uint2(mode.FragCoord), 0, 0);

    PPE_LOG_CHECK( RHI, AllocStorageForDebug_(dbg, size) );

    _shaderDebugger.Modes.push_back(std::move(dbg));
    return static_cast<EShaderDebugIndex>(_shaderDebugger.Modes.size() - 1);
}
#endif
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
EShaderDebugIndex FVulkanCommandBatch::AppendShaderForDebug(const FTaskName& name, const FComputeShaderDebugMode& mode, size_t size) {
    Assert_NoAssume(EnableShaderDebugging);
    Assert_NoAssume(not name.empty());

    const auto sharedData = _data.LockShared();
    Unused(sharedData); // only for locking

    FDebugMode dbg;
    dbg.TaskName = name;
    dbg.Mode = mode.Mode;
    dbg.Stages = EShaderStages::Compute;
    dbg.Payload = uint4(mode.GlobalId.xyz, 0);

    PPE_LOG_CHECK( RHI, AllocStorageForDebug_(dbg, size) );

    _shaderDebugger.Modes.push_back(std::move(dbg));
    return static_cast<EShaderDebugIndex>(_shaderDebugger.Modes.size() - 1);
}
#endif
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
EShaderDebugIndex FVulkanCommandBatch::AppendShaderForDebug(const FTaskName& name, const FRayTracingShaderDebugMode& mode, size_t size) {
    Assert_NoAssume(EnableShaderDebugging);
    Assert_NoAssume(not name.empty());

    const auto sharedData = _data.LockShared();
    Unused(sharedData); // only for locking

    FDebugMode dbg;
    dbg.TaskName = name;
    dbg.Mode = mode.Mode;
    dbg.Stages = EShaderStages::AllRayTracing;
    dbg.Payload = uint4(mode.LaunchId.xyz, 0);

    PPE_LOG_CHECK( RHI, AllocStorageForDebug_(dbg, size) );

    _shaderDebugger.Modes.push_back(std::move(dbg));
    return static_cast<EShaderDebugIndex>(_shaderDebugger.Modes.size() - 1);
}
#endif
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
EShaderDebugIndex FVulkanCommandBatch::AppendTimemapForDebug(const uint2& dim, EShaderStages stages) {
    Assert_NoAssume(EnableShaderDebugging);

    const auto sharedData = _data.LockShared();
    Unused(sharedData); // only for locking

    FDebugMode dbg;
    dbg.Mode = EShaderDebugMode::Timemap;
    dbg.Stages = stages;

    uint2 tmp = bit_cast<uword>(float2(1.0f));
    dbg.Payload = uint4(tmp, dim);

    const size_t size{
        sizeof(uint4) + // first 4 components
        dim.y * sizeof(u64) + // output pixels
        checked_cast<size_t>(_frameGraph->Device().Limits().minStorageBufferOffsetAlignment) + // padding for alignment
        static_cast<size_t>(dim.x) * dim.y * sizeof(u64) // temporary line
    };

    PPE_LOG_CHECK( RHI, AllocStorageForDebug_(dbg, size) );

    _shaderDebugger.Modes.push_back(std::move(dbg));
    return static_cast<EShaderDebugIndex>(_shaderDebugger.Modes.size() - 1);
}
#endif
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
bool FVulkanCommandBatch::AllocStorageForDebug_(FDebugMode& debugMode, size_t size) {
    Assert(debugMode.StorageBufferIndex == UMax);
    Assert(size > 0);

    VkPipelineStageFlags stages = 0;
    for (u32 s = 1; s <= static_cast<u32>(debugMode.Stages); s <<= 1) {
        if (not Meta::EnumHas(debugMode.Stages, s))
            continue;

        switch (static_cast<EShaderStages>(s)) {
        case EShaderStages::Vertex: stages |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT; break;
        case EShaderStages::TessControl: stages |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT; break;
        case EShaderStages::TessEvaluation: stages |= VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT; break;
        case EShaderStages::Geometry: stages |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT; break;
        case EShaderStages::Fragment: stages |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; break;
        case EShaderStages::Compute: stages |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT; break;

        case EShaderStages::MeshTask:
        case EShaderStages::Mesh:
#ifdef VK_NV_mesh_shader
            stages |= VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV;
#else
            AssertNotReached();
#endif
            break;

        case EShaderStages::RayGen:
        case EShaderStages::RayAnyHit:
        case EShaderStages::RayClosestHit:
        case EShaderStages::RayMiss:
        case EShaderStages::RayIntersection:
        case EShaderStages::RayCallable:
#ifdef VK_NV_ray_tracing
            stages |= VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV;
#else
            AssertNotReached();
#endif
            break;

        default: AssertNotImplemented();
        }
    }

    debugMode.Size = Min(size, _shaderDebugger.BufferSize);

    // find available space in existing storage buffers
    for (FDebugStorageBuffer& sb : _shaderDebugger.Buffers) {
        debugMode.Offset = Meta::RoundToNext(sb.Size, _shaderDebugger.BufferAlign);

        if (debugMode.Size <= (sb.Capacity - debugMode.Offset)) {
            debugMode.StorageBufferIndex = checked_cast<u32>(&sb - _shaderDebugger.Buffers.data());
            sb.Size = debugMode.Offset + size;
            sb.Stages |= stages;
            break;
        }
    }

    // create a new storage buffer
    if (debugMode.StorageBufferIndex == UMax) {
        debugMode.StorageBufferIndex = checked_cast<u32>(_shaderDebugger.Buffers.size());
        debugMode.Offset = 0;

        FDebugStorageBuffer sb;
        sb.Capacity = (_shaderDebugger.BufferSize * (1 + _shaderDebugger.Buffers.size() / 2));
        sb.Size = debugMode.Offset + size;
        sb.Stages = stages;

        sb.ShaderTraceBuffer = _frameGraph->CreateBuffer(
            FBufferDesc{ sb.Capacity, EBufferUsage::Storage | EBufferUsage_Transfer },
            Default, "DebugOutputStorage" );
        PPE_LOG_CHECK( RHI, sb.ShaderTraceBuffer );

        sb.ReadBackBuffer = _frameGraph->CreateBuffer(
            FBufferDesc{ sb.Capacity, EBufferUsage::TransferDst },
            FMemoryDesc{ EMemoryType::HostRead },
            "DebugOutputReadBack" );
        PPE_LOG_CHECK( RHI, sb.ReadBackBuffer );

        _shaderDebugger.Buffers.push_back(std::move(sb));
    }
    Assert_NoAssume(debugMode.StorageBufferIndex != UMax);

    PPE_LOG_CHECK( RHI, AllocDescriptorSetForDebug_(
        &debugMode.DescriptorSet, debugMode.Mode, debugMode.Stages,
        _shaderDebugger.Buffers[debugMode.StorageBufferIndex].ShaderTraceBuffer.Get(),
        debugMode.Size ));

    return true;
}
#endif
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
bool FVulkanCommandBatch::AllocDescriptorSetForDebug_(
    VkDescriptorSet* pDescSet,
    EShaderDebugMode debugMode, EShaderStages stages, FRawBufferID storageBuffer, size_t size ) {
    Assert(pDescSet);
    Assert(storageBuffer);
    Assert(size > 0);

    const FVulkanDevice& device = _frameGraph->Device();
    FVulkanResourceManager& resources = _frameGraph->ResourceManager();

    const auto [layoutId, layoutCacheHit] = resources.CreateDebugDescriptorSetLayout(debugMode, stages);
    const FVulkanDescriptorSetLayout& layout = resources.ResourceData(layoutId, layoutCacheHit);
    const FVulkanBuffer& buffer = resources.ResourceData(storageBuffer);

    // find descriptor set in cache
    const auto it = _shaderDebugger.DescriptorCache.find({ storageBuffer, layoutId });
    if (_shaderDebugger.DescriptorCache.end() != it) {
        *pDescSet = it->second.First;
        return true; // cache hit
    }

    // allocate descriptor set
    FVulkanDescriptorSet ds;
    PPE_LOG_CHECK( RHI, resources.DescriptorManager().AllocateDescriptorSet(&ds, layout.Handle() ARGS_IF_RHIDEBUG("Debug")));
    Assert_NoAssume(VK_NULL_HANDLE != ds.First);
    *pDescSet = ds.First;

    _shaderDebugger.DescriptorCache.insert_AssertUnique({ { storageBuffer, layoutId }, ds });

    // update descriptor set
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = buffer.Handle();
    bufferInfo.offset = 0;
    bufferInfo.range = checked_cast<VkDeviceSize>(size);

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = *pDescSet;
    write.dstBinding = 0;
    write.descriptorCount = 1;
    write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    write.pBufferInfo = &bufferInfo;

    device.vkUpdateDescriptorSets(device.vkDevice(), 1, &write, 0, nullptr);

    return true;
}
#endif
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
void FVulkanCommandBatch::ParseDebugOutput_(const FShaderDebugCallback& callback) {
    if (_shaderDebugger.Buffers.empty())
        return;

    FVulkanResourceManager& resources = _frameGraph->ResourceManager();

    // release descriptor sets
    for (auto& ds : _shaderDebugger.DescriptorCache) {
        resources.DescriptorManager().DeallocateDescriptorSet(ds.second);
        resources.ReleaseResource(ds.first.second);
    }
    _shaderDebugger.DescriptorCache.clear();

    // process shader debug output
    if (callback.Valid()) {
        FDebugStrings outp;
        outp.reserve(_shaderDebugger.Modes.size());

        for (FDebugMode& dbg : _shaderDebugger.Modes)
            Verify( ParseDebugOutput2_(&outp, callback, dbg) );
    }
    _shaderDebugger.Modes.clear();

    // release storage buffers
    for (FDebugStorageBuffer& buf : _shaderDebugger.Buffers) {
        resources.ReleaseResource(buf.ShaderTraceBuffer.Release());
        resources.ReleaseResource(buf.ReadBackBuffer.Release());
    }
    _shaderDebugger.Buffers.clear();
}
#endif
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
bool FVulkanCommandBatch::ParseDebugOutput2_(FDebugStrings* pDump, const FShaderDebugCallback& callback, const FDebugMode& dbg) const {
    Assert(pDump);
    Assert(callback);

    if (dbg.Mode == EShaderDebugMode::Timemap)
        return true;
    PPE_LOG_CHECK( RHI, not dbg.Modules.empty() );

    FVulkanResourceManager& resources = _frameGraph->ResourceManager();
    const FRawBufferID readBackBufferId = _shaderDebugger.Buffers[dbg.StorageBufferIndex].ReadBackBuffer.Get();

    const FVulkanBuffer& buf = resources.ResourceData(readBackBufferId);
    const FVulkanMemoryObject& mem = resources.ResourceData(buf.Read()->MemoryId.Get());

    FVulkanMemoryInfo info;
    PPE_LOG_CHECK( RHI, mem.MemoryInfo(&info, resources.MemoryManager()) );

    Assert( info.MappedPtr );
    for (const PVulkanShaderModule& shader : dbg.Modules) {
        const FRawMemoryConst trace{ static_cast<const u8*>(info.MappedPtr) + dbg.Offset, dbg.Size };

        const size_t numMessagesBefore = pDump->size();
        PPE_LOG_CHECK( RHI, shader->ParseDebugOutput(MakeAppendable(*pDump), dbg.Mode, trace) );

        callback(dbg.TaskName, shader->DebugName().MakeView(), dbg.Stages, pDump->MakeView().CutStartingAt(numMessagesBefore));
    }

    return true;
}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
