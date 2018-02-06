#include "stdafx.h"

#include "TaskHelpers.h"

#include "TaskManager.h"
#include "Thread/Fiber.h"
#include "Thread/ThreadPool.h"

namespace Core {
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
        manager = &FGlobalThreadPool::Instance();

    manager->Run(std::move(task), priority);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
