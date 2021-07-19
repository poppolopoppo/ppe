#pragma once

#include "Core_fwd.h"

#include "Thread/Task/Task.h"
#include "Thread/Task/TaskContext.h"

#include "IO/StringView.h"
#include "Memory/UniquePtr.h"

namespace PPE {
enum class EThreadPriority;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FTaskManagerImpl;
//----------------------------------------------------------------------------
class PPE_CORE_API FTaskManager {
public:
    FTaskManager(const FStringView& name, size_t threadTag, size_t workerCount, EThreadPriority priority);
    ~FTaskManager();

    FTaskManager(const FTaskManager& ) = delete;
    FTaskManager& operator =(const FTaskManager& ) = delete;

    const FStringView& Name() const { return _name; }
    size_t ThreadTag() const { return _threadTag; }
    size_t WorkerCount() const { return _workerCount; }
    EThreadPriority Priority() const { return _priority; }

    ITaskContext* GlobalContext() const { return _context.get(); }
    FTaskManagerImpl* Pimpl() const { return _pimpl.get(); }

    bool IsRunning() const NOEXCEPT;

    void Start();
    void Start(const TMemoryView<const u64>& threadAffinities);
    void Shutdown();

    void Run(FTaskFunc&& rtask, ETaskPriority priority = ETaskPriority::Normal) const;
    void Run(const TMemoryView<FTaskFunc>& rtasks, ETaskPriority priority = ETaskPriority::Normal) const;
    void Run(const TMemoryView<const FTaskFunc>& tasks, ETaskPriority priority = ETaskPriority::Normal) const;

    void Run(FAggregationPort& ap, FTaskFunc&& rtask, ETaskPriority priority = ETaskPriority::Normal) const;
    void Run(FAggregationPort& ap, const TMemoryView<FTaskFunc>& rtasks, ETaskPriority priority = ETaskPriority::Normal) const;
    void Run(FAggregationPort& ap, const TMemoryView<const FTaskFunc>& tasks, ETaskPriority priority = ETaskPriority::Normal) const;

    void RunAndWaitFor(FTaskFunc&& rtask, ETaskPriority priority = ETaskPriority::Normal) const;
    void RunAndWaitFor(const TMemoryView<FTaskFunc>& rtasks, ETaskPriority priority = ETaskPriority::Normal) const;
    void RunAndWaitFor(const TMemoryView<FTaskFunc>& rtasks, const FTaskFunc& whileWaiting, ETaskPriority priority = ETaskPriority::Normal) const;
    void RunAndWaitFor(const TMemoryView<const FTaskFunc>& tasks, ETaskPriority priority = ETaskPriority::Normal) const;

    void RunInWorker(FTaskFunc&& rtask, ETaskPriority priority = ETaskPriority::Normal) const; // run task directly if already in of worker fibers

    void BroadcastAndWaitFor(FTaskFunc&& rtask, ETaskPriority priority = ETaskPriority::Normal) const;

    void WaitForAll() const;
    bool WaitForAll(int timeoutMS) const; // can timeout, recommended over WaitForAll() to avoid blocking the program

    void DumpStats();

    void DutyCycle(); // release dangling blocks, but keep cache
    void ReleaseMemory(); // release potentially unused memory

private:
    TUniquePtr<ITaskContext> _context;
    TUniquePtr<FTaskManagerImpl> _pimpl;

    const FStringView _name;
    const size_t _threadTag;
    const size_t _workerCount;
    const EThreadPriority _priority;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
