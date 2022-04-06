#pragma once

#include "Core.h"

#include "Meta/Singleton.h"
#include "Thread/Task_fwd.h"
#include "Thread/Task/TaskManager.h"

#include <chrono>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FGlobalThreadPool : Meta::TSingleton<FTaskManager, FGlobalThreadPool> {
    friend class Meta::TSingleton<FTaskManager, FGlobalThreadPool>;
    using singleton_type = Meta::TSingleton<FTaskManager, FGlobalThreadPool>;
    static DLL_NOINLINE void* class_singleton_storage() NOEXCEPT;

public:
    using singleton_type::Get;
#if USE_PPE_ASSERT
    using singleton_type::HasInstance;
#endif

    static void Create();
    static void Destroy();
};
//----------------------------------------------------------------------------
class PPE_CORE_API FIOThreadPool : Meta::TSingleton<FTaskManager, FIOThreadPool> {
    friend class Meta::TSingleton<FTaskManager, FIOThreadPool>;
    using singleton_type = Meta::TSingleton<FTaskManager, FIOThreadPool>;
    static DLL_NOINLINE void* class_singleton_storage() NOEXCEPT;

public:
    using singleton_type::Get;
#if USE_PPE_ASSERT
    using singleton_type::HasInstance;
#endif

    static void Create();
    static void Destroy();
};
//----------------------------------------------------------------------------
class PPE_CORE_API FHighPriorityThreadPool : Meta::TSingleton<FTaskManager, FHighPriorityThreadPool> {
    friend class Meta::TSingleton<FTaskManager, FHighPriorityThreadPool>;
    using singleton_type = Meta::TSingleton<FTaskManager, FHighPriorityThreadPool>;
    static DLL_NOINLINE void* class_singleton_storage() NOEXCEPT;

public:
    using singleton_type::Get;
#if USE_PPE_ASSERT
    using singleton_type::HasInstance;
#endif

    static void Create();
    static void Destroy();
};
//----------------------------------------------------------------------------
class PPE_CORE_API FBackgroundThreadPool : Meta::TSingleton<FTaskManager, FBackgroundThreadPool> {
    friend class Meta::TSingleton<FTaskManager, FBackgroundThreadPool>;
    using singleton_type = Meta::TSingleton<FTaskManager, FBackgroundThreadPool>;
    static DLL_NOINLINE void* class_singleton_storage() NOEXCEPT;

public:
    using singleton_type::Get;
#if USE_PPE_ASSERT
    using singleton_type::HasInstance;
#endif

    static void Create();
    static void Destroy();
};
//----------------------------------------------------------------------------
class PPE_CORE_API FSyscallThreadPool : Meta::TSingleton<FTaskManager, FSyscallThreadPool> {
    friend class Meta::TSingleton<FTaskManager, FSyscallThreadPool>;
    using singleton_type = Meta::TSingleton<FTaskManager, FSyscallThreadPool>;
    static DLL_NOINLINE void* class_singleton_storage() NOEXCEPT;

public:
    using singleton_type::Get;
#if USE_PPE_ASSERT
    using singleton_type::HasInstance;
#endif

    static void Create();
    static void Destroy();
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// same than Task_fwd.h, but optional parameter definition
PPE_CORE_API void AsyncWork(FTaskFunc&& rtask, ETaskPriority priority = ETaskPriority::Normal);
PPE_CORE_API void AsyncIO(FTaskFunc&& rtask, ETaskPriority priority = ETaskPriority::Normal);
PPE_CORE_API void AsyncHighPriority(FTaskFunc&& rtask, ETaskPriority priority = ETaskPriority::Normal);
PPE_CORE_API void AsyncBackground(FTaskFunc&& rtask, ETaskPriority priority = ETaskPriority::Normal);
PPE_CORE_API void AsyncSyscall(FTaskFunc&& rtask, ETaskPriority priority = ETaskPriority::Normal);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FThreadPoolStartup {
public:
    static void Start();
    static void Shutdown();

    static void DumpStats();

    static void DutyCycle();
    static void ReleaseMemory();

    FThreadPoolStartup() { Start(); }
    ~FThreadPoolStartup() { Shutdown(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
