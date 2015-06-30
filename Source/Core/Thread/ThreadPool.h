#pragma once

#include "Core/Core.h"

#include "Core/Meta/Singleton.h"
#include "Core/Thread/Task/TaskPool.h"

#include <chrono>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class GlobalThreadPool : Meta::Singleton<TaskPool, GlobalThreadPool> {
public:
    typedef Meta::Singleton<TaskPool, GlobalThreadPool> parent_type;

    using parent_type::Instance;
    using parent_type::HasInstance;
    using parent_type::Destroy;

    static void Create();
};
//----------------------------------------------------------------------------
class IOThreadPool : Meta::Singleton<TaskPool, IOThreadPool> {
public:
    typedef Meta::Singleton<TaskPool, IOThreadPool> parent_type;

    using parent_type::Instance;
    using parent_type::HasInstance;
    using parent_type::Destroy;

    static void Create();
};
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
