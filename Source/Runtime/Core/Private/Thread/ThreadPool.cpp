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
void FGlobalThreadPool::Create() {
    CreateThreadPool_<parent_type>("Global", PPE_THREADTAG_WORKER, FPlatformThread::GlobalThreadsInfo());
}
//----------------------------------------------------------------------------
void FGlobalThreadPool::Destroy() {
    DestroyThreadPool_<parent_type>();
}
//----------------------------------------------------------------------------
void AsyncWork(FTaskFunc&& rtask, ETaskPriority priority /* = ETaskPriority::Normal */) {
    FGlobalThreadPool::Get().Run(std::move(rtask), priority);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FIOThreadPool::Create() {
    CreateThreadPool_<parent_type>("IO", PPE_THREADTAG_IO, FPlatformThread::IOThreadsInfo());
}
//----------------------------------------------------------------------------
void FIOThreadPool::Destroy() {
    DestroyThreadPool_<parent_type>();
}
//----------------------------------------------------------------------------
void AsyncIO(FTaskFunc&& rtask, ETaskPriority priority /* = ETaskPriority::Normal */) {
    FIOThreadPool::Get().Run(std::move(rtask), priority);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FHighPriorityThreadPool::Create() {
    CreateThreadPool_<parent_type>("HighPriority", PPE_THREADTAG_HIGHPRIORITY, FPlatformThread::HighPriorityThreadsInfo());
}
//----------------------------------------------------------------------------
void FHighPriorityThreadPool::Destroy() {
    DestroyThreadPool_<parent_type>();
}
//----------------------------------------------------------------------------
void AsyncHighPriority(FTaskFunc&& rtask, ETaskPriority priority /* = ETaskPriority::Normal */) {
    FHighPriorityThreadPool::Get().Run(std::move(rtask), priority);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FBackgroundThreadPool::Create() {
    CreateThreadPool_<parent_type>("Background", PPE_THREADTAG_BACKGROUND, FPlatformThread::BackgroundThreadsInfo());
}
//----------------------------------------------------------------------------
void FBackgroundThreadPool::Destroy() {
    DestroyThreadPool_<parent_type>();
}
//----------------------------------------------------------------------------
void AsyncBackround(FTaskFunc&& rtask, ETaskPriority priority /* = ETaskPriority::Normal */) {
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
}
//----------------------------------------------------------------------------
void FThreadPoolStartup::Shutdown() {
    FBackgroundThreadPool::Destroy();
    FHighPriorityThreadPool::Destroy();
    FIOThreadPool::Destroy();
    FGlobalThreadPool::Destroy();

    Assert_NoAssume(MEMORYDOMAIN_TRACKING_DATA(Fibers).empty());
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
