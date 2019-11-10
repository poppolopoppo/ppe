#pragma once

#include "Core_fwd.h"

#include "Thread/Task_fwd.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API ITaskContext {
public:
    virtual ~ITaskContext() = default;

    static ITaskContext& Get() NOEXCEPT; // get current thread task context

    virtual size_t ThreadTag() const NOEXCEPT = 0;

    virtual void Run(FCompletionPort* phandle, FTaskFunc&& rtask, ETaskPriority priority = ETaskPriority::Normal) = 0;
    virtual void Run(FCompletionPort* phandle, const TMemoryView<FTaskFunc>& rtasks, ETaskPriority priority = ETaskPriority::Normal) = 0;
    virtual void Run(FCompletionPort* phandle, const TMemoryView<const FTaskFunc>& tasks, ETaskPriority priority = ETaskPriority::Normal) = 0;

    virtual void WaitFor(FCompletionPort& handle, ETaskPriority priority = ETaskPriority::Normal) = 0;

    virtual void RunAndWaitFor(const TMemoryView<FTaskFunc>& rtasks, ETaskPriority priority = ETaskPriority::Normal) = 0;
    virtual void RunAndWaitFor(const TMemoryView<const FTaskFunc>& tasks, ETaskPriority priority = ETaskPriority::Normal) = 0;

    virtual void BroadcastAndWaitFor(FTaskFunc&& rtask, ETaskPriority priority = ETaskPriority::Normal) = 0;

    void RunOne(FCompletionPort* phandle, FTaskFunc&& rtask, ETaskPriority priority) {
        Run(phandle, MakeView(&rtask, &rtask +1), priority);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE