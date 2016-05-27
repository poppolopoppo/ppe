#pragma once

#include "Core/Core.h"

#include "Core/Thread/Task/Task.h"

#include "Core/Container/IntrusiveList.h"
#include "Core/Diagnostic/Exception.h"
#include "Core/Memory/MemoryView.h"
#include "Core/Memory/RefPtr.h"
#include "Core/Memory/UniquePtr.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(TaskCounter);
class TaskManager;
class TaskManagerImpl;
//----------------------------------------------------------------------------
class TaskWaitHandle {
public:
    friend class TaskManagerImpl;

    TaskWaitHandle();
    TaskWaitHandle(TaskPriority priority, TaskCounter* counter);
    ~TaskWaitHandle();

    TaskWaitHandle(const TaskWaitHandle& other) = delete;
    TaskWaitHandle& operator =(const TaskWaitHandle& other) = delete;

    TaskWaitHandle(TaskWaitHandle&& rvalue);
    TaskWaitHandle& operator =(TaskWaitHandle&& rvalue);

    TaskPriority Priority() const { return _priority; }
    const TaskCounter* Counter() const { return _counter.get(); }

    bool Valid() const { return (nullptr != _counter); }

private:
    TaskPriority _priority;
    PTaskCounter _counter;
};
//----------------------------------------------------------------------------
class ITaskContext {
public:
    virtual ~ITaskContext() {}

    virtual void Run(TaskWaitHandle* phandle, const MemoryView<const TaskDelegate>& tasks, TaskPriority priority = TaskPriority::Normal) = 0;
    virtual void WaitFor(TaskWaitHandle& handle) = 0;
    virtual void RunAndWaitFor(const MemoryView<const TaskDelegate>& tasks, TaskPriority priority = TaskPriority::Normal) = 0;

    void RunOne(TaskWaitHandle* phandle, const TaskDelegate& task, TaskPriority priority) {
        Run(phandle, MakeView(&task, &task+1), priority);
    }
};
//----------------------------------------------------------------------------
class TaskManager {
public:
    TaskManager(const char *name, size_t threadTag, size_t workerCount);
    ~TaskManager();

    TaskManager(const TaskManager& ) = delete;
    TaskManager& operator =(const TaskManager& ) = delete;

    const char* Name() const { return _name; }
    size_t ThreadTag() const { return _threadTag; }
    size_t WorkerCount() const { return _workerCount; }

    void Start(const MemoryView<const size_t>& threadAffinities);
    void Shutdown();

    void Run(const MemoryView<const TaskDelegate>& tasks, TaskPriority priority = TaskPriority::Normal) const;
    void RunAndWaitFor(const MemoryView<const TaskDelegate>& tasks, TaskPriority priority = TaskPriority::Normal) const;

    void Run(const TaskDelegate& task, TaskPriority priority = TaskPriority::Normal) const {
        Run(MakeView(&task, &task+1), priority);
    }

    void RunAndWaitFor(const TaskDelegate& task, TaskPriority priority = TaskPriority::Normal) const {
        RunAndWaitFor(MakeView(&task, &task+1), priority);
    }

    TaskManagerImpl* Pimpl() const { return _pimpl.get(); }

private:
    UniquePtr<TaskManagerImpl> _pimpl;

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
