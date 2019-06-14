#include "stdafx.h"

#include "Thread/Task/TaskManager.h"

#include "Thread/ThreadContext.h"
#include "Thread/Task/CompletionPort.h"
#include "Thread/Task/TaskManagerImpl.h"

#include "Diagnostic/Logger.h"
#include "HAL/PlatformMemory.h"
#include "Meta/ThreadResource.h"

#if USE_PPE_LOGGER
#   include "IO/Format.h"
#   include "IO/FormatHelpers.h"
#   include "IO/TextWriter.h"
#endif

#include <algorithm>
#include <condition_variable>
#include <mutex>

namespace PPE {
LOG_CATEGORY(PPE_CORE_API, Task)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
// This global variable sets the maximum capacity of task manager queue,
// - when using classic concurrent queue this is the *total* capacity
// - when using work stealing queue you have to multiply this number by the
//   number of workers.
STATIC_CONST_INTEGRAL(size_t, GTaskManagerQueueCapacity, 128);
//----------------------------------------------------------------------------
class FWorkerContext_ : Meta::FThreadResource {
public:
    explicit FWorkerContext_(FTaskManagerImpl* pimpl, size_t workerIndex);
    ~FWorkerContext_();

    FWorkerContext_(const FWorkerContext_& ) = delete;
    FWorkerContext_& operator =(const FWorkerContext_& ) = delete;

    ITaskContext& Context() const { return _pimpl; }
    const FTaskManager& Manager() const { return _pimpl.Manager(); }
    FTaskFiberLocalCache& Fibers() { return _fibers; }
    size_t WorkerIndex() const { return _workerIndex; }

    NODISCARD bool SetPostTaskDelegate(FTaskFunc&& postWork);

    static FWorkerContext_& Get();
    static void PostWork();
    static void ExitWorkerTask(ITaskContext& ctx);
    static ITaskContext& Consume(FTaskScheduler::FTaskQueued* task);

private:
    NO_INLINE void DutyCycle_();

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
ITaskContext& FWorkerContext_::Consume(FTaskScheduler::FTaskQueued* task) {
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
        // need to reset _afterTask before yielding, or it could be executed
        // more than once since it's stored in the thread and not in the fiber
        const FTaskFunc postWorkCpy{ std::move(ctx._postWork) };
        Assert_NoAssume(not ctx._postWork);
        postWorkCpy(ctx._pimpl);
    }
}
//----------------------------------------------------------------------------
void FWorkerContext_::DutyCycle_() {
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
    char workerName[128];
    Format(workerName, "{0}_Worker_{1}_of_{2}",
        pmanager->Name(), (workerIndex + 1), pmanager->WorkerCount() );
#else
    const char* const workerName = "";
#endif // !USE_PPE_FINAL_RELEASE
    const FThreadContextStartup threadStartup(workerName, pmanager->ThreadTag());

    threadStartup.Context().SetAffinityMask(affinityMask);
    threadStartup.Context().SetPriority(pmanager->Priority());

    FWorkerContext_ workerContext(pmanager->Pimpl(), workerIndex);
    {
        // switch to a fiber from the pool,
        // and resume when original thread when task manager is destroyed
        const FFiber::FThreadScope fiberScope;
        pmanager->Pimpl()->Fibers().StartThread();
    }
}
//----------------------------------------------------------------------------
struct FBroadcast_ {
    const FTaskFunc Broadcast;

    std::mutex FinishedBarrier;
    size_t NumNotFinished;
    std::condition_variable OnFinished;

    std::mutex ExecutedBarrier;
    std::atomic<size_t> NumNotExecuted;
    std::condition_variable OnExecuted;

    explicit FBroadcast_(FTaskFunc&& broadcast, size_t numWorkers)
    :   Broadcast(std::move(broadcast))
    ,   NumNotFinished(numWorkers)
    ,   NumNotExecuted(numWorkers)
    {}

    ~FBroadcast_() {
        Assert_NoAssume(0 == NumNotFinished);
        Assert_NoAssume(0 == NumNotExecuted);
    }

    FBroadcast_(const FBroadcast_&) = delete;
    FBroadcast_& operator =(const FBroadcast_&) = delete;

    FBroadcast_(FBroadcast_&&) = delete;
    FBroadcast_& operator =(FBroadcast_&&) = delete;

    void Task(ITaskContext& context) {
        // run the task first
        Broadcast.Invoke(context);

        // notify callee when every task was executed
        if (0 == --NumNotExecuted) // this is atomic
            OnExecuted.notify_all();

        bool notify;
        {
            // then lock the barrier to wait for calling thread
            // since the calling thread has the lock this should block
            const Meta::FLockGuard scopeLock(FinishedBarrier);

            // finally decrement counter with the lock
            Assert(NumNotFinished);
            notify = (0 == --NumNotFinished);
        }

        // notify outside of the lock to avoid touching anything after this notify
        // since we can't guarantee the lifetime of (*this) afterwards
        if (notify)
            OnFinished.notify_all();
    }
};
//----------------------------------------------------------------------------
template <bool _Const>
struct TRunAndWaitFor_ {
    using taskfunc_type = std::conditional_t<_Const, const FTaskFunc, FTaskFunc>;

    TMemoryView<taskfunc_type> Tasks;
    ETaskPriority Priority;
    bool Available;

    std::mutex Barrier;
    std::condition_variable OnFinished;

    TRunAndWaitFor_(const TMemoryView<taskfunc_type>& tasks, ETaskPriority priority)
        : Tasks(tasks)
        , Priority(priority)
        , Available(false)
    {}

    void Task(ITaskContext& context) {
        context.RunAndWaitFor(Tasks, Priority);

        {
            Meta::FLockGuard scopeLock(Barrier);
            Assert(false == Available);
            Available = true;
        }

        OnFinished.notify_all();
    }
};
//----------------------------------------------------------------------------
struct FWaitForAll_ {
    bool Available;

    std::mutex Barrier;
    std::condition_variable OnFinished;

    FWaitForAll_() : Available(false) {}

    void Task(ITaskContext& context) {
        FTaskManagerImpl& impl = *FWorkerContext_::Get().Manager().Pimpl();

        do {
            FCompletionPort port;
            context.RunOne(&port, [](ITaskContext&) {
                NOOP();
            },  ETaskPriority::Internal );
            context.WaitFor(port);

        } while (impl.Scheduler().HasPendingTask());

        {
            Meta::FLockGuard scopeLock(Barrier);
            Assert(false == Available);
            Available = true;
        }

        OnFinished.notify_all();
    }
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTaskManagerImpl::FTaskManagerImpl(FTaskManager& manager)
:   _manager(manager)
,   _scheduler(_manager.WorkerCount(), GTaskManagerQueueCapacity)
,   _fibers(MakeFunction<&FTaskManagerImpl::WorkerLoop_>())
{
    _threads.reserve(_manager.WorkerCount());
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

    _scheduler.BroadcastToEveryWorker(
        INDEX_NONE, // lowest priority
        &FWorkerContext_::ExitWorkerTask );

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
void FTaskManagerImpl::ReleaseMemory() {
#if USE_PPE_LOGGER
    size_t reserved, used;
    _fibers.UsageStats(&reserved, &used);

    LOG(Task, Debug, L"before release memory in task manager <{0}>, using {1} fibers / {2} reserved ({3} reserved for stack)",
        _manager.Name(),
        Fmt::CountOfElements(used),
        Fmt::CountOfElements(reserved),
        Fmt::SizeInBytes(reserved * FTaskFiberPool::ReservedStackSize()));
#endif

    _fibers.ReleaseMemory(); // get most memory at the top

    BroadcastAndWaitFor(
        [](ITaskContext&) {
            FWorkerContext_& worker = FWorkerContext_::Get();
            // empty fibers thread local cache
            worker.Fibers().ReleaseMemory();
            // defragment malloc TLS caches
            malloc_release_cache_memory();
            // defragment fiber pool by releasing current fiber and getting new ones (hopefuly from first chunk)
            FTaskFunc func([](ITaskContext&) {
                FTaskFiberPool::CurrentHandleRef()->YieldFiber(nullptr, true);
                });
            if (not worker.SetPostTaskDelegate(std::move(func)))
                AssertNotReached();
        },
        ETaskPriority::High/* highest priority, to avoid block waiting for all the jobs queued before */);

    _fibers.ReleaseMemory(); // release defragmented blocks

#if USE_PPE_LOGGER
    _fibers.UsageStats(&reserved, &used);

    LOG(Task, Debug, L"after release memory in task manager <{0}>, using {1} fibers / {2} reserved ({3} reserved for stack)",
        _manager.Name(),
        Fmt::CountOfElements(used),
        Fmt::CountOfElements(reserved),
        Fmt::SizeInBytes(reserved * FTaskFiberPool::ReservedStackSize()));
#endif
}
//----------------------------------------------------------------------------
void FTaskManagerImpl::DumpStats() {
#if !USE_PPE_FINAL_RELEASE && USE_PPE_LOGGER
    size_t reserved, used;
    _fibers.UsageStats(&reserved, &used);

    LOG(Task, Debug, L"task manager <{0}> is using {1} fibers / {2} reserved with {3} worker threads ({4} reserved for stack)",
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
void FTaskManagerImpl::Run(FCompletionPort* phandle, FTaskFunc&& rtask, ETaskPriority priority) {
    Assert_NoAssume(rtask);

    _scheduler.Produce(priority, std::move(rtask), StartPortIFN_(phandle, 1));
}
//----------------------------------------------------------------------------
void FTaskManagerImpl::Run(FCompletionPort* phandle, const TMemoryView<FTaskFunc>& rtasks, ETaskPriority priority) {
    Assert_NoAssume(not rtasks.empty());

    _scheduler.Produce(priority, rtasks, StartPortIFN_(phandle, rtasks.size()));
}
//----------------------------------------------------------------------------
void FTaskManagerImpl::Run(FCompletionPort* phandle, const TMemoryView<const FTaskFunc>& tasks, ETaskPriority priority) {
    Assert_NoAssume(not tasks.empty());

    _scheduler.Produce(priority, tasks, StartPortIFN_(phandle, tasks.size()));
}
//----------------------------------------------------------------------------
void FTaskManagerImpl::WaitFor(FCompletionPort& handle, ETaskPriority priority) {
    Assert_NoAssume(FFiber::IsInFiber());

    handle.AttachCurrentFiber(FWorkerContext_::Get().Fibers(), priority);
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
void FTaskManagerImpl::BroadcastAndWaitFor(FTaskFunc&& rtask, ETaskPriority priority) {
    AssertRelease(not FFiber::IsInFiber()); // well won't work if a task is busy handling this logic so no :/

    FBroadcast_ args{ std::move(rtask), _threads.size() };
    {
        // first lock the barrier so that every worker thread could not consume more than one task
        Meta::FUniqueLock scopeLock(args.FinishedBarrier);

        _scheduler.BroadcastToEveryWorker(
            size_t(priority),
            MakeFunction<&FBroadcast_::Task>(&args) );

        // wait for all task to be executed
        {
            Meta::FUniqueLock waitLock(args.ExecutedBarrier);
            args.OnExecuted.wait(waitLock, [&args]() {
                return (0 == args.NumNotExecuted);
            });
        }

        // then wait for signal and check if every task has been completed
        // this will actually release the barrier while waiting
        args.OnFinished.wait(scopeLock, [&args]() {
            return (0 == args.NumNotFinished);
        });

        // past this point every worker thread should have leaved the task scope
        // so this finally safe to destroy args
    }
}
//----------------------------------------------------------------------------
FCompletionPort* FTaskManagerImpl::StartPortIFN_(FCompletionPort* phandle, size_t n) {
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
                task.Port->OnJobComplete();
        }

        // this loop is thigh to a fiber, *NOT A THREAD*
        // so we need to ask for the current worker here
        FWorkerContext_::PostWork();
    }
}
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
    Assert(nullptr == _pimpl);
}
//----------------------------------------------------------------------------
void FTaskManager::Start(const TMemoryView<const u64>& threadAffinities) {
    Assert(nullptr == _pimpl);

    LOG(Task, Info, L"start manager <{0}> with {1} workers and tag <{2}> ...", _name, _workerCount, _threadTag);

    _pimpl.reset(new FTaskManagerImpl(*this));
    _pimpl->Start(threadAffinities);
}
//----------------------------------------------------------------------------
void FTaskManager::Shutdown() {
    Assert(nullptr != _pimpl);

    LOG(Task, Info, L"shutdown manager <#{0}> with {1} workers and tag <{2}> ...", _name, _workerCount, _threadTag);

    _pimpl->Shutdown();
    _pimpl.reset();
}
//----------------------------------------------------------------------------
ITaskContext* FTaskManager::Context() const {
    Assert(FFiber::IsInFiber());
    Assert(nullptr != _pimpl);

    return _pimpl.get();
}
//----------------------------------------------------------------------------
void FTaskManager::Run(FTaskFunc&& rtask, ETaskPriority priority /* = ETaskPriority::Normal */) const {
    Assert(rtask);
    Assert(nullptr != _pimpl);

    _pimpl->Run(nullptr, std::move(rtask), priority);
}
//----------------------------------------------------------------------------
void FTaskManager::Run(const TMemoryView<FTaskFunc>& rtasks, ETaskPriority priority /* = ETaskPriority::Normal */) const {
    Assert_NoAssume(not rtasks.empty());
    Assert(nullptr != _pimpl);

    _pimpl->Run(nullptr, rtasks, priority);
}
//----------------------------------------------------------------------------
void FTaskManager::Run(const TMemoryView<const FTaskFunc>& tasks, ETaskPriority priority /* = ETaskPriority::Normal */) const {
    Assert_NoAssume(not tasks.empty());
    Assert(nullptr != _pimpl);

    _pimpl->Run(nullptr, tasks, priority);
}
//----------------------------------------------------------------------------
void FTaskManager::RunAndWaitFor(const TMemoryView<FTaskFunc>& rtasks, ETaskPriority priority /* = ETaskPriority::Normal */) const {
    Assert_NoAssume(not rtasks.empty());
    Assert(nullptr != _pimpl);

    if (FFiber::IsInFiber()) {
        _pimpl->RunAndWaitFor(rtasks, priority);
    }
    else {
        TRunAndWaitFor_<false> args{ rtasks, priority };
        {
            Meta::FUniqueLock scopeLock(args.Barrier);

            _pimpl->RunOne(nullptr,
                MakeFunction<&TRunAndWaitFor_<false>::Task>(&args),
                priority);

            args.OnFinished.wait(scopeLock, [&args]() { return args.Available; });
            Assert(args.Available);
        }
    }
}
//----------------------------------------------------------------------------
void FTaskManager::RunAndWaitFor(const TMemoryView<FTaskFunc>& rtasks, const FTaskFunc& whileWaiting, ETaskPriority priority/* = ETaskPriority::Normal */) const {
    Assert_NoAssume(not FFiber::IsInFiber());
    Assert_NoAssume(not rtasks.empty());
    Assert(whileWaiting);
    Assert(nullptr != _pimpl);

    if (FFiber::IsInFiber()) {
        FCompletionPort wait;
        _pimpl->Run(&wait, rtasks, priority);

        whileWaiting(*_pimpl);

        _pimpl->WaitFor(wait, priority);
    }
    else {
        TRunAndWaitFor_<false> args{ rtasks, priority };
        {
            Meta::FUniqueLock scopeLock(args.Barrier);

            _pimpl->RunOne(nullptr,
                MakeFunction<&TRunAndWaitFor_<false>::Task>(&args),
                priority);

            whileWaiting(*_pimpl);

            args.OnFinished.wait(scopeLock, [&args]() { return args.Available; });
            Assert(args.Available);
        }
    }
}
//----------------------------------------------------------------------------
void FTaskManager::RunAndWaitFor(const TMemoryView<const FTaskFunc>& tasks, ETaskPriority priority /* = ETaskPriority::Normal */) const {
    Assert_NoAssume(not tasks.empty());
    Assert(nullptr != _pimpl);

    if (FFiber::IsInFiber()) {
        _pimpl->RunAndWaitFor(tasks, priority);
    }
    else {
        TRunAndWaitFor_<true> args{ tasks, priority };
        {
            Meta::FUniqueLock scopeLock(args.Barrier);

            _pimpl->RunOne(nullptr,
                MakeFunction<&TRunAndWaitFor_<true>::Task>(&args),
                priority);

            args.OnFinished.wait(scopeLock, [&args]() { return args.Available; });
            Assert(args.Available);
        }
    }
}
//----------------------------------------------------------------------------
void FTaskManager::BroadcastAndWaitFor(FTaskFunc&& rtask, ETaskPriority priority/* = ETaskPriority::Normal */) const {
    Assert_NoAssume(rtask);
    Assert(nullptr != _pimpl);

    _pimpl->BroadcastAndWaitFor(std::move(rtask), priority);
}
//----------------------------------------------------------------------------
void FTaskManager::WaitForAll() const {
    Assert_NoAssume(not FFiber::IsInFiber());
    Assert(nullptr != _pimpl);

    FWaitForAll_ args;
    // trigger a task with lowest priority and wait for it
    // should wait for all other tasks since Internal is reserved
    {
        Meta::FUniqueLock scopeLock(args.Barrier);

        _pimpl->RunOne(nullptr,
            FTaskFunc::Bind<&FWaitForAll_::Task>(&args),
            ETaskPriority::Internal/* this priority is reserved for this particular usage */);

        args.OnFinished.wait(scopeLock, [&args]() { return args.Available; });
    }
}
//----------------------------------------------------------------------------
bool FTaskManager::WaitForAll(int timeoutMS) const {
    if (FFiber::IsInFiber())
        return false;

    Assert(nullptr != _pimpl);

    FWaitForAll_ args;
    // trigger a task with lowest priority and wait for it
    // should wait for all other tasks since Internal is reserved
    {
        Meta::FUniqueLock scopeLock(args.Barrier);

        _pimpl->RunOne(nullptr,
            FTaskFunc::Bind<&FWaitForAll_::Task>(&args),
            ETaskPriority::Internal/* this priority is reserved for this particular usage */);

        return args.OnFinished.wait_for(scopeLock,
            std::chrono::milliseconds(timeoutMS),
            [&args]() { return args.Available;
        });
    }
}
//----------------------------------------------------------------------------
void FTaskManager::DumpStats() {
    if (_pimpl)
        _pimpl->DumpStats();
}
//----------------------------------------------------------------------------
void FTaskManager::ReleaseMemory() {
    if (_pimpl)
        _pimpl->ReleaseMemory();
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
void FInteruptedTask::Resume(const TMemoryView<FInteruptedTask>& tasks) {
    Assert(not tasks.empty());

    // sort all queued fibers by priority before resuming and outside of the lock
    // (no need for stable sort since each task should have a unique priority)
    std::sort(tasks.begin(), tasks.end(),
        [](const FInteruptedTask& lhs, const FInteruptedTask& rhs) {
            return (lhs.Priority() < rhs.Priority());
        });

    bool postTaskAvailable = true;
    FWorkerContext_& worker = FWorkerContext_::Get();
    for (const FInteruptedTask& task : tasks) {
        FTaskFunc func([resume{ static_cast<FTaskFiberPool::FHandleRef>(task.Fiber()) }](ITaskContext&) {
            FTaskFiberPool::CurrentHandleRef()->YieldFiber(resume, true/* release current fiber */);
        });

        // it's faster when using PostTaskDelegate versus spawning a new fiber,
        // but there's only one task that can benefit from this : we can't stole
        // the current fiber more than once.
        // also we can't use PostTaskDelegate when the stalled fiber belongs to
        // another task manager.

        if ((postTaskAvailable & (task.Context() == &worker.Context())) == false ||
            (postTaskAvailable = worker.SetPostTaskDelegate(std::move(func))) == false ) {
            Assert_NoAssume(func);

            // stalled fibers are resumed through a task to let the current fiber dispatch
            // all jobs to every worker thread before yielding (won't steal execution from current fiber)

            task.Context()->RunOne(nullptr, std::move(func), task.Priority());
        }
    }

    Assert_NoAssume(&worker == &FWorkerContext_::Get()); // didn't change thread
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
