#pragma once

#include "Vulkan/VulkanCommon.h"

#include "Vulkan/Command/VulkanDrawTask.h"
#include "Vulkan/Command/VulkanFrameTask.h"
#include "Vulkan/Command/VulkanRaytracingTask.h"

#include "Allocator/LinearAllocator.h"
#include "Container/HashSet.h"
#include "Container/SparseArray.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Visitor>
class TVulkanTaskGraph {
public:
    using FSearchableNodse = HASHSET_LINEARHEAP(PVulkanFrameTask);
    using FEntries = SPARSEARRAY_LINEARHEAP(PVulkanFrameTask);

    explicit TVulkanTaskGraph(FLinearAllocator& allocator)
    :   _nodes(allocator)
    ,   _entries(allocator)
    {}

    u32 Count() const { return _nodes.size(); }
    bool Empty() const { return _nodes.empty(); }

    const FEntries& Entries() const { return _entries; }

    template <typename _Task>
    TVulkanFrameTask<_Task>* AddTask(FVulkanCommandBuffer& cmd, const _Task& desc);

private:
    template <typename _Task>
    static void Visitor_(void* visitor, const void* task) {
        static_cast<_Visitor*>(visitor)->Visit(*static_cast<TVulkanFrameTask<_Task> const*>(task));
    }

    FSearchableNodse _nodes;
    FEntries _entries;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
