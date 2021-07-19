#include "stdafx.h"

#include "Thread/ThreadPool.h"

#include "Thread/Task/TaskFiberPool.h"
#include "Thread/ThreadContext.h"

#include "HAL/PlatformThread.h"
#include "Memory/MemoryTracking.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename _Pool>
static void CreateThreadPool_(const FStringView& name, size_t threadTag, const FPlatformThread::FThreadGroupInfo& info) {
    _Pool::Create(name, threadTag, info.NumWorkers, info.Priority);
    _Pool::Get().Start(MakeView(info.Affinities).CutBefore(info.NumWorkers));
}
//----------------------------------------------------------------------------
template <typename _Pool>
static void DestroyThreadPool_() {
    _Pool::Get().Shutdown();
    _Pool::Destroy();
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void* FGlobalThreadPool::class_singleton_storage() NOEXCEPT {
    return singleton_type::make_singleton_storage(); // for shared libs
}
//----------------------------------------------------------------------------
void FGlobalThreadPool::Create() {
    CreateThreadPool_<singleton_type>("Global", PPE_THREADTAG_WORKER, FPlatformThread::GlobalThreadsInfo());
}
//----------------------------------------------------------------------------
void FGlobalThreadPool::Destroy() {
    DestroyThreadPool_<singleton_type>();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void* FIOThreadPool::class_singleton_storage() NOEXCEPT {
    return singleton_type::make_singleton_storage(); // for shared libs
}
//----------------------------------------------------------------------------
void FIOThreadPool::Create() {
    CreateThreadPool_<singleton_type>("IO", PPE_THREADTAG_IO, FPlatformThread::IOThreadsInfo());
}
//----------------------------------------------------------------------------
void FIOThreadPool::Destroy() {
    DestroyThreadPool_<singleton_type>();
}

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void* FHighPriorityThreadPool::class_singleton_storage() NOEXCEPT {
    return singleton_type::make_singleton_storage(); // for shared libs
}
//----------------------------------------------------------------------------
void FHighPriorityThreadPool::Create() {
    CreateThreadPool_<singleton_type>("HighPriority", PPE_THREADTAG_HIGHPRIORITY, FPlatformThread::HighPriorityThreadsInfo());
}
//----------------------------------------------------------------------------
void FHighPriorityThreadPool::Destroy() {
    DestroyThreadPool_<singleton_type>();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void* FBackgroundThreadPool::class_singleton_storage() NOEXCEPT {
    return singleton_type::make_singleton_storage(); // for shared libs
}
//----------------------------------------------------------------------------
void FBackgroundThreadPool::Create() {
    CreateThreadPool_<singleton_type>("Background", PPE_THREADTAG_BACKGROUND, FPlatformThread::BackgroundThreadsInfo());
}
//----------------------------------------------------------------------------
void FBackgroundThreadPool::Destroy() {
    DestroyThreadPool_<singleton_type>();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTaskManager& GlobalThreadPool() NOEXCEPT {
    return FGlobalThreadPool::Get();
}
//----------------------------------------------------------------------------
FTaskManager& IOThreadPool() NOEXCEPT {
    return FIOThreadPool::Get();
}
//----------------------------------------------------------------------------
FTaskManager& HighPriorityThreadPool() NOEXCEPT {
    return FHighPriorityThreadPool::Get();
}
//----------------------------------------------------------------------------
FTaskManager& BackgroundThreadPool() NOEXCEPT {
    return FBackgroundThreadPool::Get();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void AsyncWork(FTaskFunc&& rtask, ETaskPriority priority /* = ETaskPriority::Normal */) {
    FGlobalThreadPool::Get().Run(std::move(rtask), priority);
}
//----------------------------------------------------------------------------
void AsyncIO(FTaskFunc&& rtask, ETaskPriority priority /* = ETaskPriority::Normal */) {
    FIOThreadPool::Get().Run(std::move(rtask), priority);
}
//----------------------------------------------------------------------------
void AsyncHighPriority(FTaskFunc&& rtask, ETaskPriority priority /* = ETaskPriority::Normal */) {
    FHighPriorityThreadPool::Get().Run(std::move(rtask), priority);
}
//----------------------------------------------------------------------------
void AsyncBackground(FTaskFunc&& rtask, ETaskPriority priority /* = ETaskPriority::Normal */) {
    FBackgroundThreadPool::Get().Run(std::move(rtask), priority);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FThreadPoolStartup::Start() {
    FGlobalThreadPool::Create();
    FIOThreadPool::Create();
    FHighPriorityThreadPool::Create();
    FBackgroundThreadPool::Create();

    DumpStats();
}
//----------------------------------------------------------------------------
void FThreadPoolStartup::Shutdown() {
    DumpStats();

    FBackgroundThreadPool::Destroy();
    FHighPriorityThreadPool::Destroy();
    FIOThreadPool::Destroy();
    FGlobalThreadPool::Destroy();

    Assert_NoAssume(MEMORYDOMAIN_TRACKING_DATA(Fibers).empty());
}
//----------------------------------------------------------------------------
void FThreadPoolStartup::DumpStats() {
    FBackgroundThreadPool::Get().DumpStats();
    FHighPriorityThreadPool::Get().DumpStats();
    FIOThreadPool::Get().DumpStats();
    FGlobalThreadPool::Get().DumpStats();
}
//----------------------------------------------------------------------------
void FThreadPoolStartup::DutyCycle() {
    FBackgroundThreadPool::Get().DutyCycle();
    FHighPriorityThreadPool::Get().DutyCycle();
    FIOThreadPool::Get().DutyCycle();
    FGlobalThreadPool::Get().DutyCycle();
}
//----------------------------------------------------------------------------
void FThreadPoolStartup::ReleaseMemory() {
    FBackgroundThreadPool::Get().ReleaseMemory();
    FHighPriorityThreadPool::Get().ReleaseMemory();
    FIOThreadPool::Get().ReleaseMemory();
    FGlobalThreadPool::Get().ReleaseMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
