﻿#pragma once

#include "Core_fwd.h"

#include "Thread/Task/Task.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API ITaskContext {
protected:
    ITaskContext() = default;

    ITaskContext(const ITaskContext&) = default;
    ITaskContext& operator =(const ITaskContext&) = default;

    ITaskContext(ITaskContext&&) = default;
    ITaskContext& operator =(ITaskContext&&) = default;

public:
    virtual ~ITaskContext() = default;

    static ITaskContext& Get() NOEXCEPT; // get current thread task context

    virtual size_t ThreadTag() const NOEXCEPT = 0;
    virtual size_t WorkerCount() const NOEXCEPT = 0;

    virtual void Run(FAggregationPort& ag, FTaskFunc&& rtask, ETaskPriority priority = ETaskPriority::Normal) = 0;
    virtual void Run(FAggregationPort& ag, const TMemoryView<FTaskFunc>& rtasks, ETaskPriority priority = ETaskPriority::Normal) = 0;
    virtual void Run(FAggregationPort& ag, const TMemoryView<const FTaskFunc>& tasks, ETaskPriority priority = ETaskPriority::Normal) = 0;

    virtual void Run(FCompletionPort* cp, FTaskFunc&& rtask, ETaskPriority priority = ETaskPriority::Normal) = 0;
    virtual void Run(FCompletionPort* cp, const TMemoryView<FTaskFunc>& rtasks, ETaskPriority priority = ETaskPriority::Normal) = 0;
    virtual void Run(FCompletionPort* cp, const TMemoryView<const FTaskFunc>& tasks, ETaskPriority priority = ETaskPriority::Normal) = 0;

    virtual void WaitFor(FCompletionPort& cp, ETaskPriority priority = ETaskPriority::Normal) = 0;

    virtual void RunAndWaitFor(FTaskFunc&& rtask, ETaskPriority priority = ETaskPriority::Normal) = 0;
    virtual void RunAndWaitFor(const TMemoryView<FTaskFunc>& rtasks, ETaskPriority priority = ETaskPriority::Normal) = 0;
    virtual void RunAndWaitFor(const TMemoryView<const FTaskFunc>& tasks, ETaskPriority priority = ETaskPriority::Normal) = 0;
    virtual void RunAndWaitFor(const TMemoryView<FTaskFunc>& rtasks, FTaskFunc&& whileWaiting, ETaskPriority priority = ETaskPriority::Normal) = 0;

public: // helpers
    void FireAndForget(FTaskFunc&& rtask, ETaskPriority priority = ETaskPriority::Normal) { Run(nullptr, std::move(rtask), priority); }
    void FireAndForget(const TMemoryView<FTaskFunc>& rtasks, ETaskPriority priority = ETaskPriority::Normal) { Run(nullptr, rtasks, priority); }
    void FireAndForget(const TMemoryView<const FTaskFunc>& tasks, ETaskPriority priority = ETaskPriority::Normal) { Run(nullptr, tasks, priority); }

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
