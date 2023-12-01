#pragma once

#include "Thread/Task/TaskHelpers.h"

#include "Container/Stack.h"
#include "HAL/PlatformMisc.h"
#include "HAL/PlatformProcess.h"
#include "Thread/AtomicSpinLock.h"
#include "Thread/Task/CompletionPort.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
TFuture<T>::TFuture(func_type&& func)
    : _state(Idle)
    , _func(std::move(func)) {
}
//----------------------------------------------------------------------------
template <typename T>
TFuture<T>::~TFuture() {
    Assert(Pending != _state);
}
//----------------------------------------------------------------------------
template <typename T>
T& TFuture<T>::Result() {
    Assert(Idle != _state);

    for (i32 backoff = 0;; ) {
        const int current = _state.load(std::memory_order_acquire);
        if (Ready == _state)
            break;
        details::SpinAtomicBarrier(&_state, current, backoff);
    }
    return _value;
}
//----------------------------------------------------------------------------
template <typename T>
T* TFuture<T>::ResultIFP() {
    return (Ready == _state ? &_value : nullptr);
}
//----------------------------------------------------------------------------
template <typename T>
void TFuture<T>::Async(ETaskPriority priority, ITaskContext* context) {
    AddSafeRef(this); // FTaskFunc only checks its kept alive, the client should handle lifetime
    PPE::Async([this](ITaskContext&) { // use a lambda to keep lifetime in check through TSafePtr<>
        _value = _func();
        RemoveSafeRef(this);
        _state = Ready; // set Ready *AFTER* releasing TSafePtr<>
        details::NotifyAllAtomicBarrier(&_state);
    },  priority, context );
}
//----------------------------------------------------------------------------
template <typename T>
PFuture<T> Future(
    TFunction<T()>&& func,
    ETaskPriority priority/* = ETaskPriority::Normal */,
    ITaskContext* context/* = nullptr *//* uses FGlobalThreadPool by default */) {
    PFuture<T> future{ NEW_REF(Task, TFuture<T>, std::move(func)) };
    future->Async(priority, context);
    return future;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
template <typename _It, typename _Value>
static void ParallelForEach_(
    _It first, _It last,
    const TFunction<void(_Value)>& foreach,
    ETaskPriority priority,
    ITaskContext* context ) {
    STATIC_ASSERT(Meta::is_random_access_iterator<_It>::value);

    if (first == last) {
        // skip completely empty sequences
        return;
    }
    else if (first + 1 == last) {
        // skip task creation if there is only 1 iteration
        IF_CONSTEXPR(std::is_same_v<_Value, _It>)
            foreach(first);
        else
            foreach(*first);
        return;
    }

    // fall back to high priority thread pool by default, since we're blocking the current thread
    if (nullptr == context)
        context = HighPriorityTaskContext();

    // less space needed to pass arguments to TFunction<> (debug iterators can be huge)
    const struct loop_t_ {
        const TFunction<void(_Value)>& foreach;
        _It first, last;
        void Task(ITaskContext&, u32 off, u32 num) const {
            const _It cend = first + (off + num);
            Assert_NoAssume(last >= cend);
            for(_It it = first + off; it != cend; ++it) {
                IF_CONSTEXPR(std::is_same_v<_Value, _It>)
                    foreach(it);
                else
                    foreach(*it);
            }
        }
    }   loop{ foreach, first, last };

    // all iterations are dispatch by regular slices to worker threads (less overhead)
    const u32 range_dist = checked_cast<u32>(std::distance(first, last));
    const u32 worker_count = Min(checked_cast<u32>(context->WorkerCount()), range_dist);
    const u32 worker_tasks = range_dist / worker_count;
    const u32 tasks_remain = range_dist - worker_tasks * worker_count;
    Assert(tasks_remain < worker_count);

    // creates tasks for multi-threaded completion
    STACKLOCAL_STACK(FTaskFunc, tasks, worker_count);

    u32 tasks_offset = 0;
    forrange(i, 0, worker_count - 1) {
        // even out workload by spreading reminder
        const u32 work_slice = (worker_tasks + (i < tasks_remain ? 1 : 0));

        // push a new task for this slice of the loop
        tasks.Push(FTaskFunc::Bind<&loop_t_::Task>(&loop, tasks_offset, work_slice));
        tasks_offset += work_slice;
    }

    Assert(tasks_offset + worker_tasks == range_dist);
    FTaskFunc lastTask = FTaskFunc::Bind<&loop_t_::Task>(&loop, tasks_offset, worker_tasks);

    // decide if it's worth to process part of load on the current thread
    if (worker_count < FPlatformMisc::NumCoresWithSMT()) {
        // wait for end of the loop while processing the last slice
        context->RunAndWaitFor(tasks.MakeView(), std::move(lastTask), priority);
    }
    else {
        // push last slice since we don't busy wait
        tasks.Push(std::move(lastTask));

        // blocking wait for end of the loop
        Assert(tasks.size() == tasks.capacity());
        context->RunAndWaitFor(tasks.MakeView(), priority);
    }
}
} //!details
//----------------------------------------------------------------------------
template <typename _It>
void ParallelForEach(
    _It first, _It last,
    const TFunction<void(_It)>& foreach_it,
    ETaskPriority priority/* = ETaskPriority::Normal */,
    ITaskContext* context/* = nullptr *//* uses FHighPriorityThreadPool by default */) {
    details::ParallelForEach_(first, last, foreach_it, priority, context);
}
//----------------------------------------------------------------------------
template <typename _It>
void ParallelForEachValue(
    _It first, _It last,
    const TFunction<void(typename Meta::TIteratorTraits<_It>::value_type)>& foreach_value,
    ETaskPriority priority/* = ETaskPriority::Normal */,
    ITaskContext* context/* = nullptr *//* uses FHighPriorityThreadPool by default */) {
    details::ParallelForEach_(first, last, foreach_value, priority, context);
}
//----------------------------------------------------------------------------
template <typename _It>
void ParallelForEachRef(
    _It first, _It last,
    const TFunction<void(typename Meta::TIteratorTraits<_It>::reference)>& foreach_ref,
    ETaskPriority priority/* = ETaskPriority::Normal */,
    ITaskContext* context/* = nullptr *//* uses FHighPriorityThreadPool by default */) {
    details::ParallelForEach_(first, last, foreach_ref, priority, context);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
