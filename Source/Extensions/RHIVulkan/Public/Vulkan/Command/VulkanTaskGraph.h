#pragma once

#include "VulkanCommandBuffer.h"
#include "Vulkan/VulkanCommon.h"

#include "Vulkan/Command/VulkanDrawTask.h"
#include "Vulkan/Command/VulkanFrameTask.h"
#include "Vulkan/Command/VulkanRaytracingTask.h"

#include "Allocator/SlabAllocator.h"
#include "Container/HashSet.h"
#include "Container/SparseArray.h"
#include "Meta/Optional.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Visitor>
class TVulkanTaskGraph {
public:
    using FSearchableNodes = HASHSET_SLAB(RHICommand, PVulkanFrameTask);
    using FEntries = SPARSEARRAY_SLAB(RHICommand, PVulkanFrameTask);

    TVulkanTaskGraph() = default;

    u32 Count() const { return _nodes->size(); }
    bool Empty() const { return _nodes->empty(); }
    const FEntries& Entries() const { return *_entries; }

    void Construct(const SLAB_ALLOCATOR(RHICommand)& allocator);
    void TearDown();

    template <typename _Task>
    NODISCARD TVulkanFrameTask<_Task>* AddTask(FVulkanCommandBuffer& cmd, const _Task& desc);

private:
    template <typename _Task>
    static void Visitor_(void* visitor, const void* task) {
        auto* const pActual = static_cast<TVulkanFrameTask<_Task> const*>(task);
        if (Ensure(pActual->Valid()))
            static_cast<_Visitor*>(visitor)->Visit(*pActual);
    }

    Meta::TOptional<FSearchableNodes> _nodes;
    Meta::TOptional<FEntries> _entries;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
