#pragma once

#include "VulkanCommandBuffer.h"
#include "Vulkan/VulkanCommon.h"

#include "Vulkan/Command/VulkanDrawTask.h"
#include "Vulkan/Command/VulkanFrameTask.h"
#include "Vulkan/Command/VulkanRaytracingTask.h"

#include "Allocator/SlabAllocator.h"
#include "Container/HashSet.h"
#include "Container/SparseArray.h"
#include "Memory/InSituPtr.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Visitor>
class TVulkanTaskGraph {
public:
    using FSearchableNodes = HASHSET_SLAB(PVulkanFrameTask);
    using FEntries = SPARSEARRAY_SLAB(PVulkanFrameTask);

    TVulkanTaskGraph() = default;

    u32 Count() const { return _nodes->size(); }
    bool Empty() const { return _nodes->empty(); }
    const FEntries& Entries() const { return *_entries; }

    void Construct(const FSlabAllocator& allocator);
    void TearDown();

    template <typename _Task>
    NODISCARD TVulkanFrameTask<_Task>* AddTask(FVulkanCommandBuffer& cmd, const _Task& desc);

private:
    template <typename _Task>
    static void Visitor_(void* visitor, const void* task) {
        static_cast<_Visitor*>(visitor)->Visit(*static_cast<TVulkanFrameTask<_Task> const*>(task));
    }

    TInSituPtr<FSearchableNodes> _nodes;
    TInSituPtr<FEntries> _entries;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
