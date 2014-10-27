#include "stdafx.h"

#include "TaskCompletionPort.h"

#include "Task.h"
#include "TaskContext.h"
#include "TaskEvaluator.h"
#include "TaskWorker.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
TaskCompletionPort::TaskCompletionPort(const char *name, TaskEvaluator *evaluator)
:   _name(name)
,   _evaluator(evaluator)
,   _produced(0)
,   _completed(0) {
    Assert(name);
    Assert(evaluator);
}
//----------------------------------------------------------------------------
TaskCompletionPort::~TaskCompletionPort() {
    Assert(_produced == _completed);
}
//----------------------------------------------------------------------------
void TaskCompletionPort::Produce(ITask *task) {
    Assert(task);
    {
        std::unique_lock<std::mutex> scopeLock(_barrier);
        Assert(_produced >= _completed);
        ++_produced;
    }
    _evaluator->AsyncProduce_(task, this);
}
//----------------------------------------------------------------------------
bool TaskCompletionPort::WaitOne() {
    std::unique_lock<std::mutex> scopeLock(_barrier);

    if (_produced == _completed)
        return false;
    Assert(_produced > _completed);

    const size_t completedBefore = _completed;
    _consumed.wait(scopeLock, [this, completedBefore] { return completedBefore < _completed; });

    Assert(completedBefore < _completed);
    return true;
}
//----------------------------------------------------------------------------
bool TaskCompletionPort::WaitOne(const std::chrono::microseconds& timeout) {
    std::unique_lock<std::mutex> scopeLock(_barrier);

    if (_produced == _completed)
        return false;
    Assert(_produced > _completed);

    const size_t completedBefore = _completed;
    _consumed.wait_for(scopeLock, timeout, [this, completedBefore] { return completedBefore < _completed; });

    Assert(completedBefore < _completed);
    return true;
}
//----------------------------------------------------------------------------
void TaskCompletionPort::WaitAll() {
    std::unique_lock<std::mutex> scopeLock(_barrier);

    if (_produced == _completed)
        return;
    Assert(_produced > _completed);

    _consumed.wait(scopeLock, [this] { return _produced == _completed; });

    Assert(_produced == _completed);
}
//----------------------------------------------------------------------------
void TaskCompletionPort::AsyncTaskCompletion_(const TaskContext& context, const ITask *task) {
    Assert(context.CompletionPort() == this);
    Assert(context.Evaluator() == _evaluator);
    Assert(task);

    std::unique_lock<std::mutex> scopeLock(_barrier);

    Assert(_completed < _produced);
    ++_completed;

    _consumed.notify_one();

    RemoveRef(task); // manual ref counting
    // See TaskEvaluator::AsyncProduce_() for symetrical AddRef()
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
