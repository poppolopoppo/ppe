#include "stdafx.h"

#include "TaskHelpers.h"

#include "Thread/Task/TaskManager.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
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
FTaskProcedure* MakeAsync(Meta::TFunction<void()>&& fireAndForget) {
    return new FTaskProcedure(std::bind(std::move(fireAndForget))); // will be deleted by RunAndSuicide()
}
//----------------------------------------------------------------------------
FTaskProcedure* MakeAsync(Meta::TFunction<void(ITaskContext&)>&& fireAndForget) {
    return new FTaskProcedure(std::move(fireAndForget)); // will be deleted by RunAndSuicide()
}
//----------------------------------------------------------------------------
void ASync(FTaskManager& manager, Meta::TFunction<void()>&& fireAndForget, ETaskPriority priority/* = ETaskPriority::Normal */) {
    auto* const task = MakeAsync(std::move(fireAndForget));
    manager.Run(*task, priority);
}
//----------------------------------------------------------------------------
void ASync(FTaskManager& manager, Meta::TFunction<void(ITaskContext&)>&& fireAndForget, ETaskPriority priority/* = ETaskPriority::Normal */) {
    auto* const task = MakeAsync(std::move(fireAndForget));
    manager.Run(*task, priority);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
