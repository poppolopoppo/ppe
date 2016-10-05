#pragma once

#include "Core/Core.h"

#include "Core/Meta/Singleton.h"
#include "Core/Thread/Task/TaskManager.h"

#include <chrono>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FGlobalThreadPool : Meta::TSingleton<FTaskManager, FGlobalThreadPool> {
public:
    typedef Meta::TSingleton<FTaskManager, FGlobalThreadPool> parent_type;

    using parent_type::Instance;
    using parent_type::HasInstance;

    static void Create();
    static void Destroy();
};
//----------------------------------------------------------------------------
void AsyncWork(const TaskDelegate& task, ETaskPriority priority = ETaskPriority::Normal);
//----------------------------------------------------------------------------
class IOThreadPool : Meta::TSingleton<FTaskManager, IOThreadPool> {
public:
    typedef Meta::TSingleton<FTaskManager, IOThreadPool> parent_type;

    using parent_type::Instance;
    using parent_type::HasInstance;

    static void Create();
    static void Destroy();
};
//----------------------------------------------------------------------------
void AsyncIO(const TaskDelegate& task, ETaskPriority priority = ETaskPriority::Normal);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FThreadPoolStartup {
public:
    static void Start();
    static void Shutdown();

    FThreadPoolStartup() { Start(); }
    ~FThreadPoolStartup() { Shutdown(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
