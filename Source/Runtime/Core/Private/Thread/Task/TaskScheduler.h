#pragma once

#include "Core_fwd.h"

#include "Thread/Task/Task.h"
#include "Thread/Task/CompletionPort.h"

#include "HAL/PlatformProcess.h"
#include "Memory/RefPtr.h"
#include "Thread/Task/Task.h"

//  Based on : Priority Work-Stealing Scheduler
//  A decentralized work-stealing scheduler that dynamically schedules fixed-priority tasks in a non-preemptive manner.
//  https://pdfs.semanticscholar.org/9d1b/eb4d2ca5c07965bb4e309864a9dcbae65fec.pdf
//  https://github.com/shamsimam/priorityworkstealing

//  But diverged importantly to keep blocking threads when there is no task in flight

// #TODO : need to measure real world performance for this
#define USE_PPE_THREAD_WORKSTEALINGQUEUE_V1 (0) // %_NOCOMMIT%
#define USE_PPE_THREAD_WORKSTEALINGQUEUE_V2 (0) // %_NOCOMMIT%
#define USE_PPE_THREAD_WORKSTEALINGQUEUE_V3 (1) // %_NOCOMMIT%

#define USE_PPE_THREAD_WORKSTEALINGQUEUE (USE_PPE_THREAD_WORKSTEALINGQUEUE_V1||USE_PPE_THREAD_WORKSTEALINGQUEUE_V2||USE_PPE_THREAD_WORKSTEALINGQUEUE_V3)

#if USE_PPE_THREAD_WORKSTEALINGQUEUE

#   include "Container/Vector.h"

#   include <atomic>
#   include <condition_variable>
#   include <mutex>

#else

//  Really simple global priority queue using a binary heap
//  Should be ok for a small count of threads, but won't scale due to contention

#   include "Thread/ConcurrentQueue.h"

#endif

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PRAGMA_MSVC_WARNING_PUSH()
PRAGMA_MSVC_WARNING_DISABLE(4324) // 'XXX' structure was padded due to alignment
class FTaskScheduler {
public:
    struct FTaskQueued {
#if USE_PPE_THREAD_WORKSTEALINGQUEUE
        size_t Priority;
#endif
        FTaskFunc Pending;
        FCompletionPort* Port;

        FTaskQueued() = default;

#if USE_PPE_THREAD_WORKSTEALINGQUEUE
        FTaskQueued(size_t priority, FTaskFunc&& pending, FCompletionPort* port) NOEXCEPT
        :   Priority(priority)
        ,   Pending(std::move(pending))
        ,   Port(port)
        {}
#else
        FTaskQueued(FTaskFunc&& pending, FCompletionPort* counter) NOEXCEPT
        :   Pending(std::move(pending))
        ,   Port(counter)
        {}
#endif

        FTaskQueued(const FTaskQueued&) = delete;
        FTaskQueued& operator =(const FTaskQueued&) = delete;

        FTaskQueued(FTaskQueued&& rvalue) = default;
        FTaskQueued& operator =(FTaskQueued&& rvalue) = default;
    };

    explicit FTaskScheduler(size_t numWorkers);
    ~FTaskScheduler();

    bool HasPendingTask() const;

    void Produce(ETaskPriority priority, FTaskFunc&& rtask, FCompletionPort* pport);
    void Produce(ETaskPriority priority, const FTaskFunc& task, FCompletionPort* pport);

    void Produce(ETaskPriority priority, const TMemoryView<FTaskFunc>& rtasks, FCompletionPort* pport);
    void Produce(ETaskPriority priority, const TMemoryView<const FTaskFunc>& tasks, FCompletionPort* pport);

    void Consume(size_t workerIndex, FTaskQueued* pop);

    void ReleaseMemory();

private:
#if USE_PPE_THREAD_WORKSTEALINGQUEUE
    struct FPrioritySort_ {
        bool operator ()(const FTaskQueued& lhs, const FTaskQueued& rhs) const {
            return (lhs.Priority > rhs.Priority); // smaller means higher priority
        }
    };

    static CONSTEXPR size_t PackPriorityWRevision_(ETaskPriority priority, size_t revision) {
        const size_t packed = (size_t(priority) << 28) | revision;
        Assert_NoAssume((packed >> 28) == size_t(priority));
        return packed;
    }

    static CONSTEXPR size_t UnpackPriorityFromRevision_(size_t packed) {
        return (packed >> 28); // return priority bits
    }
#endif

#if USE_PPE_THREAD_WORKSTEALINGQUEUE_V3
    STATIC_ASSERT((int)ETaskPriority::High == 0);
    STATIC_ASSERT((int)ETaskPriority::Internal == 3);
    STATIC_ASSERT(ETaskPriority::Internal > ETaskPriority::Low);
    STATIC_ASSERT(ETaskPriority::Low > ETaskPriority::Normal);
    STATIC_ASSERT(ETaskPriority::Normal > ETaskPriority::High);
    STATIC_CONST_INTEGRAL(size_t, NumPriorities, 4);

    struct priority_group_t {
        std::atomic<size_t> NumTasks{ 0 };
        std::atomic<size_t> Revision{ 0 };
    };

    struct CACHELINE_ALIGNED FWorkerQueue_ {
        size_t HighestPriority = INDEX_NONE;
        std::mutex Barrier;
        VECTORMINSIZE(Task, FTaskQueued, 32) PriorityHeap;
    };

    bool HasHigherPriorityTask_(const size_t highestPriority) const NOEXCEPT {
        STATIC_ASSERT(4 == NumPriorities);
        return ((0 < highestPriority && !!_priorityGroups[0].NumTasks) |
                (1 < highestPriority && !!_priorityGroups[1].NumTasks) |
                (2 < highestPriority && !!_priorityGroups[2].NumTasks) |
                (3 < highestPriority && !!_priorityGroups[3].NumTasks) );
    }

    std::condition_variable _onTask;
    priority_group_t _priorityGroups[NumPriorities];

    VECTOR(Task, FWorkerQueue_) _queues;

#elif USE_PPE_THREAD_WORKSTEALINGQUEUE_V2
    struct CACHELINE_ALIGNED FWorkerQueue_ {
        std::mutex Barrier;
        size_t HighestPriority = INDEX_NONE;
        VECTORMINSIZE(Task, FTaskQueued, 32) PriorityHeap;
    };

    static CONSTEXPR bool HasHigherPriorityWRevision_(size_t lhs, size_t rhs) {
        return (UnpackPriorityFromRevision_(lhs) < UnpackPriorityFromRevision_(rhs)); // compare only priority part
    }

    void PokeGlobalPriority_(size_t localPriority) {
        for (size_t g = _globalPriority;;) {
            if (g <= localPriority || _globalPriority.compare_exchange_weak(g, localPriority))
                break;
        }
    }

    std::condition_variable _onTask;
    std::atomic<size_t> _numPendingTask;
    std::atomic<size_t> _globalPriority;
    std::atomic<size_t> _globalRevision;

    VECTOR(Task, FWorkerQueue_) _queues;

#elif USE_PPE_THREAD_WORKSTEALINGQUEUE_V1
    struct CACHELINE_ALIGNED FLocalQueue_ {
        size_t HighestPriority = INDEX_NONE;

        std::mutex Barrier;
        std::condition_variable Empty;

        STATIC_CONST_INTEGRAL(size_t, NumInSitu, 8);
        VECTORINSITU(Task, FTaskQueued, NumInSitu) PriorityHeap;

        void Queue(Meta::FUniqueLock& scope, FTaskQueued&& rvalue) {
            Assert_NoAssume(scope.owns_lock());

            {
                PPE_LEAKDETECTOR_WHITELIST_SCOPE();
                PriorityHeap.push_back(std::move(rvalue));
            }

            std::push_heap(PriorityHeap.begin(), PriorityHeap.end(), FPrioritySort_{});
            HighestPriority = PriorityHeap.front().Priority;

            scope.unlock(); // unlock before notification to minimize mutex contention
            Empty.notify_one(); // notify one consumer thread
        }

        void Dequeue(Meta::FUniqueLock& scope, FTaskQueued* task) NOEXCEPT {
            Assert_NoAssume(scope.owns_lock());
            Assert(task);

            Empty.wait(scope, [this] { return (not PriorityHeap.empty()); });

            std::pop_heap(PriorityHeap.begin(), PriorityHeap.end(), FPrioritySort_{});
            *task = std::move(PriorityHeap.back());
            PriorityHeap.pop_back();

            HighestPriority = (PriorityHeap.empty()
                ? INDEX_NONE
                : PriorityHeap.front().Priority );

            scope.unlock(); // unlock before notification to minimize mutex contention
        }

        void ReleaseMemory(Meta::FUniqueLock& scope) {
            Assert_NoAssume(scope.owns_lock());

            PriorityHeap.shrink_to_fit();
            scope.unlock();
        }
    };

    struct CACHELINE_ALIGNED state_t {
        std::atomic<size_t> TaskInFlight{ 0 };
        std::atomic<size_t> TaskRevision{ 0 };
        std::atomic<size_t> GlobalPriority{ INDEX_NONE };
    };

    const size_t _numWorkers;
    state_t _state;
    VECTOR(Task, FLocalQueue_) _workerQueues;

    bool WorkerTryPush_(size_t workerIndex, FTaskQueued& task) {
        FLocalQueue_& worker = _workerQueues[workerIndex];

        Meta::FUniqueLock scopeLock(worker.Barrier, std::defer_lock);
        if (scopeLock.try_lock()) {
            worker.Queue(scopeLock, std::move(task));
            return true;
        }

        return false;
    }

    void WorkerPop_(size_t workerIndex, FTaskQueued* ptask) {
        FLocalQueue_& worker = _workerQueues[workerIndex];

        Meta::FUniqueLock scopeLock(worker.Barrier);
        worker.Dequeue(scopeLock, ptask);
    }

    bool WorkerTryPop_(size_t workerIndex, FTaskQueued* ptask) {
        FLocalQueue_& worker = _workerQueues[workerIndex];

        Meta::FUniqueLock scopeLock(worker.Barrier, std::defer_lock);
        if (scopeLock.try_lock() && not worker.PriorityHeap.empty()) {
            worker.Dequeue(scopeLock, ptask);
            return true;
        }

        return false;
    }

    NO_INLINE bool WorkerSteal_(size_t workerIndex, FTaskQueued* ptask) {
        const FLocalQueue_& worker = _workerQueues[workerIndex];

        size_t localHighest = worker.HighestPriority; // important to let other workers steal our jobs too
        size_t globalHighest = _state.GlobalPriority;

        // look for a queue with a higher priority job to steal
        forrange(i, workerIndex + 1, workerIndex + _numWorkers) {
            const size_t otherIndex = (i % _numWorkers);
            FLocalQueue_& other = _workerQueues[otherIndex];

            if (other.HighestPriority < worker.HighestPriority &&
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
    const size_t _numWorkers;
    CONCURRENT_PRIORITY_QUEUE(Task, FTaskQueued) _tasks;

#endif
};
PRAGMA_MSVC_WARNING_POP()
//----------------------------------------------------------------------------
#if USE_PPE_THREAD_WORKSTEALINGQUEUE_V3
//----------------------------------------------------------------------------
// Using local pools with work stealing
//  + Better occupancy than V1 (threads less idle)
//  + Minimum locking with very few contention
//  + Scales correctly with many cores (> 8)
//  + Better handling of global priorities with one counter per priority value
//  - Can't guarantee insertion order
//----------------------------------------------------------------------------
inline FTaskScheduler::FTaskScheduler(size_t numWorkers) {
    Assert(numWorkers);

    _queues.resize_AssumeEmpty(numWorkers);
}
//----------------------------------------------------------------------------
inline FTaskScheduler::~FTaskScheduler() {
    Assert_NoAssume(not HasPendingTask());
}
//----------------------------------------------------------------------------
inline bool FTaskScheduler::HasPendingTask() const {
    return ((!!_priorityGroups[size_t(ETaskPriority::High)].NumTasks |
             !!_priorityGroups[size_t(ETaskPriority::Normal)].NumTasks) ||
            (!!_priorityGroups[size_t(ETaskPriority::Low)].NumTasks |
             !!_priorityGroups[size_t(ETaskPriority::Internal)].NumTasks) );
}
//----------------------------------------------------------------------------
inline void FTaskScheduler::Produce(ETaskPriority priority, FTaskFunc&& rtask, FCompletionPort* pport) {
    Assert_NoAssume(rtask);

    priority_group_t& p = _priorityGroups[size_t(priority)];

    // construct insertion priority with _state.TaskRevision
    const size_t insertion_order_preserving_priority{ PackPriorityWRevision_(priority, ++p.Revision) };
    FTaskQueued queued{ insertion_order_preserving_priority, std::move(rtask), pport };
    const size_t numWorkers = _queues.size();

    size_t workerIndex = (insertion_order_preserving_priority % numWorkers);
    for (size_t backoff = 0;;) {
        // push the job to some worker's local queue
        forrange(n, 0, numWorkers) {
            FWorkerQueue_& w = _queues[workerIndex]; // use revision packed in first 24 bits

            Meta::FUniqueLock scopeLock(w.Barrier, std::defer_lock);
            if (Likely(scopeLock.try_lock())) {
                // push the new task to the priority heap
                w.PriorityHeap.push_back(std::move(queued));
                std::push_heap(w.PriorityHeap.begin(), w.PriorityHeap.end(), FPrioritySort_{});

                // keep track of worker priority
                w.HighestPriority = UnpackPriorityFromRevision_(w.PriorityHeap.front().Priority);

                // used as hints for work stealing : worker will try to steal a job if a more priority task is available
                ++p.NumTasks;

                scopeLock.unlock();
                _onTask.notify_one();

                return;
            }

            if (++workerIndex == numWorkers)
                workerIndex = 0;
        }

        // temper insertions in case of high contention
        FPlatformProcess::SleepForSpinning(backoff);
    }
}
//----------------------------------------------------------------------------
inline void FTaskScheduler::Consume(size_t workerIndex, FTaskQueued* pop) {
    Assert(pop);

    const size_t numWorkers = _queues.size();

    for (size_t backoff = 0;;) {
        forrange(n, 0, numWorkers) {
            FWorkerQueue_& w = _queues[workerIndex];

            Meta::FUniqueLock scopeLock(w.Barrier, std::defer_lock);
            if (not HasHigherPriorityTask_(w.HighestPriority) && scopeLock.try_lock()) {
                _onTask.wait(scopeLock, [this]() NOEXCEPT{
                    return HasPendingTask();
                });

                if (not (w.PriorityHeap.empty() | HasHigherPriorityTask_(w.HighestPriority))) {
                    std::pop_heap(w.PriorityHeap.begin(), w.PriorityHeap.end(), FPrioritySort_{});
                    *pop = std::move(w.PriorityHeap.back());
                    w.PriorityHeap.pop_back();

                    w.HighestPriority = (w.PriorityHeap.empty()
                        ? INDEX_NONE
                        : UnpackPriorityFromRevision_(w.PriorityHeap.front().Priority));

                    priority_group_t& p = _priorityGroups[UnpackPriorityFromRevision_(pop->Priority)];
                    if (0 == --p.NumTasks)
                        p.Revision = 0;

                    return;
                }
            }

            if (++workerIndex == numWorkers)
                workerIndex = 0;
        }

        FPlatformProcess::SleepForSpinning(backoff);
    }
}
//----------------------------------------------------------------------------
inline void FTaskScheduler::ReleaseMemory() {
    for (FWorkerQueue_& w : _queues) {
        Meta::FUniqueLock scopeLock(w.Barrier, std::defer_lock);
        if (scopeLock.try_lock())
            w.PriorityHeap.shrink_to_fit();
    }
}
//----------------------------------------------------------------------------
#elif USE_PPE_THREAD_WORKSTEALINGQUEUE_V2
//----------------------------------------------------------------------------
// Using local pools with work stealing
//  + Better occupancy than V1 (threads less idle)
//  + Minimum locking with very few contention
//  + Scales correctly with many cores (> 8)
//  - Approximation of correct execution order for priorities
//  - Can't guarantee insertion order
//----------------------------------------------------------------------------
inline FTaskScheduler::FTaskScheduler(size_t numWorkers)
:   _numPendingTask(0)
,   _globalPriority(INDEX_NONE)
,   _globalRevision(0) {
    Assert(numWorkers);

    _queues.resize_AssumeEmpty(numWorkers);
}
//----------------------------------------------------------------------------
inline FTaskScheduler::~FTaskScheduler() {
    Assert_NoAssume(0 == _numPendingTask);
    Assert_NoAssume(INDEX_NONE == _globalPriority);
    Assert_NoAssume(0 == _globalRevision);
}
//----------------------------------------------------------------------------
inline bool FTaskScheduler::HasPendingTask() const {
    return ((_numPendingTask > 0) | (_globalPriority != INDEX_NONE));
}
//----------------------------------------------------------------------------
inline void FTaskScheduler::Produce(ETaskPriority priority, FTaskFunc&& rtask, FCompletionPort* pport) {
    Assert_NoAssume(rtask);
    Assert(_globalRevision < 0xFFFFFF); // just to get sure that we won't be overflowing Priority

    // construct insertion priority with _state.TaskRevision
    const size_t insertion_order_preserving_priority{ PackPriorityWRevision_(priority, ++_globalRevision) };
    FTaskQueued queued{ insertion_order_preserving_priority, std::move(rtask), pport };
    const size_t numWorkers = _queues.size();

    size_t workerIndex = (insertion_order_preserving_priority % numWorkers);
    for (size_t backoff = 0;;) {
        // push the job to some worker's local queue
        forrange(n, 0, numWorkers) {
            FWorkerQueue_& w = _queues[workerIndex]; // use revision packed in first 24 bits

            Meta::FUniqueLock scopeLock(w.Barrier, std::defer_lock);
            if (Likely(scopeLock.try_lock())) {
                // push the new task to the priority heap
                w.PriorityHeap.push_back(std::move(queued));
                std::push_heap(w.PriorityHeap.begin(), w.PriorityHeap.end(), FPrioritySort_{});

                // keep track of worker priority
                const size_t localPriority = w.HighestPriority;
                w.HighestPriority = w.PriorityHeap.front().Priority;

                // used as hints for work stealing : worker will try to steal a job if a more priority task is available
                ++_numPendingTask;
                PokeGlobalPriority_(w.HighestPriority);

                scopeLock.unlock();
                _onTask.notify_one();

                return;
            }

            if (++workerIndex == numWorkers)
                workerIndex = 0;
        }

        // temper insertions in case of high contention
        FPlatformProcess::SleepForSpinning(backoff);
    }
}
//----------------------------------------------------------------------------
inline void FTaskScheduler::Consume(size_t workerIndex, FTaskQueued* pop) {
    Assert(pop);

    const size_t numWorkers = _queues.size();

    for (size_t backoff = 0;;) {
        size_t globalPriority = _globalPriority;
        size_t localPriority = INDEX_NONE;

        forrange(n, 0, numWorkers) {
            FWorkerQueue_& w = _queues[workerIndex];

            Meta::FUniqueLock scopeLock(w.Barrier, std::defer_lock);
            if (not HasHigherPriorityWRevision_(_globalPriority, w.HighestPriority) && scopeLock.try_lock()) {
                _onTask.wait(scopeLock, [this, &w]() NOEXCEPT {
                    return ((not w.PriorityHeap.empty()) | (_numPendingTask > 0));
                });

                if (not (w.PriorityHeap.empty() | HasHigherPriorityWRevision_(_globalPriority, w.HighestPriority))) {
                    std::pop_heap(w.PriorityHeap.begin(), w.PriorityHeap.end(), FPrioritySort_{});
                    *pop = std::move(w.PriorityHeap.back());
                    w.PriorityHeap.pop_back();

                    w.HighestPriority = (w.PriorityHeap.empty()
                        ? INDEX_NONE
                        : w.PriorityHeap.front().Priority);

                    Assert(_numPendingTask > 0);
                    if (0 == --_numPendingTask)
                        _globalRevision = 0;

                    PokeGlobalPriority_(w.HighestPriority);

                    return;
                }
            }

            // keep track of min local priority
            localPriority = Min(localPriority, w.HighestPriority);

            if (++workerIndex == numWorkers)
                workerIndex = 0;
        }

        // failed to consume task: update global priority with local priority found
        if (globalPriority != localPriority)
            _globalPriority.compare_exchange_weak(globalPriority, localPriority);

        FPlatformProcess::SleepForSpinning(backoff);
    }
}
//----------------------------------------------------------------------------
inline void FTaskScheduler::ReleaseMemory() {
    for (FWorkerQueue_& w : _queues) {
        Meta::FUniqueLock scopeLock(w.Barrier, std::defer_lock);
        if (scopeLock.try_lock())
            w.PriorityHeap.shrink_to_fit();
    }
}
//----------------------------------------------------------------------------
#elif USE_PPE_THREAD_WORKSTEALINGQUEUE_V1
//----------------------------------------------------------------------------
// Using local pools with work stealing
//  + Minimum locking with very few contention
//  + Scales correctly with many cores (> 8)
//  - Approximation of correct execution order for priorities
//  - Can't guarantee insertion order
//----------------------------------------------------------------------------
inline FTaskScheduler::FTaskScheduler(size_t numWorkers)
:   _numWorkers(numWorkers) {
    Assert(_numWorkers);

    _workerQueues.resize_AssumeEmpty(_numWorkers);
}
//----------------------------------------------------------------------------
inline FTaskScheduler::~FTaskScheduler() {
    Assert(0 == _state.TaskInFlight);
    Assert(0 == _state.TaskRevision);
}
//----------------------------------------------------------------------------
inline bool FTaskScheduler::HasPendingTask() const {
    return (_state.TaskInFlight != 0);
}
//----------------------------------------------------------------------------
inline void FTaskScheduler::Produce(ETaskPriority priority, FTaskFunc&& rtask, FCompletionPort* pport) {
    Assert_NoAssume(rtask);
    Assert(_state.TaskRevision < 0xFFFFFF); // just to get sure that we won't be overflowing Priority

    // track in flight tasks for HasPendingTask() and to reset _state.TaskRevision when the queues are finally empty
    ++_state.TaskInFlight;

    // construct insertion priority with _state.TaskRevision
    const size_t insertion_order_preserving_priority{ (size_t(priority) << 24) | ++_state.TaskRevision };
    Assert((insertion_order_preserving_priority >> 24) == size_t(priority));

    FTaskQueued queued{ insertion_order_preserving_priority, std::move(rtask), pport };

    for (size_t backoff = 0;;) {
        // push the job to some worker's local queue
        forrange(n, insertion_order_preserving_priority, insertion_order_preserving_priority + _numWorkers) {
            const size_t workerIndex = (n % _numWorkers); // use revision packed in first 24 bits

            if (WorkerTryPush_(workerIndex, queued)) {

                // used as hint for work stealing : worker will try to steal a job if a more priority task is available
                size_t globalHighest = _state.GlobalPriority;
                if (globalHighest > insertion_order_preserving_priority)
                    _state.GlobalPriority.compare_exchange_weak(globalHighest, insertion_order_preserving_priority);

                return;
            }
        }

        // temper insertions in case of high contention
        FPlatformProcess::SleepForSpinning(backoff);
    }
}
//----------------------------------------------------------------------------
inline void FTaskScheduler::Consume(size_t workerIndex, FTaskQueued* pop) {
    if (not WorkerStealIFP_(workerIndex, pop))
        WorkerPop_(workerIndex, pop);

    // at this stage we already consumed a task for execution

    // reset _state.TaskRevision when everything is processed to avoid overflows :
    // having overflow would cause the insertion order to be inverted and would violate our assumptions
    if (0 == --_state.TaskInFlight)
        _state.TaskRevision = 0;
}
//----------------------------------------------------------------------------
inline void FTaskScheduler::ReleaseMemory() {
    for (FLocalQueue_& worker : _workerQueues) {
        Meta::FUniqueLock scopeLock(worker.Barrier, std::defer_lock);
        if (scopeLock.try_lock())
            worker.ReleaseMemory(scopeLock);
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
inline FTaskScheduler::FTaskScheduler(size_t numWorkers)
:   _numWorkers(numWorkers)
,   _tasks(8192) {
    Assert(numWorkers);
}
//----------------------------------------------------------------------------
inline FTaskScheduler::~FTaskScheduler() {
    Assert(_tasks.empty());
}
//----------------------------------------------------------------------------
inline bool FTaskScheduler::HasPendingTask() const {
    return (not _tasks.empty());
}
//----------------------------------------------------------------------------
inline void FTaskScheduler::Produce(ETaskPriority priority, FTaskFunc&& rtask, FCompletionPort* pport) {
    _tasks.Produce(u32(priority), FTaskQueued{ std::move(rtask), pport });
}
//----------------------------------------------------------------------------
inline void FTaskScheduler::Produce(ETaskPriority priority, const FTaskFunc& task, FCompletionPort* pport) {
    _tasks.Produce(u32(priority), FTaskQueued{ FTaskFunc(task), pport });
}
//----------------------------------------------------------------------------
inline void FTaskScheduler::Produce(ETaskPriority priority, const TMemoryView<FTaskFunc>& rtasks, FCompletionPort* pport) {
    _tasks.Produce(u32(priority), rtasks.size(), _numWorkers, [rtasks, pport](size_t i) {
        return FTaskQueued{ std::move(rtasks[i]), pport };
    });
}
//----------------------------------------------------------------------------
inline void FTaskScheduler::Produce(ETaskPriority priority, const TMemoryView<const FTaskFunc>& tasks, FCompletionPort* pport) {
    _tasks.Produce(u32(priority), tasks.size(), _numWorkers, [tasks, pport](size_t i) {
        return FTaskQueued{ FTaskFunc(tasks[i]), pport };
    });
}
//----------------------------------------------------------------------------
inline void FTaskScheduler::Consume(size_t , FTaskQueued* pop) {
    _tasks.Consume(pop);
}
//----------------------------------------------------------------------------
inline void FTaskScheduler::ReleaseMemory() {
    NOOP();
}
//----------------------------------------------------------------------------
#endif //!USE_PPE_THREAD_WORKSTEALINGQUEUE
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_THREAD_WORKSTEALINGQUEUE
//----------------------------------------------------------------------------
inline void FTaskScheduler::Produce(ETaskPriority priority, const TMemoryView<FTaskFunc>& rtasks, FCompletionPort* pport) {
    for (FTaskFunc& rtask : rtasks)
        Produce(priority, std::move(rtask), pport);
}
//----------------------------------------------------------------------------
inline void FTaskScheduler::Produce(ETaskPriority priority, const TMemoryView<const FTaskFunc>& tasks, FCompletionPort* pport) {
    for (const FTaskFunc& task : tasks)
        Produce(priority, task, pport);
}
//----------------------------------------------------------------------------
inline void FTaskScheduler::Produce(ETaskPriority priority, const FTaskFunc& task, FCompletionPort* pport) {
    Assert_NoAssume(task);

    Produce(priority, FTaskFunc{ task }, pport);
}
//----------------------------------------------------------------------------
#endif //!USE_PPE_THREAD_WORKSTEALINGQUEUE
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
