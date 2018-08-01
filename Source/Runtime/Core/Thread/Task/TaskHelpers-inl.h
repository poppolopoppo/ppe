#pragma once

#include "Core/Thread/Task/TaskHelpers.h"

#include "Core/Container/Stack.h"
#include "Core/HAL/PlatformMisc.h"
#include "Core/HAL/PlatformProcess.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Default, TFuture<T>, template <typename T>);
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
    size_t backoff = 0;
    while (Ready != _state)
        FPlatformProcess::SleepForSpinning(backoff);
    return _value;
}
//----------------------------------------------------------------------------
template <typename T>
T* TFuture<T>::ResultIFP() {
    return (Ready == _state ? &_value : nullptr);
}
//----------------------------------------------------------------------------
template <typename T>
void TFuture<T>::Async(ETaskPriority priority, FTaskManager* manager) {
    AddSafeRef(this); // FTaskFunc only checks its kept alive, the client should handle lifetime
    Core::Async([this](ITaskContext&) { // use a lambda to keep lifetime in check through TSafePtr<>
        _value = _func();
        RemoveSafeRef(this);
        _state = Ready; // set Ready *AFTER* releasing TSafePtr<>
    },  priority, manager );
}
//----------------------------------------------------------------------------
template <typename T>
PFuture<T> Future(
    TFunction<T()>&& func,
    ETaskPriority priority/* = ETaskPriority::Normal */,
    FTaskManager* manager/* = nullptr *//* uses FGlobalThreadPool by default */) {
    PFuture<T> future(new TFuture<T>(std::move(func)));
    future->Async(priority, manager);
    return future;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _It>
void ParallelFor(
    _It first, _It last,
    const TFunction<void(_It)>& foreach,
    ETaskPriority priority/* = ETaskPriority::Normal */,
    FTaskManager* manager/* = nullptr *//* uses FHighPriorityThreadPool by default */) {
    STATIC_ASSERT(Meta::is_random_access_iterator<_It>::value);

    const size_t range_count = std::distance(first, last);

    if (0 == range_count) {
        // skip completely empty sequences
        return;
    }
    else if (1 == range_count) {
        // skip task creation if there is only 1 iteration
        foreach(first);
        return;
    }

    // fall back to global thread pool by default
    if (!manager)
        manager = &FHighPriorityThreadPool::Get();

    // less space needed to pass arguments to TFunction<>
    const struct loop_t_ {
        _It first, last;
        decltype(foreach) foreach;
    }   loop{ first, last, foreach };

    // all iterations are dispatch by regular slices to worker threads (less overhead)
    const size_t worker_count = Min(manager->WorkerCount(), range_count);
    const size_t worker_tasks = range_count / worker_count;
    const size_t tasks_remain = range_count - worker_tasks * worker_count;
    Assert(tasks_remain < worker_count);

    // creates tasks for multi-threaded completion
    size_t task_first = 0;
    STACKLOCAL_STACK(FTaskFunc, tasks, worker_count);
    forrange(i, 0, worker_count - 1) {
        // even out workload by spreading reminder
        const size_t task_last = (task_first + worker_tasks + (i < tasks_remain ? 1 : 0));

        // push a new task for this slice of the loop
        tasks.Push([&loop, task_first, task_last](ITaskContext&) {
            forrange(it, loop.first + task_first, loop.first + task_last)
                loop.foreach(it);
        });

        task_first = task_last;
    }

    Assert(task_first + worker_tasks == range_count);
    FTaskFunc last_task = [&loop, task_first, task_last{ range_count }](ITaskContext&) {
        forrange(it, loop.first + task_first, loop.first + task_last)
            loop.foreach(it);
    };

    // decide if it's worth to process part of load on the current thread
    const bool busy_wait = (worker_count < FPlatformMisc::NumCoresWHyperThreading());

    if (busy_wait) {
        // wait for end of the loop while processing the last slice
        manager->RunAndWaitFor(tasks.MakeView(), last_task, priority);
    }
    else {
        // push last slice since we don't busy wait
        tasks.Push(std::move(last_task));

        // blocking wait for end of the loop
        Assert(tasks.size() == tasks.capacity());
        manager->RunAndWaitFor(tasks.MakeView(), priority);
    }
}
//----------------------------------------------------------------------------
template <typename _It>
void ParallelForEach(
    _It first, _It last,
    const TFunction<void(decltype(*std::declval<_It>()))>& foreach_item,
    ETaskPriority priority/* = ETaskPriority::Normal */,
    FTaskManager* manager/* = nullptr */) {
    const TFunction<void(_It)> foreach = [&foreach_item](_It it) { foreach_item(*it); };
    ParallelFor(first, last, foreach, priority, manager);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
