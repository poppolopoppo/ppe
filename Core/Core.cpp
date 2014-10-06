#include "stdafx.h"

#include "Core.h"

#include "Allocation.h"
#include "AutoSingleton.h"
#include "Callstack.h"
#include "CrtDebug.h"
#include "CurrentProcess.h"
#include "FileSystem.h"
#include "Logger.h"
#include "ProcessTime.h"
#include "RTTI.h"
#include "ThreadContext.h"
#include "ThreadLocalStorage.h"
#include "ThreadPool.h"
#include "VirtualFileSystem.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void CoreStartup::Start(void *applicationHandle, int nShowCmd, size_t argc, const wchar_t** argv) {
    // 1 - process time
    ProcessTime::Start();
    // 2 - logger
    LoggerStartup::Start();
    // 3 - current process
    CurrentProcess::Create(applicationHandle, nShowCmd, argc, argv);
    // 4 - callstack symbols
    Callstack::Start();
    // 5 - memory guards
    GLOBAL_CHECK_MEMORY_LEAKS(true);
    // 6 - thread local storage
    ThreadLocalManager::Create();
    // 7 - main thread context
    ThreadContextStartup::Start_MainThread();
    // 8 - heap allocators
    Heaps::Process::Create(Heap::current_process_t());
    // 9 - auto singleton manager
    Meta::AutoSingletonManager::Start();
    //10 - thread pool
    ThreadPoolStartup::Start();
    //11 - filesystem
    FileSystemStartup::Start();
    //12 - virtual filesystem
    VirtualFileSystemStartup::Start();
    //13 - RTTI
    RTTI::RTTIStartup::Start();
}
//----------------------------------------------------------------------------
void CoreStartup::Shutdown() {
    //13 - RTTI
    RTTI::RTTIStartup::Shutdown();
    //12 - virtual filesystem
    VirtualFileSystemStartup::Shutdown();
    //11 - filesystem
    FileSystemStartup::Shutdown();
    //10 - thread pool
    ThreadPoolStartup::Shutdown();
    // 9 - auto singleton manager
    Meta::AutoSingletonManager::Shutdown();
    // 8 - heap allocators
    Heaps::Process::Destroy();
    // 7 - main thread context
    ThreadContextStartup::Shutdown();
    // 6 - thread local storage
    ThreadLocalManager::Destroy();
    // 5 - memory guards
    GLOBAL_CHECK_MEMORY_LEAKS(false);
    // 4 - callstack symbols
    Callstack::Shutdown();
    // 3 - current process
    CurrentProcess::Destroy();
    // 2 - logger
    LoggerStartup::Shutdown();
    // 1 - process time
    ProcessTime::Shutdown();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
