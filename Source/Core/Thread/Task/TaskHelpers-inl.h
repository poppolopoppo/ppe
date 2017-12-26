#pragma once

#include "Core/Thread/Task/TaskHelpers.h"

#include "Core/Container/Stack.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Default, TFuture<T>, template <typename T>);
//----------------------------------------------------------------------------
template <typename T>
TFuture<T>::TFuture(func_type&& func)
    : _available(0)
    , _func(std::move(func))
{}
//----------------------------------------------------------------------------
template <typename T>
TFuture<T>::~TFuture() {
    Assert(_available);
}
//----------------------------------------------------------------------------
template <typename T>
T& TFuture<T>::Result() {
    while (!_available)
        ::_mm_pause();
    return _value;
}
//----------------------------------------------------------------------------
template <typename T>
T* TFuture<T>::ResultIFP() {
    return (_available ? &_value : nullptr);
}
//----------------------------------------------------------------------------
template <typename T>
FTaskFunc TFuture<T>::MakeTask() {
    // FTaskFunc only checks the lifetime, the user is responsible for TFuture<> lifetime !
    return FTaskFunc([future{ SFuture<T>(this) }](ITaskContext&) {
        Assert(not future->_available);
        future->_value = future->_func();
        future->_available = true;
    });
}
//----------------------------------------------------------------------------
template <typename T>
PFuture<T> Future(
    Meta::TFunction<T()>&& func,
    ETaskPriority priority/* = ETaskPriority::Normal */,
    FTaskManager* manager/* = nullptr *//* uses FGlobalThreadPool by default */) {
    PFuture<T> future(new TFuture<T>(std::move(func)));
    Async(future->MakeTask(), priority, manager);
    return future;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _It>
void ParallelFor(
    _It first, _It last,
    const Meta::TFunction<void(_It)>& foreach,
    ETaskPriority priority/* = ETaskPriority::Normal */,
    FTaskManager* manager/* = nullptr *//* uses FGlobalThreadPool by default */) {
    const size_t count = std::distance(first, last);
    if (0 == count) {
        // skip completely empty sequences
        return;
    }
    else if (1 == count) {
        // skip task creation if there is only 1 iteration
        foreach(first);
    }
    else {
        // fall back to global thread pool by default
        if (!manager)
            manager = &FGlobalThreadPool::Instance();

        // all iterations are dispatch by regular slices to worker threads (less overhead)
        const size_t worker_count = (manager->WorkerCount() + 1/* current thread */);
        const size_t worker_tasks = ((count + worker_count - 1) / worker_count);

        const size_t owner_first = (worker_tasks * (worker_count - 1));
        const size_t owner_count = (count - owner_first);

        // less space needed to pass arguments to workers
        const struct loop_t_ {
            _It first, last;
            decltype(foreach) foreach;
        }   loop{ first, last, foreach };

        // creates tasks for multi-threaded completion
        STACKLOCAL_STACK(FTaskFunc, tasks, owner_first);

        // prepare iteration tasks for the manager
        for (size_t i = 0; i != owner_first; i += worker_tasks)
            tasks.Push([&loop, i, j{ i + worker_tasks }](ITaskContext&) {
                forrange(it, loop.first + i, loop.first + j)
                    loop.foreach(*it);
            });

        // blocking wait for end of the loop
        Assert(tasks.size() == tasks.capacity());
        manager->RunAndWaitFor(
            tasks.MakeConstView(),
            // slice processed on current thread while waiting :
            [&loop, owner_first, count](ITaskContext&) {
                forrange(it, loop.first + owner_first, loop.first + count)
                    loop.foreach(*it);
            },
            priority );
    }
}
//----------------------------------------------------------------------------
template <typename _It>
void ParallelFor(
    _It first, _It last,
    const Meta::TFunction<void(decltype(*std::declval<_It>()))>& foreach_item,
    ETaskPriority priority/* = ETaskPriority::Normal */,
    FTaskManager* manager/* = nullptr *//* uses FGlobalThreadPool by default */) {
    const size_t count = std::distance(first, last);
    if (0 == count) {
        // skip completely empty sequences
        return;
    }
    else if (1 == count) {
        // skip task creation if there is only 1 iteration
        foreach_item(*first);
    }
    else {
        // fall back to global thread pool by default
        if (!manager)
            manager = &FGlobalThreadPool::Instance();

        // all iterations are dispatch by regular slices to worker threads (less overhead)
        const size_t worker_count = (manager->WorkerCount() + 1/* current thread */);
        const size_t worker_tasks = ((count + worker_count - 1) / worker_count);

        const size_t owner_first = (worker_tasks * (worker_count - 1));
        const size_t owner_count = (count - owner_first);

        // less space needed to pass arguments to workers
        const struct loop_t_ {
            const _It first;
            const _It last;
            decltype(foreach_item) foreach_item;
        }   loop{ first, last, foreach_item };

        // creates tasks for multi-threaded completion
        STACKLOCAL_STACK(FTaskFunc, tasks, worker_count - 1);

        // prepare iteration tasks for the manager
        for (size_t i = 0; i != owner_first; i += worker_tasks)
            tasks.Push([&loop, i, j{ i + worker_tasks }](ITaskContext&) {
                forrange(it, loop.first + i, loop.first + j)
                    loop.foreach_item(*it);
            });

        // blocking wait for end of the loop
        Assert(tasks.size() == tasks.capacity());
        manager->RunAndWaitFor(
            tasks.MakeConstView(),
            // slice processed on current thread while waiting :
            [&loop, owner_first](ITaskContext&) {
                forrange(it, loop.first + owner_first, loop.last)
                    loop.foreach_item(*it);
            },
            priority );
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
