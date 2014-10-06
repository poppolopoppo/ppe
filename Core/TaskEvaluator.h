#pragma once

#include "Core.h"

#include "ConcurrentQueue.h"
#include "RefPtr.h"
#include "ThreadResource.h"
#include "UniqueView.h"

#include <atomic>
#include <thread>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ITask;
FWD_REFPTR(TaskCompletionPort);
//----------------------------------------------------------------------------
class TaskEvaluator : Meta::ThreadResource {
public:
    friend class TaskCompletionPort;
    friend class TaskWorker;

    enum class State {
        Idle,
        Running,
        Destroying,
    };

    struct Produced {
        ITask *Task;
        TaskCompletionPort *CompletionPort;
    };

    TaskEvaluator(const char *name, size_t workerCount, size_t capacity);
    ~TaskEvaluator();

    TaskEvaluator(const TaskEvaluator& ) = delete;
    TaskEvaluator& operator =(const TaskEvaluator& ) = delete;

    const char* Name() const { return _name; }
    size_t WorkerCount() const { return _workers.size(); }

    bool IsRunning() const { return (_state == State::Running); }
    bool IsDestroying() const { return (_state == State::Destroying); }

    void Start();
    void Shutdown();

private:
    void AsyncProduce_(ITask *task, TaskCompletionPort *completionPort);

    const char *_name;

    UniqueArray<std::thread> _workers;
    CONCURRENT_QUEUE(Task, Produced) _queue;

    std::atomic<State> _state;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
