#include "stdafx.h"

#include "TaskHelpers.h"

#include "TaskManager.h"
#include "Thread/ThreadPool.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void Async(
    FTaskFunc&& task,
    ETaskPriority priority /* = ETaskPriority::Normal */,
    FTaskManager* manager /* = nullptr */) {
    if (nullptr == manager)
        manager = &FGlobalThreadPool::Instance();

    manager->Run(std::move(task), priority);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
