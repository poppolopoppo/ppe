#pragma once

#include "Core/Memory/AlignedStorage.h"
#include "Core/Meta/Delegate.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class TaskPool;
//----------------------------------------------------------------------------
typedef void (*TaskFuncWithPool_t)(const TaskPool& pool);
typedef Delegate<TaskFuncWithPool_t> TaskWithPool;
//----------------------------------------------------------------------------
typedef void (*TaskFuncWithoutPool_t)();
typedef Delegate<TaskFuncWithoutPool_t> TaskWithoutPool;
//----------------------------------------------------------------------------
STATIC_ASSERT(sizeof(TaskWithPool) == sizeof(TaskWithoutPool));
//----------------------------------------------------------------------------
struct Task {
    POD_STORAGE(TaskWithPool) Data;

    Task() {}
    Task(const TaskWithPool& task) { operator =(task); }
    Task(const TaskWithoutPool& task) { operator =(task); }

    Task& operator =(const TaskWithPool& task) {
        TaskWithPool *const _task = reinterpret_cast<TaskWithPool *>(&Data);
        *_task = task;
        _task->SetFlag0(false);
        return *this;
    }

    Task& operator =(const TaskWithoutPool& task) {
        TaskWithoutPool *const _task = reinterpret_cast<TaskWithoutPool *>(&Data);
        *_task = task;
        _task->SetFlag0(true);
        return *this;
    }

    bool Valid() const { 
        return reinterpret_cast<const TaskWithPool *>(&Data)->Valid();
    }

    void Invoke(const TaskPool& pool) const {
        const TaskWithPool *const _taskWithPool = reinterpret_cast<const TaskWithPool *>(&Data);
        if (_taskWithPool->Flag0())
            reinterpret_cast<const TaskWithoutPool *>(&Data)->Invoke();
        else
            _taskWithPool->Invoke(pool);
    }

    FORCE_INLINE void operator ()(const TaskPool& pool) {
        Invoke(pool);
    }
};
//----------------------------------------------------------------------------
enum class TaskPriority {
    High = 0,
    Normal,
    Low,

    _Count
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
