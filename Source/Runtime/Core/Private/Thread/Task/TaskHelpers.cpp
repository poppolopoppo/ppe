#include "stdafx.h"

#include "Thread/Task/TaskHelpers.h"

#include "Thread/Task/TaskManager.h"
#include "Thread/Fiber.h"
#include "Thread/ThreadPool.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FireAndForget(
    FFireAndForget* task,
    ETaskPriority priority/* = ETaskPriority::Normal */,
    FTaskManager* manager/* = nullptr */) {
    Assert(task);

    Async(FTaskFunc(task, &FFireAndForget::RunAndSuicide), priority, manager);
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
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
