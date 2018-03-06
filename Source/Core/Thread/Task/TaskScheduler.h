#pragma once

#include "Core/Core.h"

#include "Core/Memory/RefPtr.h"
#include "Core/Misc/TargetPlatform.h"
#include "Core/Thread/Task/Task.h"

#define USE_CORE_THREAD_WORKSTEALINGQUEUE 1 // %_NOCOMMIT%

#if USE_CORE_THREAD_WORKSTEALINGQUEUE

//  Based on : Priority Work-Stealing Scheduler
//  A decentralized work-stealing scheduler that dynamically schedules fixed-priority tasks in a non-preemptive manner.
//  https://pdfs.semanticscholar.org/9d1b/eb4d2ca5c07965bb4e309864a9dcbae65fec.pdf
//  https://github.com/shamsimam/priorityworkstealing

//  But diverged importantly to keep blocking threads where there is no task in flight

#   include "Core/Container/Vector.h"

#   include <atomic>
#   include <condition_variable>
#   include <mutex>

#else

//  Really simple global priority queue using a binary heap
//  Should be ok for a small count of threads, but won't scale due to contention

#   include "Core/Thread/ConcurrentQueue.h"

#endif

namespace Core {
FWD_REFPTR(TaskCounter);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FTaskScheduler {
public:
    struct FTaskQueued {
#if USE_CORE_THREAD_WORKSTEALINGQUEUE
        size_t Priority;
#endif
        FTaskFunc Pending;
        STaskCounter Counter;
    };

    FTaskScheduler(size_t numWorkers, size_t maxTasks);
    ~FTaskScheduler();

    bool HasPendingTask() const;

    void Produce(ETaskPriority priority, FTaskFunc&& rtask, FTaskCounter* pcounter);
    void Produce(ETaskPriority priority, const TMemoryView<FTaskFunc>& rtasks, FTaskCounter* pcounter);
    void Consume(size_t workerIndex, FTaskQueued* pop);

    void SignalExitToWorkers(const FTaskFunc& exitTask);

private:
    const size_t _numWorkers;

#if USE_CORE_THREAD_WORKSTEALINGQUEUE
    struct FPrioritySort_ {
        bool operator ()(const FTaskQueued& lhs, const FTaskQueued& rhs) const {
            return (lhs.Priority > rhs.Priority); // smaller means higher priority
        }
    };

    typedef std::priority_queue<
        FTaskQueued,
        VECTOR(Task, FTaskQueued),
        FPrioritySort_
    >   priority_queue_type;

    struct CACHELINE_ALIGNED FLocalQueue_ {
        size_t HighestPriority = INDEX_NONE;

        std::mutex Barrier;

        std::condition_variable Empty;
        std::condition_variable Overflow;

        priority_queue_type Queue;
    };

    const size_t _maxTasks;
    std::atomic<size_t> _taskInFlight;
    std::atomic<size_t> _taskRevision;
    std::atomic<size_t> _globalPriority;
    VECTOR(Task, FLocalQueue_) _workerQueues;

    bool WorkerTryPush_(size_t workerIndex, FTaskQueued& task) {
        FLocalQueue_& worker = _workerQueues[workerIndex];
        Meta::FUniqueLock scopeLock(worker.Barrier, std::defer_lock);

        if (scopeLock.try_lock() == false)
            return false;

        if (worker.Queue.size() >= _maxTasks)
            return false;

        worker.Queue.emplace(std::move(task));
        worker.HighestPriority = worker.Queue.top().Priority;

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

        worker.Empty.wait(scopeLock, [&worker] { return (not worker.Queue.empty()); });

        *ptask = std::move(worker.Queue.top());

        worker.Queue.pop();
        worker.HighestPriority = (worker.Queue.empty()
            ? INDEX_NONE
            : worker.Queue.top().Priority );

        scopeLock.unlock();    // unlock before notification to minimize mutex contention
        worker.Overflow.notify_all(); // always notifies all producer threads

        return true;
    }

    bool WorkerStealIFP_(size_t workerIndex, FTaskQueued* ptask) {
        FLocalQueue_& worker = _workerQueues[workerIndex];

        // check if there's a higher priority job in another worker
        size_t globalHighest = _globalPriority;
        if (worker.HighestPriority > globalHighest) {
            // look for a queue with a higher priority job to steal
            forrange(i, workerIndex + 1, workerIndex + _numWorkers) {
                const size_t otherIndex = (i % _numWorkers);
                FLocalQueue_& other = _workerQueues[otherIndex];

                if (other.HighestPriority < worker.HighestPriority &&
                    WorkerTryPop_(otherIndex, ptask) )
                    return true; // stole a job, return
            }

            // found nothing : tell other workers
            _globalPriority.compare_exchange_weak(globalHighest, INDEX_NONE);
        }

        return false;
    }

#else
    CONCURRENT_PRIORITY_QUEUE(Task, FTaskQueued) _tasks;

#endif
};
//----------------------------------------------------------------------------
#if USE_CORE_THREAD_WORKSTEALINGQUEUE
//----------------------------------------------------------------------------
// Using local pools with work stealing
//  + Minimum locking with very few contention
//  + Scales correctly with many cores (> 8)
//  - Approximation of correct execution order for priorities
//  - Can't guarantee insertion order
//----------------------------------------------------------------------------
FTaskScheduler::FTaskScheduler(size_t numWorkers, size_t maxTasks)
    : _numWorkers(numWorkers)
    , _maxTasks(maxTasks)
    , _taskRevision(0)
    , _taskInFlight(0)
    , _globalPriority(INDEX_NONE) {
    Assert(_numWorkers);
    Assert(_maxTasks > _numWorkers);

    _workerQueues.resize_AssumeEmpty(_numWorkers);
}
//----------------------------------------------------------------------------
FTaskScheduler::~FTaskScheduler() {
    Assert(0 == _taskInFlight);
    Assert(0 == _taskRevision);
}
//----------------------------------------------------------------------------
bool FTaskScheduler::HasPendingTask() const {
    return (_taskInFlight != 0);
}
//----------------------------------------------------------------------------
void FTaskScheduler::Produce(ETaskPriority priority, const TMemoryView<FTaskFunc>& rtasks, FTaskCounter* pcounter) {
    for (FTaskFunc& rtask : rtasks)
        Produce(priority, std::move(rtask), pcounter);
}
//----------------------------------------------------------------------------
void FTaskScheduler::Produce(ETaskPriority priority, FTaskFunc&& rtask, FTaskCounter* pcounter) {
    Assert(_taskRevision < 0xFFFF); // just to get sure that we won't be overflowing Priority

    // construct insertion priority with _taskRevision
    const size_t insertion_order_preserving_priority = size_t((u64(priority) << 16) | _taskRevision++);
    Assert((insertion_order_preserving_priority >> 16) == size_t(priority));

    // used as hint for work stealing : worker will try to steal a job if a more priority task is available
    size_t globalHighest = _globalPriority;
    if (globalHighest > insertion_order_preserving_priority)
        _globalPriority.compare_exchange_weak(globalHighest, insertion_order_preserving_priority);

    // track in flight tasks for HasPendingTask() and to reset _taskRevision when the queues are finally empty
    ++_taskInFlight;

    // push the job to some worker's local queue
    FTaskQueued queued{ insertion_order_preserving_priority, std::move(rtask), pcounter };
    for (size_t n = insertion_order_preserving_priority;;) {
        const size_t workerIndex = (n++ % _numWorkers); // use first 16 bits with previous _taskPushed value
        if (WorkerTryPush_(workerIndex, queued))
            break;
    }
}
//----------------------------------------------------------------------------
void FTaskScheduler::Consume(size_t workerIndex, FTaskQueued* pop) {
    if (not WorkerStealIFP_(workerIndex, pop))
        WorkerPop_(workerIndex, pop);

    // at this stage we already consumed a task for execution

    // reset _taskRevision when everything is processed to avoid overflows :
    // having overflow would the insertion order to be inverted and would violate our assumptions
    if (0 == --_taskInFlight)
        _taskRevision = 0;
}
//----------------------------------------------------------------------------
void FTaskScheduler::SignalExitToWorkers(const FTaskFunc& exitTask) {
    // track in flight tasks for HasPendingTask() and to reset _taskRevision when the queues are finally empty
    _taskInFlight += _numWorkers;

    forrange(i, 0, _numWorkers) {
        // make sure that each worker is getting his exit task
        FTaskQueued queuedExit{ INDEX_NONE/* lowest priority */, exitTask, nullptr };
        while (not WorkerTryPush_(i, queuedExit))
            FPlatformAtomics::ShortSyncWait();
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
void FTaskScheduler::Produce(ETaskPriority priority, const TMemoryView<FTaskFunc>& rtasks, FTaskCounter* pcounter) {
    _tasks.Produce(u32(priority), rtasks.size(), _numWorkers, [rtasks, pcounter](size_t i) {
        return FTaskQueued{ std::move(rtasks[i]), pcounter };
    });
}
//----------------------------------------------------------------------------
void FTaskScheduler::Consume(size_t , FTaskQueued* pop) {
    _tasks.Consume(pop);
}
//----------------------------------------------------------------------------
void FTaskScheduler::SignalExitToWorkers(const FTaskFunc& exitTask) {
    forrange(i, 0, _numWorkers)
        Produce(ETaskPriority::Internal, FTaskFunc(exitTask), nullptr);
}
//----------------------------------------------------------------------------
#endif //!USE_CORE_THREAD_WORKSTEALINGQUEUE
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
