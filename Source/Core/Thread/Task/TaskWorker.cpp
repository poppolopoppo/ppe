#include "stdafx.h"

#include "TaskWorker.h"

#include "Task.h"
#include "TaskCompletionPort.h"
#include "TaskContext.h"
#include "TaskEvaluator.h"

#include "Diagnostic/Logger.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
TaskWorker::TaskWorker(const char *name, TaskEvaluator *evaluator, size_t threadIndex)
:   _name(name)
,   _evaluator(evaluator)
,   _threadIndex(threadIndex) {
    Assert(name);
    Assert(evaluator);
    Assert(threadIndex > 0);

    LOG(Information, L"[Tasks] Starting worker \"{0}\" with index #{1} from evaluator \"{2}\" ...",
        _name, _threadIndex, _evaluator->Name() );
}
//----------------------------------------------------------------------------
TaskWorker::~TaskWorker() {
    LOG(Information, L"[Tasks] Destroying worker \"{0}\" with index #{1} from evaluator \"{2}\" ...",
        _name, _threadIndex, _evaluator->Name() );
}
//----------------------------------------------------------------------------
void TaskWorker::Execute() {
    TaskEvaluator::Produced produced;
    _evaluator->_queue.Consume(&produced);

    if (produced.Task) {
        Assert(produced.CompletionPort);

        const TaskContext context(_evaluator, this, produced.CompletionPort);
        produced.Task->Invoke(context);

        produced.CompletionPort->AsyncTaskCompletion_(context, produced.Task);
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
