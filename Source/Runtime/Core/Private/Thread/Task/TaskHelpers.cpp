// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Thread/Task/TaskHelpers.h"

#include "Thread/Task/Task.h"
#include "Thread/Task/TaskManager.h"
#include "Thread/Fiber.h"
#include "Thread/ThreadPool.h"

#include "Meta/Iterator.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FireAndForget(
    FFireAndForget* task,
    ETaskPriority priority/* = ETaskPriority::Normal */,
    ITaskContext* context/* = nullptr */) {
    Assert(task);
    Async(FTaskFunc::Bind<&FFireAndForget::RunAndSuicide>(task), priority, context);
}
//----------------------------------------------------------------------------
void Async(
    FTaskFunc&& task,
    ETaskPriority priority/* = ETaskPriority::Normal */,
    ITaskContext* context/* = nullptr */) {
    if (nullptr == context)
        context = GlobalTaskContext();

    context->Run(nullptr, std::move(task), priority);
}
//----------------------------------------------------------------------------
void ParallelFor(
    size_t first, size_t last,
    const TFunction<void(size_t)>& foreach,
    ETaskPriority priority /* = ETaskPriority::Normal */,
    ITaskContext* context /* = nullptr *//* uses FHighPriorityThreadPool by default */) {
    return ParallelForEachValue(
        MakeCountingIterator(first),
        MakeCountingIterator(last),
        foreach, priority, context );
}
//----------------------------------------------------------------------------
int ParallelSum(
    size_t first, size_t last,
    const TFunction<int(size_t)>& sum,
    ETaskPriority priority /* = ETaskPriority::Normal */,
    ITaskContext* context /* = nullptr *//* uses FHighPriorityThreadPool by default */) {
    std::atomic<int> total{ 0 };
    ParallelForEachValue(
        MakeCountingIterator(first),
        MakeCountingIterator(last),
        [&sum, &total](size_t index) -> void {
            total += sum(index);
        }, priority, context );
    return total;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
