#pragma once

#include "Vulkan/VulkanCommon.h"

#if USE_PPE_RHIDEBUG

#include "Vulkan/Debugger/VulkanLocalDebugger.h"

#include "Container/AssociativeVector.h"
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

    void AddBatchDump(const FStringView& name, FString&& dump);
    bool FrameDump(FStringBuilder* pout);

    void AddBatchGraph(FBatchGraph&& in);
    bool GraphDump(FStringBuilder* pout);

    void LogDump();

private:
    struct FInternalData {
        mutable ASSOCIATIVE_VECTOR(RHIDebug, FString, FString) FullDump;
        mutable VECTOR(RHIDebug, FBatchGraph) Graphs;
    };

    TThreadSafe<FInternalData, EThreadBarrier::DataRaceCheck> _data;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

#endif //!USE_PPE_RHIDEBUG
