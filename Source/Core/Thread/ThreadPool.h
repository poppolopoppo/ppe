#pragma once

#include "Core/Core.h"

#include "Core/Meta/Singleton.h"
#include "Core/Thread/Task/TaskManager.h"

#include <chrono>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class GlobalThreadPool : Meta::Singleton<TaskManager, GlobalThreadPool> {
public:
    typedef Meta::Singleton<TaskManager, GlobalThreadPool> parent_type;

    using parent_type::Instance;
    using parent_type::HasInstance;

    static void Create();
    static void Destroy();
};
//----------------------------------------------------------------------------
void AsyncWork(const TaskDelegate& task, TaskPriority priority = TaskPriority::Normal);
//----------------------------------------------------------------------------
class IOThreadPool : Meta::Singleton<TaskManager, IOThreadPool> {
public:
    typedef Meta::Singleton<TaskManager, IOThreadPool> parent_type;

    using parent_type::Instance;
    using parent_type::HasInstance;

    static void Create();
    static void Destroy();
};
//----------------------------------------------------------------------------
void AsyncIO(const TaskDelegate& task, TaskPriority priority = TaskPriority::Normal);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ThreadPoolStartup {
public:
    static void Start();
    static void Shutdown();

    ThreadPoolStartup() { Start(); }
    ~ThreadPoolStartup() { Shutdown(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
