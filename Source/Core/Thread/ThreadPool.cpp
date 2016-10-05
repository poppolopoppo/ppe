#include "stdafx.h"

#include "ThreadPool.h"

#include "Diagnostic/LastError.h"
#include "ThreadContext.h"

#include <thread>

#ifdef OS_WINDOWS
#   include <windows.h>
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
STATIC_CONST_INTEGRAL(size_t, MaxGlobalWorkerCount_,    6);
STATIC_CONST_INTEGRAL(size_t, MinGlobalWorkerCount_,    2);
static size_t GlobalWorkerCount_() {
    return Max(MinGlobalWorkerCount_,
        Min(MaxGlobalWorkerCount_, std::thread::hardware_concurrency() - 2));
}
//----------------------------------------------------------------------------
STATIC_CONST_INTEGRAL(size_t, MaxIOWorkerCount_,        2);
STATIC_CONST_INTEGRAL(size_t, MinIOWorkerCount_,        1);
static size_t IOWorkerCount_() {
    return Max(MinIOWorkerCount_,
        Min(MaxIOWorkerCount_, std::thread::hardware_concurrency() - GlobalWorkerCount_()));
}
//----------------------------------------------------------------------------
static const size_t GlobalWorkerThreadAffinities[MaxGlobalWorkerCount_] = {
    1<<2, 1<<3, 1<<4, 1<<5, 1<<6, 1<<7 // from 3rd to 8th core
};
//----------------------------------------------------------------------------
static const size_t IOWorkerThreadAffinities[MaxIOWorkerCount_] = {
    (1<<0)|(1<<1), (1<<0)|(1<<1) // 1th and 2nd core, allowed to change threads
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FGlobalThreadPool::Create() {
    const size_t count = GlobalWorkerCount_();
    parent_type::Create("FGlobalThreadPool", CORE_THREADTAG_WORKER, count);
    parent_type::Instance().Start(MakeView(GlobalWorkerThreadAffinities).CutBefore(count));
}
//----------------------------------------------------------------------------
void FGlobalThreadPool::Destroy() {
    parent_type::Instance().Shutdown();
    parent_type::Destroy();
}
//----------------------------------------------------------------------------
void AsyncWork(const TaskDelegate& task, ETaskPriority priority /* = ETaskPriority::Normal */) {
    FGlobalThreadPool::Instance().Run(task, priority);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void IOThreadPool::Create() {
    // IO should be operated in 2 threads max to prevent slow seeks :
    const size_t count = IOWorkerCount_();
    parent_type::Create("IOThreadPool", CORE_THREADTAG_IO, count);
    parent_type::Instance().Start(MakeView(IOWorkerThreadAffinities).CutBefore(count));
}
//----------------------------------------------------------------------------
void IOThreadPool::Destroy() {
    parent_type::Instance().Shutdown();
    parent_type::Destroy();
}
//----------------------------------------------------------------------------
void AsyncIO(const TaskDelegate& task, ETaskPriority priority /* = ETaskPriority::Normal */) {
    IOThreadPool::Instance().Run(task, priority);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FThreadPoolStartup::Start() {
    FGlobalThreadPool::Create();
    IOThreadPool::Create();
}
//----------------------------------------------------------------------------
void FThreadPoolStartup::Shutdown() {
    IOThreadPool::Destroy();
    FGlobalThreadPool::Destroy();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
