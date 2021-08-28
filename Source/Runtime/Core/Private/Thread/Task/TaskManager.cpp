#include "stdafx.h"

#include "Thread/Task/TaskManager.h"

#include "Thread/ThreadContext.h"
#include "Thread/Task/CompletionPort.h"
#include "Thread/Task/TaskManagerImpl.h"

#include "Diagnostic/Logger.h"
#include "Memory/RefPtr.h"
#include "Meta/ThreadResource.h"

#if USE_PPE_LOGGER
#   include "IO/Format.h"
#   include "IO/FormatHelpers.h"
#   include "IO/TextWriter.h"
//  call-of-duty:
#   include "Maths/Units.h"
#   include "Time/Timeline.h"
#endif

#include <algorithm>
#include <condition_variable>
#include <iterator>
#include <mutex>

namespace PPE {
LOG_CATEGORY(PPE_CORE_API, Task)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
class FWorkerContext_ : Meta::FThreadResource {
public:
    explicit FWorkerContext_(FTaskManagerImpl* pimpl, size_t workerIndex);
    ~FWorkerContext_();

    FWorkerContext_(const FWorkerContext_& ) = delete;
    FWorkerContext_& operator =(const FWorkerContext_& ) = delete;

    FTaskManagerImpl& Context() const { return _pimpl; }
    const FTaskManager& Manager() const { return _pimpl.Manager(); }
    FTaskFiberLocalCache& Fibers() { return _fibers; }
    size_t WorkerIndex() const { return _workerIndex; }

    NODISCARD bool SetPostTaskDelegate(FTaskFunc&& postWork);

    static FWorkerContext_& Get();
    static void PostWork();
    static void ExitWorkerTask(ITaskContext& ctx);
    static FTaskManagerImpl& Consume(FTaskScheduler::FTaskQueued* task);

private:
    NO_INLINE void DutyCycle_() const;

    FTaskManagerImpl& _pimpl;
    FTaskFiberLocalCache _fibers;

    const size_t _workerIndex;
    FTaskFiberPool::FHandleRef _fiberToRelease;

    size_t _revision = 0;
    FTaskFunc _postWork;

    static THREAD_LOCAL FWorkerContext_* _gInstanceTLS;
};
THREAD_LOCAL FWorkerContext_* FWorkerContext_::_gInstanceTLS = nullptr;
//----------------------------------------------------------------------------
FWorkerContext_::FWorkerContext_(FTaskManagerImpl* pimpl, size_t workerIndex)
:   _pimpl(*pimpl)
,   _fibers(pimpl->Fibers())
,   _workerIndex(workerIndex)
,   _fiberToRelease(nullptr) {
    Assert(pimpl);
    Assert(nullptr == _gInstanceTLS);
    Assert_NoAssume(not FFiber::IsInFiber());

    _gInstanceTLS = this;
}
//----------------------------------------------------------------------------
FWorkerContext_::~FWorkerContext_() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(this == _gInstanceTLS);
    Assert(_fiberToRelease); // last fiber used to switch back to thread fiber
    Assert_NoAssume(not _postWork); // can't handle post work delegate here !
    Assert_NoAssume(not FFiber::IsInFiber());

    _gInstanceTLS = nullptr;

    _fibers.ReleaseFiber(_fiberToRelease);
}
//----------------------------------------------------------------------------
bool FWorkerContext_::SetPostTaskDelegate(FTaskFunc&& postWork) {
    Assert_NoAssume(postWork);
    if (_postWork.Valid())
        return false;

    // this delegate will be executed after current task execution, and more
    // importantly *AFTER* the decref, where it's safe to switch to another
    // fiber.

    _postWork = std::move(postWork);
    return true;
}
//----------------------------------------------------------------------------
FTaskManagerImpl& FWorkerContext_::Consume(FTaskScheduler::FTaskQueued* task) {
    FWorkerContext_& ctx = Get();
    ctx._pimpl.Consume(ctx._workerIndex, task);
    return ctx._pimpl;
}
//----------------------------------------------------------------------------
void FWorkerContext_::PostWork() {
    FWorkerContext_& ctx = Get();

    // trigger cleaning duty cycle every 128 calls per thread
    if ((++ctx._revision & 127) == 0)
        ctx.DutyCycle_();

    // special tasks can inject a postfix callback to avoid skipping the decref
    if (ctx._postWork) {
        // need to reset _postWork before yielding, or it could be executed
        // more than once since it's stored in the thread and not in the fiber
        const FTaskFunc postWorkCpy{ std::move(ctx._postWork) };
        Assert_NoAssume(not ctx._postWork);
        postWorkCpy(ctx._pimpl);
    }
}
//----------------------------------------------------------------------------
void FWorkerContext_::DutyCycle_() const {
    THIS_THREADRESOURCE_CHECKACCESS();

    _pimpl.DumpStats();

    CurrentThreadContext().DutyCycle();
}
//----------------------------------------------------------------------------
FWorkerContext_& FWorkerContext_::Get() {
    Assert(FFiber::IsInFiber());
    Assert(nullptr != _gInstanceTLS);
    return (*_gInstanceTLS);
}
//----------------------------------------------------------------------------
void FWorkerContext_::ExitWorkerTask(ITaskContext&) {
    Assert(FFiber::ThreadFiber() != FFiber::RunningFiber());

    const FTaskFiberPool::FHandleRef self = FTaskFiberPool::CurrentHandleRef();
    Assert(self);

    FWorkerContext_& worker = Get();
    AssertRelease(not worker._postWork);
    Assert(nullptr == worker._fiberToRelease);
    worker._fiberToRelease = self; // released in worker context destructor ^^^

    FFiber currentThread(FFiber::ThreadFiber());
    currentThread.Resume();

    AssertNotReached(); // final state of the fiber, this should never be executed !
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static void WorkerThreadLaunchpad_(FTaskManager* pmanager, size_t workerIndex, u64 affinityMask) {
    Assert(pmanager);

#if !USE_PPE_FINAL_RELEASE
#   ifdef PLATFORM_LINUX
    // debug name is limited to 16 characters with pthread
    char workerName[16];
    Format(workerName, "{0}#{1}", pmanager->Name(), workerIndex );
#   else
    char workerName[128];
    Format(workerName, "{0}_Worker_{1}_of_{2}",
        pmanager->Name(), (workerIndex + 1), pmanager->WorkerCount() );
#   endif
#else
    const char* const workerName = "";
#endif // !USE_PPE_FINAL_RELEASE
    const FThreadContextStartup threadStartup(workerName, pmanager->ThreadTag());

    threadStartup.Context().SetPriority(pmanager->Priority());
    if (affinityMask)
        threadStartup.Context().SetAffinityMask(affinityMask);

    FWorkerContext_ workerContext(pmanager->Pimpl(), workerIndex);
    {
        // switch to a fiber from the pool,
        // and resume when original thread when task manager is destroyed
        const FFiber::FThreadScope fiberScope;
        pmanager->Pimpl()->Fibers().StartThread();
    }
}
//----------------------------------------------------------------------------
// Helpers used when calling FTaskManager outside of a Fiber
FWD_REFPTR(WaitForTask_);
class FWaitForTask_ : public FRefCountable {
public:
    const FTaskFunc Task;
    std::mutex Barrier;
    std::condition_variable OnTaskFinished;
    const int NumTotal;
    int NumPending;

    explicit FWaitForTask_(FTaskFunc&& rtask, int numTotal) NOEXCEPT
    :   Task(std::move(rtask))
    ,   NumTotal(numTotal)
    ,   NumPending(NumTotal)
    {}

    static void DoNothing(ITaskContext&) { NOOP(); }

    static void Broadcast(FTaskManagerImpl& pimpl, FTaskFunc&& rtask, ETaskPriority priority) {
        Assert_NoAssume(not FFiber::IsInFiber()); // won't work if we stall a worker thread !

        std::condition_variable onTaskBroadcast;
        FWaitForTask_ waitfor{ std::move(rtask), checked_cast<int>(pimpl.Threads().size()) };

        const FTaskFunc broadcast = [&waitfor, &onTaskBroadcast](ITaskContext& ctx) {
            waitfor.Task(ctx);

            {
                Meta::FUniqueLock scopeLock(waitfor.Barrier);
                Assert_NoAssume(0 < waitfor.NumPending);

                --waitfor.NumPending;

                waitfor.OnTaskFinished.wait(scopeLock, [&]() NOEXCEPT {
                    return (0 >= waitfor.NumPending);
                });

                --waitfor.NumPending;

                if (-1 == waitfor.NumPending)
                    waitfor.OnTaskFinished.notify_all();
                if (-waitfor.NumTotal == waitfor.NumPending)
                    onTaskBroadcast.notify_one();
            }
        };
        Assert_NoAssume(broadcast.FitInSitu());

        Meta::FUniqueLock scopeLock(waitfor.Barrier);
        forrange(n, 0, waitfor.NumTotal)
            pimpl.Scheduler().Produce(priority, broadcast, nullptr);

        onTaskBroadcast.wait(scopeLock, [&waitfor]() NOEXCEPT {
            return (waitfor.NumTotal == -waitfor.NumPending);
        });
    }

    static void Wait(FTaskManagerImpl& pimpl, FTaskFunc&& rtask, ETaskPriority priority) {
        FWaitForTask_ waitfor{ std::move(rtask), 1 };

        Meta::FUniqueLock scopeLock(waitfor.Barrier);
        pimpl.Scheduler().Produce(priority, [&waitfor](ITaskContext& ctx) {
            waitfor.Task(ctx);
            {
                const Meta::FLockGuard scopeLock(waitfor.Barrier);
                Assert_NoAssume(1 == waitfor.NumPending);
                waitfor.NumPending = 0;
            }
            waitfor.OnTaskFinished.notify_all();
        },  nullptr );

        waitfor.OnTaskFinished.wait(scopeLock, [&]() NOEXCEPT {
            return (0 == waitfor.NumPending);
        });
    }

    static bool WaitFor(FTaskManagerImpl& pimpl, FTaskFunc&& rtask, ETaskPriority priority, int timeoutMS) {
        PWaitForTask_ pWaitfor{ NEW_REF(Task, FWaitForTask_, std::move(rtask), 1) };

        Meta::FUniqueLock scopeLock(pWaitfor->Barrier);
        pimpl.Scheduler().Produce(priority, [pWaitfor](ITaskContext& ctx) {
            pWaitfor->Task(ctx);
            {
                const Meta::FLockGuard scopeLock(pWaitfor->Barrier);
                Assert_NoAssume(1 == pWaitfor->NumPending);
                pWaitfor->NumPending = 0;
            }
            pWaitfor->OnTaskFinished.notify_all();
        },  nullptr);

        pWaitfor->OnTaskFinished.wait_for(scopeLock, std::chrono::milliseconds(timeoutMS), [&]() NOEXCEPT {
            return (0 == pWaitfor->NumPending);
        });

        return (0 == pWaitfor->NumPending);
    }
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTaskManagerImpl::FTaskManagerImpl(FTaskManager& manager)
:   _scheduler(manager.WorkerCount())
,   _fibers(MakeFunction<&FTaskManagerImpl::WorkerLoop_>())
,   _manager(manager) {
    _threads.reserve(_manager.WorkerCount());
#if USE_PPE_LOGGER
    _dumpStatsCooldown = FTimeline::StartNow();
#endif
}
//----------------------------------------------------------------------------
FTaskManagerImpl::~FTaskManagerImpl() {
    Assert_NoAssume(_threads.empty());
}
//----------------------------------------------------------------------------
void FTaskManagerImpl::Start(const TMemoryView<const u64>& threadAffinities) {
    const size_t n = _manager.WorkerCount();
    AssertRelease(threadAffinities.size() == n);
    Assert(_threads.empty());

    forrange(i, 0, n) {
        _threads.emplace_back(&WorkerThreadLaunchpad_, &_manager, i, threadAffinities[i]);
        Assert_NoAssume(_threads.back().joinable());
    }
}
//----------------------------------------------------------------------------
void FTaskManagerImpl::Shutdown() {
    Assert(_threads.size() == _manager.WorkerCount());

    forrange(i, 0, _threads.size())
        _scheduler.Produce(
            ETaskPriority::Internal, // lowest priority possible
            &FWorkerContext_::ExitWorkerTask, nullptr );

    for (std::thread& workerThread : _threads) {
        Assert_NoAssume(workerThread.joinable());
        workerThread.join();
    }

    _threads.clear();
}
//----------------------------------------------------------------------------
void FTaskManagerImpl::Consume(size_t workerIndex, FTaskQueued* task) {
    Assert(task);

    _scheduler.Consume(workerIndex, task);
}
//----------------------------------------------------------------------------
void FTaskManagerImpl::DumpStats() {
#if !USE_PPE_FINAL_RELEASE && USE_PPE_LOGGER
    FTimespan elapsed;
    if (not _dumpStatsCooldown.Tick_Every(5.0_s, elapsed))
        return;

    size_t reserved, used;
    _fibers.UsageStats(&reserved, &used);

    LOG(Task, Debug, L"task manager <{0}> is using {1} fibers / {2} reserved with {3} worker threads ({4}   reserved for stack)",
        _manager.Name(),
        Fmt::CountOfElements(used),
        Fmt::CountOfElements(reserved),
        Fmt::CountOfElements(_threads.size()),
        Fmt::SizeInBytes(reserved * FTaskFiberPool::ReservedStackSize()));
#endif
}
//----------------------------------------------------------------------------
size_t FTaskManagerImpl::ThreadTag() const NOEXCEPT {
    return _manager.ThreadTag();
}
//----------------------------------------------------------------------------
size_t FTaskManagerImpl::WorkerCount() const NOEXCEPT {
    return _manager.WorkerCount();
}
//----------------------------------------------------------------------------
void FTaskManagerImpl::Run(FAggregationPort& ap, FTaskFunc&& rtask, ETaskPriority priority) {
    Assert_NoAssume(rtask);

    _scheduler.Produce(priority, std::move(rtask), ap.Increment(1));
}
//----------------------------------------------------------------------------
void FTaskManagerImpl::Run(FAggregationPort& ap, const TMemoryView<FTaskFunc>& rtasks, ETaskPriority priority) {
    Assert_NoAssume(not rtasks.empty());

    _scheduler.Produce(priority, rtasks, ap.Increment(rtasks.size()));
}
//----------------------------------------------------------------------------
void FTaskManagerImpl::Run(FAggregationPort& ap, const TMemoryView<const FTaskFunc>& tasks, ETaskPriority priority) {
    Assert_NoAssume(not tasks.empty());

    _scheduler.Produce(priority, tasks, ap.Increment(tasks.size()));
}
//----------------------------------------------------------------------------
void FTaskManagerImpl::Run(FCompletionPort* cp, FTaskFunc&& rtask, ETaskPriority priority) {
    Assert_NoAssume(rtask);

    _scheduler.Produce(priority, std::move(rtask), StartPortIFN_(cp, 1));
}
//----------------------------------------------------------------------------
void FTaskManagerImpl::Run(FCompletionPort* cp, const TMemoryView<FTaskFunc>& rtasks, ETaskPriority priority) {
    Assert_NoAssume(not rtasks.empty());

    _scheduler.Produce(priority, rtasks, StartPortIFN_(cp, rtasks.size()));
}
//----------------------------------------------------------------------------
void FTaskManagerImpl::Run(FCompletionPort* cp, const TMemoryView<const FTaskFunc>& tasks, ETaskPriority priority) {
    Assert_NoAssume(not tasks.empty());

    _scheduler.Produce(priority, tasks, StartPortIFN_(cp, tasks.size()));
}
//----------------------------------------------------------------------------
void FTaskManagerImpl::WaitFor(FCompletionPort& cp, ETaskPriority priority) {
    Assert_NoAssume(FFiber::IsInFiber());

    cp.AttachCurrentFiber(FWorkerContext_::Get().Fibers(), priority);
}
//----------------------------------------------------------------------------
void FTaskManagerImpl::RunAndWaitFor(FTaskFunc&& rtask, ETaskPriority priority) {
    FCompletionPort handle;
    Run(&handle, std::move(rtask), priority);
    WaitFor(handle, priority);
}
//----------------------------------------------------------------------------
void FTaskManagerImpl::RunAndWaitFor(const TMemoryView<FTaskFunc>& rtasks, ETaskPriority priority) {
    FCompletionPort handle;
    Run(&handle, rtasks, priority);
    WaitFor(handle, priority);
}
//----------------------------------------------------------------------------
void FTaskManagerImpl::RunAndWaitFor(const TMemoryView<const FTaskFunc>& tasks, ETaskPriority priority) {
    FCompletionPort handle;
    Run(&handle, tasks, priority);
    WaitFor(handle, priority);
}
//----------------------------------------------------------------------------
void FTaskManagerImpl::RunAndWaitFor(const TMemoryView<FTaskFunc>& rtasks, FTaskFunc&& whileWaiting, ETaskPriority priority) {
    FCompletionPort handle;
    Run(&handle, rtasks, priority);
    whileWaiting(*this);
    WaitFor(handle, priority);
}
//----------------------------------------------------------------------------
FCompletionPort* FTaskManagerImpl::StartPortIFN_(FCompletionPort* phandle, size_t n) NOEXCEPT {
    Assert(n);

    if (phandle)
        phandle->Start(n);

    return phandle;
}
//----------------------------------------------------------------------------
// !! it's important to keep this function *STATIC* !!
// the scheduler is only accessed through worker context since the fibers
// can be stolen between each task manager, but FWorkerContext_ belongs to
// threads.
void FTaskManagerImpl::WorkerLoop_() {
    Assert(FFiber::IsInFiber());
    Assert(FFiber::RunningFiber() != FFiber::ThreadFiber());

    for (;;) {
        // need to destroy the task handle asap
        {
            FTaskScheduler::FTaskQueued task;
            {
                // after Invoke() we could be in another thread,
                // this is why this is a static function and why
                // ctx is protected inside this scope.

                ITaskContext& ctx = FWorkerContext_::Consume(&task);
                Assert(task.Pending.Valid());
                task.Pending.Invoke(ctx);
            }

            // Decrement the counter and resume waiting tasks if any.
            // Won't steal the thread from this fiber, so the loop can safely finish
            if (task.Port)
                task.Port.Release()/* release safe ptr before port */->OnJobComplete();
        }

        // this loop is thigh to a fiber, *NOT A THREAD*
        // so we need to ask for the current worker here
        FWorkerContext_::PostWork();
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
class FGlobalTaskContext_ final : public ITaskContext {
public:
    explicit FGlobalTaskContext_(const FTaskManager& manager) : _manager(manager) {}
    ~FGlobalTaskContext_() override = default;

    size_t ThreadTag() const NOEXCEPT override { return _manager.ThreadTag(); }
    size_t WorkerCount() const NOEXCEPT override { return _manager.WorkerCount(); }

    void Run(FAggregationPort& ag, FTaskFunc&& rtask, ETaskPriority priority) override {
        _manager.Run(ag, std::move(rtask), priority);
    }
    void Run(FAggregationPort& ag, const TMemoryView<FTaskFunc>& rtasks, ETaskPriority priority) override {
        _manager.Run(ag, rtasks, priority);
    }
    void Run(FAggregationPort& ag, const TMemoryView<const FTaskFunc>& tasks, ETaskPriority priority) override {
        _manager.Run(ag, tasks, priority);
    }

    void Run(FCompletionPort* cp, FTaskFunc&& rtask, ETaskPriority priority) override {
        Assert_NoAssume(!cp || FFiber::IsInFiber());
        _manager.Pimpl()->Run(cp, std::move(rtask), priority);
    }
    void Run(FCompletionPort* cp, const TMemoryView<FTaskFunc>& rtasks, ETaskPriority priority) override {
        Assert_NoAssume(!cp || FFiber::IsInFiber());
        _manager.Pimpl()->Run(cp, rtasks, priority);
    }
    void Run(FCompletionPort* cp, const TMemoryView<const FTaskFunc>& tasks, ETaskPriority priority) override {
        Assert_NoAssume(!cp || FFiber::IsInFiber());
        _manager.Pimpl()->Run(cp, tasks, priority);
    }
    void WaitFor(FCompletionPort& cp, ETaskPriority priority) override {
        Assert_NoAssume(FFiber::IsInFiber());
        _manager.Pimpl()->WaitFor(cp, priority);
    }

    void RunAndWaitFor(FTaskFunc&& rtask, ETaskPriority priority) override {
        _manager.RunAndWaitFor(std::move(rtask), priority);
    }
    void RunAndWaitFor(const TMemoryView<FTaskFunc>& rtasks, ETaskPriority priority) override {
        _manager.RunAndWaitFor(rtasks, priority);
    }
    void RunAndWaitFor(const TMemoryView<const FTaskFunc>& tasks, ETaskPriority priority) override {
        _manager.RunAndWaitFor(tasks, priority);
    }
    void RunAndWaitFor(const TMemoryView<FTaskFunc>& rtasks, FTaskFunc&& whileWaiting, ETaskPriority priority) override {
        auto waitfor = [rtasks, &whileWaiting, priority](ITaskContext& ctx) {
            FCompletionPort port;
            ctx.Run(&port, rtasks, priority);

            whileWaiting(ctx);

            ctx.WaitFor(port, priority);
        };

        if (FFiber::IsInFiber())
            waitfor(*this);
        else
            FWaitForTask_::Wait(*_manager.Pimpl(), std::move(waitfor), priority);
    }

private:
    const FTaskManager& _manager;
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTaskManager::FTaskManager(const FStringView& name, size_t threadTag, size_t workerCount, EThreadPriority priority)
:   _name(name)
,   _threadTag(threadTag)
,   _workerCount(workerCount)
,   _priority(priority) {
    Assert(_name.size());
    Assert(_workerCount > 0);
}
//----------------------------------------------------------------------------
FTaskManager::~FTaskManager() {
    Assert(not _pimpl);
}
//----------------------------------------------------------------------------
bool FTaskManager::IsRunning() const NOEXCEPT {
    return (!!_pimpl);
}
//----------------------------------------------------------------------------
void FTaskManager::Start() {
    STACKLOCAL_POD_ARRAY(u64, fakeThreadAffinities, _workerCount);
    std::fill(fakeThreadAffinities.begin(), fakeThreadAffinities.end(), 0);
    Start(fakeThreadAffinities);
}
//----------------------------------------------------------------------------
void FTaskManager::Start(const TMemoryView<const u64>& threadAffinities) {
    Assert(not _pimpl);

    LOG(Task, Info, L"start manager <{0}> with {1} workers and tag <{2}> ...", _name, _workerCount, _threadTag);

    _pimpl.reset(*this);
    _pimpl->Start(threadAffinities);

    _context.reset<FGlobalTaskContext_>(*this);
}
//----------------------------------------------------------------------------
void FTaskManager::Shutdown() {
    Assert(_pimpl);

    LOG(Task, Info, L"shutdown manager <#{0}> with {1} workers and tag <{2}> ...", _name, _workerCount, _threadTag);

    _context.reset();

    _pimpl->Shutdown();
    _pimpl.reset();
}
//----------------------------------------------------------------------------
void FTaskManager::Run(FTaskFunc&& rtask, ETaskPriority priority /* = ETaskPriority::Normal */) const {
    Assert(rtask);
    Assert(_pimpl);

    _pimpl->Run(nullptr, std::move(rtask), priority);
}
//----------------------------------------------------------------------------
void FTaskManager::Run(const TMemoryView<FTaskFunc>& rtasks, ETaskPriority priority /* = ETaskPriority::Normal */) const {
    Assert_NoAssume(not rtasks.empty());
    Assert(_pimpl);

    _pimpl->Run(nullptr, rtasks, priority);
}
//----------------------------------------------------------------------------
void FTaskManager::Run(const TMemoryView<const FTaskFunc>& tasks, ETaskPriority priority /* = ETaskPriority::Normal */) const {
    Assert_NoAssume(not tasks.empty());
    Assert(_pimpl);

    _pimpl->Run(nullptr, tasks, priority);
}
//----------------------------------------------------------------------------
void FTaskManager::Run(FAggregationPort& ap, FTaskFunc&& rtask, ETaskPriority priority /* = ETaskPriority::Normal */) const {
    Assert(rtask);
    Assert(_pimpl);

    _pimpl->Run(ap, std::move(rtask), priority);
}
//----------------------------------------------------------------------------
void FTaskManager::Run(FAggregationPort& ap, const TMemoryView<FTaskFunc>& rtasks, ETaskPriority priority /* = ETaskPriority::Normal */) const {
    Assert_NoAssume(not rtasks.empty());
    Assert(_pimpl);

    _pimpl->Run(ap, rtasks, priority);
}
//----------------------------------------------------------------------------
void FTaskManager::Run(FAggregationPort& ap, const TMemoryView<const FTaskFunc>& tasks, ETaskPriority priority /* = ETaskPriority::Normal */) const {
    Assert_NoAssume(not tasks.empty());
    Assert(_pimpl);

    _pimpl->Run(ap, tasks, priority);
}
//----------------------------------------------------------------------------
void FTaskManager::RunAndWaitFor(FTaskFunc&& rtask, ETaskPriority priority /* = ETaskPriority::Normal */) const {
    Assert_NoAssume(rtask);
    Assert(_pimpl);

    if (FFiber::IsInFiber())
        _pimpl->RunAndWaitFor(std::move(rtask), priority);
    else
        FWaitForTask_::Wait(*_pimpl, std::move(rtask), priority);
}
//----------------------------------------------------------------------------
void FTaskManager::RunAndWaitFor(const TMemoryView<FTaskFunc>& rtasks, ETaskPriority priority /* = ETaskPriority::Normal */) const {
    Assert_NoAssume(not rtasks.empty());
    Assert(_pimpl);

    if (FFiber::IsInFiber())
        _pimpl->RunAndWaitFor(rtasks, priority);
    else
        FWaitForTask_::Wait(*_pimpl, [rtasks, priority](ITaskContext& ctx) {
            ctx.RunAndWaitFor(rtasks, priority);
        },  priority );
}
//----------------------------------------------------------------------------
void FTaskManager::RunAndWaitFor(const TMemoryView<FTaskFunc>& rtasks, const FTaskFunc& whileWaiting, ETaskPriority priority/* = ETaskPriority::Normal */) const {
    Assert_NoAssume(not FFiber::IsInFiber());
    Assert_NoAssume(not rtasks.empty());
    Assert(whileWaiting);
    Assert(_pimpl);

    auto waitfor = [rtasks, &whileWaiting, priority](ITaskContext& ctx) {
        FCompletionPort port;
        ctx.Run(&port, rtasks, priority);

        whileWaiting(ctx);

        ctx.WaitFor(port, priority);
    };

    if (FFiber::IsInFiber())
        waitfor(*_pimpl);
    else
        FWaitForTask_::Wait(*_pimpl, std::move(waitfor), priority);
}
//----------------------------------------------------------------------------
void FTaskManager::RunAndWaitFor(const TMemoryView<const FTaskFunc>& tasks, ETaskPriority priority /* = ETaskPriority::Normal */) const {
    Assert_NoAssume(not tasks.empty());
    Assert(_pimpl);

    auto waitfor = [tasks, priority](ITaskContext& ctx) {
        ctx.RunAndWaitFor(tasks, priority);
    };

    if (FFiber::IsInFiber())
        waitfor(*_pimpl);
    else
        FWaitForTask_::Wait(*_pimpl, std::move(waitfor), priority);
}
//----------------------------------------------------------------------------
void FTaskManager::RunInWorker(FTaskFunc&& rtask, ETaskPriority priority /* = ETaskPriority::Normal */) const {
    if (Likely(FFiber::IsInFiber() && &FWorkerContext_::Get().Manager() == this))
        rtask(*_pimpl); // run inline when we're already in of current pool's workers
    else
        RunAndWaitFor(std::move(rtask), priority);
}
//----------------------------------------------------------------------------
void FTaskManager::BroadcastAndWaitFor(FTaskFunc&& rtask, ETaskPriority priority/* = ETaskPriority::Normal */) const {
    Assert_NoAssume(rtask);
    Assert(_pimpl);

    if (FFiber::IsInFiber())
        AssertNotImplemented(); // not easily done inside fibers because of mutex locking used atm
    else
        FWaitForTask_::Broadcast(*_pimpl, std::move(rtask), priority);
}
//----------------------------------------------------------------------------
void FTaskManager::WaitForAll() const {
    Assert(_pimpl);

    if (FFiber::IsInFiber())
        AssertNotImplemented(); // not easily done inside fibers because of mutex locking used atm
    else
        FWaitForTask_::Broadcast(*_pimpl, &FWaitForTask_::DoNothing, ETaskPriority::Internal);
}
//----------------------------------------------------------------------------
bool FTaskManager::WaitForAll(int timeoutMS) const {
    Assert(_pimpl);

    if (FFiber::IsInFiber())
        _pimpl->RunAndWaitFor(&FWaitForTask_::DoNothing, ETaskPriority::Internal);
    else
        FWaitForTask_::WaitFor(*_pimpl, &FWaitForTask_::DoNothing, ETaskPriority::Internal, timeoutMS);

    return _pimpl->Scheduler().HasPendingTask();
}
//----------------------------------------------------------------------------
void FTaskManager::DumpStats() {
    if (_pimpl)
        _pimpl->DumpStats();
}
//----------------------------------------------------------------------------
void FTaskManager::DutyCycle() {
    if (not _pimpl)
        return;

    BroadcastAndWaitFor(
        [](ITaskContext&) {
            CurrentThreadContext().DutyCycle();
        },
        ETaskPriority::High/* highest priority, to avoid block waiting for all the jobs queued before */);
}
//----------------------------------------------------------------------------
void FTaskManager::ReleaseMemory() {
    if (not _pimpl)
        return;

#if USE_PPE_LOGGER
    size_t reserved, used;
    _pimpl->Fibers().UsageStats(&reserved, &used);

    LOG(Task, Debug, L"before release memory in task manager <{0}>, using {1} fibers / {2} reserved ({3} reserved for stack)",
        Name(),
        Fmt::CountOfElements(used),
        Fmt::CountOfElements(reserved),
        Fmt::SizeInBytes(reserved * FTaskFiberPool::ReservedStackSize()));
#endif

    _pimpl->Fibers().ReleaseMemory(); // get most memory at the top

    BroadcastAndWaitFor(
        [](ITaskContext&) {
            FWorkerContext_& worker = FWorkerContext_::Get();
            // empty fibers thread local cache
            worker.Fibers().ReleaseMemory();
            // defragment malloc TLS caches
            malloc_release_cache_memory();
            // defragment fiber pool by releasing current fiber and getting new ones (hopefully from first chunk)
            FTaskFunc func([](ITaskContext&) {
                FTaskFiberPool::CurrentHandleRef()->YieldFiber(nullptr, true);
            });
            Assert_NoAssume(func.FitInSitu());
            if (not worker.SetPostTaskDelegate(std::move(func)))
                AssertNotReached();
        },
        ETaskPriority::High/* highest priority, to avoid block waiting for all the jobs queued before */);

    _pimpl->Fibers().ReleaseMemory(); // release defragmented blocks
    _pimpl->Scheduler().ReleaseMemory(); // release worker queues IFP

#if USE_PPE_LOGGER
    _pimpl->Fibers().UsageStats(&reserved, &used);

    LOG(Task, Debug, L"after release memory in task manager <{0}>, using {1} fibers / {2} reserved ({3} reserved for stack)",
        Name(),
        Fmt::CountOfElements(used),
        Fmt::CountOfElements(reserved),
        Fmt::SizeInBytes(reserved * FTaskFiberPool::ReservedStackSize()));
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ITaskContext& ITaskContext::Get() NOEXCEPT {
    // need to be called from a worker thread ! (asserted by FWorkerContext_)
    const FTaskManager& manager = FWorkerContext_::Get().Manager();
    FTaskManagerImpl* const pimpl = manager.Pimpl();
    Assert(pimpl);
    return *pimpl;
}
//----------------------------------------------------------------------------
FTaskFunc FInterruptedTask::ResumeTask(const FInterruptedTask& task) {
    return [resume{ static_cast<FTaskFiberPool::FHandleRef>(task.Fiber()) }](ITaskContext&) {
        FTaskFiberPool::CurrentHandleRef()->YieldFiber(resume, true/* release current fiber */);
    };
}
//----------------------------------------------------------------------------
void FInterruptedTask::Resume(const TMemoryView<FInterruptedTask>& tasks) {
    Assert(not tasks.empty());

    // sort all queued fibers by priority before resuming and outside of the lock
    // (no need for stable sort since each task should have a unique priority)
    std::sort(tasks.begin(), tasks.end());

    bool postTaskAvailable = false;
    FWorkerContext_* workerIFP = nullptr;
    FTaskManagerImpl* workerCtx = nullptr;
    if (FFiber::IsInFiber()) {
        postTaskAvailable = true;
        workerIFP = &FWorkerContext_::Get();
        workerCtx = &workerIFP->Context();
    }
    Assert(not workerIFP || workerCtx);

    for (const FInterruptedTask& task : tasks) {
        FTaskFunc func{ ResumeTask(task) };
        Assert_NoAssume(func.FitInSitu());

        // it's faster when using PostTaskDelegate versus spawning a new fiber,
        // but there's only one task that can benefit from this : we can't stole
        // the current fiber more than once.
        // also we can't use PostTaskDelegate when the stalled fiber belongs to
        // another task manager.

        if ((postTaskAvailable & (task.Context() == workerCtx)) == false ||
            (postTaskAvailable = workerIFP->SetPostTaskDelegate(std::move(func))) == false ) {
            Assert_NoAssume(func);

            // stalled fibers are resumed through a task to let the current fiber dispatch
            // all jobs to every worker thread before yielding (won't steal execution from current fiber)

            task.Context()->FireAndForget(std::move(func), task.Priority());
        }
    }

    Assert_NoAssume(not workerIFP || workerIFP == &FWorkerContext_::Get()); // didn't change thread
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
