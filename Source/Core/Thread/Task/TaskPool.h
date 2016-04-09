#pragma once

#include "Core/Core.h"

#include "Core/Allocator/PoolAllocatorTag.h"
#include "Core/Diagnostic/Exception.h"
#include "Core/Memory/MemoryView.h"
#include "Core/Thread/Task/Task.h"

#include <memory>

namespace Core {
POOL_TAG_DECL(TaskPool);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class TaskCounter;
class TaskPool;
class TaskPoolImpl;
//----------------------------------------------------------------------------
class TaskException : public Exception {
public:
    TaskException(const char* what, const TaskPool* pool) : Exception(what), _pool(pool) {}

    const TaskPool* Pool() const { return _pool; }

private:
    const TaskPool* _pool;
};
//----------------------------------------------------------------------------
class TaskPool {
public:
    TaskPool(const char *name, size_t workerCount);
    ~TaskPool();

    TaskPoolImpl *Pimpl() const { return _pimpl.get(); }

    const char *Name() const { return _name; }
    size_t WorkerCount() const { return _workerCount; }

    void Run(const Task *ptasks, size_t count, TaskCounter **pcounter = nullptr, TaskPriority priority = TaskPriority::Normal) const;
    void WaitFor(TaskCounter **pcounter) const;
    void RunAndWaitFor(const Task *ptasks, size_t count, TaskPriority priority = TaskPriority::Normal) const;

    void Start();
    void Shutdown();

    struct StartupScope {
        TaskPool *PPool;
        StartupScope(TaskPool& pool) : PPool(&pool) { pool.Start(); }
        ~StartupScope() { PPool->Shutdown(); }
    };

public: // helpers :
    void Run(const MemoryView<Task>& tasks, TaskCounter **pcounter = nullptr, TaskPriority priority = TaskPriority::Normal) const {
        Run(tasks.Pointer(), tasks.size(), pcounter, priority);
    }

    void Run(const MemoryView<const Task>& tasks, TaskCounter **pcounter = nullptr, TaskPriority priority = TaskPriority::Normal) const {
        Run(tasks.Pointer(), tasks.size(), pcounter, priority);
    }

    void RunAndWaitFor(const MemoryView<Task>& tasks, TaskPriority priority = TaskPriority::Normal) const {
        RunAndWaitFor(tasks.Pointer(), tasks.size(), priority);
    }

    void RunAndWaitFor(const MemoryView<const Task>& tasks, TaskPriority priority = TaskPriority::Normal) const {
        RunAndWaitFor(tasks.Pointer(), tasks.size(), priority);
    }

    template <size_t _Count>
    void RunAndWaitFor(const Task (&tasks)[_Count], TaskPriority priority = TaskPriority::Normal) const {
        RunAndWaitFor(tasks, _Count, priority);
    }

private:
    std::unique_ptr<TaskPoolImpl> _pimpl;

    const char *_name;
    const size_t _workerCount;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
