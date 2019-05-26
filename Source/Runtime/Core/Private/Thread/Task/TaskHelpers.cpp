#include "stdafx.h"

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
    FTaskManager* manager/* = nullptr */) {
    Assert(task);
    Async(FTaskFunc::Bind<&FFireAndForget::RunAndSuicide>(task), priority, manager);
}
//----------------------------------------------------------------------------
void Async(
    FTaskFunc&& task,
    ETaskPriority priority/* = ETaskPriority::Normal */,
    FTaskManager* manager/* = nullptr */) {
    if (nullptr == manager)
        manager = &FGlobalThreadPool::Get();

    manager->Run(std::move(task), priority);
}
//----------------------------------------------------------------------------
void ParallelFor(
    size_t first, size_t last,
    const TFunction<void(size_t)>& foreach,
    ETaskPriority priority /* = ETaskPriority::Normal */,
    FTaskManager* manager /* = nullptr *//* uses FHighPriorityThreadPool by default */) {
    return ParallelForEach(
        MakeCountingIterator(first),
        MakeCountingIterator(last),
        foreach, priority, manager);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
