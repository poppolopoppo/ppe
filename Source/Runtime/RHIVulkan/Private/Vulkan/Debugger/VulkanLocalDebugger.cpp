
#include "stdafx.h"

#include "Vulkan/Debugger/VulkanLocalDebugger.h"

#if USE_PPE_RHIDEBUG

#include "Vulkan/Command/VulkanDrawTask.h"
#include "Vulkan/Command/VulkanFrameTask.h"
#include "Vulkan/Command/VulkanRaytracingTask.h"
#include "Vulkan/Instance/VulkanFrameGraph.h"

#include "Vulkan/Debugger/VulkanLocalDebugger_RawText.h"
//#include "Vulkan/Debugger/VulkanLocalDebugger_GraphViz.h" #TODO

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Local debugger
//----------------------------------------------------------------------------
FVulkanLocalDebugger::FVulkanLocalDebugger()
:   _flags(Default) {
    _tasks.reserve_AssumeEmpty(256);
}
//----------------------------------------------------------------------------
void FVulkanLocalDebugger::Begin(EDebugFlags flags) {
    _flags = flags;
    _tasks.resize(1);
}
//----------------------------------------------------------------------------
void FVulkanLocalDebugger::End(
    FStringBuilder* pdump,
    FBatchGraph* pgraph,
    const FVulkanFrameGraph& fg,
    FStringView name, u32 cmdBufferUID,
    TMemoryView<const SVulkanCommandBatch> dependencies ) {
    CONSTEXPR auto GDumpFlags = (
        EDebugFlags::LogTasks |
        EDebugFlags::LogBarriers |
        EDebugFlags::LogResourceUsage );

    if (_flags ^ GDumpFlags) {
        _subBatchUID = StringFormat("{0:X}", (cmdBufferUID & 0xFFF) | (_counter << 12));

        if (pdump) {
            FRawTextDump_ rawText{ pdump, *this, fg.Device() };
            rawText.DumpFrame(name, dependencies);
        }

        if (pgraph) {
#if 0
            FGraphVizDump_ graphViz{ pgraph, *this, fg.Device() };
            graphViz.DumpGraph();
#else
            UNUSED(pgraph); // #TODO
#endif
        }
    }

    _counter++;
    _subBatchUID.clear();
    _tasks.clear();
    _images.clear();
    _buffers.clear();
}
//----------------------------------------------------------------------------
void FVulkanLocalDebugger::AddTask(PVulkanFrameTask task) {
    Assert(task);

    if (not (_flags & EDebugFlags::LogTasks))
        return;

    const size_t idx = static_cast<const size_t>(task->ExecutionOrder());
    _tasks.resize(idx+1);

    Assert(nullptr == _tasks[idx].Task);
    _tasks[idx] = FTaskInfo{ task };
}
//----------------------------------------------------------------------------
void FVulkanLocalDebugger::AddHostWriteAccess(const FVulkanBuffer* , u32 , u32 ) {

}
//----------------------------------------------------------------------------
void FVulkanLocalDebugger::AddHostReadAccess(const FVulkanBuffer* , u32 , u32 ) {

}
//----------------------------------------------------------------------------
// AddBarrier()
//----------------------------------------------------------------------------
void FVulkanLocalDebugger::AddBarrier(const FVulkanBuffer* buffer, EVulkanExecutionOrder srcIndex, EVulkanExecutionOrder dstIndex, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, const VkBufferMemoryBarrier& barrier) {
    if (not (_flags & EDebugFlags::LogBarriers))
        return;

    Assert(buffer);
    auto& barriers = _buffers.insert({ buffer, {} }).first->second.Barriers;
    barriers.emplace_back(barrier, srcIndex, dstIndex, srcStageMask, dstStageMask, dependencyFlags);
}
//----------------------------------------------------------------------------
void FVulkanLocalDebugger::AddBarrier(const FVulkanImage* image, EVulkanExecutionOrder srcIndex, EVulkanExecutionOrder dstIndex, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, const VkImageMemoryBarrier& barrier) {
    if (not (_flags & EDebugFlags::LogBarriers))
        return;

    Assert(image);
    auto& barriers = _images.insert({ image, {} }).first->second.Barriers;
    barriers.emplace_back(barrier, srcIndex, dstIndex, srcStageMask, dstStageMask, dependencyFlags);
}
//----------------------------------------------------------------------------
void FVulkanLocalDebugger::AddBarrier(const FVulkanRayTracingGeometry* rtGeometry, EVulkanExecutionOrder srcIndex, EVulkanExecutionOrder dstIndex, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, const VkMemoryBarrier& barrier) {
    if (not (_flags & EDebugFlags::LogBarriers))
        return;

    Assert(rtGeometry);
    auto& barriers = _rtGeometries.insert({ rtGeometry, {} }).first->second.Barriers;
    barriers.emplace_back(barrier, srcIndex, dstIndex, srcStageMask, dstStageMask, dependencyFlags);
}
//----------------------------------------------------------------------------
void FVulkanLocalDebugger::AddBarrier(const FVulkanRayTracingScene* rtScene, EVulkanExecutionOrder srcIndex, EVulkanExecutionOrder dstIndex, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, const VkMemoryBarrier& barrier) {
    if (not (_flags & EDebugFlags::LogBarriers))
        return;

    Assert(rtScene);
    auto& barriers = _rtScenes.insert({ rtScene, {} }).first->second.Barriers;
    barriers.emplace_back(barrier, srcIndex, dstIndex, srcStageMask, dstStageMask, dependencyFlags);
}
//----------------------------------------------------------------------------
// AddUsage()
//----------------------------------------------------------------------------
void FVulkanLocalDebugger::AddUsage(const FVulkanBuffer* buffer, const FVulkanLocalBuffer::FBufferState& state) {
    if (not (_flags & EDebugFlags::LogResourceUsage))
        return;

    Assert(buffer);
    Assert(state.Task);

    const size_t idx = static_cast<const size_t>(state.Task->ExecutionOrder());
    if (idx >= _tasks.size() || nullptr == _tasks[idx].Task) {
        Assert(!"task doesn't exists!");
        return;
    }

    _tasks[idx].Resources.push_back(FBufferUsage{ buffer, state });
}
//----------------------------------------------------------------------------
void FVulkanLocalDebugger::AddUsage(const FVulkanImage* image, const FVulkanLocalImage::FImageState& state) {
    if (not (_flags & EDebugFlags::LogResourceUsage))
        return;

    Assert(image);
    Assert(state.Task);

    const size_t idx = static_cast<const size_t>(state.Task->ExecutionOrder());
    if (idx >= _tasks.size() || nullptr == _tasks[idx].Task) {
        Assert(!"task doesn't exists!");
        return;
    }

    _tasks[idx].Resources.emplace_back(FImageUsage{ image, state });
}
//----------------------------------------------------------------------------
void FVulkanLocalDebugger::AddUsage(const FVulkanRayTracingGeometry* rtGeometry, const FVulkanRayTracingLocalGeometry::FGeometryState& state) {
    if (not (_flags & EDebugFlags::LogResourceUsage))
        return;

    Assert(rtGeometry);
    Assert(state.Task);

    const size_t idx = static_cast<const size_t>(state.Task->ExecutionOrder());
    if (idx >= _tasks.size() || nullptr == _tasks[idx].Task) {
        Assert(!"task doesn't exists!");
        return;
    }

    _tasks[idx].Resources.emplace_back(FRTGeometryUsage{ rtGeometry, state });
}
//----------------------------------------------------------------------------
void FVulkanLocalDebugger::AddUsage(const FVulkanRayTracingScene* rtScene, const FVulkanRayTracingLocalScene::FSceneState& state) {
    if (not (_flags & EDebugFlags::LogResourceUsage))
        return;

    Assert(rtScene);
    Assert(state.Task);

    const size_t idx = static_cast<const size_t>(state.Task->ExecutionOrder());
    if (idx >= _tasks.size() || nullptr == _tasks[idx].Task) {
        Assert(!"task doesn't exist!");
        return;
    }

    _tasks[idx].Resources.emplace_back(FRTSceneUsage{ rtScene, state });
}
//----------------------------------------------------------------------------
// Private helpers
//----------------------------------------------------------------------------
PVulkanFrameTask FVulkanLocalDebugger::Task_(EVulkanExecutionOrder idx) const {
    return (static_cast<size_t>(idx) < _tasks.size()
        ? _tasks[static_cast<size_t>(idx)].Task
        : nullptr );
}
//----------------------------------------------------------------------------
FString FVulkanLocalDebugger::TaskName_(EVulkanExecutionOrder idx) const {
    if (EVulkanExecutionOrder::Initial == idx)
        return "<initial>";
    if (EVulkanExecutionOrder::Final == idx)
        return "<final>";
    if (const PVulkanFrameTask task = Task_(idx))
        return TaskName_(task);

    Assert(!"task doesn't exists");
    return "<unknown>";
}
//----------------------------------------------------------------------------
FString FVulkanLocalDebugger::TaskName_(const PVulkanFrameTask& task) const {
    Assert(task);
    return StringFormat("{0} (#{1})", task->TaskName(), static_cast<u32>(task->ExecutionOrder()));
}
//----------------------------------------------------------------------------
FStringView FVulkanLocalDebugger::QueueName_(const FVulkanDevice& device, u32 queueFamilyIndex) const {
    switch (queueFamilyIndex) {
    case static_cast<u32>(EVulkanQueueFamily::External): return "External";
    case static_cast<u32>(EVulkanQueueFamily::Foreign): return "Foreign";
    case static_cast<u32>(EVulkanQueueFamily::Ignored): return "Ignored";
    default: return device.DeviceQueue(queueFamilyIndex).DebugName;
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

#endif //!USE_PPE_RHIDEBUG
