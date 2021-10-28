#pragma once

#include "RHIVulkan_fwd.h"

#if USE_PPE_RHIDEBUG

#include "Vulkan/Command/VulkanFrameTask.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FVulkanDummyTask_ final : public IVulkanFrameTask {
public:
#if USE_PPE_RHIDEBUG
    virtual FVulkanFrameTaskRef DebugRef() const NOEXCEPT override { return Default; }
#endif
};
//----------------------------------------------------------------------------
inline auto GenerateDummyTasks_(size_t n) {
    VECTOR(RHIDebug, TUniquePtr<FVulkanDummyTask_>) result;
    result.reserve(n);

    forrange(i, 0, n) {
        TUniquePtr<FVulkanDummyTask_> task;
        task.reset<FVulkanDummyTask_>()->SetExecutionOrder(static_cast<EVulkanExecutionOrder>(
            static_cast<u32>(EVulkanExecutionOrder::First) + i));

        result.push_back(std::move(task));
    }

    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

#endif //!USE_PPE_RHIDEBUG
