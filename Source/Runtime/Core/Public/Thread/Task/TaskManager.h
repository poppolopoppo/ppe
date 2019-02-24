#pragma once

#include "Core.h"

#include "Thread/Task/Task.h"

#include "IO/StringView.h"
#include "Memory/MemoryView.h"
#include "Memory/RefPtr.h"
#include "Memory/UniquePtr.h"

namespace PPE {
enum class EThreadPriority;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(TaskCounter);
class FTaskManager;
class FTaskManagerImpl;
//----------------------------------------------------------------------------
PPE_CORE_API ITaskContext& CurrentTaskContext();
//----------------------------------------------------------------------------
class PPE_CORE_API FTaskWaitHandle {
public:
    friend class FTaskManagerImpl;

    FTaskWaitHandle();
    explicit FTaskWaitHandle(PTaskCounter&& counter);
    ~FTaskWaitHandle();

    FTaskWaitHandle(const FTaskWaitHandle& other) = delete;
    FTaskWaitHandle& operator =(const FTaskWaitHandle& other) = delete;

    FTaskWaitHandle(FTaskWaitHandle&& rvalue);
    FTaskWaitHandle& operator =(FTaskWaitHandle&& rvalue);

    const FTaskCounter* Counter() const { return _counter.get(); }

    bool Valid() const { return _counter.valid(); }
    bool Finished() const;

private:
    PTaskCounter _counter;
};
//----------------------------------------------------------------------------
class ITaskContext {
public:
    virtual ~ITaskContext() {}

    virtual void Run(FTaskWaitHandle* phandle, FTaskFunc&& rtask, ETaskPriority priority = ETaskPriority::Normal) = 0;
    virtual void Run(FTaskWaitHandle* phandle, const TMemoryView<FTaskFunc>& rtasks, ETaskPriority priority = ETaskPriority::Normal) = 0;
    virtual void Run(FTaskWaitHandle* phandle, const TMemoryView<const FTaskFunc>& tasks, ETaskPriority priority = ETaskPriority::Normal) = 0;

    virtual void WaitFor(FTaskWaitHandle& handle, ITaskContext* resume = nullptr, ETaskPriority priority = ETaskPriority::Normal) = 0;

    virtual void RunAndWaitFor(const TMemoryView<FTaskFunc>& rtasks, ETaskPriority priority = ETaskPriority::Normal, ITaskContext* resume = nullptr) = 0;
    virtual void RunAndWaitFor(const TMemoryView<const FTaskFunc>& tasks, ETaskPriority priority = ETaskPriority::Normal, ITaskContext* resume = nullptr) = 0;

    virtual void BroadcastAndWaitFor(FTaskFunc&& rtask, ETaskPriority priority = ETaskPriority::Normal) = 0;

    void RunOne(FTaskWaitHandle* phandle, FTaskFunc&& rtask, ETaskPriority priority) {
        Run(phandle, MakeView(&rtask, &rtask +1), priority);
    }
};
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

    void Start(const TMemoryView<const u64>& threadAffinities);
    void Shutdown();

    ITaskContext* Context() const;

    void Run(const TMemoryView<FTaskFunc>& rtasks, ETaskPriority priority = ETaskPriority::Normal) const;
    void Run(const TMemoryView<const FTaskFunc>& tasks, ETaskPriority priority = ETaskPriority::Normal) const;

    void RunAndWaitFor(const TMemoryView<FTaskFunc>& rtasks, ETaskPriority priority = ETaskPriority::Normal) const;
    void RunAndWaitFor(const TMemoryView<FTaskFunc>& rtasks, const FTaskFunc& whileWaiting, ETaskPriority priority = ETaskPriority::Normal) const;
    void RunAndWaitFor(const TMemoryView<const FTaskFunc>& tasks, ETaskPriority priority = ETaskPriority::Normal) const;

    void Run(FTaskFunc&& rtask, ETaskPriority priority = ETaskPriority::Normal) const;

    void RunAndWaitFor(FTaskFunc&& rtask, ETaskPriority priority = ETaskPriority::Normal) const {
        RunAndWaitFor(MakeView(&rtask, &rtask+1), priority);
    }

    void BroadcastAndWaitFor(FTaskFunc&& rtask, ETaskPriority priority = ETaskPriority::Normal) const;

    void WaitForAll() const;
    bool WaitForAll(int timeoutMS) const; // can timeout, recommended over WaitForAll() to avoid blocking the program

    void ReleaseMemory(); // release potentially unused memory

    FTaskManagerImpl* Pimpl() const { return _pimpl.get(); }

private:
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
