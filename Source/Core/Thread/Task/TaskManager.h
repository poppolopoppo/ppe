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

private:
    ETaskPriority _priority;
    PTaskCounter _counter;
};
//----------------------------------------------------------------------------
class ITaskContext {
public:
    virtual ~ITaskContext() {}

    virtual void Run(FTaskWaitHandle* phandle, const TMemoryView<const TaskDelegate>& tasks, ETaskPriority priority = ETaskPriority::Normal) = 0;
    virtual void WaitFor(FTaskWaitHandle& handle) = 0;
    virtual void RunAndWaitFor(const TMemoryView<const TaskDelegate>& tasks, ETaskPriority priority = ETaskPriority::Normal) = 0;

    void RunOne(FTaskWaitHandle* phandle, const TaskDelegate& task, ETaskPriority priority) {
        Run(phandle, MakeView(&task, &task+1), priority);
    }
};
//----------------------------------------------------------------------------
class FTaskManager {
public:
    FTaskManager(const char *name, size_t threadTag, size_t workerCount);
    ~FTaskManager();

    FTaskManager(const FTaskManager& ) = delete;
    FTaskManager& operator =(const FTaskManager& ) = delete;

    const char* Name() const { return _name; }
    size_t ThreadTag() const { return _threadTag; }
    size_t WorkerCount() const { return _workerCount; }

    void Start(const TMemoryView<const size_t>& threadAffinities);
    void Shutdown();

    void Run(const TMemoryView<const TaskDelegate>& tasks, ETaskPriority priority = ETaskPriority::Normal) const;
    void RunAndWaitFor(const TMemoryView<const TaskDelegate>& tasks, ETaskPriority priority = ETaskPriority::Normal) const;
    void RunAndWaitFor(const TMemoryView<FTask* const>& tasks, ETaskPriority priority = ETaskPriority::Normal) const;

    void Run(const TaskDelegate& task, ETaskPriority priority = ETaskPriority::Normal) const {
        Run(MakeView(&task, &task+1), priority);
    }

    void RunAndWaitFor(const TaskDelegate& task, ETaskPriority priority = ETaskPriority::Normal) const {
        RunAndWaitFor(MakeView(&task, &task+1), priority);
    }

    FTaskManagerImpl* Pimpl() const { return _pimpl.get(); }

private:
    TUniquePtr<FTaskManagerImpl> _pimpl;

    const char* _name;
    const size_t _threadTag;
    const size_t _workerCount;
};
//----------------------------------------------------------------------------
ITaskContext& CurrentTaskContext();
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
