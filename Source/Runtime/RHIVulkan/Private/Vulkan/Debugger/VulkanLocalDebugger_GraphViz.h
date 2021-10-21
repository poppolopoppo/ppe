#pragma once

#include "Vulkan/Debugger/VulkanLocalDebugger.h"

#if !USE_PPE_RHIDEBUG
#   error "should not compile this file"
#endif

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
struct FVulkanLocalDebugger::FGraphVizDump_ {
    FBatchGraph& Out;
    const FVulkanLocalDebugger& Debugger;

    FGraphVizDump_(
        FBatchGraph* pout,
        const FVulkanLocalDebugger& debugger ) NOEXCEPT
    :   Out(*pout)
    ,   Debugger(Debugger) {
        Assert(pout);
    }

    void DumpGraph() const;

    void AddInitialStates(FString& str, FString* firstNode) const;
    void AddFinalStates(FString& str, FString* *deps, FString* lastNode) const;

    void ResourceUsage(const FTaskInfo& info, FString& deps, FString* resourceStyle, FString* barStyle) const;

    void Barrier(const FVulkanBuffer* buffer, PVulkanFrameTask task, FString& deps, FString& barStyle) const;
    void Barrier(const FVulkanImage* image, PVulkanFrameTask task, FString& deps, FString& barStyle) const;


};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
