#pragma once

#include "Core.h"

#include <chrono>
#include <condition_variable>
#include <mutex>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ITask;
class TaskContext;
class TaskEvaluator;
class TaskWorker;
//----------------------------------------------------------------------------
class TaskCompletionPort {
public:
    friend class TaskWorker;

    TaskCompletionPort(const char *name, TaskEvaluator *evaluator);
    ~TaskCompletionPort();

    TaskCompletionPort(const TaskCompletionPort& ) = delete;
    TaskCompletionPort& operator =(const TaskCompletionPort& ) = delete;

    const char *Name() const { return _name; }
    TaskEvaluator *Evaluator() const { return _evaluator; }

    void Produce(ITask *task);

    bool WaitOne();
    bool WaitOne(const std::chrono::microseconds& timeout);
    void WaitAll();

private:
    void AsyncTaskCompletion_(const TaskContext& context, const ITask *task);

    const char *_name;
    TaskEvaluator *_evaluator;

    std::mutex _barrier;
    std::condition_variable _consumed;

    size_t _produced;
    size_t _completed;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
