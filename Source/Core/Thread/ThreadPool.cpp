#include "stdafx.h"

#include "ThreadPool.h"

#include "Diagnostic/LastError.h"
#include "ThreadContext.h"

#include <thread>

#ifdef PLATFORM_WINDOWS
#   include "Misc/Platform_Windows.h"
#endif

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
// Worker threads are locked to cores
// * Avoid context switches and unwanted core switching
// * Kernel threads can otherwise cause ripple effects across the cores
// http://www.benicourt.com/blender/wp-content/uploads/2015/03/parallelizing_the_naughty_dog_engine_using_fibers.pdf
//----------------------------------------------------------------------------
// For common tasks, keep main thread and second thread out of pool
STATIC_CONST_INTEGRAL(size_t, MaxGlobalWorkerCount_,    10);
STATIC_CONST_INTEGRAL(size_t, MinGlobalWorkerCount_,    2 );
static size_t GlobalWorkerCount_() {
    return Max(MinGlobalWorkerCount_,
        Min(MaxGlobalWorkerCount_, std::thread::hardware_concurrency() - 2));
}
//----------------------------------------------------------------------------
// For IO tasks, high priority but uses only the 2 last cores
STATIC_CONST_INTEGRAL(size_t, MaxIOWorkerCount_,        2 );
STATIC_CONST_INTEGRAL(size_t, MinIOWorkerCount_,        1 );
static size_t IOWorkerCount_() {
    return Max(MinIOWorkerCount_,
        Min(MaxIOWorkerCount_, std::thread::hardware_concurrency() - GlobalWorkerCount_()));
}
//----------------------------------------------------------------------------
// For blocking tasks which need all available cores
static size_t HighPriorityWorkerCount_() {
    return std::thread::hardware_concurrency();
}
//----------------------------------------------------------------------------
// For slow and infrequent tasks running in background only one core (not locked, low priority)
static size_t BackgroundWorkerCount_() {
    return 1;
}
//----------------------------------------------------------------------------
static constexpr size_t GlobalWorkerThreadAffinities[] = {
    1<<2, 1<<3, 1<<4, 1<<5, 1<<6, 1<<7, 1<<8, 1<<9, 1<<10, 1<<11 // from 3rd to 12th core
};
//----------------------------------------------------------------------------
static constexpr size_t IOWorkerThreadAffinities[] = {
    (1<<0)|(1<<1), (1<<0)|(1<<1), (1<<0)|(1<<1), (1<<0)|(1<<1) // 1th and 2nd core, allowed to change threads
};
//----------------------------------------------------------------------------
static constexpr size_t BackgroundWorkerThreadAffinities[] = {
    0xFF - 1 // all cores except first
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FGlobalThreadPool::Create() {
    const size_t count = GlobalWorkerCount_();
    parent_type::Create("GlobalThreadPool", CORE_THREADTAG_WORKER, count, EThreadPriority::Normal);
    parent_type::Get().Start(MakeView(GlobalWorkerThreadAffinities).CutBefore(count));
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
    const size_t count = IOWorkerCount_();
    parent_type::Create("IOThreadPool", CORE_THREADTAG_IO, count, EThreadPriority::Highest);
    parent_type::Get().Start(MakeView(IOWorkerThreadAffinities).CutBefore(count));
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
    const size_t count = HighPriorityWorkerCount_();
    parent_type::Create("HighPriorityThreadPool", CORE_THREADTAG_HIGHPRIORITY, count, EThreadPriority::AboveNormal);

    STACKLOCAL_POD_ARRAY(size_t, affinities, count);
    forrange(i, 0, count)
        affinities[i] = (size_t(1) << i);

    parent_type::Get().Start(affinities);
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
    const size_t count = BackgroundWorkerCount_();
    parent_type::Create("BackgroundThreadPool", CORE_THREADTAG_BACKGROUND, count, EThreadPriority::BelowNormal);
    parent_type::Get().Start(MakeView(BackgroundWorkerThreadAffinities).CutBefore(count));
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
} //!namespace Core
