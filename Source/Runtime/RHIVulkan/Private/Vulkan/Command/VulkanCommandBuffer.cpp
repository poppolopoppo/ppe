#include "stdafx.h"

#include "Vulkan/Command/VulkanCommandBuffer.h"

#include "Vulkan/Command/VulkanTaskGraph-inl.h"
#include "Vulkan/Debugger/VulkanLocalDebugger.h"
#include "Vulkan/Instance/VulkanFrameGraph.h"

#include "Diagnostic/Logger.h"
#include "Time/TimedScope.h"
#include "Vulkan/RayTracing/VulkanGeometryInstance.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static CONSTEXPR EQueueUsage GGraphicsBit_ = EQueueUsage::Graphics;
static CONSTEXPR EQueueUsage GComputeBit_ = EQueueUsage::Graphics | EQueueUsage::AsyncCompute;
static CONSTEXPR EQueueUsage GRayTracingBit_ = EQueueUsage::Graphics | EQueueUsage::AsyncCompute;
static CONSTEXPR EQueueUsage GTransferBit_ = EQueueUsage::Graphics | EQueueUsage::AsyncCompute |
    EQueueUsage::AsyncTransfer;
#if USE_PPE_RHIDEBUG
static CONSTEXPR EDebugFlags GCmdDebugFlags_ = EDebugFlags::FullBarrier | EDebugFlags::QueueSync;
#endif
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVulkanCommandBuffer::FVulkanCommandBuffer(const SVulkanFrameGraph& fg, u32 indexInPool)
    : _frameGraph(fg)
      , _indexInPool(indexInPool) {
    Assert(_frameGraph);

    const auto exclusive = _data.LockExclusive();
    exclusive->MainAllocator.SetSlabSize(16_MiB);

    ResetLocalRemapping_(*exclusive);
}
//----------------------------------------------------------------------------
FVulkanCommandBuffer::~FVulkanCommandBuffer() {
    auto exclusive = Write();
    Assert_NoAssume(EState::Initial == exclusive->State);

    const FVulkanDevice& device = _frameGraph->Device();
    for (FVulkanCommandPool& pool : exclusive->PerQueue) {
        if (pool.Valid())
            pool.TearDown(device);
    }

    exclusive->PerQueue.clear();
}
//----------------------------------------------------------------------------
const FVulkanDevice& FVulkanCommandBuffer::Device() const NOEXCEPT {
    return _frameGraph->Device();
}
//----------------------------------------------------------------------------
FVulkanResourceManager& FVulkanCommandBuffer::ResourceManager() const NOEXCEPT {
    return _frameGraph->ResourceManager();
}
//----------------------------------------------------------------------------
bool FVulkanCommandBuffer::Begin(
    const FCommandBufferDesc& desc,
    const SVulkanCommandBatch& batch,
    const PVulkanDeviceQueue& queue) {
    const auto exclusive = Write();
    Assert_NoAssume(EState::Initial == exclusive->State);

    exclusive->Batch = batch;
    exclusive->State = EState::Recording;
    exclusive->QueueIndex = queue->FamilyIndex;
#if USE_PPE_RHIDEBUG
    exclusive->DebugName = desc.Name;
    exclusive->DebugFullBarriers = (desc.DebugFlags & EDebugFlags::FullBarrier);
    exclusive->DebugQueueSync = (desc.DebugFlags & EDebugFlags::QueueSync);
#endif

    // create command pool
    {
        const u32 index = static_cast<u32>(exclusive->QueueIndex);
        exclusive->PerQueue.resize(index + 1);

        if (not exclusive->PerQueue[index].Valid() &&
            not exclusive->PerQueue[index].Construct(Device(), queue ARGS_IF_RHIDEBUG(queue->DebugName))) {
            RHI_LOG(Error, L"failed to construct command pool for '{0}'", exclusive->DebugName);
            return false;
        }
    }

    exclusive->Batch->OnBegin(desc);

#if USE_PPE_RHIDEBUG
    // setup local debugger
    const EDebugFlags debugFlags = (desc.DebugFlags - GCmdDebugFlags_);

    if (debugFlags != Default) {
        if (not exclusive->Debugger)
            exclusive->Debugger.reset<FVulkanLocalDebugger>();

        exclusive->Debugger->Begin(debugFlags);
    }
    else {
        exclusive->Debugger.reset();
    }
#endif

    exclusive->TaskGraph.Construct(exclusive->MainAllocator);
    return true;
}
//----------------------------------------------------------------------------
bool FVulkanCommandBuffer::Execute() {
    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);
    exclusive->State = EState::Compiling;

    RHI_PROFILINGSCOPE("BakeCommandBuffer", &exclusive->Batch->_statistics.Renderer.CpuTime);

    if (Unlikely(not BuildCommandBuffers_(*exclusive))) {
        RHI_LOG(Error, L"failed to build command buffers for '{0}'", exclusive->DebugName);
        return false;
    }

#if USE_PPE_RHIDEBUG
    if (Unlikely(exclusive->Debugger)) {
        exclusive->Debugger->End(
            &exclusive->Batch->_frameDebugger.DebugDump,
            &exclusive->Batch->_frameDebugger.DebugGraph,
            exclusive->DebugName, _indexInPool);
    }
#endif

    if (Unlikely(not exclusive->Batch->OnBaked(exclusive->RM.ResourceMap))) {
        RHI_LOG(Error, L"failed to bake command batch for '{0}'", exclusive->DebugName);
        return false;
    }

    exclusive->TaskGraph.TearDown();

    AfterCompilation_(*exclusive);

    exclusive->MainAllocator.DiscardAll();
    exclusive->Batch = nullptr;
    exclusive->State = EState::Initial;
    return true;
}
//----------------------------------------------------------------------------
void FVulkanCommandBuffer::AfterCompilation_(FInternalData& data) {
#if USE_PPE_RHIDEBUG
    // reset global shader debugger
    {
        data.ShaderDbg.TimemapIndex = Default;
        data.ShaderDbg.TimemapStages = Default;
    }
#endif

    // destroy logical render passes
    FVulkanResourceManager& resources = ResourceManager();

    forrange(i, 0, checked_cast<FResourceIndex>(data.RM.LogicalRenderPassCount)) {
        TResourceProxy<FVulkanLogicalRenderPass>* const pPass = data.RM.LogicalRenderPasses[i];
        Assert(pPass);

        if (pPass->Valid()) {
            pPass->TearDown(resources);
            Meta::Destroy(pPass);
            data.RM.LogicalRenderPasses.Deallocate(i);
        }
    }

    data.RM.LogicalRenderPassCount = 0;
}
//----------------------------------------------------------------------------
bool FVulkanCommandBuffer::BuildCommandBuffers_(FInternalData& data) {
    const FVulkanDevice& device = Device();

    // create command buffer
    VkCommandBuffer cmd;
    {
        FVulkanCommandPool& pool = data.PerQueue[static_cast<u32>(data.QueueIndex)];
        cmd = pool.AllocPrimary(device);
        Assert(VK_NULL_HANDLE != cmd);

        data.Batch->PushCommandToBack(&pool, cmd);
    }

    // begin
    {
        VkCommandBufferBeginInfo info{};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        VK_CALL(device.vkBeginCommandBuffer(cmd, &info));

        data.Batch->OnBeforeRecording(cmd);

        VkMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        data.BarrierManager.AddMemoryBarrier(
            VK_PIPELINE_STAGE_HOST_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            barrier);
    }

    // commit image layout transition and other
    data.BarrierManager.Commit(device, cmd);

    if (not ProcessTasks_(data, cmd)) {
        RHI_LOG(Error, L"failed to process tasks for '{0}'", data.DebugName);
        return false;
    }

    // transit image layout to default state
    // add memory dependency to flush caches
    {
        VkMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
        data.BarrierManager.AddMemoryBarrier(
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_HOST_BIT,
            barrier);

        FlushLocalResourceStates_(
            data, EVulkanExecutionOrder::Final, data.BarrierManager
            ARGS_IF_RHIDEBUG(data.Debugger.get()) );

        data.BarrierManager.ForceCommit(
            device, cmd,
            device.AllWritableStages(),
            device.AllReadableStages() );
    }

    // end
    {
        data.Batch->OnAfterRecording(cmd);

        VK_CALL(device.vkEndCommandBuffer(cmd));
    }

    return true;
}
//----------------------------------------------------------------------------
bool FVulkanCommandBuffer::ProcessTasks_(FInternalData& data, VkCommandBuffer cmd) {
    FVulkanTaskProcessor processor{this, cmd};

    const u32 visitorId{ 1 };
    EVulkanExecutionOrder executionOrder{EVulkanExecutionOrder::First};

    FTransientTaskArray pending{data.MainAllocator};
    pending.reserve(128);
    pending.assign(data.TaskGraph.Entries().begin(), data.TaskGraph.Entries().end());

    STATIC_CONST_INTEGRAL(u32, MaxLoops, 10);
    for (u32 loop = 0; loop < MaxLoops && not pending.empty(); ++loop) {
        for (size_t i = 0; i < pending.size();) {
            const PVulkanFrameTask pTask = pending[i];

            if (pTask->VisitorId() == visitorId) {
                ++i;
                continue;
            }

            // wait for input
            const bool inputProcessed = pTask->Inputs().Any(
                [visitorId](PVulkanFrameTask pInput) NOEXCEPT -> bool {
                    return (pInput->VisitorId() != visitorId);
                });
            if (not inputProcessed) {
                ++i;
                continue;
            }

            // process the task
            pTask->SetVisitorId(visitorId);
            pTask->SetExecutionOrder(++executionOrder);

            processor.Run(*pTask);

            pending.erase(pending.begin() + i);
            Append(pending, pTask->Outputs());
        }
    }

    Assert_NoAssume(pending.empty());
    return true;
}
//----------------------------------------------------------------------------
void FVulkanCommandBuffer::SignalSemaphore(VkSemaphore semaphore) {
    const auto exclusive = Write();
    Assert_NoAssume(
        EState::Recording == exclusive->State ||
        EState::Compiling == exclusive->State);

    exclusive->Batch->SignalSemaphore(semaphore);
}
//----------------------------------------------------------------------------
void FVulkanCommandBuffer::WaitSemaphore(VkSemaphore semaphore, VkPipelineStageFlags stage) {
    const auto exclusive = Write();
    Assert_NoAssume(
        EState::Recording == exclusive->State ||
        EState::Compiling == exclusive->State);

    exclusive->Batch->WaitSemaphore(semaphore, stage);
}
//----------------------------------------------------------------------------
FRawImageID FVulkanCommandBuffer::SwapchainImage(FRawSwapchainID swapchainId, ESwapchainImage type) {
    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);

    const FVulkanSwapchain* const pSwapchain = AcquireTransient(swapchainId);
    Assert(pSwapchain);

    FRawImageID imageId;
    VerifyRelease(pSwapchain->Acquire(&imageId, *this, type ARGS_IF_RHIDEBUG(exclusive->DebugQueueSync)));
    Assert_NoAssume(imageId);

    // transit to undefined layout
    AcquireImage(imageId, true, true);

    Add_Unique(exclusive->Batch->_data.LockExclusive()->Swapchains,
               static_cast<const FVulkanSwapchain*>(pSwapchain));

    return imageId;
}
//----------------------------------------------------------------------------
bool FVulkanCommandBuffer::ExternalCommands(const FVulkanExternalCommandBatch& info) {
    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);
    AssertRelease(info.QueueFamily == exclusive->QueueIndex);

    for (VkCommandBuffer cmd : info.Commands)
        exclusive->Batch->PushCommandToBack(nullptr, cmd);

    for (VkSemaphore semaphore : info.SignalSemaphores)
        exclusive->Batch->SignalSemaphore(semaphore);

    for (const TPair<VkSemaphore, VkPipelineStageFlags>& it : info.WaitSemaphores)
        exclusive->Batch->WaitSemaphore(it.first, it.second);

    return true;
}
//----------------------------------------------------------------------------
bool FVulkanCommandBuffer::DependsOn(const FCommandBufferBatch& cmd) {
    if (not cmd.Batch || cmd.Buffer == this)
        return false;

    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);

    exclusive->Batch->DependsOn(
        checked_cast<FVulkanCommandBatch*>(cmd.Batch.get()));

    return true;
}
//----------------------------------------------------------------------------
bool FVulkanCommandBuffer::StagingAlloc(FStagingBlock* pStaging, size_t size, size_t align) {
    Assert(pStaging);

    const auto exclusive = Write();
    Assert_NoAssume(
        EState::Recording == exclusive->State ||
        EState::Compiling == exclusive->State);

    u32 bufferSize;
    return exclusive->Batch->StageWrite(
        &pStaging->RawBufferID,
        &pStaging->Offset,
        &bufferSize,
        &pStaging->Mapped,
        checked_cast<u32>(size),
        1u,
        checked_cast<u32>(align),
        checked_cast<u32>(size));
}
//----------------------------------------------------------------------------
void FVulkanCommandBuffer::AcquireImage(FRawImageID id, bool makeMutable, bool invalidate) {
    Assert(id);

    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);

    if (FVulkanLocalImage* const pLocalImage = ToLocal(id))
        pLocalImage->SetInitialState(not makeMutable, invalidate);
}
//----------------------------------------------------------------------------
void FVulkanCommandBuffer::AcquireBuffer(FRawBufferID id, bool makeMutable) {
    Assert(id);

    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);

    if (FVulkanLocalBuffer* const pLocalBuffer = ToLocal(id))
        pLocalBuffer->SetInitialState(not makeMutable);
}
//----------------------------------------------------------------------------
// Task()
//----------------------------------------------------------------------------
PFrameTask FVulkanCommandBuffer::Task(const FSubmitRenderPass& task) {
    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);
    Assert_NoAssume(exclusive->Batch->QueueUsage() & GGraphicsBit_);

    TVulkanFrameTask<FSubmitRenderPass>* const pFrameTask = exclusive->TaskGraph.AddTask(*this, task);
    Assert(pFrameTask);

#if USE_PPE_RHIDEBUG
    // #TODO: add scale to shader timemap
    if (exclusive->ShaderDbg.TimemapStages & EShaderStages::Fragment &&
        exclusive->ShaderDbg.TimemapIndex != Default) {
        pFrameTask->LogicalPass()->SetShaderDebugIndex(exclusive->ShaderDbg.TimemapIndex);
    }
#endif

    return pFrameTask;
}
//----------------------------------------------------------------------------
PFrameTask FVulkanCommandBuffer::Task(const FDispatchCompute& task) {
    Assert(task.Pipeline);

    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);
    Assert_NoAssume(exclusive->Batch->QueueUsage() & GComputeBit_);

    TVulkanFrameTask<FDispatchCompute>* const pFrameTask = exclusive->TaskGraph.AddTask(*this, task);
    Assert(pFrameTask);

#if USE_PPE_RHIDEBUG
    if (exclusive->ShaderDbg.TimemapStages & EShaderStages::Compute &&
        exclusive->ShaderDbg.TimemapIndex != Default) {
        Assert_NoAssume(pFrameTask->DebugModeIndex != Default);
        pFrameTask->DebugModeIndex = exclusive->ShaderDbg.TimemapIndex;
    }
#endif

    return pFrameTask;
}
//----------------------------------------------------------------------------
PFrameTask FVulkanCommandBuffer::Task(const FDispatchComputeIndirect& task) {
    Assert(task.Pipeline);

    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);
    Assert_NoAssume(exclusive->Batch->QueueUsage() & GComputeBit_);

    TVulkanFrameTask<FDispatchComputeIndirect>* const pFrameTask = exclusive->TaskGraph.AddTask(*this, task);
    Assert(pFrameTask);

#if USE_PPE_RHIDEBUG
    if (exclusive->ShaderDbg.TimemapStages & EShaderStages::Compute &&
        exclusive->ShaderDbg.TimemapIndex != Default) {
        Assert_NoAssume(pFrameTask->DebugModeIndex != Default);
        pFrameTask->DebugModeIndex = exclusive->ShaderDbg.TimemapIndex;
    }
#endif

    return pFrameTask;
}
//----------------------------------------------------------------------------
PFrameTask FVulkanCommandBuffer::Task(const FCopyBuffer& task) {
    Assert(not task.Regions.empty());

    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);
    Assert_NoAssume(exclusive->Batch->QueueUsage() & GTransferBit_);

    TVulkanFrameTask<FCopyBuffer>* const pFrameTask = exclusive->TaskGraph.AddTask(*this, task);
    Assert(pFrameTask);

    return pFrameTask;
}
//----------------------------------------------------------------------------
PFrameTask FVulkanCommandBuffer::Task(const FCopyImage& task) {
    Assert(not task.Regions.empty());

    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);
    Assert_NoAssume(exclusive->Batch->QueueUsage() & GTransferBit_);

    TVulkanFrameTask<FCopyImage>* const pFrameTask = exclusive->TaskGraph.AddTask(*this, task);
    Assert(pFrameTask);

    return pFrameTask;
}
//----------------------------------------------------------------------------
PFrameTask FVulkanCommandBuffer::Task(const FCopyBufferToImage& task) {
    Assert(not task.Regions.empty());

    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);
    Assert_NoAssume(exclusive->Batch->QueueUsage() & GTransferBit_);

    TVulkanFrameTask<FCopyBufferToImage>* const pFrameTask = exclusive->TaskGraph.AddTask(*this, task);
    Assert(pFrameTask);

    return pFrameTask;
}
//----------------------------------------------------------------------------
PFrameTask FVulkanCommandBuffer::Task(const FCopyImageToBuffer& task) {
    Assert(not task.Regions.empty());

    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);
    Assert_NoAssume(exclusive->Batch->QueueUsage() & GTransferBit_);

    TVulkanFrameTask<FCopyImageToBuffer>* const pFrameTask = exclusive->TaskGraph.AddTask(*this, task);
    Assert(pFrameTask);

    return pFrameTask;
}
//----------------------------------------------------------------------------
PFrameTask FVulkanCommandBuffer::Task(const FBlitImage& task) {
    Assert(not task.Regions.empty());

    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);
    Assert_NoAssume(exclusive->Batch->QueueUsage() & GGraphicsBit_);

    TVulkanFrameTask<FBlitImage>* const pFrameTask = exclusive->TaskGraph.AddTask(*this, task);
    Assert(pFrameTask);

    return pFrameTask;
}
//----------------------------------------------------------------------------
PFrameTask FVulkanCommandBuffer::Task(const FResolveImage& task) {
    Assert(not task.Regions.empty());

    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);
    Assert_NoAssume(exclusive->Batch->QueueUsage() & GGraphicsBit_);

    TVulkanFrameTask<FResolveImage>* const pFrameTask = exclusive->TaskGraph.AddTask(*this, task);
    Assert(pFrameTask);

    return pFrameTask;
}
//----------------------------------------------------------------------------
PFrameTask FVulkanCommandBuffer::Task(const FGenerateMipmaps& task) {
    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);
    Assert_NoAssume(exclusive->Batch->QueueUsage() & GGraphicsBit_);

    TVulkanFrameTask<FGenerateMipmaps>* const pFrameTask = exclusive->TaskGraph.AddTask(*this, task);
    Assert(pFrameTask);

    return pFrameTask;
}
//----------------------------------------------------------------------------
PFrameTask FVulkanCommandBuffer::Task(const FFillBuffer& task) {
    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);

    TVulkanFrameTask<FFillBuffer>* const pFrameTask = exclusive->TaskGraph.AddTask(*this, task);
    Assert(pFrameTask);

    return pFrameTask;
}
//----------------------------------------------------------------------------
PFrameTask FVulkanCommandBuffer::Task(const FClearColorImage& task) {
    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);
    Assert_NoAssume(exclusive->Batch->QueueUsage() & GComputeBit_);

    TVulkanFrameTask<FClearColorImage>* const pFrameTask = exclusive->TaskGraph.AddTask(*this, task);
    Assert(pFrameTask);

    return pFrameTask;
}
//----------------------------------------------------------------------------
PFrameTask FVulkanCommandBuffer::Task(const FClearDepthStencilImage& task) {
    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);
    Assert_NoAssume(exclusive->Batch->QueueUsage() & GComputeBit_);

    TVulkanFrameTask<FClearDepthStencilImage>* const pFrameTask = exclusive->TaskGraph.AddTask(*this, task);
    Assert(pFrameTask);

    return pFrameTask;
}
//----------------------------------------------------------------------------
PFrameTask FVulkanCommandBuffer::Task(const FUpdateBuffer& task) {
    Assert(not task.Regions.empty());

    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);
    Assert_NoAssume(exclusive->Batch->QueueUsage() & GTransferBit_);

    return MakeUpdateBufferTask_(*exclusive, task);
}
//----------------------------------------------------------------------------
PFrameTask FVulkanCommandBuffer::Task(const FUpdateImage& task) {
    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);
    Assert_NoAssume(exclusive->Batch->QueueUsage() & GTransferBit_);

    return MakeUpdateImageTask_(*exclusive, task);
}
//----------------------------------------------------------------------------
PFrameTask FVulkanCommandBuffer::Task(const FReadBuffer& task) {
    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);
    Assert_NoAssume(exclusive->Batch->QueueUsage() & GTransferBit_);

    return MakeReadBufferTask_(*exclusive, task);
}
//----------------------------------------------------------------------------
PFrameTask FVulkanCommandBuffer::Task(const FReadImage& task) {
    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);
    Assert_NoAssume(exclusive->Batch->QueueUsage() & GTransferBit_);

    return MakeReadImageTask_(*exclusive, task);
}
//----------------------------------------------------------------------------
PFrameTask FVulkanCommandBuffer::Task(const FPresent& task) {
    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);

    TVulkanFrameTask<FPresent>* const pFrameTask = exclusive->TaskGraph.AddTask(*this, task);

    if (Likely(pFrameTask))
        exclusive->Batch->_data.LockExclusive()->Swapchains.Push(pFrameTask->Swapchain);

    return pFrameTask;
}
//----------------------------------------------------------------------------
PFrameTask FVulkanCommandBuffer::Task(const FCustomTask& task) {
    Assert(task.Callback);

    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);

    TVulkanFrameTask<FCustomTask>* const pFrameTask = exclusive->TaskGraph.AddTask(*this, task);
    Assert_NoAssume(pFrameTask);

    return pFrameTask;
}
//----------------------------------------------------------------------------
// RayTracing
//----------------------------------------------------------------------------
PFrameTask FVulkanCommandBuffer::Task(const FUpdateRayTracingShaderTable& task) {
    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);
    Assert_NoAssume(Device().Enabled().RayTracingKHR);
    Assert_NoAssume(exclusive->Batch->QueueUsage() & GRayTracingBit_);

    TVulkanFrameTask<FUpdateRayTracingShaderTable>* const pFrameTask =
        exclusive->TaskGraph.AddTask(*this, task);

    return pFrameTask;
}
//----------------------------------------------------------------------------
PFrameTask FVulkanCommandBuffer::Task(const FBuildRayTracingGeometry& task) {
    Assert(task.Geometry);

#if 0 // #TODO: porting NV to KHR
    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);
    Assert_NoAssume(Device().EnableRayTracingKHR());
    Assert_NoAssume(exclusive->Batch->QueueUsage() & RayTracingBit_);

    FVulkanRTLocalGeometry* const rtGeometry = ToLocal(task.Geometry);
    Assert(rtGeometry);
    Assert_NoAssume(task.Aabbs.size() <= rtGeometry->Aabbs().size());
    Assert_NoAssume(task.Triangles.size() <= rtGeometry->Triangles().size());

    TVulkanFrameTask<FBuildRayTracingGeometry>* const pFrameTask =
        exclusive->TaskGraph.AddTask(*this, task);
    Assert(pFrameTask);
    pFrameTask->RTGeometry = rtGeometry;

    VkMemoryRequirements2 memReq2{};
    VkBufferMemoryRequirementsInfo2 bufInfo2{};
    bufInfo2.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2_KHR;
    bufInfo2.buffer = rtGeometry->();
#else
    AssertNotImplemented();
#endif
}
//----------------------------------------------------------------------------
PFrameTask FVulkanCommandBuffer::Task(const FBuildRayTracingScene& task) {
    Assert(task.Scene);

    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);
    Assert_NoAssume(Device().Enabled().RayTracingKHR);
    Assert_NoAssume(exclusive->Batch->QueueUsage() & GRayTracingBit_);

    FVulkanRTLocalScene* const rtScene = ToLocal(task.Scene);
    Assert(rtScene);
    Assert_NoAssume(task.Instances.size() <= rtScene->MaxInstanceCount());

    TVulkanFrameTask<FBuildRayTracingScene>* const pBuildTask =
        exclusive->TaskGraph.AddTask(*this, task);
    Assert(pBuildTask);
    Assert_NoAssume(task.Instances.size() <= rtScene->MaxInstanceCount());
    pBuildTask->RTScene = rtScene;

    VkMemoryRequirements2 memReq2{};
    // #TODO: support for VK_KHR_acceleration_structure instead of VK_NV_ray_tracing

    FMemoryDesc mem;
    mem.Type = EMemoryType::Default;
    mem.Alignment = checked_cast<u32>(memReq2.memoryRequirements.alignment);
    mem.ExternalRequirements = checked_cast<u32>(memReq2.memoryRequirements.memoryTypeBits);

    // #TODO: virtual buffer of buffer cache
    FBufferID scratchBuf = _frameGraph->CreateBuffer(FBufferDesc{
        checked_cast<u32>(memReq2.memoryRequirements.size),
        EBufferUsage::RayTracing,
    },  mem ARGS_IF_RHIDEBUG("ScratchBuffer") );
    Assert(scratchBuf);

    pBuildTask->ScratchBuffer = ToLocal(*scratchBuf);
    ReleaseResource(scratchBuf.Release());

    // #TODO: virtual buffer of buffer cache
    FBufferID instanceBuf = _frameGraph->CreateBuffer(FBufferDesc{
        checked_cast<u32>(task.Instances.MakeView().SizeInBytes()),
        EBufferUsage::TransferDst + EBufferUsage::RayTracing
    },  mem ARGS_IF_RHIDEBUG("InstanceBuffer") );
    Assert(instanceBuf);

    pBuildTask->InstanceBuffer = ToLocal(*instanceBuf);
    ReleaseResource(instanceBuf.Release());

    FVulkanGeometryInstance* pInstances = nullptr;
    LOG_CHECK(RHI, StagingAlloc_<FVulkanGeometryInstance>(
        *exclusive,
        &pBuildTask->InstanceStagingBuffer,
        &pBuildTask->InstanceStagingBufferOffset,
        &pInstances,
        task.Instances.size() ));
    Assert_NoAssume(pBuildTask->ScratchBuffer->InternalData()->Desc.Usage & EBufferUsage::RayTracing);
    Assert_NoAssume(pBuildTask->InstanceBuffer->InternalData()->Desc.Usage & EBufferUsage::RayTracing);

    // sort instance by instance ID
    STACKLOCAL_POD_ARRAY(u32, sorted, task.Instances.size());
    forrange(i, 0, checked_cast<u32>(sorted.size())) sorted[i] = i;
    std::sort(sorted.begin(), sorted.end(), [&instances{task.Instances}](auto lhs, auto rhs) NOEXCEPT {
        return (instances[lhs].InstanceId < instances[rhs].InstanceId);
    });

    pBuildTask->HitShadersPerInstance = Max(1u, task.HitShadersPerInstance);
    pBuildTask->NumInstances = checked_cast<u32>(task.Instances.size());

    using FInstance = FVulkanBuildRayTracingSceneTask::FInstance;
    pBuildTask->Instances = exclusive->MainAllocator.AllocateT<FInstance>(pBuildTask->NumInstances);
    pBuildTask->RTGeometries = exclusive->MainAllocator.AllocateT<const FVulkanRTLocalGeometry*>(pBuildTask->NumInstances);

    forrange(i, 0, pBuildTask->NumInstances) {
        const u32 idx = sorted[i];
        const auto& src = task.Instances[i];
        FVulkanGeometryInstance& dst = pInstances[idx];
        const FVulkanRayTracingLocalGeometry*& pLocalGeom = pBuildTask->RTGeometries[i];
        Assert_NoAssume(src.InstanceId.Valid());
        Assert_NoAssume(src.GeometryId.Valid());
        Assert_NoAssume((src.CustomId >> 24) == 0);

        pLocalGeom = ToLocal(src.GeometryId);
        AssertRelease(pLocalGeom);

        dst.BlasHandle = pLocalGeom->BLAS();
        dst.Transform = src.Transform;
        dst.CustomIndex = src.CustomId;
        dst.Mask = src.Mask;
        dst.InstanceOffset = pBuildTask->MaxHitShaderCount;
        dst.Flags = VkCast(src.Flags);

        INPLACE_NEW(&pBuildTask->Instances[idx], FInstance) {
            src.InstanceId,
            FRTGeometryID{ src.GeometryId },
            dst.InstanceOffset
        };

        pBuildTask->MaxHitShaderCount += (pLocalGeom->MaxGeometryCount() * pBuildTask->HitShadersPerInstance);
    }

    return pBuildTask;
}
//----------------------------------------------------------------------------
PFrameTask FVulkanCommandBuffer::Task(const FTraceRays& task) {
    Assert(task.ShaderTable);

    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);
    Assert_NoAssume(Device().Enabled().RayTracingKHR);
    Assert_NoAssume(exclusive->Batch->QueueUsage() & GRayTracingBit_);

    TVulkanFrameTask<FTraceRays>* const pFrameTask =
        exclusive->TaskGraph.AddTask(*this, task);
    Assert(pFrameTask);

#if USE_PPE_RHIDEBUG
    if (exclusive->ShaderDbg.TimemapIndex != Default &&
        exclusive->ShaderDbg.TimemapStages ^ EShaderStages::AllRayTracing) {
        Assert_NoAssume(pFrameTask->DebugModeIndex == Default);

        pFrameTask->DebugModeIndex = exclusive->ShaderDbg.TimemapIndex;
    }
#endif

    return pFrameTask;
}
//----------------------------------------------------------------------------
// Draw
//----------------------------------------------------------------------------
void FVulkanCommandBuffer::Task(FLogicalPassID renderPass, const FDrawVertices& draw) {
    Assert(not draw.Commands.empty());
    Assert(draw.Pipeline);

    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);

    FVulkanLogicalRenderPass* const pRenderPass = ToLocal(renderPass);
    Assert(pRenderPass);

    pRenderPass->EmplaceTask<TVulkanDrawTask<FDrawVertices>>(
        *this, draw,
        &FVulkanTaskProcessor::Visit1_DrawVertices,
        &FVulkanTaskProcessor::Visit2_DrawVertices);
}
//----------------------------------------------------------------------------
void FVulkanCommandBuffer::Task(FLogicalPassID renderPass, const FDrawIndexed& draw) {
    Assert(not draw.Commands.empty());
    Assert(draw.Pipeline);

    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);

    FVulkanLogicalRenderPass* const pRenderPass = ToLocal(renderPass);
    Assert(pRenderPass);

    pRenderPass->EmplaceTask<TVulkanDrawTask<FDrawIndexed>>(
        *this, draw,
        &FVulkanTaskProcessor::Visit1_DrawIndexed,
        &FVulkanTaskProcessor::Visit2_DrawIndexed);
}
//----------------------------------------------------------------------------
void FVulkanCommandBuffer::Task(FLogicalPassID renderPass, const FDrawMeshes& draw) {
    Assert(not draw.Commands.empty());
    Assert(draw.Pipeline);

    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);
    Assert_NoAssume(Device().Enabled().MeshShaderNV);

    FVulkanLogicalRenderPass* const pRenderPass = ToLocal(renderPass);
    Assert(pRenderPass);

    pRenderPass->EmplaceTask<TVulkanDrawTask<FDrawMeshes>>(
        *this, draw,
        &FVulkanTaskProcessor::Visit1_DrawMeshes,
        &FVulkanTaskProcessor::Visit2_DrawMeshes);
}
//----------------------------------------------------------------------------
void FVulkanCommandBuffer::Task(FLogicalPassID renderPass, const FDrawVerticesIndirect& draw) {
    Assert(not draw.Commands.empty());
    Assert(draw.Pipeline);
    Assert(draw.IndirectBuffer);

    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);

    FVulkanLogicalRenderPass* const pRenderPass = ToLocal(renderPass);
    Assert(pRenderPass);

    pRenderPass->EmplaceTask<TVulkanDrawTask<FDrawVerticesIndirect>>(
        *this, draw,
        &FVulkanTaskProcessor::Visit1_DrawVerticesIndirect,
        &FVulkanTaskProcessor::Visit2_DrawVerticesIndirect);
}
//----------------------------------------------------------------------------
void FVulkanCommandBuffer::Task(FLogicalPassID renderPass, const FDrawIndexedIndirect& draw) {
    Assert(not draw.Commands.empty());
    Assert(draw.Pipeline);
    Assert(draw.IndirectBuffer);

    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);

    FVulkanLogicalRenderPass* const pRenderPass = ToLocal(renderPass);
    Assert(pRenderPass);

    pRenderPass->EmplaceTask<TVulkanDrawTask<FDrawIndexedIndirect>>(
        *this, draw,
        &FVulkanTaskProcessor::Visit1_DrawIndexedIndirect,
        &FVulkanTaskProcessor::Visit2_DrawIndexedIndirect);
}
//----------------------------------------------------------------------------
void FVulkanCommandBuffer::Task(FLogicalPassID renderPass, const FDrawMeshesIndirect& draw) {
    Assert(not draw.Commands.empty());
    Assert(draw.Pipeline);
    Assert(draw.IndirectBuffer);

    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);

    FVulkanLogicalRenderPass* const pRenderPass = ToLocal(renderPass);
    Assert(pRenderPass);

    pRenderPass->EmplaceTask<TVulkanDrawTask<FDrawMeshesIndirect>>(
        *this, draw,
        &FVulkanTaskProcessor::Visit1_DrawMeshesIndirect,
        &FVulkanTaskProcessor::Visit2_DrawMeshesIndirect);
}
//----------------------------------------------------------------------------
void FVulkanCommandBuffer::Task(FLogicalPassID renderPass, const FDrawVerticesIndirectCount& draw) {
    Assert(not draw.Commands.empty());
    Assert(draw.Pipeline);
    Assert(draw.IndirectBuffer);
    Assert(draw.CountBuffer);

    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);
    Assert_NoAssume(Device().Enabled().DrawIndirectCount);

    FVulkanLogicalRenderPass* const pRenderPass = ToLocal(renderPass);
    Assert(pRenderPass);

    pRenderPass->EmplaceTask<TVulkanDrawTask<FDrawVerticesIndirectCount>>(
        *this, draw,
        &FVulkanTaskProcessor::Visit1_DrawVerticesIndirectCount,
        &FVulkanTaskProcessor::Visit2_DrawVerticesIndirectCount);
}
//----------------------------------------------------------------------------
void FVulkanCommandBuffer::Task(FLogicalPassID renderPass, const FDrawIndexedIndirectCount& draw) {
    Assert(not draw.Commands.empty());
    Assert(draw.Pipeline);
    Assert(draw.IndirectBuffer);
    Assert(draw.CountBuffer);

    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);
    Assert_NoAssume(Device().Enabled().DrawIndirectCount);

    FVulkanLogicalRenderPass* const pRenderPass = ToLocal(renderPass);
    Assert(pRenderPass);

    pRenderPass->EmplaceTask<TVulkanDrawTask<FDrawIndexedIndirectCount>>(
        *this, draw,
        &FVulkanTaskProcessor::Visit1_DrawIndexedIndirectCount,
        &FVulkanTaskProcessor::Visit2_DrawIndexedIndirectCount);
}
//----------------------------------------------------------------------------
void FVulkanCommandBuffer::Task(FLogicalPassID renderPass, const FDrawMeshesIndirectCount& draw) {
    Assert(not draw.Commands.empty());
    Assert(draw.Pipeline);
    Assert(draw.IndirectBuffer);
    Assert(draw.CountBuffer);

    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);
    Assert_NoAssume(Device().Enabled().MeshShaderNV);

    FVulkanLogicalRenderPass* const pRenderPass = ToLocal(renderPass);
    Assert(pRenderPass);

    pRenderPass->EmplaceTask<TVulkanDrawTask<FDrawMeshesIndirectCount>>(
        *this, draw,
        &FVulkanTaskProcessor::Visit1_DrawMeshesIndirectCount,
        &FVulkanTaskProcessor::Visit2_DrawMeshesIndirectCount);
}
//----------------------------------------------------------------------------
void FVulkanCommandBuffer::Task(FLogicalPassID renderPass, const FCustomDraw& draw) {
    Assert(draw.Callback);

    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);

    FVulkanLogicalRenderPass* const pRenderPass = ToLocal(renderPass);
    Assert(pRenderPass);

    pRenderPass->EmplaceTask<TVulkanDrawTask<FCustomDraw>>(
        *this, draw,
        &FVulkanTaskProcessor::Visit1_CustomDraw,
        &FVulkanTaskProcessor::Visit2_CustomDraw);
}
//----------------------------------------------------------------------------
// RenderPass
//----------------------------------------------------------------------------
FLogicalPassID FVulkanCommandBuffer::CreateRenderPass(const FRenderPassDesc& desc) {
    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);
    Assert_NoAssume(exclusive->Batch->QueueUsage() & GGraphicsBit_);

    FLogicalRenderPasses& pool = exclusive->RM.LogicalRenderPasses;
    const FResourceIndex index{pool.Allocate()};

    TResourceProxy<FVulkanLogicalRenderPass>* const pLogicalPass = pool[index];
    AssertMessage(L"logical render pass pool overflow", pLogicalPass);

    Meta::Construct(pLogicalPass);

    if (Unlikely(not pLogicalPass->Construct(*this, desc))) {
        RHI_LOG(Error, L"failed to construct logical render pass");
        Meta::Destroy(pLogicalPass);
        pool.Deallocate(index);
        return Default;
    }

    exclusive->RM.LogicalRenderPassCount = Max(
        static_cast<u32>(index) + 1,
        exclusive->RM.LogicalRenderPassCount);

    const FLogicalPassID logicalPassId{index, pLogicalPass->InstanceID()};
    Assert_NoAssume(logicalPassId.Valid());
    return logicalPassId;
}
//----------------------------------------------------------------------------
// Shader time map
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
bool FVulkanCommandBuffer::BeginShaderTimeMap(const uint2& dim, EShaderStages stages) {
    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);
    Assert_NoAssume(exclusive->ShaderDbg.TimemapIndex == Default); // already started
    Assert_NoAssume(exclusive->Batch->QueueUsage() & GComputeBit_);

    exclusive->ShaderDbg.TimemapStages = stages;
    exclusive->ShaderDbg.TimemapIndex = exclusive->Batch->AppendTimemapForDebug(dim, stages);

    return (exclusive->ShaderDbg.TimemapIndex != Default);
}
#endif
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
PFrameTask FVulkanCommandBuffer::EndShaderTimeMap(
    FRawImageID dstImage,
    FImageLayer layer,
    FMipmapLevel level,
    TMemoryView<PFrameTask> dependsOn) {
    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);
    Assert_NoAssume(exclusive->ShaderDbg.TimemapIndex != Default); // need to call BeginShaderTimeMap()
    Assert_NoAssume(exclusive->Batch->QueueUsage() & GComputeBit_);

    FVulkanResourceManager& rm = ResourceManager();
    const FImageDesc desc = rm.ResourceDescription(dstImage);
    Assert_NoAssume(desc.Type == EImageDim_2D);
    Assert_NoAssume(desc.Usage & EImageUsage::Storage);

    FVulkanResourceManager::FShaderTimemapPipelines pplns;
    rm.ShaderTimemapPipelines(&pplns);
    Assert(pplns[0] && pplns[1] && pplns[2]);

    FRawBufferID ssb;
    u32 ssbTimemapOffset, ssbTimemapSize;
    uint2 ssbDim;
    LOG_CHECK(RHI, exclusive->Batch->FindShaderTimemapForDebug(
        &ssb, &ssbTimemapOffset, &ssbTimemapSize, &ssbDim,
        exclusive->ShaderDbg.TimemapIndex) );

    const u32 ssbAlign = checked_cast<u32>(_frameGraph->Device().Limits().minStorageBufferOffsetAlignment);
    const u32 ssbMaxValuesSize = ssbDim.y * sizeof(u64);
    ssbTimemapSize -= ssbMaxValuesSize + ssbAlign;
    const u32 ssbMaxValuesOffset = Meta::RoundToNext(ssbTimemapOffset + ssbTimemapSize, ssbAlign);

    FPipelineResources resources;
    CONSTEXPR const FDescriptorSetID descriptorSetId0{ "0" };
    CONSTEXPR const FUniformID un_Timemap{ "un_Timemap" };
    CONSTEXPR const FUniformID un_MaxValues{ "un_MaxValues" };

    PFrameTask pPreviousTask;

    // pass 1
    {
        const FRawCPipelineID ppln = pplns[0];
        LOG_CHECK(RHI, _frameGraph->InitPipelineResources(&resources, ppln, descriptorSetId0) );

        resources.BindBuffer(un_Timemap, ssb, ssbTimemapOffset, ssbTimemapSize);
        resources.BindBuffer(un_MaxValues, ssb, ssbMaxValuesOffset, ssbMaxValuesSize);

        FDispatchCompute cs;
        cs.SetPipeline(ppln).SetLocalSize({ 32, 1, 1 }).Dispatch({ (ssbDim.y + 31) / 32, 1, 1 });
        cs.AddResources(descriptorSetId0, &resources);
        cs.Dependencies.Assign(dependsOn.begin(), dependsOn.end());

        pPreviousTask = Task(cs);
    }
    // pass 2
    {
        const FRawCPipelineID ppln = pplns[1];
        LOG_CHECK(RHI, _frameGraph->InitPipelineResources(&resources, ppln, descriptorSetId0) );

        resources.BindBuffer(un_Timemap, ssb, ssbTimemapOffset, ssbTimemapSize);
        resources.BindBuffer(un_MaxValues, ssb, ssbMaxValuesOffset, ssbMaxValuesSize);

        FDispatchCompute cs;
        cs.SetPipeline(ppln).SetLocalSize(uint3::One).Dispatch(uint3::One);
        cs.AddResources(descriptorSetId0, &resources);
        cs.DependsOn(pPreviousTask);

        pPreviousTask = Task(cs);
    }
    // pass 3
    {
        const FRawCPipelineID ppln = pplns[2];
        LOG_CHECK(RHI, _frameGraph->InitPipelineResources(&resources, ppln, descriptorSetId0) );

        resources.BindBuffer(un_Timemap, ssb, ssbTimemapOffset, ssbTimemapSize);
        resources.BindImage(FUniformID{ "un_OutImage" }, dstImage, FImageViewDesc{}.SetType(EImageView_2D).SetArrayLayers(*layer, 1).SetBaseLevel(*level));

        FDispatchCompute cs;
        cs.SetPipeline(ppln).SetLocalSize({ 8, 8, 1 }).Dispatch({ (desc.Dimensions.xy + 7u) / 8u });
        cs.AddResources(descriptorSetId0, &resources);
        cs.DependsOn(pPreviousTask);

        pPreviousTask = Task(cs);
    }

    return pPreviousTask;
}
#endif
//----------------------------------------------------------------------------
// ResetLocalRemapping_
//----------------------------------------------------------------------------
void FVulkanCommandBuffer::ResetLocalRemapping_(FInternalData& data) {
    auto resetLocalResources = [](auto& resources) {
        FPlatformMemory::Memset(resources.ToLocal.data(), UMax,
            sizeof(*resources.ToLocal.data()) * resources.MaxGlobalIndex);
        resources.MaxGlobalIndex = 0;
    };

    resetLocalResources(data.RM.Images);
    resetLocalResources(data.RM.Buffers);
    resetLocalResources(data.RM.RTGeometries);
    resetLocalResources(data.RM.RTScenes);
}
//----------------------------------------------------------------------------
// FlushLocalResourceStates
//----------------------------------------------------------------------------
void FVulkanCommandBuffer::FlushLocalResourceStates_(
    FInternalData& data,
    EVulkanExecutionOrder order,
    FVulkanBarrierManager& barriers
    ARGS_IF_RHIDEBUG(FVulkanLocalDebugger* pDebugger)) {
    auto resetStateAndDestroyLocalResources = [order, &barriers ARGS_IF_RHIDEBUG(pDebugger)](auto& localResources) {
        forrange(i, 0, checked_cast<FResourceIndex>(localResources.MaxLocalIndex)) {
            auto* const pResource = localResources.Pool[i];
            Assert(pResource);

            if (not pResource->IsDestroyed()) {
                pResource->Data().ResetState(order, barriers ARGS_IF_RHIDEBUG(pDebugger));
                pResource->TearDown();
                localResources.Pool.Deallocate(i);
            }
        }
        localResources.MaxLocalIndex = 0;
    };

    resetStateAndDestroyLocalResources(data.RM.Images);
    resetStateAndDestroyLocalResources(data.RM.Buffers);
    resetStateAndDestroyLocalResources(data.RM.RTGeometries);
    resetStateAndDestroyLocalResources(data.RM.RTScenes);

    ResetLocalRemapping_(data);
}
//----------------------------------------------------------------------------
// ToLocal_
//----------------------------------------------------------------------------
template <u32 _Uid, typename _Resource, typename _MainPool, size_t _MaxChunks>
_Resource* FVulkanCommandBuffer::ToLocal_(
    details::TResourceId<_Uid> id,
    TLocalPool<_Resource, _MainPool, _MaxChunks>& localResources
    ARGS_IF_RHIDEBUG(FWStringView debugMessage) ) {
    Assert(id);
    Assert_NoAssume(EState::Recording == _data.Value_NotThreadSafe().State ||
                    EState::Compiling == _data.Value_NotThreadSafe().State );

    if (id.Index >= localResources.ToLocal.size())
        return nullptr;

    FResourceIndex& local = localResources.ToLocal[id.Index];
    if (local != UMax) {
        _Resource* const pResource = std::addressof(localResources.Pool[local]->Data());
        Assert_NoAssume(pResource->GlobalData());
        return pResource;
    }

    auto* const pResource = AcquireTransient(id);
    if (not pResource)
        return nullptr;

    local = localResources.Pool.Allocate();
    auto* const pData = localResources.Pool[local];
    Meta::Construct(pData);

    if (not pData->Construct(pResource)) {
        Meta::Destroy(pData);
        localResources.Pool.Deallocate(local);
        ONLY_IF_RHIDEBUG(RHI_LOG(Error, L"{1}: {0}", debugMessage, _data.Value_NotThreadSafe().DebugName));
        return nullptr;
    }

    localResources.MaxLocalIndex = Max(checked_cast<u32>(local) + 1, localResources.MaxLocalIndex);
    localResources.MaxGlobalIndex = Max(checked_cast<u32>(id.Index) + 1, localResources.MaxGlobalIndex);

    return std::addressof(pData->Data());
}
//----------------------------------------------------------------------------
FVulkanLocalBuffer* FVulkanCommandBuffer::ToLocal(FRawBufferID id) {
    return ToLocal_(id, Write()->RM.Buffers
        ARGS_IF_RHIDEBUG(L"failed when creating local buffer"));
}
//----------------------------------------------------------------------------
FVulkanLocalImage* FVulkanCommandBuffer::ToLocal(FRawImageID id) {
    return ToLocal_(id, Write()->RM.Images
           ARGS_IF_RHIDEBUG(L"failed when creating local image"));
}
//----------------------------------------------------------------------------
FVulkanRayTracingLocalGeometry* FVulkanCommandBuffer::ToLocal(FRawRTGeometryID id) {
    return ToLocal_(id, Write()->RM.RTGeometries
           ARGS_IF_RHIDEBUG(L"failed when creating local ray tracing geometry"));
}
//----------------------------------------------------------------------------
FVulkanRayTracingLocalScene* FVulkanCommandBuffer::ToLocal(FRawRTSceneID id) {
    return ToLocal_(id, Write()->RM.RTScenes
           ARGS_IF_RHIDEBUG(L"failed when creating local ray tracing scene"));
}
//----------------------------------------------------------------------------
FVulkanLogicalRenderPass* FVulkanCommandBuffer::ToLocal(FLogicalPassID id) {
    Assert(id);
    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State ||
                    EState::Compiling == exclusive->State );

    TResourceProxy<FVulkanLogicalRenderPass>* const pLogicalPass = exclusive->RM.LogicalRenderPasses[id.Index];
    Assert(pLogicalPass);
    Assert_NoAssume(pLogicalPass->IsCreated());

    return std::addressof(pLogicalPass->Data());
}
//----------------------------------------------------------------------------
// Staging copy helpers
//----------------------------------------------------------------------------
PFrameTask FVulkanCommandBuffer::MakeUpdateBufferTask_(FInternalData& data, const FUpdateBuffer& task) {
    FCopyBuffer copy{};
    copy.DstBuffer = task.DstBuffer;
    copy.Dependencies = task.Dependencies;
#if USE_PPE_RHITASKNAME
    copy.TaskName = task.TaskName;
    copy.DebugColor = task.DebugColor;
#endif

    // copy source staging buffer
    for (const auto& region : task.Regions) {

        for (u32 srcOffset = 0; srcOffset < region.Data.SizeInBytes();) {
            FRawBufferID srcBuffer;
            u32 offset, size;
            if (not StorePartialData_(data,
                                         &srcBuffer, &offset, &size,
                                         region.Data, srcOffset)) {
                RHI_LOG(Error, L"failed to write partial staging data for '{0}' in '{1}'", task.TaskName,
                        data.DebugName);
                return nullptr;
            }

            if (copy.SrcBuffer && srcBuffer != copy.SrcBuffer) {
                const PFrameTask pLastTask = Task(copy);
                Assert(pLastTask);

                copy.Regions.clear();
                copy.Dependencies.clear();
                copy.Dependencies.Push(pLastTask);
            }

            copy.AddRegion(offset, region.Offset + srcOffset, size);

            srcOffset += size;
            copy.SrcBuffer = srcBuffer;
        }
    }

    return Task(copy);
}
//----------------------------------------------------------------------------
PFrameTask FVulkanCommandBuffer::MakeUpdateImageTask_(FInternalData& data, const FUpdateImage& task) {
    Assert(task.DstImage);
    Assert(Any(GreaterMask(task.ImageSize, uint3::Zero)));

    const FVulkanImage* const pImage = AcquireTransient(task.DstImage);

    const FImageDesc& desc = pImage->Read()->Desc;
    Assert_NoAssume(task.MipmapLevel < desc.MaxLevel);
    Assert_NoAssume(task.ArrayLayer < desc.ArrayLayers);

    const FPixelFormatInfo pixelInfo = EPixelFormat_Infos(desc.Format);
    const uint3 imageSize = Max(task.ImageSize, 1u);
    const uint2& blockDim = pixelInfo.BlockDim;
    const u32 blockBits = (task.AspectMask != EImageAspect::Stencil
                               ? pixelInfo.BitsPerBlock0
                               : pixelInfo.BitsPerBlock1);

    const u32 rowPitch = Max(task.DataRowPitch,
                             (imageSize.x * blockBits + blockDim.x - 1) / (blockDim.x * 8/* bits to bytes */));
    const u32 minSlicePitch = ((imageSize.y * rowPitch + blockDim.y - 1) / blockDim.y);
    const u32 slicePitch = Max(task.DataSlicePitch, minSlicePitch);
    const u32 totalSizeInBytes = (imageSize.z > 1 ? slicePitch * imageSize.z : minSlicePitch);

    if (totalSizeInBytes != task.Data.SizeInBytes()) {
        RHI_LOG(Error, L"invalid data supplied for image '{0}' update in '{1}'", pImage->DebugName(), data.DebugName);
        return nullptr;
    }

    const u32 minSizeInBytes = _frameGraph->ResourceManager().HostWriteBufferSize() / 4;
    const u32 rowLength = ((rowPitch * blockDim.x * 8/* bits to bytes */) / blockBits);
    const u32 imageHeight = ((slicePitch * blockDim.y) / rowPitch);

    FCopyBufferToImage copy{};
    copy.DstImage = task.DstImage;
    copy.Dependencies = task.Dependencies;
#if USE_PPE_RHIDEBUG
    copy.TaskName = task.TaskName;
    copy.DebugColor = task.DebugColor;
#endif

    Assert_NoAssume(Meta::IsAligned(blockDim.x, task.ImageOffset.x));
    Assert_NoAssume(Meta::IsAligned(blockDim.y, task.ImageOffset.y));

    // copy to staging buffer slice by slice
    if (totalSizeInBytes < minSizeInBytes) {
        u32 zOffset = 0;
        for (u32 srcOffset = 0; srcOffset < totalSizeInBytes;) {
            FRawBufferID srcBuffer;
            u32 offset, size;
            if (not StagingImageStore_(data,
                                       &srcBuffer, &offset, &size,
                                       task.Data, srcOffset, slicePitch, totalSizeInBytes)) {
                RHI_LOG(Error, L"failed to write image slice to staging for '{0}' in '{1}'", task.TaskName,
                        data.DebugName);
                return nullptr;
            }

            if (copy.SrcBuffer && srcBuffer != copy.SrcBuffer) {
                const PFrameTask pLastTask = Task(copy);
                Assert(pLastTask);

                copy.Regions.clear();
                copy.Dependencies.clear();
                copy.Dependencies.Push(pLastTask);
            }

            const u32 zSize = checked_cast<u32>(size / slicePitch);
            Assert_NoAssume(Meta::IsAligned(blockDim.x, imageSize.x));
            Assert_NoAssume(Meta::IsAligned(blockDim.y, imageSize.y));

            copy.AddRegion(
                offset, rowLength, imageHeight,
                FImageSubresourceRange{task.MipmapLevel, task.ArrayLayer, 1, task.AspectMask},
                task.ImageOffset + int3(0, 0, zOffset),
                uint3{imageSize.xy, zSize});

            srcOffset += size;
            zOffset += zSize;
            copy.SrcBuffer = srcBuffer;
        }

        Assert_NoAssume(zOffset == imageSize.z);
    }
        // copy to staging buffer row by row
    else {
        forrange(slice, 0, imageSize.z) {
            u32 yOffset = 0;
            const FRawMemoryConst sliceData = task.Data.SubRange(slice * slicePitch, slicePitch);
            const u32 sliceSize = checked_cast<u32>(sliceData.SizeInBytes());

            for (u32 srcOffset = 0; srcOffset < sliceSize;) {
                FRawBufferID srcBuffer;
                u32 offset, size;
                if (not StagingImageStore_(data,
                                           &srcBuffer, &offset, &size,
                                           sliceData, srcOffset, rowPitch * blockDim.y, totalSizeInBytes)) {
                    RHI_LOG(Error, L"failed to write image row to staging for '{0}' in '{1}'", task.TaskName,
                            data.DebugName);
                    return nullptr;
                }

                if (copy.SrcBuffer && srcBuffer != copy.SrcBuffer) {
                    const PFrameTask pLastTask = Task(copy);
                    Assert(pLastTask);

                    copy.Regions.clear();
                    copy.Dependencies.clear();
                    copy.Dependencies.Push(pLastTask);
                }

                const u32 ySize = checked_cast<u32>((size * blockDim.y) / rowPitch);
                Assert_NoAssume(Meta::IsAligned(blockDim.x, imageSize.x));
                Assert_NoAssume(Meta::IsAligned(blockDim.y, ySize));
                Assert_NoAssume(Meta::IsAligned(blockDim.y, task.ImageOffset.y + yOffset));

                copy.AddRegion(
                    offset, rowLength, ySize,
                    FImageSubresourceRange{task.MipmapLevel, task.ArrayLayer, 1, task.AspectMask},
                    task.ImageOffset + int3(0, yOffset, slice),
                    uint3{imageSize.x, ySize, 1});

                srcOffset += size;
                yOffset += ySize;
                copy.SrcBuffer = srcBuffer;
            }

            Assert_NoAssume(yOffset == imageSize.y);
        }
    }

    return Task(copy);
}
//----------------------------------------------------------------------------
PFrameTask FVulkanCommandBuffer::MakeReadBufferTask_(FInternalData& data, const FReadBuffer& task) {
    Assert(task.SrcBuffer && task.Callback);

    using FOnDataLoadedEvent = FVulkanCommandBatch::FOnBufferDataLoadedEvent;

    FCopyBuffer copy{};
    copy.SrcBuffer = task.SrcBuffer;
    copy.Dependencies = task.Dependencies;
#if USE_PPE_RHIDEBUG
    copy.TaskName = task.TaskName;
    copy.DebugColor = task.DebugColor;
#endif

    FOnDataLoadedEvent loadEvent{task.Callback, task.SrcSize};

    // copy to staging buffer
    for (u32 srcOffset = 0; srcOffset < task.SrcSize;) {
        FRawBufferID dstBuffer;
        FVulkanCommandBatch::FStagingDataRange range;
        if (not data.Batch->AddPendingLoad(&dstBuffer, &range, srcOffset, task.SrcSize)) {
            RHI_LOG(Error, L"failed copy buffer to staging of '{0}' in '{1}'", copy.TaskName, data.DebugName);
            return nullptr;
        }

        if (copy.DstBuffer && dstBuffer != copy.DstBuffer) {
            const PFrameTask pLastTask = Task(copy);
            Assert(pLastTask);

            copy.Regions.clear();
            copy.Dependencies.clear();
            copy.Dependencies.Push(pLastTask);
        }

        loadEvent.Parts.Push(range);
        copy.AddRegion(task.SrcOffset + srcOffset, range.Offset, range.Size);

        srcOffset += range.Size;
        copy.DstBuffer = dstBuffer;
    }

    data.Batch->AddDataLoadedEvent(std::move(loadEvent));

    return Task(copy);
}
//----------------------------------------------------------------------------
PFrameTask FVulkanCommandBuffer::MakeReadImageTask_(FInternalData& data, const FReadImage& task) {
    Assert(task.SrcImage && task.Callback);
    Assert(Any(GreaterMask(task.ImageSize, uint3::Zero)));

    using FOnDataLoadedEvent = FVulkanCommandBatch::FOnImageDataLoadedEvent;

    const FVulkanImage* const pImage = AcquireTransient(task.SrcImage);
    Assert(pImage);

    const FImageDesc& desc = pImage->Read()->Desc;
    Assert_NoAssume(task.MipmapLevel < desc.MaxLevel);
    Assert_NoAssume(task.ArrayLayer < desc.ArrayLayers);

    const uint3 imageSize = Max(task.ImageSize, 1u);
    const u32 minSizeInBytes = _frameGraph->ResourceManager().HostReadBufferSize();
    const FPixelFormatInfo pixelInfo = EPixelFormat_Infos(desc.Format);
    const uint2 blockDim = pixelInfo.BlockDim;
    const u32 blockBits = (task.AspectMask != EImageAspect::Stencil
                               ? pixelInfo.BitsPerBlock0
                               : pixelInfo.BitsPerBlock1);
    const u32 rowPitch = ((imageSize.x * blockBits + blockDim.x - 1) / (blockDim.x * 8/* bits to byte */));
    const u32 slicePitch = ((imageSize.y * rowPitch + blockDim.y - 1) / blockDim.y);
    const u32 totalSizeInBytes = slicePitch * imageSize.z;
    const u32 rowLength = imageSize.x;
    const u32 imageHeight = imageSize.y;

    FCopyImageToBuffer copy{};
    copy.SrcImage = task.SrcImage;
    copy.Dependencies = task.Dependencies;
#if USE_PPE_RHIDEBUG
    copy.TaskName = task.TaskName;
    copy.DebugColor = task.DebugColor;
#endif

    FOnDataLoadedEvent loadEvent{
        task.Callback, totalSizeInBytes, imageSize, rowPitch, slicePitch, desc.Format, task.AspectMask
    };

    // copy to staging buffer slice by slice
    if (totalSizeInBytes < minSizeInBytes) {
        u32 zOffset = 0;
        for (u32 srcOffset = 0; srcOffset < totalSizeInBytes;) {
            FRawBufferID dstBuffer;
            FVulkanCommandBatch::FStagingDataRange range;
            if (not data.Batch->AddPendingLoad(
                &dstBuffer, &range,
                srcOffset, totalSizeInBytes, slicePitch)) {
                RHI_LOG(Error, L"failed copy image row to staging of '{0}' in '{1}'", copy.TaskName, data.DebugName);
                return nullptr;
            }

            if (copy.DstBuffer && copy.DstBuffer != dstBuffer) {
                const PFrameTask pLastTask = Task(copy);
                Assert_NoAssume(pLastTask);

                copy.Regions.clear();
                copy.Dependencies.clear();
                copy.Dependencies.Push(pLastTask);
            }

            const u32 zSize = (range.Size / slicePitch);
            copy.AddRegion(
                FImageSubresourceRange(task.MipmapLevel, task.ArrayLayer, 1, task.AspectMask),
                task.ImageOffset + int3(0, 0, zOffset),
                uint3(task.ImageSize.xy, zSize),
                range.Offset, rowLength, imageHeight);

            loadEvent.Parts.Push(range);

            srcOffset += range.Size;
            zOffset += zSize;
            copy.DstBuffer = dstBuffer;
        }

        Assert_NoAssume(zOffset == imageSize.z);
    }
        // copy to staging buffer row by row
    else {
        forrange(slice, 0, imageSize.z) {
            u32 yOffset = 0;
            for (u32 srcOffset = 0; srcOffset < slicePitch;) {
                FRawBufferID dstBuffer;
                FVulkanCommandBatch::FStagingDataRange range;
                if (not data.Batch->AddPendingLoad(
                    &dstBuffer, &range,
                    srcOffset, totalSizeInBytes, rowPitch * blockDim.y)) {
                    RHI_LOG(Error, L"failed copy image slice to staging of '{0}' in '{1}'", copy.TaskName,
                            data.DebugName);
                    return nullptr;
                }

                if (copy.DstBuffer && copy.DstBuffer != dstBuffer) {
                    const PFrameTask pLastTask = Task(copy);
                    Assert_NoAssume(pLastTask);

                    copy.Regions.clear();
                    copy.Dependencies.clear();
                    copy.Dependencies.Push(pLastTask);
                }

                const u32 ySize = ((range.Size * blockDim.y) / rowPitch);
                copy.AddRegion(
                    FImageSubresourceRange(task.MipmapLevel, task.ArrayLayer, 1, task.AspectMask),
                    task.ImageOffset + int3(0, yOffset, slice),
                    uint3(task.ImageSize.x, ySize, 1u),
                    range.Offset, rowLength, imageHeight);

                loadEvent.Parts.Push(range);

                srcOffset += range.Size;
                yOffset += ySize;
                copy.DstBuffer = dstBuffer;
            }

            Assert_NoAssume(yOffset == imageSize.y);
        }
    }

    data.Batch->AddDataLoadedEvent(std::move(loadEvent));

    return Task(copy);
}

//----------------------------------------------------------------------------
// Data store
//----------------------------------------------------------------------------
bool FVulkanCommandBuffer::StorePartialData_(
    FInternalData& data,
    FRawBufferID* pDstBuffer, u32* pDstOffset, u32* pOutSize,
    FRawMemoryConst srcData, u32 srcOffset ) {
    Assert(pDstBuffer);
    Assert(pDstOffset);
    Assert(pOutSize);

    // skip blocks less than 1/N of  data size
    const u32 srcSize = checked_cast<u32>(srcData.SizeInBytes());
    const u32 minSize = Min(
        Min(srcSize, MinBufferPart),
        (srcSize + MaxBufferParts - 1u) / MaxBufferParts );

    void* ptr;
    if (data.Batch->StageWrite(pDstBuffer, pDstOffset, pOutSize, &ptr,
        srcSize - srcOffset, 1, 16, minSize)) {
        FPlatformMemory::Memcpy(ptr, srcData.data() + srcOffset, *pOutSize);
        return true;
    }

    return false;
}
//----------------------------------------------------------------------------
bool FVulkanCommandBuffer::StagingImageStore_(
    FInternalData& data,
    FRawBufferID* pDstBuffer, u32* pDstOffset, u32* pOutSize,
    FRawMemoryConst srcData, u32 srcOffset, u32 srcPitch, u32 srcTotalSize ) {
    Assert(pDstBuffer);
    Assert(pDstOffset);
    Assert(pOutSize);

    // skip blocks less than 1/N of total data size
    const u32 srcSize = checked_cast<u32>(srcData.SizeInBytes());
    const u32 minSize = Max(srcPitch, (srcTotalSize + MaxImageParts - 1) / MaxImageParts);

    void* ptr;
    if (data.Batch->StageWrite(pDstBuffer, pDstOffset, pOutSize, &ptr,
        srcSize - srcOffset, srcPitch, 16, minSize) ) {
        FPlatformMemory::Memcpy(ptr, srcData.data() + srcOffset, *pOutSize);
        return true;
    }

    return false;
}
//----------------------------------------------------------------------------
// Staging
//----------------------------------------------------------------------------
template <typename T>
bool FVulkanCommandBuffer::StagingAlloc_(FInternalData& data,
                                         const FVulkanLocalBuffer** pBuffer, VkDeviceSize* pOffset, T** pData,
                                         size_t count) {
    Assert(count > 0);

    FRawBufferID buf;
    u32 bufOffset, bufSize;
    void* mappedPtr = nullptr;

    const u32 requiredSize = checked_cast<u32>(sizeof(T) * count);

    if (not data.Batch->StageWrite(
        &buf, &bufOffset, &bufSize, &mappedPtr,
        requiredSize, 1, 16, requiredSize)) {
        RHI_LOG(Error, L"failed to write to staging alloc in {0}", data.DebugName);
        return false;
    }

    *pBuffer = ToLocal(buf);
    *pOffset = checked_cast<VkDeviceSize>(bufOffset);
    *pData = static_cast<T*>(mappedPtr);
    return true;
}
//----------------------------------------------------------------------------
bool FVulkanCommandBuffer::StagingStore_(FInternalData& data,
                                         const FVulkanLocalBuffer** pBuffer, VkDeviceSize* pOffset,
                                         const void* srcData, u32 dataSize, u32 offsetAlign) {
    FRawBufferID buf;
    u32 bufOffset, bufSize;
    void* mappedPtr = nullptr;

    if (not data.Batch->StageWrite(
        &buf, &bufOffset, &bufSize, &mappedPtr,
        dataSize, 1, offsetAlign, dataSize)) {
        RHI_LOG(Error, L"failed to store in staging at {0}", data.DebugName);
        return false;
    }

    *pBuffer = ToLocal(buf);
    *pOffset = checked_cast<VkDeviceSize>(bufOffset);

    FPlatformMemory::Memcpy(mappedPtr, srcData, bufSize);
    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
