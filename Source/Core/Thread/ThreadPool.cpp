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
STATIC_CONST_INTEGRAL(size_t, MaxGlobalWorkerCount_,    10);
STATIC_CONST_INTEGRAL(size_t, MinGlobalWorkerCount_,    2 );
static size_t GlobalWorkerCount_() {
    return Max(MinGlobalWorkerCount_,
        Min(MaxGlobalWorkerCount_, std::thread::hardware_concurrency() - 2));
}
//----------------------------------------------------------------------------
STATIC_CONST_INTEGRAL(size_t, MaxIOWorkerCount_,        2 );
STATIC_CONST_INTEGRAL(size_t, MinIOWorkerCount_,        1 );
static size_t IOWorkerCount_() {
    return Max(MinIOWorkerCount_,
        Min(MaxIOWorkerCount_, std::thread::hardware_concurrency() - GlobalWorkerCount_()));
}
//----------------------------------------------------------------------------
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
    parent_type::Instance().Start(ThreadAffinities().CutBefore(count));
}
//----------------------------------------------------------------------------
void FGlobalThreadPool::Destroy() {
    parent_type::Instance().Shutdown();
    parent_type::Destroy();
}
//----------------------------------------------------------------------------
TMemoryView<const size_t> FGlobalThreadPool::ThreadAffinities() {
    return MakeView(GlobalWorkerThreadAffinities);
}
//----------------------------------------------------------------------------
void AsyncWork(const FTaskFunc& task, ETaskPriority priority /* = ETaskPriority::Normal */) {
    FGlobalThreadPool::Instance().Run(task, priority);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FIOThreadPool::Create() {
    // IO should be operated in 2 threads max to prevent slow seeks :
    const size_t count = IOWorkerCount_();
    parent_type::Create("IOThreadPool", CORE_THREADTAG_IO, count, EThreadPriority::AboveNormal);
    parent_type::Instance().Start(ThreadAffinities().CutBefore(count));
}
//----------------------------------------------------------------------------
void FIOThreadPool::Destroy() {
    parent_type::Instance().Shutdown();
    parent_type::Destroy();
}
//----------------------------------------------------------------------------
TMemoryView<const size_t> FIOThreadPool::ThreadAffinities() {
    return MakeView(IOWorkerThreadAffinities);
}
//----------------------------------------------------------------------------
void AsyncIO(const FTaskFunc& task, ETaskPriority priority /* = ETaskPriority::Normal */) {
    FIOThreadPool::Instance().Run(task, priority);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FBackgroundThreadPool::Create() {
    const size_t count = BackgroundWorkerCount_();
    parent_type::Create("BackgroundThreadPool", CORE_THREADTAG_BACKGROUND, count, EThreadPriority::BelowNormal);
    parent_type::Instance().Start(ThreadAffinities().CutBefore(count));
}
//----------------------------------------------------------------------------
void FBackgroundThreadPool::Destroy() {
    parent_type::Instance().Shutdown();
    parent_type::Destroy();
}
//----------------------------------------------------------------------------
TMemoryView<const size_t> FBackgroundThreadPool::ThreadAffinities() {
    return MakeView(BackgroundWorkerThreadAffinities);
}
//----------------------------------------------------------------------------
void AsyncBackround(const FTaskFunc& task, ETaskPriority priority /* = ETaskPriority::Normal */) {
    FBackgroundThreadPool::Instance().Run(task, priority);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FThreadPoolStartup::Start() {
    FGlobalThreadPool::Create();
    FIOThreadPool::Create();
    FBackgroundThreadPool::Create();
}
//----------------------------------------------------------------------------
void FThreadPoolStartup::Shutdown() {
    FBackgroundThreadPool::Destroy();
    FIOThreadPool::Destroy();
    FGlobalThreadPool::Destroy();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
