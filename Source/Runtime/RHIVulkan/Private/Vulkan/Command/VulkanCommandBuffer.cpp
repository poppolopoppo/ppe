#include "stdafx.h"

#include "Vulkan/Command/VulkanCommandBuffer.h"

#include "Vulkan/Command/VulkanTaskGraph-inl.h"

#include "Vulkan/Command/VulkanCommandBatch.h"
#include "Vulkan/Debugger/VulkanLocalDebugger.h"
#include "Vulkan/Instance/VulkanFrameGraph.h"
#include "Vulkan/RayTracing/VulkanRayTracingGeometryInstance.h"

#include "Diagnostic/Logger.h"
#include "Time/TimedScope.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
#if USE_PPE_ASSERT
static CONSTEXPR EQueueUsage GGraphicsBit_ = EQueueUsage::Graphics;
static CONSTEXPR EQueueUsage GComputeBit_ = EQueueUsage::Graphics | EQueueUsage::AsyncCompute;
static CONSTEXPR EQueueUsage GRayTracingBit_ = EQueueUsage::Graphics | EQueueUsage::AsyncCompute;
static CONSTEXPR EQueueUsage GTransferBit_ = EQueueUsage::Graphics | EQueueUsage::AsyncCompute | EQueueUsage::AsyncTransfer;
#endif
#if USE_PPE_RHIDEBUG
static CONSTEXPR EDebugFlags GCmdDebugFlags_ = EDebugFlags::FullBarrier | EDebugFlags::QueueSync;
#endif
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVulkanCommandBuffer::FVulkanCommandBuffer(const SVulkanFrameGraph& fg, u32 indexInPool)
:   _frameGraph(fg)
,   _indexInPool(indexInPool) {
    Assert(_frameGraph);

    const auto exclusive = _data.LockExclusive();
    exclusive->MainAllocator.SetSlabSize(2_MiB);

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
void FVulkanCommandBuffer::OnStrongRefCountReachZero() NOEXCEPT {
    Assert_NoAssume(RefCount() == 0);

    const auto exclusiveData = _data.LockExclusive();
    Unused(exclusiveData); // just for locking

    Assert_NoAssume(exclusiveData->State == EState::Initial);
    _frameGraph->RecycleBuffer(this);
}
//----------------------------------------------------------------------------
SFrameGraph FVulkanCommandBuffer::FrameGraph() const NOEXCEPT {
    return _frameGraph;
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
        exclusive->PerQueue.resize(Max(exclusive->PerQueue.size(), index + 1_size_t));

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
    Assert_NoAssume(Default != exclusive->QueueIndex);

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
            *exclusive->Batch->_frameGraph,
            exclusive->DebugName, _indexInPool,
            exclusive->Batch->Read()->Dependencies.MakeView() );
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

    for (const FResourceIndex& id : data.RM.AllocatedRenderPasses) {
        TResourceProxy<FVulkanLogicalRenderPass>* const pPass = data.RM.LogicalRenderPasses[id];
        Assert(pPass);

        if (not pPass->IsDestroyed()) {
            pPass->TearDown(resources);
            Meta::Destroy(pPass);
            data.RM.LogicalRenderPasses.Deallocate(id);
        }
    }

    data.RM.AllocatedRenderPasses.clear();
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

#if USE_PPE_RHIDEBUG
        device.SetObjectName(cmd, data.DebugName.c_str(), VK_OBJECT_TYPE_COMMAND_BUFFER);
#endif

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
        RHI_LOG(Error, L"failed to process tasks for '{0}'", Read()->DebugName);
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

    STATIC_CONST_INTEGRAL(u32, visitorId, 1);
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
            const bool inputProcessed = not pTask->Inputs().Any(
                [](PVulkanFrameTask pInput) NOEXCEPT -> bool {
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

            pending.erase(pending.begin() + checked_cast<ptrdiff_t>(i));
            Append(pending, pTask->Outputs());
        }
    }

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
FRawImageID FVulkanCommandBuffer::SwapchainImage(FRawSwapchainID swapchainId) {
    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);

    const FVulkanSwapchain* const pSwapchain = AcquireTransient(swapchainId);
    LOG_CHECK(RHI, pSwapchain);

    FRawImageID imageId;
    VerifyRelease(pSwapchain->Acquire(&imageId, *this ARGS_IF_RHIDEBUG(exclusive->DebugQueueSync)));
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
bool FVulkanCommandBuffer::DependsOn(const SCommandBatch& cmd) {
    if (not cmd)
        return false;

    SVulkanCommandBatch batch = checked_cast<FVulkanCommandBatch>(cmd);

    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);

    exclusive->Batch->DependsOn(batch);

    return true;
}
//----------------------------------------------------------------------------
bool FVulkanCommandBuffer::StagingAlloc(FStagingBlock* pStaging, size_t size, size_t align) {
    Assert(pStaging);

    const auto exclusive = Write();
    Assert_NoAssume(
        EState::Recording == exclusive->State ||
        EState::Compiling == exclusive->State);

    size_t bufferSize;
    return exclusive->Batch->StageWrite(
        pStaging,
        &bufferSize,
        size,
        1u,
        align,
        size );
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
    Assert_NoAssume(GGraphicsBit_ & exclusive->Batch->QueueUsage());

    TVulkanFrameTask<FSubmitRenderPass>* const pFrameTask = exclusive->TaskGraph.AddTask(*this, task);
    LOG_CHECK(RHI, !!pFrameTask);

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
    Assert_NoAssume(GComputeBit_ & exclusive->Batch->QueueUsage());

    TVulkanFrameTask<FDispatchCompute>* const pFrameTask = exclusive->TaskGraph.AddTask(*this, task);
    LOG_CHECK(RHI, !!pFrameTask);

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
    Assert_NoAssume(GComputeBit_ & exclusive->Batch->QueueUsage());

    TVulkanFrameTask<FDispatchComputeIndirect>* const pFrameTask = exclusive->TaskGraph.AddTask(*this, task);
    LOG_CHECK(RHI, !!pFrameTask);

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
    Assert_NoAssume(GTransferBit_ & exclusive->Batch->QueueUsage());

    TVulkanFrameTask<FCopyBuffer>* const pFrameTask = exclusive->TaskGraph.AddTask(*this, task);
    LOG_CHECK(RHI, !!pFrameTask);

    return pFrameTask;
}
//----------------------------------------------------------------------------
PFrameTask FVulkanCommandBuffer::Task(const FCopyImage& task) {
    Assert(not task.Regions.empty());

    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);
    Assert_NoAssume(GTransferBit_ & exclusive->Batch->QueueUsage());

    TVulkanFrameTask<FCopyImage>* const pFrameTask = exclusive->TaskGraph.AddTask(*this, task);
    LOG_CHECK(RHI, !!pFrameTask);

    return pFrameTask;
}
//----------------------------------------------------------------------------
PFrameTask FVulkanCommandBuffer::Task(const FCopyBufferToImage& task) {
    Assert(not task.Regions.empty());

    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);
    Assert_NoAssume(GTransferBit_ & exclusive->Batch->QueueUsage());

    TVulkanFrameTask<FCopyBufferToImage>* const pFrameTask = exclusive->TaskGraph.AddTask(*this, task);
    LOG_CHECK(RHI, !!pFrameTask);

    return pFrameTask;
}
//----------------------------------------------------------------------------
PFrameTask FVulkanCommandBuffer::Task(const FCopyImageToBuffer& task) {
    Assert(not task.Regions.empty());

    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);
    Assert_NoAssume(GTransferBit_ & exclusive->Batch->QueueUsage());

    TVulkanFrameTask<FCopyImageToBuffer>* const pFrameTask = exclusive->TaskGraph.AddTask(*this, task);
    LOG_CHECK(RHI, !!pFrameTask);

    return pFrameTask;
}
//----------------------------------------------------------------------------
PFrameTask FVulkanCommandBuffer::Task(const FBlitImage& task) {
    Assert(not task.Regions.empty());

    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);
    Assert_NoAssume(GGraphicsBit_ & exclusive->Batch->QueueUsage());

    TVulkanFrameTask<FBlitImage>* const pFrameTask = exclusive->TaskGraph.AddTask(*this, task);
    LOG_CHECK(RHI, !!pFrameTask);

    return pFrameTask;
}
//----------------------------------------------------------------------------
PFrameTask FVulkanCommandBuffer::Task(const FResolveImage& task) {
    Assert(not task.Regions.empty());

    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);
    Assert_NoAssume(GGraphicsBit_ & exclusive->Batch->QueueUsage());

    TVulkanFrameTask<FResolveImage>* const pFrameTask = exclusive->TaskGraph.AddTask(*this, task);
    LOG_CHECK(RHI, !!pFrameTask);

    return pFrameTask;
}
//----------------------------------------------------------------------------
PFrameTask FVulkanCommandBuffer::Task(const FGenerateMipmaps& task) {
    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);
    Assert_NoAssume(GGraphicsBit_ & exclusive->Batch->QueueUsage());

    TVulkanFrameTask<FGenerateMipmaps>* const pFrameTask = exclusive->TaskGraph.AddTask(*this, task);
    LOG_CHECK(RHI, !!pFrameTask);

    return pFrameTask;
}
//----------------------------------------------------------------------------
PFrameTask FVulkanCommandBuffer::Task(const FFillBuffer& task) {
    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);

    TVulkanFrameTask<FFillBuffer>* const pFrameTask = exclusive->TaskGraph.AddTask(*this, task);
    LOG_CHECK(RHI, !!pFrameTask);

    return pFrameTask;
}
//----------------------------------------------------------------------------
PFrameTask FVulkanCommandBuffer::Task(const FClearColorImage& task) {
    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);
    Assert_NoAssume(GComputeBit_ & exclusive->Batch->QueueUsage());

    TVulkanFrameTask<FClearColorImage>* const pFrameTask = exclusive->TaskGraph.AddTask(*this, task);
    LOG_CHECK(RHI, !!pFrameTask);

    return pFrameTask;
}
//----------------------------------------------------------------------------
PFrameTask FVulkanCommandBuffer::Task(const FClearDepthStencilImage& task) {
    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);
    Assert_NoAssume(GComputeBit_ & exclusive->Batch->QueueUsage());

    TVulkanFrameTask<FClearDepthStencilImage>* const pFrameTask = exclusive->TaskGraph.AddTask(*this, task);
    LOG_CHECK(RHI, !!pFrameTask);

    return pFrameTask;
}
//----------------------------------------------------------------------------
PFrameTask FVulkanCommandBuffer::Task(const FUpdateBuffer& task) {
    Assert(not task.Regions.empty());

    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);
    Assert_NoAssume(GTransferBit_ & exclusive->Batch->QueueUsage());

    return MakeUpdateBufferTask_(*exclusive, task);
}
//----------------------------------------------------------------------------
PFrameTask FVulkanCommandBuffer::Task(const FUpdateImage& task) {
    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);
    Assert_NoAssume(GTransferBit_ & exclusive->Batch->QueueUsage());

    return MakeUpdateImageTask_(*exclusive, task);
}
//----------------------------------------------------------------------------
PFrameTask FVulkanCommandBuffer::Task(const FReadBuffer& task) {
    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);
    Assert_NoAssume(GTransferBit_ & exclusive->Batch->QueueUsage());

    return MakeReadBufferTask_(*exclusive, task);
}
//----------------------------------------------------------------------------
PFrameTask FVulkanCommandBuffer::Task(const FReadImage& task) {
    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);
    Assert_NoAssume(GTransferBit_ & exclusive->Batch->QueueUsage());

    return MakeReadImageTask_(*exclusive, task);
}
//----------------------------------------------------------------------------
PFrameTask FVulkanCommandBuffer::Task(const FPresent& task) {
    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);

    TVulkanFrameTask<FPresent>* const pFrameTask = exclusive->TaskGraph.AddTask(*this, task);
    LOG_CHECK(RHI, !!pFrameTask);

    exclusive->Batch->_data.LockExclusive()->Swapchains.Push(pFrameTask->Swapchain);

    return pFrameTask;
}
//----------------------------------------------------------------------------
PFrameTask FVulkanCommandBuffer::Task(const FCustomTask& task) {
    Assert(task.Callback);

    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);

    TVulkanFrameTask<FCustomTask>* const pFrameTask = exclusive->TaskGraph.AddTask(*this, task);
    LOG_CHECK(RHI, !!pFrameTask);

    return pFrameTask;
}
//----------------------------------------------------------------------------
// RayTracing
//----------------------------------------------------------------------------
PFrameTask FVulkanCommandBuffer::Task(const FUpdateRayTracingShaderTable& task) {
    Assert(task.Pipeline);

#if VK_NV_ray_tracing
    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);
    Assert_NoAssume(Device().Enabled().RayTracingNV);
    Assert_NoAssume(GRayTracingBit_ & exclusive->Batch->QueueUsage());

    TVulkanFrameTask<FUpdateRayTracingShaderTable>* const pFrameTask =
        exclusive->TaskGraph.AddTask(*this, task);
    LOG_CHECK(RHI, !!pFrameTask);

    return pFrameTask;

#else
    AssertNotImplemented();
#endif
}
//----------------------------------------------------------------------------
PFrameTask FVulkanCommandBuffer::Task(const FBuildRayTracingGeometry& task) {
    Assert(task.Geometry);

#if VK_NV_ray_tracing
    const auto exclusive = Write();

    const FVulkanDevice& device = Device();

    Assert_NoAssume(EState::Recording == exclusive->State);
    Assert_NoAssume(device.Enabled().RayTracingNV);
    Assert_NoAssume(GRayTracingBit_ & exclusive->Batch->QueueUsage());

    FVulkanRTLocalGeometry* const rtGeometry = ToLocal(task.Geometry);
    LOG_CHECK(RHI, !!rtGeometry);
    Assert_NoAssume(task.Aabbs.size() <= rtGeometry->Aabbs().size());
    Assert_NoAssume(task.Triangles.size() <= rtGeometry->Triangles().size());

    TVulkanFrameTask<FBuildRayTracingGeometry>* const pFrameTask =
        exclusive->TaskGraph.AddTask(*this, task);
    LOG_CHECK(RHI, !!pFrameTask);
    pFrameTask->RTGeometry = rtGeometry;

    VkAccelerationStructureMemoryRequirementsInfoNV asInfo{};
    asInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
    asInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV;
    asInfo.accelerationStructure = rtGeometry->Handle();

    VkMemoryRequirements2 memReq{};
    device.vkGetAccelerationStructureMemoryRequirementsNV(device.vkDevice(), &asInfo, &memReq);

    // #TODO: virtual buffer of buffer cache
    FBufferID buffer = _frameGraph->CreateBuffer(FBufferDesc{
        memReq.memoryRequirements.size,
        EBufferUsage::RayTracing },
        Default ARGS_IF_RHIDEBUG("ScratchBuffer"));
    LOG_CHECK(RHI, !!buffer);

    pFrameTask->ScratchBuffer = ToLocal(*buffer);
    ReleaseResource(buffer.Release());

    Assert(pFrameTask->ScratchBuffer->Desc().Usage & EBufferUsage::RayTracing);

    const TMemoryView<VkGeometryNV> geometries = exclusive->MainAllocator.AllocateT<VkGeometryNV>(task.Triangles.size() + task.Aabbs.size());
    pFrameTask->Geometries = geometries;

    // initialize geometries
    forrange(i, 0, geometries.size()) {
        VkGeometryNV& dst = geometries[i];
        dst = VkGeometryNV{};
        dst.sType = VK_STRUCTURE_TYPE_GEOMETRY_NV;
        dst.geometry.aabbs.sType = VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV;
        dst.geometry.triangles.sType = VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV;
        dst.geometryType = (i < task.Triangles.size()
            ? VK_GEOMETRY_TYPE_TRIANGLES_NV
            : VK_GEOMETRY_TYPE_AABBS_NV );
    }

    // add triangles
    for (const FBuildRayTracingGeometry::FTriangles& src : task.Triangles) {
        const auto ref = Meta::LowerBound(rtGeometry->Triangles().begin(), rtGeometry->Triangles().end(), src.Geometry);
        LOG_CHECK(RHI, rtGeometry->Triangles().end() != ref);

        const size_t pos = std::distance(rtGeometry->Triangles().begin(), ref);

        VkGeometryNV& dst = geometries[pos];
        dst.flags = VkCast(ref->Flags);

        Assert(src.VertexBuffer or not src.VertexData.empty());
        Assert(src.VertexCount > 0);
        Assert(src.VertexCount <= ref->MaxVertexCount);
        Assert(src.IndexCount <= ref->MaxIndexCount);
        Assert(src.VertexFormat <= ref->VertexFormat);
        Assert(src.IndexFormat == ref->IndexFormat);

        // vertices
        dst.geometry.triangles.vertexCount = src.VertexCount;
        dst.geometry.triangles.vertexStride = src.VertexStride;
        dst.geometry.triangles.vertexFormat = VkCast(src.VertexFormat);

        if (not src.VertexData.empty()) {
            const FVulkanLocalBuffer* vb = nullptr;
            LOG_CHECK(RHI, StagingStore_(*exclusive, &vb, &dst.geometry.triangles.vertexOffset, src.VertexData.data(), src.VertexData.SizeInBytes(), ref->VertexSize));
            dst.geometry.triangles.vertexData = vb->Handle();
            //pFrameTask->UsableBuffers.Add(vb); // staging buffer is already immutable
        }
        else {
            const FVulkanLocalBuffer* vb = ToLocal(src.VertexBuffer);
            LOG_CHECK(RHI, !!vb);
            dst.geometry.triangles.vertexData = vb->Handle();
            dst.geometry.triangles.vertexOffset = src.VertexOffset;
            pFrameTask->UsableBuffers.Add(vb);
        }

        // indices
        if (src.IndexCount > 0) {
            Assert(src.IndexBuffer or not src.IndexData.empty());
            dst.geometry.triangles.indexCount = src.IndexCount;
            dst.geometry.triangles.indexType = VkCast(src.IndexFormat);
        }
        else {
            Assert(not src.IndexBuffer or src.IndexData.empty());
            Assert(EIndexFormat::Unknown == src.IndexFormat);
            dst.geometry.triangles.indexType = VK_INDEX_TYPE_NONE_NV;
        }

        if (not src.IndexData.empty()) {
            const FVulkanLocalBuffer* ib = nullptr;
            LOG_CHECK(RHI, StagingStore_(*exclusive, &ib, &dst.geometry.triangles.indexOffset, src.IndexData.data(), src.IndexData.SizeInBytes(), ref->IndexSize));
            dst.geometry.triangles.indexData = ib->Handle();
            //pFrameTask->UsableBuffers.Add(ib); // staging buffer is already immutable
        }
        else if (!!src.IndexBuffer) {
            const FVulkanLocalBuffer* ib = ToLocal(src.IndexBuffer);
            LOG_CHECK(RHI, !!ib);
            dst.geometry.triangles.indexData = ib->Handle();
            dst.geometry.triangles.indexOffset = src.IndexOffset;
            pFrameTask->UsableBuffers.Add(ib);
        }

        // transforms
        if (!!src.TransformBuffer) {
            const FVulkanLocalBuffer* tb = ToLocal(src.TransformBuffer);
            LOG_CHECK(RHI, !!tb);
            dst.geometry.triangles.transformData = tb->Handle();
            dst.geometry.triangles.transformOffset = src.TransformOffset;
            pFrameTask->UsableBuffers.Add(tb);
        }
        else if (src.TransformData.has_value()) {
            const FVulkanLocalBuffer* tb = nullptr;
            const float3x4& transform = src.TransformData.value();
            LOG_CHECK(RHI, StagingStore_(*exclusive, &tb, &dst.geometry.triangles.transformOffset, &transform, sizeof(transform), 16_b));
            dst.geometry.triangles.transformData = tb->Handle();
            //pFrameTask->UsableBuffers.Add(tb); // staging buffer is already immutable
        }
    }

    // add aabbs
    for (const FBuildRayTracingGeometry::FBoundingVolumes& src : task.Aabbs) {
        const auto ref = Meta::LowerBound(rtGeometry->Aabbs().begin(), rtGeometry->Aabbs().end(), src.Geometry);
        LOG_CHECK(RHI, rtGeometry->Aabbs().end() != ref);

        const size_t pos = std::distance(rtGeometry->Aabbs().begin(), ref);

        VkGeometryNV& dst = geometries[pos + task.Triangles.size()];
        dst.flags = VkCast(ref->Flags);

        Assert(src.AabbBuffer or not src.AabbData.empty());
        Assert(src.AabbCount > 0);
        Assert(src.AabbCount <= ref->MaxAabbCount);
        Assert(src.AabbStride % 8 == 0);

        dst.flags = VkCast(ref->Flags);
        dst.geometry.aabbs.sType = VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV;
        dst.geometry.aabbs.numAABBs = src.AabbCount;
        dst.geometry.aabbs.stride = src.AabbStride;

        if (not src.AabbData.empty()) {
            const FVulkanLocalBuffer* ab = nullptr;
            LOG_CHECK(RHI, StagingStore_(*exclusive, &ab, &dst.geometry.aabbs.offset, src.AabbData.data(), src.AabbData.SizeInBytes(), 8_b));
            dst.geometry.aabbs.aabbData = ab->Handle();
            //pFrameTask->UsableBuffers.Add(ab); // staging buffer is already immutable
        }
        else {
            const FVulkanLocalBuffer* ab = ToLocal(src.AabbBuffer);
            LOG_CHECK(RHI, !!ab);
            dst.geometry.aabbs.aabbData = ab->Handle();
            dst.geometry.aabbs.offset = src.AabbOffset;
            pFrameTask->UsableBuffers.Add(ab);
        }
    }

    return pFrameTask;

#else
    AssertNotImplemented();
#endif
}
//----------------------------------------------------------------------------
PFrameTask FVulkanCommandBuffer::Task(const FBuildRayTracingScene& task) {
    Assert(task.Scene);

#if VK_NV_ray_tracing
    const auto exclusive = Write();

    const FVulkanDevice& device = Device();

    Assert_NoAssume(EState::Recording == exclusive->State);
    Assert_NoAssume(Device().Enabled().RayTracingNV);
    Assert_NoAssume(GRayTracingBit_ & exclusive->Batch->QueueUsage());

    FVulkanRTLocalScene* const rtScene = ToLocal(task.Scene);
    LOG_CHECK(RHI, !!rtScene);
    Assert_NoAssume(task.Instances.size() <= rtScene->MaxInstanceCount());

    TVulkanFrameTask<FBuildRayTracingScene>* const pBuildTask =
        exclusive->TaskGraph.AddTask(*this, task);
    LOG_CHECK(RHI, !!pBuildTask);
    pBuildTask->RTScene = rtScene;

    VkAccelerationStructureMemoryRequirementsInfoNV asInfo{};
    asInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
    asInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV;
    asInfo.accelerationStructure = rtScene->Handle();

    VkMemoryRequirements2 memReq{};
    device.vkGetAccelerationStructureMemoryRequirementsNV(device.vkDevice(), &asInfo, &memReq);

    FMemoryDesc mem;
    mem.Type = EMemoryType::Default;
    mem.Alignment = checked_cast<u32>(memReq.memoryRequirements.alignment);
    mem.ExternalRequirements = FMemoryRequirements{
        checked_cast<u32>(memReq.memoryRequirements.memoryTypeBits),
        checked_cast<u32>(memReq.memoryRequirements.alignment)
    };

    // #TODO: virtual buffer or buffer cache
    FBufferID scratchBuf = _frameGraph->CreateBuffer(FBufferDesc{
        checked_cast<size_t>(memReq.memoryRequirements.size),
        EBufferUsage::RayTracing,
    },  mem ARGS_IF_RHIDEBUG("ScratchBuffer") );
    LOG_CHECK(RHI, !!scratchBuf);

    pBuildTask->ScratchBuffer = ToLocal(*scratchBuf);
    ReleaseResource(scratchBuf.Release());

    // #TODO: virtual buffer or buffer cache
    FBufferID instanceBuf = _frameGraph->CreateBuffer(FBufferDesc{
        task.Instances.MakeView().SizeInBytes(),
        EBufferUsage::TransferDst | EBufferUsage::RayTracing
    },  mem ARGS_IF_RHIDEBUG("InstanceBuffer") );
    LOG_CHECK(RHI, !!instanceBuf);

    pBuildTask->InstanceBuffer = ToLocal(*instanceBuf);
    ReleaseResource(instanceBuf.Release());

    FVulkanRayTracingGeometryInstance* pInstances = nullptr;
    LOG_CHECK(RHI, StagingAlloc_<FVulkanRayTracingGeometryInstance >(
        *exclusive,
        &pBuildTask->InstanceStagingBuffer,
        &pBuildTask->InstanceStagingBufferOffset,
        &pInstances,
        task.Instances.size() ));
    Assert_NoAssume(pBuildTask->ScratchBuffer->Read()->Desc.Usage & EBufferUsage::RayTracing);
    Assert_NoAssume(pBuildTask->InstanceBuffer->Read()->Desc.Usage & EBufferUsage::RayTracing);

    // sort instances by instance ID
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
        FVulkanRayTracingGeometryInstance& dst = pInstances[idx];
        const FVulkanRayTracingLocalGeometry*& pLocalGeom = pBuildTask->RTGeometries[i];
        Assert_NoAssume(src.InstanceId.Valid());
        Assert_NoAssume(src.GeometryId.Valid());
        Assert_NoAssume((src.CustomId >> 24) == 0);

        pLocalGeom = ToLocal(src.GeometryId);
        Assert(pLocalGeom);

        dst.BlasHandle = pLocalGeom->BLAS();
        dst.TransformRow0 = src.Transform.Row_x(); // decompose for row-major order
        dst.TransformRow1 = src.Transform.Row_y();
        dst.TransformRow2 = src.Transform.Row_z();
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

#else
    AssertNotImplemented();
#endif
}
//----------------------------------------------------------------------------
PFrameTask FVulkanCommandBuffer::Task(const FTraceRays& task) {
    Assert(task.ShaderTable);

#if VK_NV_ray_tracing
    const auto exclusive = Write();
    Assert_NoAssume(EState::Recording == exclusive->State);
    Assert_NoAssume(Device().Enabled().RayTracingNV);
    Assert_NoAssume(GRayTracingBit_ & exclusive->Batch->QueueUsage());

    TVulkanFrameTask<FTraceRays>* const pFrameTask =
        exclusive->TaskGraph.AddTask(*this, task);
    LOG_CHECK(RHI, !!pFrameTask);

#if USE_PPE_RHIDEBUG
    if (exclusive->ShaderDbg.TimemapIndex != Default &&
        exclusive->ShaderDbg.TimemapStages ^ EShaderStages::AllRayTracing) {
        Assert_NoAssume(pFrameTask->DebugModeIndex == Default);

        pFrameTask->DebugModeIndex = exclusive->ShaderDbg.TimemapIndex;
    }
#endif

    return pFrameTask;

#else
    AssertNotImplemented();
#endif
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
    Assert_NoAssume(GGraphicsBit_ & exclusive->Batch->QueueUsage());

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

    exclusive->RM.AllocatedRenderPasses.Push(index);

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
    Assert_NoAssume(GComputeBit_ & exclusive->Batch->QueueUsage());

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
    Assert_NoAssume(GComputeBit_ & exclusive->Batch->QueueUsage());

    FVulkanResourceManager& rm = ResourceManager();
    const FImageDesc desc = rm.ResourceDescription(dstImage);
    Assert_NoAssume(desc.Type == EImageDim_2D);
    Assert_NoAssume(desc.Usage & EImageUsage::Storage);

    FVulkanResourceManager::FShaderTimemapPipelines pplns;
    rm.ShaderTimemapPipelines(&pplns);
    Assert(pplns[0] && pplns[1] && pplns[2]);

    FRawBufferID ssb;
    size_t ssbTimemapOffset, ssbTimemapSize;
    uint2 ssbDim;
    LOG_CHECK(RHI, exclusive->Batch->FindShaderTimemapForDebug(
        &ssb, &ssbTimemapOffset, &ssbTimemapSize, &ssbDim,
        exclusive->ShaderDbg.TimemapIndex) );

    const size_t ssbAlign = checked_cast<size_t>(_frameGraph->Device().Limits().minStorageBufferOffsetAlignment);
    const size_t ssbMaxValuesSize = ssbDim.y * sizeof(u64);
    ssbTimemapSize -= ssbMaxValuesSize + ssbAlign;
    const size_t ssbMaxValuesOffset = Meta::RoundToNextPow2(ssbTimemapOffset + ssbTimemapSize, ssbAlign);

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
        if (resources.GlobalIndexRange.Empty())
            return;

        auto globalIndices = resources.ToLocal.MakeView()
            .SubRange(resources.GlobalIndexRange.First, resources.GlobalIndexRange.Extent());
        FPlatformMemory::Memset(
            globalIndices.data(),
            UMax,
            globalIndices.SizeInBytes());

        resources.GlobalIndexRange.Reset();
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
        if (localResources.LocalIndexRange.Empty())
            return;

        forrange(i, checked_cast<FResourceIndex>(localResources.LocalIndexRange.First), checked_cast<FResourceIndex>(localResources.LocalIndexRange.Last)) {
            auto* const pResource = localResources.Pool[i];
            Assert(pResource);

            if (not pResource->IsDestroyed()) {
                pResource->Data().ResetState(order, barriers ARGS_IF_RHIDEBUG(pDebugger));
                pResource->TearDown();
                Meta::Destroy(pResource);
                localResources.Pool.Deallocate(i);
            }
        }

        localResources.LocalIndexRange.Reset();
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
    LOG_CHECK(RHI, pResource);

    local = localResources.Pool.Allocate();
    auto* const pData = localResources.Pool[local];
    Meta::Construct(pData);

    if (not pData->Construct(pResource)) {
        Meta::Destroy(pData);
        localResources.Pool.Deallocate(local);
        ONLY_IF_RHIDEBUG(Unused(debugMessage));
        RHI_LOG(Error, L"{1}: {0}", debugMessage, _data.Value_NotThreadSafe().DebugName);
        return nullptr;
    }

    localResources.LocalIndexRange.SelfUnion(
        TRange<u32>{ checked_cast<u32>(local), checked_cast<u32>(local + 1) });
    localResources.GlobalIndexRange.SelfUnion(
        TRange<u32>{ checked_cast<u32>(id.Index), checked_cast<u32>(id.Index + 1) });

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

        for (size_t srcOffset = 0; srcOffset < region.Data.SizeInBytes();) {
            FStagingBlock staging;
            size_t blockSize;
            if (not StorePartialData_(data, &staging, &blockSize, region.Data, srcOffset)) {
                RHI_LOG(Error, L"failed to write partial staging data for '{0}' in '{1}'", task.TaskName, data.DebugName);
                return nullptr;
            }

            if (copy.SrcBuffer && staging.RawBufferID != copy.SrcBuffer) {
                const PFrameTask pLastTask = Task(copy);
                Assert(pLastTask);

                copy.Regions.clear();
                copy.Dependencies.clear();
                copy.Dependencies.Push(pLastTask);
            }

            copy.AddRegion(staging.Offset, region.Offset + srcOffset, blockSize);

            srcOffset += blockSize;
            copy.SrcBuffer = staging.RawBufferID;
        }
    }

    return Task(copy);
}
//----------------------------------------------------------------------------
PFrameTask FVulkanCommandBuffer::MakeUpdateImageTask_(FInternalData& data, const FUpdateImage& task) {
    Assert(task.DstImage);
    Assert(Any(GreaterMask(task.ImageSize, uint3::Zero)));

    const FVulkanImage* const pImage = AcquireTransient(task.DstImage);
    LOG_CHECK(RHI, pImage);

    const FImageDesc& desc = pImage->Read()->Desc;
    Assert_NoAssume(task.MipmapLevel < desc.MaxLevel);
    Assert_NoAssume(task.ArrayLayer < desc.ArrayLayers);

    const FPixelFormatInfo pixelInfo = EPixelFormat_Infos(desc.Format);
    const uint3 imageSize = Max(task.ImageSize, 1u);
    const uint2& blockDim = pixelInfo.BlockDim;
    const u32 blockBits = (task.AspectMask != EImageAspect::Stencil
                               ? pixelInfo.BitsPerBlock0
                               : pixelInfo.BitsPerBlock1);

    const size_t rowPitch = Max(task.DataRowPitch,
                             (imageSize.x * blockBits + blockDim.x - 1) / (blockDim.x * 8_size_t/* bits to bytes */));
    const size_t minSlicePitch = ((imageSize.y * rowPitch + blockDim.y - 1) / blockDim.y);
    const size_t slicePitch = Max(task.DataSlicePitch, minSlicePitch);
    const size_t totalSizeInBytes = (imageSize.z > 1 ? slicePitch * imageSize.z : minSlicePitch);

    if (totalSizeInBytes != task.Data.SizeInBytes()) {
        RHI_LOG(Error, L"invalid data supplied for image '{0}' update in '{1}'", pImage->DebugName(), data.DebugName);
        return nullptr;
    }

    const size_t minSizeInBytes = _frameGraph->ResourceManager().HostWriteBufferSize() / 4;
    const u32 rowLength = checked_cast<u32>((rowPitch * blockDim.x * 8/* bits to bytes */) / blockBits);
    const u32 imageHeight = checked_cast<u32>((slicePitch * blockDim.y) / rowPitch);

    FCopyBufferToImage copy{};
    copy.DstImage = task.DstImage;
    copy.Dependencies = task.Dependencies;
#if USE_PPE_RHIDEBUG
    copy.TaskName = task.TaskName;
    copy.DebugColor = task.DebugColor;
#endif

    Assert_NoAssume(Meta::IsAlignedPow2(blockDim.x, task.ImageOffset.x));
    Assert_NoAssume(Meta::IsAlignedPow2(blockDim.y, task.ImageOffset.y));

    // copy to staging buffer slice by slice
    if (totalSizeInBytes < minSizeInBytes) {
        u32 zOffset = 0;
        for (size_t srcOffset = 0; srcOffset < totalSizeInBytes;) {
            FStagingBlock staging;
            size_t blockSize;
            if (not StagingImageStore_(data, &staging, &blockSize, task.Data, srcOffset, slicePitch, totalSizeInBytes)) {
                RHI_LOG(Error, L"failed to write image slice to staging for '{0}' in '{1}'", task.TaskName, data.DebugName);
                return nullptr;
            }

            if (copy.SrcBuffer && staging.RawBufferID != copy.SrcBuffer) {
                const PFrameTask pLastTask = Task(copy);
                Assert(pLastTask);

                copy.Regions.clear();
                copy.Dependencies.clear();
                copy.Dependencies.Push(pLastTask);
            }

            const u32 zSize = checked_cast<u32>(blockSize / slicePitch);
            Assert_NoAssume(Meta::IsAlignedPow2(blockDim.x, imageSize.x));
            Assert_NoAssume(Meta::IsAlignedPow2(blockDim.y, imageSize.y));

            copy.AddRegion(
                staging.Offset, rowLength, imageHeight,
                FImageSubresourceRange{task.MipmapLevel, task.ArrayLayer, 1, task.AspectMask},
                task.ImageOffset + int3(0, 0, checked_cast<int>(zOffset)),
                uint3{imageSize.xy, zSize});

            srcOffset += blockSize;
            zOffset += zSize;
            copy.SrcBuffer = staging.RawBufferID;
        }

        Assert_NoAssume(zOffset == imageSize.z);
    }
        // copy to staging buffer row by row
    else {
        forrange(slice, 0, imageSize.z) {
            u32 yOffset = 0;
            const FRawMemoryConst sliceData = task.Data.SubRange(slice * slicePitch, slicePitch);
            const size_t sliceSize = sliceData.SizeInBytes();

            for (size_t srcOffset = 0; srcOffset < sliceSize;) {
                FStagingBlock staging;
                size_t blockSize;
                if (not StagingImageStore_(data, &staging, &blockSize, sliceData, srcOffset, rowPitch * blockDim.y, totalSizeInBytes)) {
                    RHI_LOG(Error, L"failed to write image row to staging for '{0}' in '{1}'", task.TaskName, data.DebugName);
                    return nullptr;
                }

                if (copy.SrcBuffer && staging.RawBufferID != copy.SrcBuffer) {
                    const PFrameTask pLastTask = Task(copy);
                    Assert(pLastTask);

                    copy.Regions.clear();
                    copy.Dependencies.clear();
                    copy.Dependencies.Push(pLastTask);
                }

                const u32 ySize = checked_cast<u32>((blockSize * blockDim.y) / rowPitch);
                Assert_NoAssume(Meta::IsAlignedPow2(blockDim.x, imageSize.x));
                Assert_NoAssume(Meta::IsAlignedPow2(blockDim.y, ySize));
                Assert_NoAssume(Meta::IsAlignedPow2(blockDim.y, task.ImageOffset.y + yOffset));

                copy.AddRegion(
                    staging.Offset, rowLength, ySize,
                    FImageSubresourceRange{task.MipmapLevel, task.ArrayLayer, 1, task.AspectMask},
                    task.ImageOffset + int3(0, checked_cast<int>(yOffset), checked_cast<int>(slice)),
                    uint3{imageSize.x, ySize, 1});

                srcOffset += blockSize;
                yOffset += ySize;
                copy.SrcBuffer = staging.RawBufferID;
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
    for (size_t srcOffset = 0; srcOffset < task.SrcSize;) {
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
    LOG_CHECK(RHI, pImage);

    const FImageDesc& desc = pImage->Read()->Desc;
    Assert_NoAssume(task.MipmapLevel < desc.MaxLevel);
    Assert_NoAssume(task.ArrayLayer < desc.ArrayLayers);

    const uint3 imageSize = Max(task.ImageSize, 1u);
    const size_t minSizeInBytes = _frameGraph->ResourceManager().HostReadBufferSize();
    const FPixelFormatInfo pixelInfo = EPixelFormat_Infos(desc.Format);
    const uint2 blockDim = pixelInfo.BlockDim;
    const u32 blockBits = (task.AspectMask != EImageAspect::Stencil
                               ? pixelInfo.BitsPerBlock0
                               : pixelInfo.BitsPerBlock1);
    const size_t rowPitch = ((imageSize.x * blockBits + blockDim.x - 1) / (blockDim.x * 8/* bits to byte */));
    const size_t slicePitch = ((imageSize.y * rowPitch + blockDim.y - 1) / blockDim.y);
    const size_t totalSizeInBytes = slicePitch * imageSize.z;
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
        for (size_t srcOffset = 0; srcOffset < totalSizeInBytes;) {
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

            const u32 zSize = checked_cast<u32>(range.Size / slicePitch);
            copy.AddRegion(
                FImageSubresourceRange(task.MipmapLevel, task.ArrayLayer, 1, task.AspectMask),
                task.ImageOffset + int3(0, 0, checked_cast<int>(zOffset)),
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
        AssertRelease(rowPitch < minSizeInBytes);

        forrange(slice, 0, imageSize.z) {
            u32 yOffset = 0;
            for (size_t srcOffset = 0; srcOffset < slicePitch;) {
                FRawBufferID dstBuffer;
                FVulkanCommandBatch::FStagingDataRange range;
                if (not data.Batch->AddPendingLoad(
                    &dstBuffer, &range,
                    srcOffset, totalSizeInBytes, rowPitch * blockDim.y)) {
                    RHI_LOG(Error, L"failed copy image slice to staging of '{0}' in '{1}'", copy.TaskName, data.DebugName);
                    return nullptr;
                }

                if (copy.DstBuffer && copy.DstBuffer != dstBuffer) {
                    const PFrameTask pLastTask = Task(copy);
                    Assert_NoAssume(pLastTask);

                    copy.Regions.clear();
                    copy.Dependencies.clear();
                    copy.Dependencies.Push(pLastTask);
                }

                const u32 ySize = checked_cast<u32>((range.Size * blockDim.y) / rowPitch);
                copy.AddRegion(
                    FImageSubresourceRange(task.MipmapLevel, task.ArrayLayer, 1, task.AspectMask),
                    task.ImageOffset + int3(0, checked_cast<int>(yOffset), checked_cast<int>(slice)),
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
    FStagingBlock* pDstStaging, size_t* pOutSize,
    const FRawMemoryConst& srcData, size_t srcOffset ) {
    Assert(pDstStaging);
    Assert(pOutSize);

    // skip blocks less than 1/N of  data size
    const size_t srcSize = srcData.SizeInBytes();
    const size_t minSize = Min(
        Min(srcSize, MinBufferPart),
        (srcSize + MaxBufferParts - 1u) / MaxBufferParts );

    if (data.Batch->StageWrite(pDstStaging, pOutSize, srcSize - srcOffset, 1, 16, minSize)) {
        FPlatformMemory::Memcpy(pDstStaging->Mapped, srcData.data() + srcOffset, *pOutSize);
        return true;
    }

    return false;
}
//----------------------------------------------------------------------------
bool FVulkanCommandBuffer::StagingImageStore_(
    FInternalData& data,
    FStagingBlock* pDstStaging, size_t* pOutSize,
    const FRawMemoryConst& srcData, size_t srcOffset, size_t srcPitch, size_t srcTotalSize ) {
    Assert(pDstStaging);
    Assert(pOutSize);

    // skip blocks less than 1/N of total data size
    const size_t srcSize = srcData.SizeInBytes();
    const size_t minSize = Max(srcPitch, (srcTotalSize + MaxImageParts - 1) / MaxImageParts);

    if (data.Batch->StageWrite(pDstStaging, pOutSize, srcSize - srcOffset, srcPitch, 16, minSize) ) {
        FPlatformMemory::Memcpy(pDstStaging->Mapped, srcData.data() + srcOffset, *pOutSize);
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

    const size_t requiredSize = (sizeof(T) * count);

    FStagingBlock stagingBlock;
    size_t blockSize;
    if (not data.Batch->StageWrite(
        &stagingBlock, &blockSize,
        requiredSize, 1, 16, requiredSize)) {
        RHI_LOG(Error, L"failed to write to staging alloc in {0}", data.DebugName);
        return false;
    }

    *pBuffer = ToLocal(stagingBlock.RawBufferID);
    *pOffset = checked_cast<VkDeviceSize>(stagingBlock.Offset);
    *pData = reinterpret_cast<T*>(stagingBlock.Mapped);
    return true;
}
//----------------------------------------------------------------------------
bool FVulkanCommandBuffer::StagingStore_(FInternalData& data,
                                         const FVulkanLocalBuffer** pBuffer, VkDeviceSize* pOffset,
                                         const void* srcData, size_t dataSize, size_t offsetAlign) {
    FStagingBlock stagingBlock;
    size_t blockSize;
    if (not data.Batch->StageWrite(
        &stagingBlock, &blockSize,
        dataSize, 1, offsetAlign, dataSize)) {
        RHI_LOG(Error, L"failed to store in staging at {0}", data.DebugName);
        return false;
    }

    *pBuffer = ToLocal(stagingBlock.RawBufferID);
    *pOffset = checked_cast<VkDeviceSize>(stagingBlock.Offset);

    FPlatformMemory::Memcpy(stagingBlock.Mapped, srcData, blockSize);
    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
