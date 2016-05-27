#pragma once

#include "Core/Thread/Task/TaskHelpers.h"

#include "Core/Allocator/PoolAllocator-impl.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_DEF(TaskFuture<_Result>, template <typename _Result>);
//----------------------------------------------------------------------------
template <typename _Result>
TaskFuture<_Result>::TaskFuture(function_type&& func)
:   _func(std::move(func))
,   _available(false) {}
//----------------------------------------------------------------------------
template <typename _Result>
TaskFuture<_Result>::~TaskFuture() {
    Assert(_available);
}
//----------------------------------------------------------------------------
template <typename _Result>
const _Result& TaskFuture<_Result>::Result() const {
    while (false == _available)
        ::_mm_pause();
    Assert(_available);
    return _result;
}
//----------------------------------------------------------------------------
template <typename _Result>
void TaskFuture<_Result>::Run(ITaskContext& ctx) {
    UNUSED(ctx);
    Assert(false == _available);
    _result = _func();
#ifdef WITH_CORE_ASSERT
    const bool previous = _available.exchange(true);
    Assert(false == previous);
#else
    _available = true;
#endif
}
//----------------------------------------------------------------------------
template <typename _Lambda>
TaskFuture< decltype(std::declval<_Lambda>()()) >*
    Future(TaskManager& manager, _Lambda&& func, TaskPriority priority/* = TaskPriority::Normal */) {
    typedef decltype(std::declval<_Lambda>()()) return_type;
    auto* const task = new TaskFuture<return_type>(std::move(func));// will be deleted by RunAndSuicide()
    manager.Run(*task, priority);
    return task;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _It, typename _Lambda>
void ParallelForRange(
    TaskManager& manager,
    _It first, _It last, _Lambda&& lambda,
    TaskPriority priority/* = TaskPriority::Normal */) {
    const size_t count = std::distance(first, last);
    if (0 == count)
        return;

    STACKLOCAL_STACK(TaskDelegate, tasks, count);

    forrange(it, first, last) {
        auto* const task = new TaskProcedure(std::bind(lambda, std::cref(*it)));
        tasks.Push(*task/* will be deleted by RunAndSuicide() */);
    }

    manager.RunAndWaitFor(tasks.MakeView(), priority);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
