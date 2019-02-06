#pragma once

#include "Core.h"

#include "HAL/PlatformProcess.h"
#include "Memory/RefPtr.h"
#include "Thread/Task/Task.h"

// #TODO : need to measure real world performance for this
#define USE_PPE_THREAD_WORKSTEALINGQUEUE 1 // %_NOCOMMIT%

#if USE_PPE_THREAD_WORKSTEALINGQUEUE

//  Based on : Priority Work-Stealing Scheduler
//  A decentralized work-stealing scheduler that dynamically schedules fixed-priority tasks in a non-preemptive manner.
//  https://pdfs.semanticscholar.org/9d1b/eb4d2ca5c07965bb4e309864a9dcbae65fec.pdf
//  https://github.com/shamsimam/priorityworkstealing

//  But diverged importantly to keep blocking threads when there is no task in flight

#   include "Container/BitMask.h"
#   include "Container/Vector.h"

#   include <atomic>
#   include <condition_variable>
#   include <mutex>
#   include <queue> // std::priority_queue<>

#else

//  Really simple global priority queue using a binary heap
//  Should be ok for a small count of threads, but won't scale due to contention

#   include "Thread/ConcurrentQueue.h"

#endif

PRAGMA_MSVC_WARNING_PUSH()
PRAGMA_MSVC_WARNING_DISABLE(4324) // 'XXX': structure was padded due to alignment specifier

namespace PPE {
FWD_REFPTR(TaskCounter);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class CACHELINE_ALIGNED FTaskScheduler {
public:
    struct FTaskQueued {
#if USE_PPE_THREAD_WORKSTEALINGQUEUE
        size_t Priority;
#endif
        FTaskFunc Pending;
        STaskCounter Counter;
    };

    FTaskScheduler(size_t numWorkers, size_t maxTasks);
    ~FTaskScheduler();

    bool HasPendingTask() const;

    void Produce(ETaskPriority priority, FTaskFunc&& rtask, FTaskCounter* pcounter);
    void Produce(ETaskPriority priority, const FTaskFunc& task, FTaskCounter* pcounter);

    void Produce(ETaskPriority priority, const TMemoryView<FTaskFunc>& rtasks, FTaskCounter* pcounter);
    void Produce(ETaskPriority priority, const TMemoryView<const FTaskFunc>& tasks, FTaskCounter* pcounter);

    void Consume(size_t workerIndex, FTaskQueued* pop);

    void BroadcastToEveryWorker(size_t priority, const FTaskFunc& task);

private:
    const size_t _numWorkers;
    const size_t _maxTasks;

#if USE_PPE_THREAD_WORKSTEALINGQUEUE
    struct FPrioritySort_ {
        bool operator ()(const FTaskQueued& lhs, const FTaskQueued& rhs) const {
            return (lhs.Priority > rhs.Priority); // smaller means higher priority
        }
    };

    struct CACHELINE_ALIGNED FLocalQueue_ {
        size_t HighestPriority = INDEX_NONE;

        std::mutex Barrier;

        std::condition_variable Empty;
        std::condition_variable Overflow;

        std::priority_queue<
            FTaskQueued,
            VECTORINSITU(Task, FTaskQueued, 8),
            FPrioritySort_
        >   Queue;
    };

    struct CACHELINE_ALIGNED state_t {
        std::atomic<size_t> TaskInFlight{ 0 };
        std::atomic<size_t> TaskRevision{ 0 };
        std::atomic<size_t> GlobalPriority{ INDEX_NONE };
    };

    state_t _state;

    VECTOR(Task, FLocalQueue_) _workerQueues;

    bool WorkerTryPush_(size_t workerIndex, FTaskQueued& task) {
        FLocalQueue_& worker = _workerQueues[workerIndex];
        Meta::FUniqueLock scopeLock(worker.Barrier, std::defer_lock);

        if (scopeLock.try_lock() && worker.Queue.size() < _maxTasks) {
            PPE_LEAKDETECTOR_WHITELIST_SCOPE();
            worker.Queue.emplace(std::move(task));
            worker.HighestPriority = worker.Queue.top().Priority;
        }
        else {
            return false;
        }

        scopeLock.unlock(); // unlock before notification to minimize mutex contention
        worker.Empty.notify_one(); // notify one consumer thread

        return true;
    }

    void WorkerPop_(size_t workerIndex, FTaskQueued* ptask) {
        FLocalQueue_& worker = _workerQueues[workerIndex];
        Meta::FUniqueLock scopeLock(worker.Barrier);
        worker.Empty.wait(scopeLock, [&worker] { return (not worker.Queue.empty()); });

        *ptask = std::move(worker.Queue.top());

        worker.Queue.pop();
        worker.HighestPriority = (worker.Queue.empty()
            ? INDEX_NONE
            : worker.Queue.top().Priority );

        scopeLock.unlock();    // unlock before notification to minimize mutex contention
        worker.Overflow.notify_all(); // always notifies all producer threads
    }

    bool WorkerTryPop_(size_t workerIndex, FTaskQueued* ptask) {
        FLocalQueue_& worker = _workerQueues[workerIndex];
        Meta::FUniqueLock scopeLock(worker.Barrier, std::defer_lock);
        if (scopeLock.try_lock() == false)
            return false;

        if (worker.Queue.empty())
            return false;

        worker.Empty.wait(scopeLock, [&worker] {
            return (not worker.Queue.empty());
        });

        *ptask = std::move(worker.Queue.top());

        worker.Queue.pop();
        worker.HighestPriority = (worker.Queue.empty()
            ? INDEX_NONE
            : worker.Queue.top().Priority );

        scopeLock.unlock();    // unlock before notification to minimize mutex contention
        worker.Overflow.notify_all(); // always notifies all producer threads

        return true;
    }

    NO_INLINE bool WorkerSteal_(size_t workerIndex, FTaskQueued* ptask) {
        const size_t workerHighest = _workerQueues[workerIndex].HighestPriority;

        size_t localHighest = workerHighest; // important to let other workers steal our jobs too
        size_t globalHighest = _state.GlobalPriority;

        // look for a queue with a higher priority job to steal
        forrange(i, workerIndex + 1, workerIndex + _numWorkers) {
            const size_t otherIndex = (i % _numWorkers);
            FLocalQueue_& other = _workerQueues[otherIndex];

            if (other.HighestPriority < workerHighest &&
                WorkerTryPop_(otherIndex, ptask)) {
                // note that we leave _state.GlobalPriority untouched since we didn't fail
                return true; // stole a job, return
            }

            localHighest = Min(localHighest, other.HighestPriority);
        }

        // found nothing, update global priority with local minimum
        // will naturally converge to INDEX_NONE when all worker queues are empty
        // this is important to not reset the global priority to avoid letting the last worker finish its queue alone
        _state.GlobalPriority.compare_exchange_weak(globalHighest, localHighest);

        return false;
    }

    bool WorkerStealIFP_(size_t workerIndex, FTaskQueued* ptask) {
        FLocalQueue_& worker = _workerQueues[workerIndex];

        // check if there's a higher priority job in another worker
        if (worker.HighestPriority > _state.GlobalPriority)
            return WorkerSteal_(workerIndex, ptask); // cold path

        return false;
    }

#else
    CONCURRENT_PRIORITY_QUEUE(Task, FTaskQueued) _tasks;

#endif
};
//----------------------------------------------------------------------------
#if USE_PPE_THREAD_WORKSTEALINGQUEUE
//----------------------------------------------------------------------------
// Using local pools with work stealing
//  + Minimum locking with very few contention
//  + Scales correctly with many cores (> 8)
//  - Approximation of correct execution order for priorities
//  - Can't guarantee insertion order
//----------------------------------------------------------------------------
FTaskScheduler::FTaskScheduler(size_t numWorkers, size_t maxTasks)
    : _numWorkers(numWorkers)
    , _maxTasks(maxTasks) {
    Assert(_numWorkers);
    Assert(_maxTasks > _numWorkers);

    _workerQueues.resize_AssumeEmpty(_numWorkers);
}
//----------------------------------------------------------------------------
FTaskScheduler::~FTaskScheduler() {
    Assert(0 == _state.TaskInFlight);
    Assert(0 == _state.TaskRevision);
}
//----------------------------------------------------------------------------
bool FTaskScheduler::HasPendingTask() const {
    return (_state.TaskInFlight != 0);
}
//----------------------------------------------------------------------------
void FTaskScheduler::Produce(ETaskPriority priority, const TMemoryView<FTaskFunc>& rtasks, FTaskCounter* pcounter) {
    for (FTaskFunc& rtask : rtasks)
        Produce(priority, std::move(rtask), pcounter);
}
//----------------------------------------------------------------------------
void FTaskScheduler::Produce(ETaskPriority priority, const TMemoryView<const FTaskFunc>& tasks, FTaskCounter* pcounter) {
    for (const FTaskFunc& task : tasks)
        Produce(priority, task, pcounter);
}
//----------------------------------------------------------------------------
void FTaskScheduler::Produce(ETaskPriority priority, const FTaskFunc& task, FTaskCounter* pcounter) {
    Assert(_state.TaskRevision < 0xFFFF); // just to get sure that we won't be overflowing Priority

    // track in flight tasks for HasPendingTask() and to reset _state.TaskRevision when the queues are finally empty
    ++_state.TaskInFlight;

    // construct insertion priority with _state.TaskRevision
    const size_t insertion_order_preserving_priority = size_t((u64(priority) << 16) | _state.TaskRevision++);
    Assert((insertion_order_preserving_priority >> 16) == size_t(priority));

    FTaskQueued queued{ insertion_order_preserving_priority, task, pcounter };

    // used as hint for work stealing : worker will try to steal a job if a more priority task is available
    size_t globalHighest = _state.GlobalPriority;
    if (globalHighest > insertion_order_preserving_priority)
        _state.GlobalPriority.compare_exchange_weak(globalHighest, insertion_order_preserving_priority);

    // push the job to some worker's local queue
    for (size_t n = insertion_order_preserving_priority; ; ++n) {
        const size_t workerIndex = (n % _numWorkers); // use first 16 bits with previous _taskPushed value
        if (WorkerTryPush_(workerIndex, queued))
            break;
    }
}
//----------------------------------------------------------------------------
void FTaskScheduler::Produce(ETaskPriority priority, FTaskFunc&& rtask, FTaskCounter* pcounter) {
    Assert(_state.TaskRevision < 0xFFFF); // just to get sure that we won't be overflowing Priority

    // track in flight tasks for HasPendingTask() and to reset _state.TaskRevision when the queues are finally empty
    ++_state.TaskInFlight;

    // construct insertion priority with _state.TaskRevision
    const size_t insertion_order_preserving_priority = size_t((u64(priority) << 16) | _state.TaskRevision++);
    Assert((insertion_order_preserving_priority >> 16) == size_t(priority));

    FTaskQueued queued{ insertion_order_preserving_priority, std::move(rtask), pcounter };

    // used as hint for work stealing : worker will try to steal a job if a more priority task is available
    size_t globalHighest = _state.GlobalPriority;
    if (globalHighest > insertion_order_preserving_priority)
        _state.GlobalPriority.compare_exchange_weak(globalHighest, insertion_order_preserving_priority);

    // push the job to some worker's local queue
    for (size_t n = insertion_order_preserving_priority; ; ++n) {
        const size_t workerIndex = (n % _numWorkers); // use first 16 bits with previous _taskPushed value
        if (WorkerTryPush_(workerIndex, queued))
            break;
    }
}
//----------------------------------------------------------------------------
void FTaskScheduler::Consume(size_t workerIndex, FTaskQueued* pop) {
    if (not WorkerStealIFP_(workerIndex, pop))
        WorkerPop_(workerIndex, pop);

    // at this stage we already consumed a task for execution

    // reset _state.TaskRevision when everything is processed to avoid overflows :
    // having overflow would cause the insertion order to be inverted and would violate our assumptions
    if (0 == --_state.TaskInFlight)
        _state.TaskRevision = 0;
}
//----------------------------------------------------------------------------
void FTaskScheduler::BroadcastToEveryWorker(size_t priority, const FTaskFunc& task) {
    // track in flight tasks for HasPendingTask() and to reset _state.TaskRevision when the queues are finally empty
    _state.TaskInFlight += _numWorkers;

    // loop until all workers were reached
    size_t backoff = 0;
    auto workers = TBitMask<size_t>::SetFirstN(_numWorkers);

    for (;;) {
        // make sure that each worker is getting his exit task
        forrange(i, 0, _numWorkers) {
            if (workers.Get(i)) {
                FTaskQueued queuedExit{ priority, task, nullptr };
                if (WorkerTryPush_(i, queuedExit))
                    workers.SetFalse(i);
            }
        }

        // sleep before looping again if not completed
        if (workers)
            FPlatformProcess::SleepForSpinning(backoff);
        else
            break;
    }
}
//----------------------------------------------------------------------------
#else
//----------------------------------------------------------------------------
// Using a global priority queue implemented with a heap
//  + Handles overflow and throttling
//  + Guarantees correct execution order for priorities and insertion order
//  - Global resource with lot of contention
//
//  TODO: issue when all workers push many jobs at the same time ?
//  might deadlock if the queue is full and every thread is throttled
//  low probability but still
//----------------------------------------------------------------------------
FTaskScheduler::FTaskScheduler(size_t numWorkers, size_t maxTasks)
    : _numWorkers(numWorkers)
    , _tasks(maxTasks) {
    Assert(_numWorkers);
    Assert(maxTasks > _numWorkers);
}
//----------------------------------------------------------------------------
FTaskScheduler::~FTaskScheduler() {
    Assert(_tasks.empty());
}
//----------------------------------------------------------------------------
bool FTaskScheduler::HasPendingTask() const {
    return (not _tasks.empty());
}
//----------------------------------------------------------------------------
void FTaskScheduler::Produce(ETaskPriority priority, FTaskFunc&& rtask, FTaskCounter* pcounter) {
    _tasks.Produce(u32(priority), FTaskQueued{ std::move(rtask), pcounter });
}
//----------------------------------------------------------------------------
void FTaskScheduler::Produce(ETaskPriority priority, const FTaskFunc& task, FTaskCounter* pcounter) {
    _tasks.Produce(u32(priority), FTaskQueued{ task, pcounter });
}
//----------------------------------------------------------------------------
void FTaskScheduler::Produce(ETaskPriority priority, const TMemoryView<FTaskFunc>& rtasks, FTaskCounter* pcounter) {
    _tasks.Produce(u32(priority), rtasks.size(), _numWorkers, [rtasks, pcounter](size_t i) {
        return FTaskQueued{ std::move(rtasks[i]), pcounter };
    });
}
//----------------------------------------------------------------------------
void FTaskScheduler::Produce(ETaskPriority priority, const TMemoryView<const FTaskFunc>& tasks, FTaskCounter* pcounter) {
    _tasks.Produce(u32(priority), tasks.size(), _numWorkers, [tasks, pcounter](size_t i) {
        return FTaskQueued{ tasks[i], pcounter };
    });
}
//----------------------------------------------------------------------------
void FTaskScheduler::Consume(size_t , FTaskQueued* pop) {
    _tasks.Consume(pop);
}
//----------------------------------------------------------------------------
void FTaskScheduler::BroadcastToEveryWorker(size_t priority, const FTaskFunc& task) {
    Assert(task);

    forrange(i, 0, _numWorkers)
        Produce(ETaskPriority(priority), FTaskFunc(task), nullptr);
}
//----------------------------------------------------------------------------
#endif //!USE_PPE_THREAD_WORKSTEALINGQUEUE
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

PRAGMA_MSVC_WARNING_POP()
