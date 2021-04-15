#pragma once

#include "Core_fwd.h"

#include "Thread/Task/Task.h"
#include "Thread/Task/CompletionPort.h"

#include "Container/MinMaxHeap.h"
#include "HAL/PlatformProcess.h"
#include "Memory/RefPtr.h"
#include "Thread/Task/Task.h"

//  Based on : Priority Work-Stealing Scheduler
//  A decentralized work-stealing scheduler that dynamically schedules fixed-priority tasks in a non-preemptive manner.
//  https://pdfs.semanticscholar.org/9d1b/eb4d2ca5c07965bb4e309864a9dcbae65fec.pdf
//  https://github.com/shamsimam/priorityworkstealing

//  But diverged importantly to keep blocking threads when there is no task in flight

#include "Container/Vector.h"

#define USE_PPE_TASK_SCHEDULER_SHUFFLING (USE_PPE_ASSERT)

#if USE_PPE_TASK_SCHEDULER_SHUFFLING
#    include "Maths/RandomGenerator.h"
#endif

#include <atomic>
#include <condition_variable>
#include <mutex>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PRAGMA_MSVC_WARNING_PUSH()
PRAGMA_MSVC_WARNING_DISABLE(4324) // 'XXX' structure was padded due to alignment
class FTaskScheduler {
public:
    struct FTaskQueued {
        size_t Priority;
        SCompletionPort Port;
        FTaskFunc Pending;

        FTaskQueued() = default;

        FTaskQueued(size_t priority, FTaskFunc&& pending, FCompletionPort* port) NOEXCEPT
        :   Priority(priority)
        ,   Port(port)
        ,   Pending(std::move(pending))
        {}

        friend bool operator <(const FTaskQueued& lhs, const FTaskQueued& rhs) NOEXCEPT {
            return (lhs.Priority < rhs.Priority);
        }
        friend bool operator >=(const FTaskQueued& lhs, const FTaskQueued& rhs) NOEXCEPT {
            return (not operator <(lhs, rhs));
        }

        FTaskQueued(const FTaskQueued&) = delete;
        FTaskQueued& operator =(const FTaskQueued&) = delete;

        FTaskQueued(FTaskQueued&& rvalue) = default;
        FTaskQueued& operator =(FTaskQueued&& rvalue) = default;
    };

    explicit FTaskScheduler(size_t numWorkers);
    ~FTaskScheduler();

    bool HasPendingTask() const NOEXCEPT;
    bool HasPendingTask(ETaskPriority atleast) const NOEXCEPT;

    void Produce(ETaskPriority priority, FTaskFunc&& rtask, FCompletionPort* pport);
    void Produce(ETaskPriority priority, const FTaskFunc& task, FCompletionPort* pport);

    void Produce(ETaskPriority priority, const TMemoryView<FTaskFunc>& rtasks, FCompletionPort* pport);
    void Produce(ETaskPriority priority, const TMemoryView<const FTaskFunc>& tasks, FCompletionPort* pport);

    void Consume(size_t workerIndex, FTaskQueued* pop);

    void ReleaseMemory();

private:
    STATIC_ASSERT((int)ETaskPriority::High == 0);
    STATIC_ASSERT((int)ETaskPriority::Internal == 3);
    STATIC_ASSERT(ETaskPriority::Internal > ETaskPriority::Low);
    STATIC_ASSERT(ETaskPriority::Low > ETaskPriority::Normal);
    STATIC_ASSERT(ETaskPriority::Normal > ETaskPriority::High);
    STATIC_CONST_INTEGRAL(size_t, NumPriorities, 4);

    struct priority_group_t {
        std::atomic<int> NumTasks{ 0 };
        std::atomic<size_t> Revision{ 0 };
    };

    struct CACHELINE_ALIGNED FWorkerQueue_ {
        size_t HighestPriority = INDEX_NONE;
        std::mutex Barrier;
        VECTORMINSIZE(Task, FTaskQueued, 32) PriorityHeap;
    };

    bool HasHigherPriorityTask_(const size_t highestPriority) const NOEXCEPT {
        STATIC_ASSERT(4 == NumPriorities);
        std::atomic_thread_fence(std::memory_order_acquire);
        return ((0 < highestPriority && !!_priorityGroups[0].NumTasks.load(std::memory_order_relaxed)) |
                (1 < highestPriority && !!_priorityGroups[1].NumTasks.load(std::memory_order_relaxed)) |
                (2 < highestPriority && !!_priorityGroups[2].NumTasks.load(std::memory_order_relaxed)) |
                (3 < highestPriority && !!_priorityGroups[3].NumTasks.load(std::memory_order_relaxed)) );
    }

    static CONSTEXPR size_t PackPriorityWRevision_(ETaskPriority priority, size_t revision) {
        Assert(revision);
        const size_t packed = (size_t(priority) << 28) | revision;
        Assert_NoAssume((packed >> 28) == size_t(priority));
        return packed;
    }

    static CONSTEXPR size_t UnpackPriorityFromRevision_(size_t packed) {
        return (packed >> 28); // return priority bits
    }

    std::condition_variable _onTask;
    priority_group_t _priorityGroups[NumPriorities];
    VECTOR(Task, FWorkerQueue_) _queues;
};
PRAGMA_MSVC_WARNING_POP()
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
inline bool FTaskScheduler::HasPendingTask() const NOEXCEPT {
    std::atomic_thread_fence(std::memory_order_acquire);

    return ((!!_priorityGroups[size_t(ETaskPriority::High)].NumTasks.load(std::memory_order_relaxed) |
             !!_priorityGroups[size_t(ETaskPriority::Normal)].NumTasks.load(std::memory_order_relaxed)) ||
            (!!_priorityGroups[size_t(ETaskPriority::Low)].NumTasks.load(std::memory_order_relaxed) |
             !!_priorityGroups[size_t(ETaskPriority::Internal)].NumTasks.load(std::memory_order_relaxed)) );
}
//----------------------------------------------------------------------------
inline bool FTaskScheduler::HasPendingTask(ETaskPriority atleast) const NOEXCEPT {
    std::atomic_thread_fence(std::memory_order_acquire);

    bool pending = false;
    switch (atleast) {
    case ETaskPriority::Internal:
        pending |= !!_priorityGroups[size_t(ETaskPriority::Internal)].NumTasks.load(std::memory_order_relaxed);
    case ETaskPriority::Low:
        pending |= !!_priorityGroups[size_t(ETaskPriority::Low)].NumTasks.load(std::memory_order_relaxed);
    case ETaskPriority::Normal:
        pending |= !!_priorityGroups[size_t(ETaskPriority::Normal)].NumTasks.load(std::memory_order_relaxed);
    case ETaskPriority::High:
        pending |= !!_priorityGroups[size_t(ETaskPriority::High)].NumTasks.load(std::memory_order_relaxed);
        break;
    default:
        AssertNotImplemented();
    }
    return pending;
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
    for (i32 backoff = 0;;) {
        // push the job to some worker's local queue
        forrange(n, 0, numWorkers) {
            FWorkerQueue_& w = _queues[workerIndex]; // use revision packed in first 24 bits

            Meta::FUniqueLock scopeLock(w.Barrier, std::defer_lock);
            if (Likely(scopeLock.try_lock())) {
                // push the new task to the priority heap
                {
                    PPE_LEAKDETECTOR_WHITELIST_SCOPE();
                    w.PriorityHeap.push_back(std::move(queued));
                    Push_MinMaxHeap(w.PriorityHeap.begin(), w.PriorityHeap.end());
                }

                // keep track of worker priority
                w.HighestPriority = UnpackPriorityFromRevision_(Min_MinMaxHeap(w.PriorityHeap.begin(), w.PriorityHeap.end())->Priority);

                // used as hints for work stealing : worker will try to steal a job if a higher priority task is available
                p.NumTasks.fetch_add(1, std::memory_order_release);

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

    for (i32 backoff = 0;;) {
        forrange(n, 0, numWorkers) {
            FWorkerQueue_& w = _queues[workerIndex];

            Meta::FUniqueLock scopeLock(w.Barrier, std::defer_lock);
            if (not HasHigherPriorityTask_(w.HighestPriority) && scopeLock.try_lock()) {
                _onTask.wait(scopeLock, [this]() NOEXCEPT{
                    return HasPendingTask();
                });

                if (not (w.PriorityHeap.empty() || HasHigherPriorityTask_(w.HighestPriority))) {
                    PopMin_MinMaxHeap(w.PriorityHeap.begin(), w.PriorityHeap.end());
                    *pop = std::move(w.PriorityHeap.back());
                    w.PriorityHeap.pop_back();

                    w.HighestPriority = (w.PriorityHeap.empty()
                        ? INDEX_NONE
                        : UnpackPriorityFromRevision_(Min_MinMaxHeap(w.PriorityHeap.begin(), w.PriorityHeap.end())->Priority));

                    priority_group_t& p = _priorityGroups[UnpackPriorityFromRevision_(pop->Priority)];
                    if (1 == p.NumTasks.fetch_sub(1, std::memory_order_relaxed))
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
inline void FTaskScheduler::Produce(ETaskPriority priority, const TMemoryView<FTaskFunc>& rtasks, FCompletionPort* pport) {
#if USE_PPE_TASK_SCHEDULER_SHUFFLING
    FRandomGenerator rng;
    rng.Shuffle(rtasks);
#endif

    for (FTaskFunc& rtask : rtasks)
        Produce(priority, std::move(rtask), pport);
}
//----------------------------------------------------------------------------
inline void FTaskScheduler::Produce(ETaskPriority priority, const TMemoryView<const FTaskFunc>& tasks, FCompletionPort* pport) {
#if USE_PPE_TASK_SCHEDULER_SHUFFLING
    STACKLOCAL_POD_ARRAY(u32, indices, tasks.size());
    forrange(i, 0, checked_cast<u32>(tasks.size())) indices[i] = i;
    FRandomGenerator rng;
    rng.Shuffle(indices);
    for (u32 i : indices)
        Produce(priority, tasks[i], pport);
#else
    for (const FTaskFunc& task : tasks)
        Produce(priority, task, pport);
#endif
}
//----------------------------------------------------------------------------
inline void FTaskScheduler::Produce(ETaskPriority priority, const FTaskFunc& task, FCompletionPort* pport) {
    Assert_NoAssume(task);

    Produce(priority, FTaskFunc{ task }, pport);
}
//----------------------------------------------------------------------------
inline void FTaskScheduler::ReleaseMemory() {
    for (FWorkerQueue_& w : _queues) {
        Meta::FUniqueLock scopeLock(w.Barrier, std::defer_lock);
        if (scopeLock.try_lock()) {
            PPE_LEAKDETECTOR_WHITELIST_SCOPE();
            w.PriorityHeap.shrink_to_fit();
        }
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
