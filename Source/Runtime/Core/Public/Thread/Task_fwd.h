#pragma once

#include "Core_fwd.h"

#include "Misc/Function_fwd.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class ETaskPriority : u32;
//----------------------------------------------------------------------------
class PPE_CORE_API ITaskContext;
class PPE_CORE_API FTaskManager;
//----------------------------------------------------------------------------
class PPE_CORE_API FAggregationPort;
class PPE_CORE_API FCompletionPort;
//----------------------------------------------------------------------------
using FTaskFunc = TFunction<void(ITaskContext&)>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_CORE_API FTaskManager& GlobalThreadPool() NOEXCEPT;
PPE_CORE_API FTaskManager& IOThreadPool() NOEXCEPT;
PPE_CORE_API FTaskManager& HighPriorityThreadPool() NOEXCEPT;
PPE_CORE_API FTaskManager& BackgroundThreadPool() NOEXCEPT;
//----------------------------------------------------------------------------
PPE_CORE_API void AsyncWork(FTaskFunc&& rtask, ETaskPriority priority);
PPE_CORE_API void AsyncIO(FTaskFunc&& rtask, ETaskPriority priority);
PPE_CORE_API void AsyncHighPriority(FTaskFunc&& rtask, ETaskPriority priority);
PPE_CORE_API void AsyncBackground(FTaskFunc&& rtask, ETaskPriority priority);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
