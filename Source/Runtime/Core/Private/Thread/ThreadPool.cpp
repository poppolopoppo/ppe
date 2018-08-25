#include "stdafx.h"

#include "Thread/ThreadPool.h"

#include "Thread/ThreadContext.h"

#include "HAL/PlatformThread.h"

namespace PPE {

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FGlobalThreadPool::Create() {
    const auto info = FPlatformThread::GlobalThreadsInfo();
    parent_type::Create("Global", PPE_THREADTAG_WORKER, info.NumWorkers, EThreadPriority::Normal);
    parent_type::Get().Start(MakeView(info.Affinities).CutBefore(info.NumWorkers));
}
//----------------------------------------------------------------------------
void FGlobalThreadPool::Destroy() {
    parent_type::Get().Shutdown();
    parent_type::Destroy();
}
//----------------------------------------------------------------------------
void AsyncWork(FTaskFunc&& rtask, ETaskPriority priority /* = ETaskPriority::Normal */) {
    FGlobalThreadPool::Get().Run(std::move(rtask), priority);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FIOThreadPool::Create() {
    // IO should be operated in 2 threads max to prevent slow seeks :
    const auto info = FPlatformThread::IOThreadsInfo();
    parent_type::Create("IO", PPE_THREADTAG_IO, info.NumWorkers, EThreadPriority::Highest);
    parent_type::Get().Start(MakeView(info.Affinities).CutBefore(info.NumWorkers));
}
//----------------------------------------------------------------------------
void FIOThreadPool::Destroy() {
    parent_type::Get().Shutdown();
    parent_type::Destroy();
}
//----------------------------------------------------------------------------
void AsyncIO(FTaskFunc&& rtask, ETaskPriority priority /* = ETaskPriority::Normal */) {
    FIOThreadPool::Get().Run(std::move(rtask), priority);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FHighPriorityThreadPool::Create() {
    const auto info = FPlatformThread::HighPriorityThreadsInfo();
    parent_type::Create("HighPriority", PPE_THREADTAG_HIGHPRIORITY, info.NumWorkers, EThreadPriority::AboveNormal);
    parent_type::Get().Start(MakeView(info.Affinities).CutBefore(info.NumWorkers));
}
//----------------------------------------------------------------------------
void FHighPriorityThreadPool::Destroy() {
    parent_type::Get().Shutdown();
    parent_type::Destroy();
}
//----------------------------------------------------------------------------
void AsyncHighPriority(FTaskFunc&& rtask, ETaskPriority priority /* = ETaskPriority::Normal */) {
    FHighPriorityThreadPool::Get().Run(std::move(rtask), priority);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FBackgroundThreadPool::Create() {
    const auto info = FPlatformThread::BackgroundThreadsInfo();
    parent_type::Create("Background", PPE_THREADTAG_BACKGROUND, info.NumWorkers, EThreadPriority::BelowNormal);
    parent_type::Get().Start(MakeView(info.Affinities).CutBefore(info.NumWorkers));
}
//----------------------------------------------------------------------------
void FBackgroundThreadPool::Destroy() {
    parent_type::Get().Shutdown();
    parent_type::Destroy();
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
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
