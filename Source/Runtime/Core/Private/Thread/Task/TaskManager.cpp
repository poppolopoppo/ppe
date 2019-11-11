#include "stdafx.h"

#include "Thread/Task/TaskManager.h"

#include "Thread/ThreadContext.h"
#include "Thread/Task/CompletionPort.h"
#include "Thread/Task/TaskManagerImpl.h"

#include "Diagnostic/Logger.h"
#include "HAL/PlatformMemory.h"
#include "Memory/RefPtr.h"
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
        // need to reset _postWork before yielding, or it could be executed
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
// Helpers used when calling FTaskManager outside of a Fiber
FWD_REFPTR(WaitForTask_);
class FWaitForTask_ : public FRefCountable {
public:
    const FTaskFunc Task;
    std::mutex Barrier;
    std::condition_variable OnTaskFinished;
    int NumTasks = 0;

    explicit FWaitForTask_(FTaskFunc&& rtask) NOEXCEPT
    :   Task(std::move(rtask))
    {}

    static void Broadcast(FTaskManagerImpl& pimpl, FTaskFunc&& rtask, ETaskPriority priority) {
        Assert_NoAssume(not FFiber::IsInFiber()); // won't work if we stall a worker thread !

        const int numWorkers = checked_cast<int>(pimpl.Threads().size());

        std::condition_variable onTaskBroadcast;
        FWaitForTask_ waitfor{ std::move(rtask) };
        waitfor.NumTasks = numWorkers;

        const FTaskFunc broadcast = [&waitfor, &onTaskBroadcast](ITaskContext& ctx) {
            waitfor.Task(ctx);
            {
                Meta::FUniqueLock scopeLock(waitfor.Barrier);
                Assert_NoAssume(0 < waitfor.NumTasks);

                --waitfor.NumTasks;

                waitfor.OnTaskFinished.wait(scopeLock, [&]() NOEXCEPT {
                    return (0 >= waitfor.NumTasks);
                });

                --waitfor.NumTasks;
            }
            waitfor.OnTaskFinished.notify_all();
            onTaskBroadcast.notify_one();
        };

        Meta::FUniqueLock scopeLock(waitfor.Barrier);
        forrange(n, 0, numWorkers)
            pimpl.Scheduler().Produce(priority, broadcast, nullptr);

        onTaskBroadcast.wait(scopeLock, [&waitfor, numWorkers]() NOEXCEPT {
            return (numWorkers == -waitfor.NumTasks);
        });
    }

    static void Wait(FTaskManagerImpl& pimpl, FTaskFunc&& rtask, ETaskPriority priority) {
        FWaitForTask_ waitfor{ std::move(rtask) };
        waitfor.NumTasks = 1;

        Meta::FUniqueLock scopeLock(waitfor.Barrier);
        pimpl.Scheduler().Produce(priority, [&waitfor](ITaskContext& ctx) {
            waitfor.Task(ctx);
            {
                const Meta::FLockGuard scopeLock(waitfor.Barrier);
                Assert_NoAssume(1 == waitfor.NumTasks);
                waitfor.NumTasks = 0;
            }
            waitfor.OnTaskFinished.notify_all();
        },  nullptr );

        waitfor.OnTaskFinished.wait(scopeLock, [&]() NOEXCEPT {
            return (not waitfor.NumTasks);
        });
    }

    static bool WaitFor(FTaskManagerImpl& pimpl, FTaskFunc&& rtask, ETaskPriority priority, int timeoutMS) {
        PWaitForTask_ pWaitfor{ NEW_REF(Task, FWaitForTask_)(std::move(rtask)) };
        pWaitfor->NumTasks = 1;

        Meta::FUniqueLock scopeLock(pWaitfor->Barrier);
        pimpl.Scheduler().Produce(priority, [pWaitfor](ITaskContext& ctx) {
            pWaitfor->Task(ctx);
            {
                const Meta::FLockGuard scopeLock(pWaitfor->Barrier);
                Assert_NoAssume(1 == pWaitfor->NumTasks);
                pWaitfor->NumTasks = 0;
            }
            pWaitfor->OnTaskFinished.notify_all();
        },  nullptr);

        pWaitfor->OnTaskFinished.wait_for(scopeLock, std::chrono::milliseconds(timeoutMS), [&]() NOEXCEPT {
            return (not pWaitfor->NumTasks);
        });

        return (0 == pWaitfor->NumTasks);
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
,   _manager(manager)
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
void FTaskManager::RunAndWaitFor(FTaskFunc&& rtask, ETaskPriority priority /* = ETaskPriority::Normal */) const {
    Assert_NoAssume(rtask);
    Assert(nullptr != _pimpl);

    if (FFiber::IsInFiber())
        _pimpl->RunAndWaitFor(std::move(rtask), priority);
    else
        FWaitForTask_::Wait(*_pimpl, std::move(rtask), priority);
}
//----------------------------------------------------------------------------
void FTaskManager::RunAndWaitFor(const TMemoryView<FTaskFunc>& rtasks, ETaskPriority priority /* = ETaskPriority::Normal */) const {
    Assert_NoAssume(not rtasks.empty());
    Assert(nullptr != _pimpl);

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
    Assert(nullptr != _pimpl);

    const auto waitfor = [rtasks, &whileWaiting, priority](ITaskContext& ctx) {
        FCompletionPort port;
        ctx.Run(&port, rtasks, priority);

        whileWaiting(ctx);

        ctx.WaitFor(port, priority);
    };

    if (FFiber::IsInFiber())
        waitfor(*_pimpl);
    else
        FWaitForTask_::Wait(*_pimpl, waitfor, priority);
}
//----------------------------------------------------------------------------
void FTaskManager::RunAndWaitFor(const TMemoryView<const FTaskFunc>& tasks, ETaskPriority priority /* = ETaskPriority::Normal */) const {
    Assert_NoAssume(not tasks.empty());
    Assert(nullptr != _pimpl);

    const auto waitfor = [tasks, priority](ITaskContext& ctx) {
        ctx.RunAndWaitFor(tasks, priority);
    };

    if (FFiber::IsInFiber())
        waitfor(*_pimpl);
    else
        FWaitForTask_::Wait(*_pimpl, waitfor, priority);
}
//----------------------------------------------------------------------------
void FTaskManager::BroadcastAndWaitFor(FTaskFunc&& rtask, ETaskPriority priority/* = ETaskPriority::Normal */) const {
    Assert_NoAssume(rtask);
    Assert(nullptr != _pimpl);

    if (FFiber::IsInFiber())
        AssertNotImplemented(); // not easily done inside fibers because of mutex locking used atm
    else
        FWaitForTask_::Broadcast(*_pimpl, std::move(rtask), priority);
}
//----------------------------------------------------------------------------
void FTaskManager::WaitForAll() const {
    Assert(nullptr != _pimpl);

    if (FFiber::IsInFiber())
        AssertNotImplemented(); // not easily done inside fibers because of mutex locking used atm
    else
        FWaitForTask_::Broadcast(*_pimpl, [](ITaskContext&) { NOOP(); }, ETaskPriority::Internal);
}
//----------------------------------------------------------------------------
bool FTaskManager::WaitForAll(int timeoutMS) const {
    Assert(nullptr != _pimpl);

    if (FFiber::IsInFiber())
        _pimpl->RunAndWaitFor([](ITaskContext&) { NOOP(); }, ETaskPriority::Internal);
    else
        FWaitForTask_::WaitFor(*_pimpl, [](ITaskContext&) { NOOP(); }, ETaskPriority::Internal, timeoutMS);

    return _pimpl->Scheduler().HasPendingTask();
}
//----------------------------------------------------------------------------
void FTaskManager::DumpStats() {
    if (_pimpl)
        _pimpl->DumpStats();
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

            task.Context()->Run(nullptr, std::move(func), task.Priority());
        }
    }

    Assert_NoAssume(&worker == &FWorkerContext_::Get()); // didn't change thread
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
