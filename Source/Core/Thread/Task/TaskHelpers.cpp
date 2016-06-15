#include "stdafx.h"

#include "TaskHelpers.h"

#include "Allocator/PoolAllocator-impl.h"
#include "Thread/Task/TaskManager.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_DEF(TaskProcedure, );
//----------------------------------------------------------------------------
TaskProcedure::TaskProcedure(function_type&& func)
:   _func(std::move(func)) {}
//----------------------------------------------------------------------------
void TaskProcedure::Run(ITaskContext& ctx) {
    _func(ctx);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
TaskProcedure* MakeAsync(std::function<void()>&& fireAndForget) {
    return new TaskProcedure(std::bind(std::move(fireAndForget))); // will be deleted by RunAndSuicide()
}
//----------------------------------------------------------------------------
TaskProcedure* MakeAsync(std::function<void(ITaskContext&)>&& fireAndForget) {
    return new TaskProcedure(std::move(fireAndForget)); // will be deleted by RunAndSuicide()
}
//----------------------------------------------------------------------------
void ASync(TaskManager& manager, std::function<void()>&& fireAndForget, TaskPriority priority/* = TaskPriority::Normal */) {
    auto* const task = MakeAsync(std::move(fireAndForget));
    manager.Run(*task, priority);
}
//----------------------------------------------------------------------------
void ASync(TaskManager& manager, std::function<void(ITaskContext&)>&& fireAndForget, TaskPriority priority/* = TaskPriority::Normal */) {
    auto* const task = MakeAsync(std::move(fireAndForget));
    manager.Run(*task, priority);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
