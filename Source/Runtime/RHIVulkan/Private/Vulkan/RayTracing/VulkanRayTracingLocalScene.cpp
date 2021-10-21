
#include "stdafx.h"

#include "Vulkan/RayTracing/VulkanRayTracingLocalScene.h"

#include "Vulkan/Command/VulkanBarrierManager.h"
#include "Vulkan/Command/VulkanFrameTask.h"
#include "Vulkan/Debugger/VulkanLocalDebugger.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
FVulkanRayTracingLocalScene::~FVulkanRayTracingLocalScene() {
    Assert_NoAssume(nullptr == _rtScene);
}
#endif
//----------------------------------------------------------------------------
bool FVulkanRayTracingLocalScene::Construct(const FVulkanRayTracingScene* sceneData) {
    Assert(sceneData);
    Assert(nullptr == _rtScene);

    _rtScene = sceneData;
    return true;
}
//----------------------------------------------------------------------------
void FVulkanRayTracingLocalScene::TearDown() {
    Assert(nullptr != _rtScene);

    _rtScene = nullptr;

    // check for uncommitted barriers
    Assert(not _pendingAccesses.Valid());
    Assert(not _accessForReadWrite.Valid());

    _pendingAccesses = Default;
    _accessForReadWrite = Default;
}
//----------------------------------------------------------------------------
void FVulkanRayTracingLocalScene::AddPendingState(const FSceneState& state) const {
    Assert(state.Task);

    _pendingAccesses.Stages = EResourceState_ToPipelineStages(state.State);
    _pendingAccesses.Access = EResourceState_ToAccess(state.State);
    _pendingAccesses.IsReadable = EResourceState_IsReadable(state.State);
    _pendingAccesses.IsWritable = EResourceState_IsWritable(state.State);
    _pendingAccesses.Index = state.Task->ExecutionOrder();
}
//----------------------------------------------------------------------------
void FVulkanRayTracingLocalScene::ResetState(
    EVulkanExecutionOrder index, FVulkanBarrierManager& barriers
    ARGS_IF_RHIDEBUG(FVulkanLocalDebugger* debuggerIFP) ) {
    AssertReleaseMessage(L"you must commit all pending states before reseting", not _pendingAccesses.Valid());

    // add full range barrier
    _pendingAccesses.IsReadable = true;
    _pendingAccesses.IsWritable = false;
    _pendingAccesses.Stages = 0;
    _pendingAccesses.Access = 0;
    _pendingAccesses.Index = index;

    CommitBarrier(barriers ARGS_IF_RHIDEBUG(debuggerIFP));

    // flush
    _accessForReadWrite = Default;
}
//----------------------------------------------------------------------------
void FVulkanRayTracingLocalScene::CommitBarrier(
    FVulkanBarrierManager& barriers
    ARGS_IF_RHIDEBUG(FVulkanLocalDebugger* debuggerIFP) ) const {
    if ((_accessForReadWrite.IsReadable && _pendingAccesses.IsWritable) || // read -> write
        (_accessForReadWrite.IsWritable) ) {

        VkMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        barrier.srcAccessMask = _accessForReadWrite.Access;
        barrier.dstAccessMask = _pendingAccesses.Access;

        barriers.AddMemoryBarrier(
            _accessForReadWrite.Stages,
            _pendingAccesses.Stages,
            barrier );

#if USE_PPE_RHIDEBUG
        if (debuggerIFP) {
            debuggerIFP->AddBarrier(
                _rtScene,
                _accessForReadWrite.Index,
                _pendingAccesses.Index,
                _accessForReadWrite.Stages,
                _pendingAccesses.Stages,
                0, barrier );
        }
#endif

        _accessForReadWrite = _pendingAccesses;
    }
    else {
        _accessForReadWrite.Stages |= _pendingAccesses.Stages;
        _accessForReadWrite.Access = _pendingAccesses.Access;
        _accessForReadWrite.Index = _pendingAccesses.Index;
        _accessForReadWrite.IsReadable |= _pendingAccesses.IsReadable;
        _accessForReadWrite.IsWritable |= _pendingAccesses.IsWritable;
    }

    _pendingAccesses = Default;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
