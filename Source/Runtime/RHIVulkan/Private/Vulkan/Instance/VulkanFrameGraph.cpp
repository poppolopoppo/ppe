#include "stdafx.h"

#include "Vulkan/Instance/VulkanFrameGraph.h"

#include "Diagnostic/Logger.h"
#include "HAL/PlatformTime.h"

#if USE_PPE_RHIDEBUG
#   include "Time/TimedScope.h"
#endif

#define PPE_RHIVK__FLUSH_ITERATIONS (10u)
#define PPE_RHIVK__VALIDATION_ITERATIONS (100)

namespace PPE {
namespace RHI {
EXTERN_LOG_CATEGORY(PPE_RHI_API, RHI)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVulkanFrameGraph::FVulkanFrameGraph(const FVulkanDeviceInfo& deviceInfo)
:   _state(EState::Initial)
,   _device(deviceInfo)
,   _queueUsage(Default)
,   _resourceManager(_device)
,   _vkQueryPool(VK_NULL_HANDLE)
{}
//----------------------------------------------------------------------------
FVulkanFrameGraph::~FVulkanFrameGraph() {
    AssertRelease_NoAssume(EState::Destroyed == _state); // must call TearDown() !
}
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::Construct() {
    Verify(SetState_(EState::Initial, EState::Initialization));

    LOG(RHI, Emphasis, L"constructing vulkan frame graph");

    if (not _resourceManager.Construct()) {
        Verify(SetState_(EState::Initialization, EState::Initial));
        return false;
    }

    // setup queues
    {
        const FCriticalScope queueLock{ &_queueCS };

        if (not AddGraphicsQueue_())
            PPE_THROW_IT(FVulkanException("FVulkanFrameGraph::GraphicsQueue", VK_ERROR_UNKNOWN));
        if (not AddAsyncComputeQueue_())
            PPE_THROW_IT(FVulkanException("FVulkanFrameGraph::AsyncCompute", VK_ERROR_UNKNOWN));
        if (not AddAsyncTransferQueue_())
            PPE_THROW_IT(FVulkanException("FVulkanFrameGraph::GraphicsQueue", VK_ERROR_UNKNOWN));

        Assert_NoAssume(not _queueMap.empty());
    }

#if USE_PPE_RHIDEBUG || USE_PPE_RHIPROFILING
    // create query pool
    {
        VkQueryPoolCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
        createInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
        createInfo.queryCount = checked_cast<u32>(FCommandBatchPool::Capacity * 2);

        VK_CHECK( _device.vkCreateQueryPool(_device.vkDevice(), &createInfo, _device.vkAllocator(), &_vkQueryPool));
    }
#endif

    Verify(SetState_(EState::Initialization, EState::Idle));
    return true;
}
//----------------------------------------------------------------------------
void FVulkanFrameGraph::TearDown() {
    Verify(SetState_(EState::Idle, EState::Destroyed));

    LOG(RHI, Emphasis, L"tearing down vulkan frame graph");

    VerifyRelease(WaitIdle());

    ReleaseMemory();

    // delete per queue data
    {
        const FCriticalScope queueLock{ &_queueCS };

        for (FQueueData& q : _queueMap) {
            Assert_NoAssume(q.Pending.empty());
            Assert_NoAssume(q.Submitted.empty());

            q.CommandPool.TearDown(_device);

            for (auto& vkSemaphore : q.Semaphores) {
                Assert_NoAssume(VK_NULL_HANDLE != vkSemaphore);
                _device.vkDestroySemaphore(_device.vkDevice(), vkSemaphore, _device.vkAllocator());
                vkSemaphore = VK_NULL_HANDLE;
            }
        }
    }

    _resourceManager.TearDown();

#if USE_PPE_RHIDEBUG || USE_PPE_RHIPROFILING
    if (VK_NULL_HANDLE != _vkQueryPool) {
        _device.vkDestroyQueryPool(_device.vkDevice(), _vkQueryPool, _device.vkAllocator());
        _vkQueryPool = VK_NULL_HANDLE;
    }

    _shaderDebugCallback = {};
#endif
}
//----------------------------------------------------------------------------
void FVulkanFrameGraph::RecycleBatch(FVulkanCommandBatch* batch) {
    Assert(batch);
    Assert_NoAssume(IsInitialized_());

    _cmdBatchPool.Release(batch);
}
//----------------------------------------------------------------------------
void FVulkanFrameGraph::ReleaseMemory() noexcept {
    Assert(IsInitialized_());

    // delete command buffers
    {
        LOG(RHI, Verbose, L"max command batches = {0}", _cmdBatchPool.NumCreatedBlocks());
        LOG(RHI, Verbose, L"max command buffers = {0}", _cmdBufferPool.NumCreatedBlocks());
        LOG(RHI, Verbose, L"max submitted batches = {0}", _submittedPool.NumCreatedBlocks());

        _cmdBatchPool.Clear_ReleaseMemory();
        _cmdBufferPool.Clear_ReleaseMemory();
        _submittedPool.Clear_ReleaseMemory([pDevice{ &_device }](FVulkanSubmitted* p) NOEXCEPT {
            p->TearDown_(*pDevice);
        });
    }

    // shrink resource manager
    _resourceManager.ReleaseMemory();
}
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::AddPipelineCompiler(const PPipelineCompiler& pcompiler) {
    Assert(pcompiler);
    Assert_NoAssume(IsInitialized_());

    _resourceManager.AddCompiler(pcompiler);
    return true;
}
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
bool FVulkanFrameGraph::SetShaderDebugCallback(FShaderDebugCallback&& rcallback) {
    Assert_NoAssume(IsInitialized_());
    _shaderDebugCallback = std::move(rcallback);
    return true;
}
#endif
//----------------------------------------------------------------------------
void FVulkanFrameGraph::TransitImageLayoutToDefault_(FRawImageID imageId, VkImageLayout initialLayout, u32 queueFamily) {
    Assert_NoAssume(IsInitialized_());

    const FVulkanImage& image = _resourceManager.ResourceData(imageId);

    const auto& desc = image.Read();
    if (desc->DefaultLayout == initialLayout)
        return; // no transition required

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = 0;
    barrier.oldLayout = initialLayout;
    barrier.newLayout = desc->DefaultLayout;
    barrier.image = image.Handle();
    barrier.subresourceRange = {
        checked_cast<VkImageAspectFlags>(desc->AspectMask),
        0, desc->MipmapLevels(),
        0, desc->ArrayLayers()
    };

    // error will be generated by validation layer if current queue family
    // doesn't match with queue family in the command buffer
    barrier.srcQueueFamilyIndex = queueFamily;
    barrier.dstQueueFamilyIndex = queueFamily;

    const FCriticalScope queueLock{ &_queueCS };

    const auto it = _queueMap.MakeView().FindIf([queueFamily](const FQueueData& q) NOEXCEPT {
        return (q.Ptr && (
            queueFamily == static_cast<u32>(q.Ptr->FamilyIndex) ||
            queueFamily == VK_QUEUE_FAMILY_IGNORED ));
    });
    Assert_NoAssume(_queueMap.MakeView().end() != it);

    it->ImageBarriers.push_back(barrier);
}
//----------------------------------------------------------------------------
// CreatePipeline
//----------------------------------------------------------------------------
FMPipelineID FVulkanFrameGraph::CreatePipeline(FMeshPipelineDesc& desc ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    Assert_NoAssume(IsInitialized_());
    return FMPipelineID{ _resourceManager.CreatePipeline(desc ARGS_IF_RHIDEBUG(debugName)) };
}
//----------------------------------------------------------------------------
FRTPipelineID FVulkanFrameGraph::CreatePipeline(FRayTracingPipelineDesc& desc ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    Assert_NoAssume(IsInitialized_());
    return FRTPipelineID{ _resourceManager.CreatePipeline(desc ARGS_IF_RHIDEBUG(debugName)) };
}
//----------------------------------------------------------------------------
FGPipelineID FVulkanFrameGraph::CreatePipeline(FGraphicsPipelineDesc& desc ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    Assert_NoAssume(IsInitialized_());
    return FGPipelineID{ _resourceManager.CreatePipeline(desc ARGS_IF_RHIDEBUG(debugName)) };
}
//----------------------------------------------------------------------------
FCPipelineID FVulkanFrameGraph::CreatePipeline(FComputePipelineDesc& desc ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    Assert_NoAssume(IsInitialized_());
    return FCPipelineID{ _resourceManager.CreatePipeline(desc ARGS_IF_RHIDEBUG(debugName)) };
}
//----------------------------------------------------------------------------
// CreateImage
//----------------------------------------------------------------------------
FImageID FVulkanFrameGraph::CreateImage(const FImageDesc& desc, const FMemoryDesc& mem ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    return CreateImage(desc, mem, Default ARGS_IF_RHIDEBUG(debugName));
}
//----------------------------------------------------------------------------
FImageID FVulkanFrameGraph::CreateImage(const FImageDesc& desc, const FMemoryDesc& mem, EResourceState defaultState ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    Assert_NoAssume(IsInitialized_());

    const FRawImageID rawImage = _resourceManager.CreateImage(desc, mem, QueuesMask_(desc.Queues), defaultState ARGS_IF_RHIDEBUG(debugName));

    if (rawImage)
        TransitImageLayoutToDefault_(rawImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_QUEUE_FAMILY_IGNORED);

    return FImageID{ rawImage };
}
//----------------------------------------------------------------------------
// CreateBuffer
//----------------------------------------------------------------------------
FBufferID FVulkanFrameGraph::CreateBuffer(const FBufferDesc& desc, const FMemoryDesc& mem ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    Assert_NoAssume(IsInitialized_());
    return FBufferID{ _resourceManager.CreateBuffer(desc, mem, QueuesMask_(desc.Queues) ARGS_IF_RHIDEBUG(debugName)) };
}
//----------------------------------------------------------------------------
// CreateSampler
//----------------------------------------------------------------------------
FSamplerID FVulkanFrameGraph::CreateSampler(const FSamplerDesc& desc ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    Assert_NoAssume(IsInitialized_());
    return FSamplerID{ _resourceManager.CreateSampler(desc ARGS_IF_RHIDEBUG(debugName)) };
}
//----------------------------------------------------------------------------
// CreateSwapchain
//----------------------------------------------------------------------------
FSwapchainID FVulkanFrameGraph::CreateSwapchain(const FSwapchainDesc& desc, FRawSwapchainID oldSwapchain ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    Assert_NoAssume(IsInitialized_());
    return FSwapchainID{ _resourceManager.CreateSwapchain(desc, oldSwapchain, *this ARGS_IF_RHIDEBUG(debugName)) };
}
//----------------------------------------------------------------------------
// CreateRayTracingGeometry
//----------------------------------------------------------------------------
FRTGeometryID FVulkanFrameGraph::CreateRayTracingGeometry(const FRayTracingGeometryDesc& desc, const FMemoryDesc& mem ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    Assert_NoAssume(IsInitialized_());
    return FRTGeometryID{ _resourceManager.CreateRayTracingGeometry(desc, mem ARGS_IF_RHIDEBUG(debugName)) };
}
//----------------------------------------------------------------------------
// CreateRayTracingScene
//----------------------------------------------------------------------------
FRTSceneID FVulkanFrameGraph::CreateRayTracingScene(const FRayTracingSceneDesc& desc, const FMemoryDesc& mem ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    Assert_NoAssume(IsInitialized_());
    return FRTSceneID{ _resourceManager.CreateRayTracingScene(desc, mem ARGS_IF_RHIDEBUG(debugName)) };
}
//----------------------------------------------------------------------------
// CreateRayTracingShaderTable
//----------------------------------------------------------------------------
FRTShaderTableID FVulkanFrameGraph::CreateRayTracingShaderTable(ARG0_IF_RHIDEBUG(const FStringView& name)) {
    Assert_NoAssume(IsInitialized_());
    return FRTShaderTableID{ _resourceManager.CreateRayTracingShaderTable(ARG0_IF_RHIDEBUG(name)) };
}
//----------------------------------------------------------------------------
// InitPipelineResources
//----------------------------------------------------------------------------
template <typename _PplnId>
bool FVulkanFrameGraph::InitPipelineResources_(FPipelineResources* pResources, const _PplnId& pplnId, const FDescriptorSetID& dsId) const {
    Assert(pResources);
    Assert(pplnId.Valid());
    Assert(dsId.Valid());
    Assert_NoAssume(IsInitialized_());

    const auto& pplnData = _resourceManager.ResourceData(pplnId);
    const FVulkanPipelineLayout& pplnLayout = _resourceManager.ResourceData(*pplnData.Read()->BaseLayoutId);

    u32 binding;
    FRawDescriptorSetLayoutID layoutId;
    if (not pplnLayout.DescriptorLayout(&binding, &layoutId, dsId))
        return false;

    const FVulkanDescriptorSetLayout& dsLayout = _resourceManager.ResourceData(layoutId);
    FPipelineResources::Initialize(pResources, layoutId, dsLayout.Read()->DynamicData);
    return true;
}
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::InitPipelineResources(FPipelineResources* pResources, FRawGPipelineID pipeline, const FDescriptorSetID& id) const {
    return InitPipelineResources_(pResources, pipeline, id);
}
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::InitPipelineResources(FPipelineResources* pResources, FRawCPipelineID pipeline, const FDescriptorSetID& id) const {
    return InitPipelineResources_(pResources, pipeline, id);
}
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::InitPipelineResources(FPipelineResources* pResources, FRawMPipelineID pipeline, const FDescriptorSetID& id) const {
    return InitPipelineResources_(pResources, pipeline, id);
}
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::InitPipelineResources(FPipelineResources* pResources, FRawRTPipelineID pipeline, const FDescriptorSetID& id) const {
    return InitPipelineResources_(pResources, pipeline, id);
}
//----------------------------------------------------------------------------
// CachePipelineResources
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::CachePipelineResources(FPipelineResources& resources) {
    Assert_NoAssume(IsInitialized_());
    return _resourceManager.CacheDescriptorSet(resources);
}
//----------------------------------------------------------------------------
// ReleaseResource
//----------------------------------------------------------------------------
template <typename T>
bool FVulkanFrameGraph::ReleaseResource_(details::TResourceWrappedId<T>& id) {
    if (not id) {
        LOG(RHI, Warning, L"trying to release an empty resource");
        return false;
    }

    Assert_NoAssume(IsInitialized_());
    return _resourceManager.ReleaseResource(id.Release());
}
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::ReleaseResource(FGPipelineID& id) { return ReleaseResource_(id); }
bool FVulkanFrameGraph::ReleaseResource(FCPipelineID& id) { return ReleaseResource_(id); }
bool FVulkanFrameGraph::ReleaseResource(FMPipelineID& id) { return ReleaseResource_(id); }
bool FVulkanFrameGraph::ReleaseResource(FRTPipelineID& id) { return ReleaseResource_(id); }
bool FVulkanFrameGraph::ReleaseResource(FImageID& id) { return ReleaseResource_(id); }
bool FVulkanFrameGraph::ReleaseResource(FBufferID& id) { return ReleaseResource_(id); }
bool FVulkanFrameGraph::ReleaseResource(FSamplerID& id) { return ReleaseResource_(id); }
bool FVulkanFrameGraph::ReleaseResource(FSwapchainID& id) { return ReleaseResource_(id); }
bool FVulkanFrameGraph::ReleaseResource(FRTGeometryID& id) { return ReleaseResource_(id); }
bool FVulkanFrameGraph::ReleaseResource(FRTSceneID& id) { return ReleaseResource_(id); }
bool FVulkanFrameGraph::ReleaseResource(FRTShaderTableID& id) { return ReleaseResource_(id); }
//----------------------------------------------------------------------------
void FVulkanFrameGraph::ReleaseResource(FPipelineResources& resources) {
    Assert_NoAssume(IsInitialized_());
    _resourceManager.ReleaseResource(resources);
}
//----------------------------------------------------------------------------
// IsSupported
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::IsSupported(FRawImageID image, const FImageViewDesc& desc) const NOEXCEPT {
    Assert_NoAssume(IsInitialized_());
    const FVulkanImage& res = _resourceManager.ResourceData(image);
    return res.IsSupported(_device, desc);
}
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::IsSupported(FRawBufferID buffer, const FBufferViewDesc& desc) const NOEXCEPT {
    Assert_NoAssume(IsInitialized_());
    const FVulkanBuffer& res = _resourceManager.ResourceData(buffer);
    return res.IsSupported(_device, desc);
}
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::IsSupported(const FImageDesc& desc, EMemoryType memType) const NOEXCEPT {
    Assert_NoAssume(IsInitialized_());
    return FVulkanImage::IsSupported(_device, desc, memType);
}
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::IsSupported(const FBufferDesc& desc, EMemoryType memType) const NOEXCEPT {
    Assert_NoAssume(IsInitialized_());
    return FVulkanBuffer::IsSupported(_device, desc, memType);
}
//----------------------------------------------------------------------------
// Description
//----------------------------------------------------------------------------
const FBufferDesc& FVulkanFrameGraph::Description(FRawBufferID id) const {
    Assert_NoAssume(IsInitialized_());
    return _resourceManager.ResourceDescription(id);
}
//----------------------------------------------------------------------------
const FImageDesc& FVulkanFrameGraph::Description(FRawImageID id) const {
    Assert_NoAssume(IsInitialized_());
    return _resourceManager.ResourceDescription(id);
}
//----------------------------------------------------------------------------
// External resources
//----------------------------------------------------------------------------
NODISCARD FImageID FVulkanFrameGraph::CreateImage(
    const FImageDesc& desc,
    FExternalImage externalImage, FOnReleaseExternalImage&& onRelease,
    TMemoryView<const u32> queueFamilyIndices
    ARGS_IF_RHIDEBUG(FConstChar debugName) ) {
    Assert_NoAssume(IsInitialized_());
    return FImageID{ _resourceManager.CreateImage(desc, externalImage, std::move(onRelease), queueFamilyIndices ARGS_IF_RHIDEBUG(debugName)) };
}
//----------------------------------------------------------------------------
NODISCARD FBufferID FVulkanFrameGraph::CreateBuffer(
    const FBufferDesc& desc,
    FExternalBuffer externalBuffer, FOnReleaseExternalBuffer&& onRelease,
    TMemoryView<const u32> queueFamilyIndices
    ARGS_IF_RHIDEBUG(FConstChar debugName) ) {
    Assert_NoAssume(IsInitialized_());
    return FBufferID{ _resourceManager.CreateBuffer(desc, externalBuffer, std::move(onRelease), queueFamilyIndices ARGS_IF_RHIDEBUG(debugName)) };
}
//----------------------------------------------------------------------------
// ExternalDescription
//----------------------------------------------------------------------------
const void* FVulkanFrameGraph::ExternalDescription(FRawBufferID id) const NOEXCEPT {
    Assert_NoAssume(IsInitialized_());
    const FVulkanBuffer* const pBuffer = _resourceManager.ResourceDataIFP(id);
    return (pBuffer ? pBuffer->Read()->vkBuffer : nullptr);
}
//----------------------------------------------------------------------------
const void* FVulkanFrameGraph::ExternalDescription(FRawImageID id) const NOEXCEPT {
    Assert_NoAssume(IsInitialized_());
    const FVulkanImage* const pImage = _resourceManager.ResourceDataIFP(id);
    return (pImage ? pImage->Read()->vkImage : nullptr);
}
//----------------------------------------------------------------------------
// UpdateHostBuffer
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::UpdateHostBuffer(FRawBufferID id, size_t offset, size_t size, const void* data) {
    Assert(data || size == 0);

    void* pData = nullptr;
    if (MapBufferRange(id, offset, size, &pData)) {
        Assert(pData);
        FPlatformMemory::Memcpy(pData, data, size);
        return true;
    }

    return false;
}
//----------------------------------------------------------------------------
// MapBufferRange
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::MapBufferRange(FRawBufferID id, size_t offset, size_t& size, void** data) {
    Assert(id.Valid());
    Assert_NoAssume(IsInitialized_());

    if (const FVulkanBuffer* buf = _resourceManager.ResourceDataIFP(id)) {
        if (const FVulkanMemoryObject* mem = _resourceManager.ResourceDataIFP(*buf->Read()->MemoryId)) {

            FVulkanMemoryInfo info;
            if (mem->MemoryInfo(&info, _resourceManager.MemoryManager())) {
                Assert(info.MappedPtr);
                Assert(info.Size >= offset);

                size = Min(size, info.Size - offset);
                *data = (static_cast<byte*>(info.MappedPtr) + offset);
                return true;
            }
        }
    }

    return false;
}
//----------------------------------------------------------------------------
// Begin
//----------------------------------------------------------------------------
FCommandBufferBatch FVulkanFrameGraph::Begin(const FCommandBufferDesc& desc, TMemoryView<const FCommandBufferBatch> dependsOn) {
    Assert(static_cast<u32>(desc.QueueType) == _queueMap.size());
    Assert_NoAssume(IsInitialized_());

    PVulkanCommandBuffer cmd = _cmdBufferPool.Allocate([this](FVulkanCommandBuffer* pCmd, u32 id) {
        INPLACE_NEW(pCmd, FVulkanCommandBuffer){ this, id };
    });
    AssertReleaseMessage(L"command buffer pool overflow !", cmd);

    FQueueData& queue = QueueData_(desc.QueueType);
    Assert(queue.Ptr);

    PVulkanCommandBatch batch = _cmdBatchPool.Allocate([this, &queue, dependsOn](FVulkanCommandBatch* pBatch, u32 id) {
        INPLACE_NEW(pBatch, FVulkanCommandBatch){ this, id };
        pBatch->Construct(queue.Type, dependsOn);
    });
    AssertReleaseMessage(L"command batch pool overflow !", batch);

    if (not cmd->Begin(desc, batch, queue.Ptr))
        PPE_THROW_IT(FVulkanException("FVulkanCommandBuffer::Begin", VK_ERROR_UNKNOWN));

    return FCommandBufferBatch{ std::move(cmd), std::move(batch) };
}
//----------------------------------------------------------------------------
// Execute
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::Execute(FCommandBufferBatch& cmdBatch) {
    Assert(cmdBatch.Batch);
    Assert(cmdBatch.Buffer);
    Assert_NoAssume(IsInitialized_());

    PVulkanCommandBuffer cmd{ checked_cast<FVulkanCommandBuffer>(cmdBatch.Buffer) };

    // execute and release the command buffer
    if (not cmd->Execute())
        return false;

    PVulkanCommandBatch batch{ cmd->Batch() };
    Assert_NoAssume(batch == cmdBatch.Batch);

    RemoveRef_AssertAlive(cmdBatch.Buffer); // release client ref first
    _cmdBufferPool.Release(RemoveRef_AssertReachZero_KeepAlive(cmd)); //  then release tmp ref

    // add batch to the submission queue
    const u32 queueIndex = static_cast<u32>(batch->Read()->QueueType);
    Assert(queueIndex < _queueMap.size());

    const FCriticalScope queueLock{ &_queueCS };
    _queueMap[queueIndex].Pending.push_back(std::move(batch));

    return true;
}
//----------------------------------------------------------------------------
// Flush
//----------------------------------------------------------------------------
VkSemaphore FVulkanFrameGraph::CreateSemaphore_() {
    VkSemaphoreCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    info.flags = 0;

    VkSemaphore result = VK_NULL_HANDLE;
    VK_CALL( _device.vkCreateSemaphore(_device.vkDevice(), &info, _device.vkAllocator(), &result) );
    Assert_NoAssume(VK_NULL_HANDLE != result);

#if USE_PPE_RHITASKNAME
    _device.SetObjectName(reinterpret_cast<u64>(result), "BatchSemaphore", VK_OBJECT_TYPE_SEMAPHORE);
#endif

    return result;
}
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::FlushQueue_(EQueueType index, u32 maxIter) {
    ONLY_IF_RHIDEBUG(FAtomicTimedScope submittedTime(&_submittingTime));

    FQueueData& q = _queueMap[static_cast<u32>(index)];
    EQueueUsage usage = Default;
    bool waitIdle = false;

    FCommandBatches pending;
    FSubmitInfos submitInfos;
    FTransientSemaphores releaseSemaphores;
    FPendingSwapchains swapchains;

    // find batches that can be submitted
    forrange(b, 0, Min(maxIter, checked_cast<u32>(q.Pending.size()))) {
        bool changed = false;

        forrange(a, 0, checked_cast<u32>(q.Pending.size())) {
            PVulkanCommandBatch pBatch;
            q.Pending.pop_front_AssumeNotEmpty(&pBatch);
            Assert(pBatch);
            Assert_NoAssume(pBatch->State() == EBatchState::Baked);

            const auto& batchReadable = pBatch->Read();

            bool ready = false;
            EQueueUsage mask = Default;
            for (const SVulkanCommandBatch& dep : batchReadable->Dependencies) {
                const auto& depReadable = dep->Read();
                const auto minState = (depReadable->QueueType == index ? EBatchState::Ready : EBatchState::Submitted);
                mask |= EQueueType_Usage(depReadable->QueueType);
                ready &= (dep->State() >= minState);
            }

            if (Likely(ready)) {
                if (pending.size() == pending.capacity())
                    break; // break when pending queue is full

                waitIdle |= batchReadable->NeedQueueSync;
                pBatch->OnReadyToSubmit();
                pending.Push(std::move(pBatch));

                usage |= mask;
                changed = true;
            }
            else {
                // push again at the back of queue if still pending
                q.Pending.push_back(std::move(pBatch));
            }
        }

        if (not changed)
            break;
    }

    if (pending.empty())
        return false;

    // add semaphores
    const u32 qi = static_cast<u32>(index);
    forrange(qj, 0, checked_cast<u32>(_queueMap.size())) {
        FQueueData& other = _queueMap[qj];
        if (not other.Ptr || qj == qi)
            continue;

        // input
        if (usage & EQueueType_Usage(EQueueType(qj)) && other.Semaphores[qi]) {
            pending.MakeView().front()->WaitSemaphore(other.Semaphores[qi], VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
            releaseSemaphores.Push(other.Semaphores[qi]);
            other.Semaphores[qi] = VK_NULL_HANDLE;
        }

        // output
        if (VK_NULL_HANDLE != q.Semaphores[qj])
            releaseSemaphores.Push(q.Semaphores[qj]);

        q.Semaphores[qj] = CreateSemaphore_();
        pending.MakeView().back()->SignalSemaphore(q.Semaphores[qj]);
    }

    // acquire submitted batch
    FVulkanSubmitted* const pSubmit = _submittedPool.Allocate([](FVulkanSubmitted* s, u32 id) {
        INPLACE_NEW(s, FVulkanSubmitted){ id };
    });
    AssertReleaseMessage(L"submitted pool overflow !", pSubmit);

    // add image layout transitions
    if (not q.ImageBarriers.empty()) {
        const VkCommandBuffer cmdBuf = q.CommandPool.AllocPrimary(_device);
        Assert_NoAssume(VK_NULL_HANDLE != cmdBuf);

        VkCommandBufferBeginInfo begin{};
        begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        VK_CHECK( _device.vkBeginCommandBuffer(cmdBuf, &begin) );

        _device.vkCmdPipelineBarrier(
            cmdBuf,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            0,
            0, nullptr,
            0, nullptr,
            checked_cast<u32>(q.ImageBarriers.size()), q.ImageBarriers.data() );

        VK_CHECK( _device.vkEndCommandBuffer(cmdBuf) );
        q.ImageBarriers.clear();

        pending.MakeView().front()->PushCommandToFront(&q.CommandPool, cmdBuf);
    }

    // init submit info
    forrange(i, 0, checked_cast<u32>(pending.size()))
        pending[i]->OnBeforeSubmit(&submitInfos[i]);

    // submit & present
    {
        const FCriticalScope barrierLock(&q.Ptr->Barrier);

        VK_CALL( _device.vkQueueSubmit(
            q.Ptr->Handle,
            checked_cast<u32>(pending.size()),
            submitInfos.MakeView().data(),
            pSubmit->Write()->Fence ) );

        forrange(i, 0, checked_cast<u32>(pending.size()))
            pending[i]->OnAfterSubmit(MakeAppendable(swapchains), pSubmit);

        if (waitIdle) // sync other submission queues, only for debugging
            VK_CALL( _device.vkQueueWaitIdle(q.Ptr->Handle) );

        for (const FVulkanSwapchain* swapchain : swapchains) {
            Assert_NoAssume(q.Ptr == swapchain->PresentQueue());
            if (not swapchain->Present(_device))
                PPE_THROW_IT(FVulkanException("FVulkanSwapchain::Present", VK_ERROR_UNKNOWN));
        }

        if (waitIdle && not swapchains.empty()) // sync other presenting, only for debugging
            VK_CALL( _device.vkDeviceWaitIdle(_device.vkDevice()) );
    }

    // remove completed batches
    for (auto it = q.Submitted.begin(); q.Submitted.end() != it; ) {
        FVulkanSubmitted* const submitted{ *it };
        VkFence fence = submitted->Read()->Fence;

        bool complete = (VK_NULL_HANDLE == fence);
        if (fence && _device.vkGetFenceStatus(_device.vkDevice(), fence) == VK_SUCCESS)
            complete = true;

        if (complete) {
            {
                ONLY_IF_RHIDEBUG(const FCriticalScope statsLock(&_lastFrameStatsCS));
                submitted->Release_(_device ARGS_IF_RHIDEBUG(_debugger, _shaderDebugCallback, _lastFrameStats));
            }

            it = q.Submitted.erase(it);
            _submittedPool.Release(submitted);
        }
        else {
            break;
        }
    }

    q.Submitted.push_back(pSubmit);
    _resourceManager.OnSubmit();

    return true;
}
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::FlushAll_(EQueueUsage queues, u32 maxIter) {
    bool flushed = false;
    forrange(a, 0, Min(maxIter, checked_cast<u32>(_queueMap.size()))) {
        bool changed = false;

        forrange(qi, 0, checked_cast<u32>(_queueMap.size())) {
            if (Meta::EnumHas(queues, 1u << qi) && _queueMap[qi].Ptr)
                changed |= FlushQueue_(static_cast<EQueueType>(qi), PPE_RHIVK__FLUSH_ITERATIONS);
        }

        flushed |= changed;
        if (not changed)
            break;
    }
    return flushed;
}
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::Flush(EQueueUsage queues) {
    bool result;
    {
        const FCriticalScope queueLock{ &_queueCS };
        result = FlushAll_(queues, PPE_RHIVK__FLUSH_ITERATIONS);
    }

    _resourceManager.RunValidation(PPE_RHIVK__VALIDATION_ITERATIONS);
    return result;
}
//----------------------------------------------------------------------------
// Wait
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::Wait(TMemoryView<const FCommandBufferBatch> commands, FNanoseconds timeout) {
    ONLY_IF_RHIDEBUG(FAtomicTimedScope waitedTime(&_waitingTime));

    //bool result = true;
    FTransientFences transientFences;
    FTransientSubmitted transientSubmitted;

    const FCriticalScope queueLock{ &_queueCS };

    const auto waitAndRelease = [&, this, timeout]() -> bool {
        const VkResult result = _device.vkWaitForFences(
            _device.vkDevice(),
            checked_cast<u32>(transientFences.size()),
            transientFences.data(),
            VK_TRUE, checked_cast<u64>(timeout.Value()) );

        if (VK_SUCCESS == result) {
            ONLY_IF_RHIDEBUG(const FCriticalScope statsLock(&_lastFrameStatsCS));
            for (FVulkanSubmitted* submitted : transientSubmitted)
                submitted->Release_(_device ARGS_IF_RHIDEBUG(_debugger, _shaderDebugCallback, _lastFrameStats));
        }
        else {
            Assert_NoAssume(VK_TIMEOUT == result);
        }

        transientFences.clear();
        transientSubmitted.clear();
        return (VK_SUCCESS == result);
    };

    bool result = true;

    for (const FCommandBufferBatch& cmd : commands) {
        Assert(cmd.Batch);
        const PVulkanCommandBatch batch = checked_cast<FVulkanCommandBatch>(cmd.Batch);

        switch (batch->State()) {
        case FVulkanCommandBatch::EState::Complete: {
            break; // Ready
        }
        case FVulkanCommandBatch::EState::Submitted: {
            FVulkanSubmitted* const submitted = batch->Submitted();
            const VkFence fence = submitted->Read()->Fence;
            Assert(VK_NULL_HANDLE != fence);

            if (not transientFences.Contains(fence)) {
                transientFences.Push(fence);
                transientSubmitted.Push(submitted);
            }
            break;
        }
        default: break;
        }

        if (transientFences.size() == transientSubmitted.size())
            result = waitAndRelease();
    }

    if (not transientFences.empty())
        result = waitAndRelease();

    return result;
}
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::WaitIdle() {
    ONLY_IF_RHIDEBUG(FAtomicTimedScope waitedTime(&_waitingTime));
    {
        FTransientFences fences;

        const FCriticalScope queueLock(&_queueCS);

        if (not FlushAll_(EQueueUsage::All, PPE_RHIVK__FLUSH_ITERATIONS))
            return false;

        for (FQueueData& q : _queueMap) {
            Assert_NoAssume(q.Pending.empty());
            for (FVulkanSubmitted* submitted : q.Submitted) {
                if (const VkFence fence = submitted->Read()->Fence)
                    fences.Push(fence);
            }
        }

        if (not fences.empty()) {
            VK_CALL( _device.vkWaitForFences(
                _device.vkDevice(),
                checked_cast<u32>(fences.size()),
                fences.data(),
                VK_TRUE, UMax ));
        }

        ONLY_IF_RHIDEBUG(const FCriticalScope statsLock(&_lastFrameStatsCS));

        for (FQueueData& q : _queueMap) {
            for (FVulkanSubmitted* submitted : q.Submitted) {
                submitted->Release_(_device ARGS_IF_RHIDEBUG(_debugger, _shaderDebugCallback, _lastFrameStats));
                _submittedPool.Release(submitted);
            }

            q.Submitted.clear();
        }
    }

    _resourceManager.RunValidation(PPE_RHIVK__VALIDATION_ITERATIONS);

    return true;
}
//----------------------------------------------------------------------------
// Debug
//----------------------------------------------------------------------------
#if USE_PPE_RHIPROFILING || USE_PPE_RHIDEBUG
bool FVulkanFrameGraph::DumpStatistics(FFrameStatistics* pStats) const {
    ONLY_IF_RHIDEBUG(const FCriticalScope statsLock(&_lastFrameStatsCS));

    *pStats = _lastFrameStats;
    pStats->Renderer.SubmittingTime = FSeconds{ FPlatformTime::ToSeconds(
        _submittingTime.exchange(0, std::memory_order_relaxed)) };
    pStats->Renderer.WaitingTime = FSeconds{ FPlatformTime::ToSeconds(
        _waitingTime.exchange(0, std::memory_order_relaxed)) };

    return true;
}
#endif
//----------------------------------------------------------------------------
// Queues
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::IsUniqueQueue_(const PVulkanDeviceQueue& queue) const {
    Assert(queue);
    const auto it = _queueMap.MakeView().FindIf([queue](const FQueueData& q) NOEXCEPT -> bool {
        Assert_NoAssume(q.Ptr);
        return (q.Ptr == queue);
    });
    return (_queueMap.MakeView().end() == it);
}
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::CreateQueue_(EQueueType index, const PVulkanDeviceQueue& queue) {
    Assert(queue);
    _queueUsage |= EQueueType_Usage(index);

    FQueueData& q = _queueMap[static_cast<u32>(index)];
    q.Ptr = queue;
    q.Type = index;

    return q.CommandPool.Construct(_device, q.Ptr.get() ARGS_IF_RHIDEBUG(queue->DebugName));
}
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::AddGraphicsQueue_() {
    PVulkanDeviceQueue bestMatch{ Meta::ForceInit };
    PVulkanDeviceQueue compatible{ Meta::ForceInit };

    for (const FVulkanDeviceQueue& q : _device.vkQueues()) {
        if (!!(q.FamilyFlags & VK_QUEUE_GRAPHICS_BIT)) {
            compatible = &q;
            if (IsUniqueQueue_(q)) {
                bestMatch = q;
                break;
            }
        }
    }

    if (not bestMatch)
        bestMatch = compatible;
    if (not bestMatch)
        return false;

    return CreateQueue_(EQueueType::Graphics, bestMatch);
}
//----------------------------------------------------------------------------
bool FVulkanFrameGraph::AddAsyncComputeQueue_() {
    PVulkanDeviceQueue unique{ Meta::ForceInit };
    PVulkanDeviceQueue bestMatch{ Meta::ForceInit };
    PVulkanDeviceQueue compatible{ Meta::ForceInit };

    for (const FVulkanDeviceQueue& q : _device.vkQueues()) {
        const bool isUnique = IsUniqueQueue_(q);
        const bool hasCompute = !!(q.FamilyFlags & VK_QUEUE_COMPUTE_BIT);
        const bool hasGraphics = !!(q.FamilyFlags & VK_QUEUE_GRAPHICS_BIT);

        if (hasCompute && not hasGraphics) {
            compatible = &q;
            if (isUnique) {
                bestMatch = q;
                break;
            }
        }
        else if ((hasCompute || hasGraphics) && unique) {
            unique = q;
        }
    }

    if (not bestMatch)
        bestMatch = unique;
    if (not bestMatch)
        bestMatch = compatible;
    if (not bestMatch)
        return false;

    return CreateQueue_(EQueueType::AsyncCompute, bestMatch);
}

//----------------------------------------------------------------------------
bool FVulkanFrameGraph::AddAsyncTransferQueue_() {
    PVulkanDeviceQueue unique{ Meta::ForceInit };
    PVulkanDeviceQueue bestMatch{ Meta::ForceInit };
    PVulkanDeviceQueue compatible{ Meta::ForceInit };

    for (const FVulkanDeviceQueue& q : _device.vkQueues()) {
        const bool isUnique = IsUniqueQueue_(q);
        const bool hasTransfer = !!(q.FamilyFlags & VK_QUEUE_TRANSFER_BIT);
        const bool supportsTransfer = !!(q.FamilyFlags & (VK_QUEUE_COMPUTE_BIT | VK_QUEUE_GRAPHICS_BIT));

        if (hasTransfer && not supportsTransfer) {
            compatible = &q;
            if (isUnique) {
                bestMatch = q;
                break;
            }
        }
        else if ((hasTransfer || supportsTransfer) && unique) {
            unique = q;
        }
    }

    if (not bestMatch)
        bestMatch = unique;
    if (not bestMatch)
        bestMatch = compatible;
    if (not bestMatch)
        return false;

    return CreateQueue_(EQueueType::AsyncTransfer, bestMatch);
}
//----------------------------------------------------------------------------
PVulkanDeviceQueue FVulkanFrameGraph::FindQueue(EQueueType queueType) const {
    return (static_cast<u32>(queueType) < _queueMap.size()
        ? _queueMap[static_cast<u32>(queueType)].Ptr
        : nullptr );
}
//----------------------------------------------------------------------------
FVulkanFrameGraph::FQueueData& FVulkanFrameGraph::QueueData_(EQueueType index) {
    if (_queueMap[static_cast<u32>(index)].Ptr)
        return _queueMap[static_cast<u32>(index)];

    // fall back to default (graphics) queue
    return _queueMap[static_cast<u32>(EQueueType::Graphics)];
}
//----------------------------------------------------------------------------
EVulkanQueueFamilyMask FVulkanFrameGraph::QueuesMask_(EQueueUsage types) const {
    EVulkanQueueFamilyMask result = Default;

    for (u32 i = 0; ((1u << i) <= static_cast<u32>(types)) && (i < _queueMap.size()); ++i) {
        if (not Meta::EnumHas(types, 1u << i))
            continue;
        if (_queueMap[i].Ptr)
            result |= static_cast<EVulkanQueueFamily>(_queueMap[i].Ptr->FamilyIndex);
    }

    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
