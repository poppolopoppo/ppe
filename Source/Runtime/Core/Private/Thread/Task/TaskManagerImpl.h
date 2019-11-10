#pragma once

#include "Thread/Task/TaskManager.h"

#include "Thread/Task/TaskFiberPool.h"
#include "Thread/Task/TaskScheduler.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FTaskManagerImpl : public ITaskContext {
public:
    explicit FTaskManagerImpl(FTaskManager& manager);
    ~FTaskManagerImpl();

    FTaskManager& Manager() { return _manager; }
    FTaskScheduler& Scheduler() { return _scheduler; }
    TMemoryView<std::thread> Threads() { return MakeView(_threads); }
    FTaskFiberPool& Fibers() { return _fibers; }

    void Start(const TMemoryView<const u64>& threadAffinities);
    void Shutdown();

    void Consume(size_t workerIndex, FTaskScheduler::FTaskQueued* task);

    void DumpStats();

public: // ITaskContext
    virtual size_t ThreadTag() const NOEXCEPT override final;

    virtual void Run(FCompletionPort* phandle, FTaskFunc&& rtask, ETaskPriority priority) override final;
    virtual void Run(FCompletionPort* phandle, const TMemoryView<FTaskFunc>& rtasks, ETaskPriority priority) override final;
    virtual void Run(FCompletionPort* phandle, const TMemoryView<const FTaskFunc>& tasks, ETaskPriority priority) override final;

    virtual void WaitFor(FCompletionPort& handle, ETaskPriority priority) override final;

    virtual void RunAndWaitFor(FTaskFunc&& rtask, ETaskPriority priority) override final;
    virtual void RunAndWaitFor(const TMemoryView<FTaskFunc>& rtasks, ETaskPriority priority) override final;
    virtual void RunAndWaitFor(const TMemoryView<const FTaskFunc>& tasks, ETaskPriority priority) override final;

private:
    using FTaskQueued = FTaskScheduler::FTaskQueued;

    FTaskScheduler _scheduler;
    FTaskFiberPool _fibers;

    FTaskManager& _manager;
    VECTOR(Task, std::thread) _threads;

    static void WorkerLoop_();
    static FCompletionPort* StartPortIFN_(FCompletionPort* port, size_t n) NOEXCEPT;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
