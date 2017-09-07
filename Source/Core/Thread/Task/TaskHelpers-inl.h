#pragma once

#include "Core/Thread/Task/TaskHelpers.h"

#include "Core/Container/Stack.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Result>
TTaskFuture<_Result>::TTaskFuture(function_type&& func)
:   _func(std::move(func))
,   _available(false) {}
//----------------------------------------------------------------------------
template <typename _Result>
TTaskFuture<_Result>::~TTaskFuture() {
    Assert(_available);
}
//----------------------------------------------------------------------------
template <typename _Result>
const _Result& TTaskFuture<_Result>::Result() const {
    while (false == _available)
        ::_mm_pause();
    Assert(_available);
    return _result;
}
//----------------------------------------------------------------------------
template <typename _Result>
void TTaskFuture<_Result>::Run(ITaskContext& ctx) {
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
TTaskFuture< decltype(std::declval<_Lambda>()()) >*
    MakeFuture(_Lambda&& func) {
    using return_type = decltype(std::declval<_Lambda>()());
    return new TTaskFuture<return_type>(std::move(func));// will be deleted by RunAndSuicide()
}
//----------------------------------------------------------------------------
template <typename _Lambda>
TTaskFuture< decltype(std::declval<_Lambda>()()) >*
    Future(FTaskManager& manager, _Lambda&& func, ETaskPriority priority/* = ETaskPriority::Normal */) {
    auto* const task = MakeFuture(std::move(func));
    manager.Run(*task, priority);
    return task;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _It, typename _Lambda>
void ParallelForRange(
    FTaskManager& manager,
    _It first, _It last, _Lambda&& lambda,
    ETaskPriority priority/* = ETaskPriority::Normal */) {
    const size_t count = std::distance(first, last);
    if (0 == count) {
        // skip completely empty sequences
        return;
    }
    else if (1 == count) {
        // skip task creation if there is only 1 iteration
        lambda(*first);
    }
    else {
        // creates tasks for multi-threaded completion
        STACKLOCAL_STACK(FTaskDelegate, tasks, count);

        forrange(it, first, last) {
            auto* const task = new FTaskProcedure(std::bind(lambda, std::ref(*it)));
            tasks.Push(*task/* will be deleted by RunAndSuicide() */);
        }

        manager.RunAndWaitFor(tasks.MakeConstView(), priority);
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
