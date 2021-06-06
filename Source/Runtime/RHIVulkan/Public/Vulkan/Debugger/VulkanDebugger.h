#pragma once

#include "Vulkan/VulkanCommon.h"

#include "Vulkan/Debugger/VulkanLocalDebugger.h"

#include "Container/Vector.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanDebugger final {
public:
    using FBatchGraph = FVulkanLocalDebugger::FBatchGraph;

    FVulkanDebugger();

    void AddBatchDump(FString&& in);
    bool FrameDump(FString* pout) const;

    void AddBatchGraph(FBatchGraph&& in);
    bool GraphDump(FString* pout) const;

private:
    mutable VECTOR(RHIDebug, FString) _fullDump;
    mutable VECTOR(RHIDebug, FBatchGraph) _graphs;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
