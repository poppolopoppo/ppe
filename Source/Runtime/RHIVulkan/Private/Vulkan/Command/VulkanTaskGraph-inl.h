
#pragma once

#include "Vulkan/Command/VulkanTaskGraph.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Visitor>
void TVulkanTaskGraph<_Visitor>::Construct(const FSlabAllocator& allocator) {
    _nodes.Create_AssumeNotValid<FSearchableNodes>(allocator);
    _entries.Create_AssumeNotValid<FEntries>(allocator);
    _entries->Reserve(64);
}
//----------------------------------------------------------------------------
template <typename _Visitor>
void TVulkanTaskGraph<_Visitor>::TearDown() {
    _nodes.Destroy();
    _entries.Destroy();
}
//----------------------------------------------------------------------------
template <typename _Visitor>
template <typename _Task>
TVulkanFrameTask<_Task>* TVulkanTaskGraph<_Visitor>::AddTask(FVulkanCommandBuffer& cmd, const _Task& desc) {
    using FVulkanTask = TVulkanFrameTask<_Task>;

    const IVulkanFrameTask::FProcessFunc visit = &Visitor_<_Task>;
    FVulkanTask* const pTask = INPLACE_NEW(cmd.Allocator().AllocateT<FVulkanTask>(), FVulkanTask)(cmd, desc, visit);
    Assert_NoAssume(pTask->Valid());

    _nodes->insert(pTask);

    if  (pTask->Inputs().empty())
        _entries->Emplace(pTask);

    for (auto pInputNode : pTask->Inputs()) {
        Assert(!!_nodes->Contains(pInputNode));
        pInputNode->Attach(pTask);
    }

    return pTask;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE