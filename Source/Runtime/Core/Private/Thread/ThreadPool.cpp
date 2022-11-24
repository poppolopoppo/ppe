// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Thread/ThreadPool.h"

#include "Thread/Task/TaskFiberPool.h"
#include "Thread/ThreadContext.h"

#include "HAL/PlatformThread.h"
#include "Memory/MemoryTracking.h"
#include "Thread/Task/CompletionPort.h"

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
class FImmediateTaskContext_ final : public ITaskContext {
public:
    static FImmediateTaskContext_& Get() NOEXCEPT {
        ONE_TIME_DEFAULT_INITIALIZE(FImmediateTaskContext_, GInstance);
        return GInstance;
    }

    ~FImmediateTaskContext_() override = default;

    size_t ThreadTag() const NOEXCEPT override { return CurrentThreadContext().Tag(); }
    size_t WorkerCount() const NOEXCEPT override { return 1; }

    void Run(FAggregationPort& , FTaskFunc&& rtask, ETaskPriority ) override {
        rtask(*this);
    }
    void Run(FAggregationPort& , const TMemoryView<FTaskFunc>& rtasks, ETaskPriority ) override {
        for (auto& task : rtasks)
            task(*this);
    }
    void Run(FAggregationPort& , const TMemoryView<const FTaskFunc>& tasks, ETaskPriority ) override {
        for (const auto& task : tasks)
            task(*this);
    }

    void Run(FCompletionPort* , FTaskFunc&& rtask, ETaskPriority ) override {
        rtask(*this);
    }
    void Run(FCompletionPort* , const TMemoryView<FTaskFunc>& rtasks, ETaskPriority ) override {
        for (auto& task : rtasks)
            task(*this);
    }
    void Run(FCompletionPort* , const TMemoryView<const FTaskFunc>& tasks, ETaskPriority ) override {
        for (auto& task : tasks)
            task(*this);
    }
    void WaitFor(FCompletionPort& cp, ETaskPriority ) override {
        cp.Start(1);
        cp.OnJobComplete();
        AssertRelease(cp.Finished());
    }

    void RunAndWaitFor(FTaskFunc&& rtask, ETaskPriority ) override {
        rtask(*this);
    }
    void RunAndWaitFor(const TMemoryView<FTaskFunc>& rtasks, ETaskPriority ) override {
        for (auto& task : rtasks)
            task(*this);
    }
    void RunAndWaitFor(const TMemoryView<const FTaskFunc>& tasks, ETaskPriority ) override {
        for (auto& task : tasks)
            task(*this);
    }
    void RunAndWaitFor(const TMemoryView<FTaskFunc>& rtasks, FTaskFunc&& whileWaiting, ETaskPriority) override {
        for (auto& task : rtasks)
            task(*this);

        whileWaiting(*this);
    }

};
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
void* FSyscallThreadPool::class_singleton_storage() NOEXCEPT {
    return singleton_type::make_singleton_storage(); // for shared libs
}
//----------------------------------------------------------------------------
void FSyscallThreadPool::Create() {
    CreateThreadPool_<singleton_type>("Syscall", PPE_THREADTAG_SYSCALL, {
        1, EThreadPriority::Lowest,
        { FPlatformThread::SecondaryThreadAffinity() }
    });
}
//----------------------------------------------------------------------------
void FSyscallThreadPool::Destroy() {
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
ITaskContext* ImmediateTaskContext() NOEXCEPT {
    return std::addressof(FImmediateTaskContext_::Get());
}
//----------------------------------------------------------------------------
ITaskContext* GlobalTaskContext() NOEXCEPT {
    return FIOThreadPool::Get().GlobalContext();
}
//----------------------------------------------------------------------------
ITaskContext* IOTaskContext() NOEXCEPT {
    return FGlobalThreadPool::Get().GlobalContext();
}
//----------------------------------------------------------------------------
ITaskContext* HighPriorityTaskContext() NOEXCEPT {
    return FHighPriorityThreadPool::Get().GlobalContext();
}
//----------------------------------------------------------------------------
ITaskContext* BackgroundTaskContext() NOEXCEPT {
    return FBackgroundThreadPool::Get().GlobalContext();
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
void AsyncSyscall(FTaskFunc&& rtask, ETaskPriority priority /* = ETaskPriority::Normal */) {
    FSyscallThreadPool::Get().Run(std::move(rtask), priority);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FThreadPoolStartup::Start() {
    FGlobalThreadPool::Create();
    FIOThreadPool::Create();
    FHighPriorityThreadPool::Create();
    FBackgroundThreadPool::Create();
    FSyscallThreadPool::Create();

    DumpStats();
}
//----------------------------------------------------------------------------
void FThreadPoolStartup::Shutdown() {
    DumpStats();

    FSyscallThreadPool::Destroy();
    FBackgroundThreadPool::Destroy();
    FHighPriorityThreadPool::Destroy();
    FIOThreadPool::Destroy();
    FGlobalThreadPool::Destroy();

    Assert_NoAssume(MEMORYDOMAIN_TRACKING_DATA(Fibers).empty());
}
//----------------------------------------------------------------------------
void FThreadPoolStartup::DumpStats() {
    FSyscallThreadPool::Get().DumpStats();
    FBackgroundThreadPool::Get().DumpStats();
    FHighPriorityThreadPool::Get().DumpStats();
    FIOThreadPool::Get().DumpStats();
    FGlobalThreadPool::Get().DumpStats();
}
//----------------------------------------------------------------------------
void FThreadPoolStartup::DutyCycle() {
    FSyscallThreadPool::Get().DutyCycle();
    FBackgroundThreadPool::Get().DutyCycle();
    FHighPriorityThreadPool::Get().DutyCycle();
    FIOThreadPool::Get().DutyCycle();
    FGlobalThreadPool::Get().DutyCycle();
}
//----------------------------------------------------------------------------
void FThreadPoolStartup::ReleaseMemory() {
    FSyscallThreadPool::Get().ReleaseMemory();
    FBackgroundThreadPool::Get().ReleaseMemory();
    FHighPriorityThreadPool::Get().ReleaseMemory();
    FIOThreadPool::Get().ReleaseMemory();
    FGlobalThreadPool::Get().ReleaseMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
