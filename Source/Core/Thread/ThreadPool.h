#pragma once

#include "Core/Core.h"

#include "Core/Meta/Singleton.h"
#include "Core/Thread/Task/TaskManager.h"

#include <chrono>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class CORE_API FGlobalThreadPool : Meta::TSingleton<FTaskManager, FGlobalThreadPool> {
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
CORE_API void AsyncWork(const FTaskFunc& task, ETaskPriority priority = ETaskPriority::Normal);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class CORE_API FIOThreadPool : Meta::TSingleton<FTaskManager, FIOThreadPool> {
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
CORE_API void AsyncIO(const FTaskFunc& task, ETaskPriority priority = ETaskPriority::Normal);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class CORE_API FBackgroundThreadPool : Meta::TSingleton<FTaskManager, FBackgroundThreadPool> {
public:
    typedef Meta::TSingleton<FTaskManager, FBackgroundThreadPool> parent_type;

    using parent_type::Instance;
#ifdef WITH_CORE_ASSERT
    using parent_type::HasInstance;
#endif

    static void Create();
    static void Destroy();

    static TMemoryView<const size_t> ThreadAffinities();
};
//----------------------------------------------------------------------------
CORE_API void AsyncBackround(const FTaskFunc& task, ETaskPriority priority = ETaskPriority::Normal);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class CORE_API FThreadPoolStartup {
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
