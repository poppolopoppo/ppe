#include "stdafx.h"

#include "TaskEvaluator.h"

#include "Task.h"
#include "TaskCompletionPort.h"
#include "TaskWorker.h"

#include "Diagnostic/Logger.h"
#include "IO/Format.h"
#include "Thread/ThreadContext.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static void TaskWorkerLoop_(TaskEvaluator *evaluator, size_t threadIndex) {
    Assert(evaluator);
    Assert(threadIndex > 0); // 0 is reserved for main thread
    Assert(threadIndex <= evaluator->WorkerCount());

    char workerName[256];
    Format(workerName, "{0}_Worker#{1}", evaluator->Name(), threadIndex);

    ThreadContextStartup context(workerName, WORKER_THREADTAG);

    TaskWorker worker(workerName, evaluator, threadIndex);

    do {
        worker.Execute();
    }
    while (evaluator->IsRunning());
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
TaskEvaluator::TaskEvaluator(const char *name, size_t workerCount, size_t capacity)
:   _name(name)
,   _workers(NewArray<std::thread>(workerCount))
,   _queue(capacity)
,   _state(State::Idle) {
    Assert(name);
    Assert(workerCount > 0);
    Assert(capacity > 8);
}
//----------------------------------------------------------------------------
TaskEvaluator::~TaskEvaluator() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(_state == State::Idle);
}
//----------------------------------------------------------------------------
void TaskEvaluator::Start() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(State::Idle == _state);

    _state = State::Running;

    const size_t workerCount = _workers.size();

    LOG(Information, L"[Tasks] Starting evaluator \"{0}\" with {1} workers ...",
        _name, workerCount);

    for (size_t i = 0; i < workerCount; ++i) {
        std::thread& worker = _workers[i];
        Assert(!worker.joinable());

        const size_t threadIndex = (i + 1); // 0 is reserved for evaluator thread
        worker = std::move(std::thread(TaskWorkerLoop_, this, threadIndex));
    }
}
//----------------------------------------------------------------------------
void TaskEvaluator::Shutdown() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(State::Running == _state);

    _state = State::Destroying;

    const size_t workerCount = _workers.size();

    LOG(Information, L"[Tasks] Destroying evaluator \"{0}\" with {1} workers ...",
        _name, workerCount);

    for (size_t i = 0; i < workerCount; ++i) {
        // One fake job per worker to wake up condition variable.
        // The first thread to execute it will read the new evaluator status and stop looping,
        // thus guaranteeing to shutdown every single worker.
        _queue.Produce(Produced {nullptr, nullptr});
    }

    for (size_t i = 0; i < workerCount; ++i) {
        std::thread& worker = _workers[i];

        if (worker.joinable())
            worker.join();
    }

    Assert(State::Destroying == _state);
    _state = State::Idle;
}
//----------------------------------------------------------------------------
void TaskEvaluator::AsyncProduce_(ITask *task, TaskCompletionPort *completionPort) {
    Assert(task);
    Assert(completionPort);
    Assert(completionPort->Evaluator() == this);
    Assert(State::Running == _state);

    AddRef(task); // keep a reference without RefPtr to store a pod
    // See TaskCompletionPort::AsyncTaskCompletion_() for symetrical RemoveRef()

    _queue.Produce(Produced {task, completionPort});
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
