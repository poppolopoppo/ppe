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
#ifdef WITH_CORE_ASSERT
    using parent_type::HasInstance;
#endif

    static void Create();
    static void Destroy();

    static TMemoryView<const size_t> ThreadAffinities();
};
//----------------------------------------------------------------------------
void AsyncWork(const FTaskDelegate& task, ETaskPriority priority = ETaskPriority::Normal);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FIOThreadPool : Meta::TSingleton<FTaskManager, FIOThreadPool> {
public:
    typedef Meta::TSingleton<FTaskManager, FIOThreadPool> parent_type;

    using parent_type::Instance;
#ifdef WITH_CORE_ASSERT
    using parent_type::HasInstance;
#endif

    static void Create();
    static void Destroy();

    static TMemoryView<const size_t> ThreadAffinities();
};
//----------------------------------------------------------------------------
void AsyncIO(const FTaskDelegate& task, ETaskPriority priority = ETaskPriority::Normal);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FLowestPriorityThreadPool : Meta::TSingleton<FTaskManager, FLowestPriorityThreadPool> {
public:
	typedef Meta::TSingleton<FTaskManager, FLowestPriorityThreadPool> parent_type;

	using parent_type::Instance;
#ifdef WITH_CORE_ASSERT
	using parent_type::HasInstance;
#endif

	static void Create();
	static void Destroy();

	static TMemoryView<const size_t> ThreadAffinities();
};
//----------------------------------------------------------------------------
void AsyncLowestPriority(const FTaskDelegate& task, ETaskPriority priority = ETaskPriority::Normal);
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
