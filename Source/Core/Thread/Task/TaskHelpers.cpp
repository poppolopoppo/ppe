#include "stdafx.h"

#include "TaskHelpers.h"

#include "Allocator/PoolAllocator-impl.h"
#include "Thread/Task/TaskManager.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_DEF(FTaskProcedure, );
//----------------------------------------------------------------------------
FTaskProcedure::FTaskProcedure(function_type&& func)
:   _func(std::move(func)) {}
//----------------------------------------------------------------------------
void FTaskProcedure::Run(ITaskContext& ctx) {
    _func(ctx);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTaskProcedure* MakeAsync(std::function<void()>&& fireAndForget) {
    return new FTaskProcedure(std::bind(std::move(fireAndForget))); // will be deleted by RunAndSuicide()
}
//----------------------------------------------------------------------------
FTaskProcedure* MakeAsync(std::function<void(ITaskContext&)>&& fireAndForget) {
    return new FTaskProcedure(std::move(fireAndForget)); // will be deleted by RunAndSuicide()
}
//----------------------------------------------------------------------------
void ASync(FTaskManager& manager, std::function<void()>&& fireAndForget, ETaskPriority priority/* = ETaskPriority::Normal */) {
    auto* const task = MakeAsync(std::move(fireAndForget));
    manager.Run(*task, priority);
}
//----------------------------------------------------------------------------
void ASync(FTaskManager& manager, std::function<void(ITaskContext&)>&& fireAndForget, ETaskPriority priority/* = ETaskPriority::Normal */) {
    auto* const task = MakeAsync(std::move(fireAndForget));
    manager.Run(*task, priority);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
