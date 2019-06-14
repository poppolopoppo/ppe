#pragma once

#include "Core.h"

#include "Meta/Singleton.h"
#include "Thread/Task/TaskManager.h"

#include <chrono>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FGlobalThreadPool : Meta::TSingleton<FTaskManager, FGlobalThreadPool> {
public:
    typedef Meta::TSingleton<FTaskManager, FGlobalThreadPool> parent_type;

    using parent_type::Get;
#if USE_PPE_ASSERT
    using parent_type::HasInstance;
#endif

    static void Create();
    static void Destroy();
};
//----------------------------------------------------------------------------
PPE_CORE_API void AsyncWork(FTaskFunc&& rtask, ETaskPriority priority = ETaskPriority::Normal);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FIOThreadPool : Meta::TSingleton<FTaskManager, FIOThreadPool> {
public:
    typedef Meta::TSingleton<FTaskManager, FIOThreadPool> parent_type;

    using parent_type::Get;
#if USE_PPE_ASSERT
    using parent_type::HasInstance;
#endif

    static void Create();
    static void Destroy();
};
//----------------------------------------------------------------------------
PPE_CORE_API void AsyncIO(FTaskFunc&& rtask, ETaskPriority priority = ETaskPriority::Normal);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FHighPriorityThreadPool : Meta::TSingleton<FTaskManager, FHighPriorityThreadPool> {
public:
    typedef Meta::TSingleton<FTaskManager, FHighPriorityThreadPool> parent_type;

    using parent_type::Get;
#if USE_PPE_ASSERT
    using parent_type::HasInstance;
#endif

    static void Create();
    static void Destroy();
};
//----------------------------------------------------------------------------
PPE_CORE_API void AsyncHighPriority(FTaskFunc&& rtask, ETaskPriority priority = ETaskPriority::Normal);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FBackgroundThreadPool : Meta::TSingleton<FTaskManager, FBackgroundThreadPool> {
public:
    typedef Meta::TSingleton<FTaskManager, FBackgroundThreadPool> parent_type;

    using parent_type::Get;
#if USE_PPE_ASSERT
    using parent_type::HasInstance;
#endif

    static void Create();
    static void Destroy();
};
//----------------------------------------------------------------------------
PPE_CORE_API void AsyncBackround(FTaskFunc&& rtask, ETaskPriority priority = ETaskPriority::Normal);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FThreadPoolStartup {
public:
    static void Start();
    static void Shutdown();

    static void DumpStats();
    static void ReleaseMemory();

    FThreadPoolStartup() { Start(); }
    ~FThreadPoolStartup() { Shutdown(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
