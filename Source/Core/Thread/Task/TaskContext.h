#pragma once

#include "Core/Core.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class TaskCompletionPort;
class TaskEvaluator;
class TaskWorker;
//----------------------------------------------------------------------------
class TaskContext {
public:
    TaskContext(TaskEvaluator *evaluator,
                TaskWorker *worker,
                TaskCompletionPort *completionPort);
    ~TaskContext();

    TaskContext(const TaskContext& ) = delete;
    TaskContext& operator =(const TaskContext& ) = delete;

    TaskEvaluator *Evaluator() const { return _evaluator; }
    TaskWorker *Worker() const { return _worker; }
    TaskCompletionPort *CompletionPort() const { return _completionPort; }

private:
    TaskEvaluator *_evaluator;
    TaskWorker *_worker;
    TaskCompletionPort *_completionPort;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
