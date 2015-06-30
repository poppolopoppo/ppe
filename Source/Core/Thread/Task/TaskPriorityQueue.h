#pragma once

#include "Core/Core.h"

#include "Core/Thread/MPMCBoundedQueue.h"
#include "Core/Thread/Task/Task.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class TaskCounter;
//----------------------------------------------------------------------------
struct TaskQueued {
    Task Task;
    TaskCounter *Counter;
};
//----------------------------------------------------------------------------
using TaskQueue = MPMCBoundedQueue<TaskQueued>;
//----------------------------------------------------------------------------
class TaskPriorityQueue {
public:
    explicit TaskPriorityQueue(size_t capacity);
    ~TaskPriorityQueue();

    size_t capacity() const { return _byPriority[0]->capacity(); }

    bool empty() const;

    bool Enqueue(const TaskQueued& task, TaskPriority priority);
    bool Dequeue(TaskQueued *ptask);

private:
    std::unique_ptr<TaskQueue> _byPriority[size_t(TaskPriority::_Count)];
};
//----------------------------------------------------------------------------
TaskPriorityQueue::TaskPriorityQueue(size_t capacity) {
    for (size_t i = 0; i < size_t(TaskPriority::_Count); ++i)
        _byPriority[i].reset(new TaskQueue(capacity));
}
//----------------------------------------------------------------------------
TaskPriorityQueue::~TaskPriorityQueue() {
#ifdef WITH_CORE_ASSERT
    for (size_t i = 0; i < size_t(TaskPriority::_Count); ++i)
        Assert(_byPriority[i]->empty());
#endif
}
//----------------------------------------------------------------------------
bool TaskPriorityQueue::empty() const {
    for (size_t i = 0; i < size_t(TaskPriority::_Count); ++i)
        if (!_byPriority[i]->empty())
            return false;
    return true;
}
//----------------------------------------------------------------------------
bool TaskPriorityQueue::Enqueue(const TaskQueued& task, TaskPriority priority) {
    Assert(size_t(priority) < size_t(TaskPriority::_Count) );
    return _byPriority[size_t(priority)]->Enqueue(task);
}
//----------------------------------------------------------------------------
bool TaskPriorityQueue::Dequeue(TaskQueued *ptask) {
    for (size_t i = 0; i < size_t(TaskPriority::_Count); ++i) // highest to lowest priority
        if (_byPriority[i]->Dequeue(ptask))
            return true;
    return false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
