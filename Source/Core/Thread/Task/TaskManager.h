#pragma once

#include "Core/Core.h"

#include "Core/Thread/Task/Task.h"

#include "Core/Container/IntrusiveList.h"
#include "Core/Container/Vector.h"
#include "Core/Diagnostic/Exception.h"
#include "Core/Memory/MemoryView.h"
#include "Core/Memory/RefPtr.h"
#include "Core/Memory/UniquePtr.h"

namespace Core {
enum class EThreadPriority;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(TaskCounter);
class FTaskManager;
class FTaskManagerImpl;
//----------------------------------------------------------------------------
class FTaskWaitHandle {
public:
    friend class FTaskManagerImpl;

    FTaskWaitHandle();
    FTaskWaitHandle(ETaskPriority priority, FTaskCounter* counter);
    ~FTaskWaitHandle();

    FTaskWaitHandle(const FTaskWaitHandle& other) = delete;
    FTaskWaitHandle& operator =(const FTaskWaitHandle& other) = delete;

    FTaskWaitHandle(FTaskWaitHandle&& rvalue);
    FTaskWaitHandle& operator =(FTaskWaitHandle&& rvalue);

    ETaskPriority Priority() const { return _priority; }
    const FTaskCounter* Counter() const { return _counter.get(); }

    bool Valid() const { return (nullptr != _counter); }
    bool Finished() const;

private:
    ETaskPriority _priority;
    PTaskCounter _counter;
};
//----------------------------------------------------------------------------
class ITaskContext {
public:
    virtual ~ITaskContext() {}

    virtual void Run(FTaskWaitHandle* phandle, const TMemoryView<const FTaskDelegate>& tasks, ETaskPriority priority = ETaskPriority::Normal) = 0;
    virtual void WaitFor(FTaskWaitHandle& handle, ITaskContext* resume = nullptr) = 0;
    virtual void RunAndWaitFor(const TMemoryView<const FTaskDelegate>& tasks, ETaskPriority priority = ETaskPriority::Normal, ITaskContext* resume = nullptr) = 0;

    void RunOne(FTaskWaitHandle* phandle, const FTaskDelegate& task, ETaskPriority priority) {
        Run(phandle, MakeView(&task, &task+1), priority);
    }
};
//----------------------------------------------------------------------------
class FTaskManager {
public:
    FTaskManager(const FStringView& name, size_t threadTag, size_t workerCount, EThreadPriority priority);
    ~FTaskManager();

    FTaskManager(const FTaskManager& ) = delete;
    FTaskManager& operator =(const FTaskManager& ) = delete;

    const FStringView& Name() const { return _name; }
    size_t ThreadTag() const { return _threadTag; }
    size_t WorkerCount() const { return _workerCount; }
    EThreadPriority Priority() const { return _priority; }

    void Start(const TMemoryView<const size_t>& threadAffinities);
    void Shutdown();

    ITaskContext* Context() const;

    void Run(const TMemoryView<const FTaskDelegate>& tasks, ETaskPriority priority = ETaskPriority::Normal) const;
    void RunAndWaitFor(const TMemoryView<const FTaskDelegate>& tasks, ETaskPriority priority = ETaskPriority::Normal) const;
    void RunAndWaitFor(const TMemoryView<FTask* const>& tasks, ETaskPriority priority = ETaskPriority::Normal) const;

    void Run(const FTaskDelegate& task, ETaskPriority priority = ETaskPriority::Normal) const {
        Run(MakeView(&task, &task+1), priority);
    }

    void RunAndWaitFor(const FTaskDelegate& task, ETaskPriority priority = ETaskPriority::Normal) const {
        RunAndWaitFor(MakeView(&task, &task+1), priority);
    }

    void WaitForAll() const;

    FTaskManagerImpl* Pimpl() const { return _pimpl.get(); }

private:
    TUniquePtr<FTaskManagerImpl> _pimpl;

    const FStringView _name;
    const size_t _threadTag;
    const size_t _workerCount;
    const EThreadPriority _priority;
};
//----------------------------------------------------------------------------
ITaskContext& CurrentTaskContext();
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
