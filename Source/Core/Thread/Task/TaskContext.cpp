#include "stdafx.h"

#include "TaskContext.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
TaskContext::TaskContext(TaskEvaluator *evaluator, TaskWorker *worker, TaskCompletionPort *completionPort)
:   _evaluator(evaluator)
,   _worker(worker)
,   _completionPort(completionPort) {
    Assert(evaluator);
    Assert(worker);
    Assert(completionPort);
}
//----------------------------------------------------------------------------
TaskContext::~TaskContext() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
