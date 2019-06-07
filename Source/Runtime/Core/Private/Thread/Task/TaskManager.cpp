#include "stdafx.h"

#include "Thread/Task/TaskManager.h"

#include "Thread/Task/TaskFiberPool.h"
#include "Thread/Task/TaskScheduler.h"

#include "Thread/AtomicSpinLock.h"
#include "Thread/Fiber.h"
#include "Thread/ThreadContext.h"

#include "Allocator/Alloca.h"
#include "Allocator/TrackingMalloc.h"
#include "Container/BitMask.h"
#include "Container/RingBuffer.h"
#include "Container/Vector.h"
#include "Container/SparseArray.h"
#include "Diagnostic/Logger.h"
#include "HAL/PlatformMemory.h"
#include "IO/Format.h"
#include "IO/FormatHelpers.h"
#include "IO/TextWriter.h"
#include "Meta/PointerWFlags.h"
#include "Meta/ThreadResource.h"

#include <algorithm>

namespace PPE {
LOG_CATEGORY(PPE_CORE_API, Task)
STATIC_CONST_INTEGRAL(size_t, GTaskManagerQueueCapacity, 128);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
using FTaskFiberRef = FTaskFiberPool::FHandleRef;
//----------------------------------------------------------------------------
class FStalledFiber {
public:
    FStalledFiber()
    :   _stalled(nullptr) {
        _taskContextAndPriority.Reset(nullptr, uintptr_t(ETaskPriority::Normal));
    }

    FStalledFiber(ITaskContext* taskContext, FTaskFiberRef stalled, ETaskPriority priority)
    :   _stalled(stalled) {
        Assert(taskContext);
        _taskContextAndPriority.Reset(taskContext, uintptr_t(priority));
    }

    FTaskFiberRef Stalled() const { return _stalled; }
    ITaskContext* Context() const { return _taskContextAndPriority.Get(); }
    ETaskPriority Priority() const { return ETaskPriority(_taskContextAndPriority.Flag01()); }

    FTaskFunc ResumeTask() const {
        return [resume{ _stalled }](ITaskContext&) {
            FTaskFiberPool::CurrentHandleRef()->YieldFiber(resume, true/* release current fiber to the pool */);
        };
    }

private:
    STATIC_ASSERT(uintptr_t(ETaskPriority::High) == 0);
    STATIC_ASSERT(uintptr_t(ETaskPriority::Normal) == 1);
    STATIC_ASSERT(uintptr_t(ETaskPriority::Low) == 2);
    STATIC_ASSERT(uintptr_t(ETaskPriority::Internal) == 3);
    Meta::TPointerWFlags<ITaskContext> _taskContextAndPriority;

    FTaskFiberRef _stalled;
};
STATIC_ASSERT(sizeof(FStalledFiber) == sizeof(intptr_t) * 2);
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PRAGMA_MSVC_WARNING_PUSH()
PRAGMA_MSVC_WARNING_DISABLE(4324) // structure was padded due to alignment
class CACHELINE_ALIGNED FTaskCounter : public FRefCountable {
public:
    FTaskCounter() : _taskCount(-1) {}

    ~FTaskCounter() {
        Assert_NoAssume(CheckCanary_());
        Assert(-1 == _taskCount);
        Assert_NoAssume(_queue.empty());
        Assert_NoAssume(SafeRefCount() == 0);
    }

    FTaskCounter(const FTaskCounter& ) = delete;
    FTaskCounter& operator =(const FTaskCounter& ) = delete;

    bool Valid() const;
    bool Finished() const;

    void Start(size_t count);
    bool Queue(const FStalledFiber& waiting);

    void AttachTo(const PTaskCounter& parent);
    void Detach();

    friend void Decrement_ResumeWaitingTasksIfZero(STaskCounter& saferef);

    // override new/delete operators to get allocations aligned on cache line size

    void* (operator new)(size_t s) {
    #if USE_PPE_MEMORYDOMAINS
        Assert(s == sizeof(FTaskCounter));
        MEMORYDOMAIN_TRACKING_DATA(Task).Allocate(s, PPE::malloc_snap_size(s));
    #endif
        return PPE::aligned_malloc(s, CACHELINE_SIZE);
    }
    void (operator delete)(void* p) {
    #if USE_PPE_MEMORYDOMAINS
        const size_t s = sizeof(FTaskCounter);
        MEMORYDOMAIN_TRACKING_DATA(Task).Deallocate(s, PPE::malloc_snap_size(s));
    #endif
        PPE::aligned_free(p);
    }
    void(operator delete)(void* p, size_t) {
        (operator delete)(p);
    }

private:
    mutable FAtomicSpinLock _barrier;

    std::atomic<int> _taskCount;
    STaskCounter _parent;

    SPARSEARRAY_INSITU(Task, FStalledFiber) _queue;

    NO_INLINE void ResumeStalledFibers_();

#if USE_PPE_SAFEPTR // _safeCount disappears otherwise
    const size_t _canary = CODE3264(0x01234567ul, 0x0123456789ABCDEFull);
#   if USE_PPE_ASSERT
    bool CheckCanary_() const { return (CODE3264(0x01234567ul, 0x0123456789ABCDEFull) == _canary); }
#   endif
#endif
};
PRAGMA_MSVC_WARNING_POP()
//----------------------------------------------------------------------------
class FTaskManagerImpl : public ITaskContext {
public:
    explicit FTaskManagerImpl(FTaskManager& manager);
    ~FTaskManagerImpl();

    FTaskManager& Manager() { return _manager; }
    FTaskScheduler& Scheduler() { return _scheduler; }
    TMemoryView<std::thread> Threads() { return MakeView(_threads); }
    FTaskFiberPool& Fibers() { return _fibers; }

    void Start(const TMemoryView<const u64>& threadAffinities);
    void Shutdown();

    virtual void CreateWaitHandle(FTaskWaitHandle* phandle) override final;

    virtual void Run(FTaskWaitHandle* phandle, FTaskFunc&& rtask, ETaskPriority priority) override final;
    virtual void Run(FTaskWaitHandle* phandle, const TMemoryView<FTaskFunc>& rtasks, ETaskPriority priority) override final;
    virtual void Run(FTaskWaitHandle* phandle, const TMemoryView<const FTaskFunc>& tasks, ETaskPriority priority) override final;

    virtual void WaitFor(FTaskWaitHandle& handle, ITaskContext* resume, ETaskPriority priority) override final;

    virtual void RunAndWaitFor(const TMemoryView<FTaskFunc>& rtasks, ETaskPriority priority, ITaskContext* resume) override final;
    virtual void RunAndWaitFor(const TMemoryView<const FTaskFunc>& tasks, ETaskPriority priority, ITaskContext* resume) override final;

    virtual void BroadcastAndWaitFor(FTaskFunc&& rtask, ETaskPriority priority) override final;

    void Consume(size_t workerIndex, FTaskScheduler::FTaskQueued* task);
    void ReleaseMemory();

private:
    FTaskManager& _manager;
    FTaskScheduler _scheduler;
    FTaskFiberPool _fibers;
    VECTOR(Task, std::thread) _threads;

#if USE_PPE_ASSERT
public: // public since this is complicated to friend FWorkerContext_ due to anonymous namespace ...
    std::atomic<int> _countersInUse;
    std::atomic<int> _fibersInUse;
#endif

    using FTaskQueued = FTaskScheduler::FTaskQueued;

    static FTaskCounter* MakeCounterIFN_(FTaskWaitHandle* phandle, size_t n);

    static void WorkerLoop_();
};
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

    const FTaskManager& Manager() const { return _pimpl.Manager(); }
    size_t WorkerIndex() const { return _workerIndex; }

    PTaskCounter CreateCounter();
    PTaskCounter StartCounter(size_t n);
    void DestroyCounter(PTaskCounter& pcounter);
    void ClearCounters();
    void SetPostTaskDelegate(FTaskFunc&& afterTask);

    static FWorkerContext_& Get();

    static void ExitWorkerTask(ITaskContext& ctx);

    static void ResumeStalledFiber(const FStalledFiber& fiber);
    static void ResumeStalledFibers(const TMemoryView<const FStalledFiber>& fibers);

    static ITaskContext& Consume(FTaskScheduler::FTaskQueued* task);
    static void PostWork();

private:
    void DutyCycle();

    STATIC_CONST_INTEGRAL(size_t, CacheSize, 32); // 32 kb

    FTaskManagerImpl& _pimpl;
    FTaskFiberPool& _fibers;

    const size_t _workerIndex;
    FTaskFiberRef _fiberToRelease;

    size_t _revision = 0;
    FTaskFunc _afterTask;

    TFixedSizeRingBuffer<PTaskCounter, CacheSize> _counters;

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
    Assert_NoAssume(not FFiber::IsInFiber());

    _gInstanceTLS = nullptr;

    ClearCounters();

    _fibers.ReleaseFiber(_fiberToRelease);
}
//----------------------------------------------------------------------------
PTaskCounter FWorkerContext_::CreateCounter() {
    THIS_THREADRESOURCE_CHECKACCESS();

    PTaskCounter result;
    if (not _counters.pop_front(&result)) {
        PPE_LEAKDETECTOR_WHITELIST_SCOPE(); // because of cached counters
        result.reset(new FTaskCounter());
    }

#if USE_PPE_ASSERT
    ++_pimpl._countersInUse;
#endif

    Assert(not result->Valid());
    return result;
}
//----------------------------------------------------------------------------
PTaskCounter FWorkerContext_::StartCounter(size_t n) {
    PTaskCounter counter = CreateCounter();
    counter->Start(n);
    return counter;
}
//----------------------------------------------------------------------------
void FWorkerContext_::DestroyCounter(PTaskCounter& counter) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(counter);
    Assert_NoAssume(counter->Finished());

#if USE_PPE_ASSERT
    --_pimpl._countersInUse;
    Assert(0 <= _pimpl._countersInUse);
#endif

    FTaskCounter* const pcounter = RemoveRef_AssertReachZero_KeepAlive(counter);
    Assert_NoAssume(pcounter->SafeRefCount() == 0);
    Assert_NoAssume(pcounter->Finished());

    pcounter->Detach(); // detach from potential parent

    _counters.push_back_OverflowIFN(nullptr, pcounter);
}
//----------------------------------------------------------------------------
void FWorkerContext_::ClearCounters() {
    THIS_THREADRESOURCE_CHECKACCESS();

    PTaskCounter counter;
    while (_counters.Dequeue(&counter)) {
        Assert(not counter->Valid());
        RemoveRef_AssertReachZero(counter);
    }

    Assert_NoAssume(0 == _counters.size());
}
//----------------------------------------------------------------------------
void FWorkerContext_::SetPostTaskDelegate(FTaskFunc&& afterTask) {
    Assert_NoAssume(afterTask);
    AssertRelease(not _afterTask.Valid());

    // this delegate will be executed after current task execution, and more
    // importantly *AFTER* the decref, where it's safe to switch to another
    // fiber.

    _afterTask = std::move(afterTask);
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
    if ((++ctx._revision & 127) == 0) {
        ctx.DutyCycle();
    }

    // special tasks can inject a postfix callback to avoid skipping the decref
    if (ctx._afterTask) {
        // need to reset _afterTask before yielding, or it could be executed
        // more than once since it's stored in the thread and not in the fiber
        const FTaskFunc afterTaskCpy{ std::move(ctx._afterTask) };
        Assert_NoAssume(not ctx._afterTask);
        afterTaskCpy(ctx._pimpl);
    }
}
//----------------------------------------------------------------------------
NO_INLINE void FWorkerContext_::DutyCycle() {
    THIS_THREADRESOURCE_CHECKACCESS();

#if !USE_PPE_FINAL_RELEASE && USE_PPE_LOGGER
    size_t reserved, used;
    _pimpl.Fibers().UsageStats(&reserved, &used);

    LOG(Task, Debug, L"duty cycle for worker <{0}>, using {1} fibers / {2} reserved (max {3} used for stack)",
        MakeCStringView(CurrentThreadContext().Name()),
        Fmt::CountOfElements(used),
        Fmt::CountOfElements(reserved),
        Fmt::SizeInBytes(reserved * FTaskFiberPool::ReservedStackSize()));
#endif

    CurrentThreadContext().DutyCycle();
}
//----------------------------------------------------------------------------
FWorkerContext_& FWorkerContext_::Get() {
    Assert(FFiber::IsInFiber());
    Assert(nullptr != _gInstanceTLS);
    return (*_gInstanceTLS);
}
//----------------------------------------------------------------------------
void FWorkerContext_::ResumeStalledFiber(const FStalledFiber& fiber) {
    Assert(fiber.Stalled());
    Assert_NoAssume(FTaskFiberPool::CurrentHandleRef() != fiber.Stalled());

    FWorkerContext_& ctx = Get();

    // if the fiber uses the same task manager we can resume the fiber without
    // using a third fiber to release the current fiber
    if (&ctx._pimpl == fiber.Context())
        ctx.SetPostTaskDelegate(fiber.ResumeTask());
    // if the fiber is waiting from another the task we *NEED* to use its own
    // context to resume it, it's one of the key features of FTaskManager
    else
        fiber.Context()->RunOne(nullptr, fiber.ResumeTask(), fiber.Priority());
}
//----------------------------------------------------------------------------
void FWorkerContext_::ResumeStalledFibers(const TMemoryView<const FStalledFiber>& fibers) {
    Assert_NoAssume(not fibers.empty());

    // don't use an intermediate fiber to release this task (first should be the task with max priority)
    ResumeStalledFiber(fibers.front());

    // but it still needed for count(tasks) > 1 since we'll lose EIP afterwards
    for (const FStalledFiber& fiber : fibers.ShiftFront()) {
        Assert(fiber.Stalled());

        fiber.Context()->RunOne(nullptr, fiber.ResumeTask(), fiber.Priority());
    }
}
//----------------------------------------------------------------------------
void FWorkerContext_::ExitWorkerTask(ITaskContext&) {
    Assert(FFiber::ThreadFiber() != FFiber::RunningFiber());

    FTaskFiberRef self = FTaskFiberPool::CurrentHandleRef();
    Assert(self);

    FWorkerContext_& workerContext = Get();
    Assert(nullptr == workerContext._fiberToRelease);
    workerContext._fiberToRelease = self; // released in worker context destructor ^^^

    FFiber thread(FFiber::ThreadFiber());
    thread.Resume();

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
            FTaskWaitHandle waitHandle;
            context.RunOne(&waitHandle, [](ITaskContext&) {
                NOOP();
            },  ETaskPriority::Internal );
            context.WaitFor(waitHandle);

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
#if USE_PPE_ASSERT
,   _countersInUse(0)
,   _fibersInUse(0)
#endif
{
    _threads.reserve(_manager.WorkerCount());
}
//----------------------------------------------------------------------------
FTaskManagerImpl::~FTaskManagerImpl() {
    Assert(_threads.empty());

#if USE_PPE_ASSERT
    Assert(0 == _countersInUse);
    Assert(0 == _fibersInUse);
#endif
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
void FTaskManagerImpl::CreateWaitHandle(FTaskWaitHandle* phandle) {
    Assert(phandle);

    MakeCounterIFN_(phandle, 0);
}
//----------------------------------------------------------------------------
void FTaskManagerImpl::Run(FTaskWaitHandle* phandle, FTaskFunc&& rtask, ETaskPriority priority) {
    Assert_NoAssume(rtask);

    _scheduler.Produce(priority, std::move(rtask), MakeCounterIFN_(phandle, 1));
}
//----------------------------------------------------------------------------
void FTaskManagerImpl::Run(FTaskWaitHandle* phandle, const TMemoryView<FTaskFunc>& rtasks, ETaskPriority priority) {
    Assert_NoAssume(not rtasks.empty());

    _scheduler.Produce(priority, rtasks, MakeCounterIFN_(phandle, rtasks.size()));
}
//----------------------------------------------------------------------------
void FTaskManagerImpl::Run(FTaskWaitHandle* phandle, const TMemoryView<const FTaskFunc>& tasks, ETaskPriority priority) {
    Assert_NoAssume(not tasks.empty());

    _scheduler.Produce(priority, tasks, MakeCounterIFN_(phandle, tasks.size()));
}
//----------------------------------------------------------------------------
void FTaskManagerImpl::WaitFor(FTaskWaitHandle& handle, ITaskContext* resume, ETaskPriority priority) {
    Assert_NoAssume(FFiber::IsInFiber());
    Assert(handle.Valid());

    if (nullptr == resume)
        resume = &CurrentTaskContext();

    FStalledFiber const self{ resume, FTaskFiberPool::CurrentHandleRef(), priority };
    FTaskFiberRef const next = _fibers.AcquireFiber();
    STATIC_ASSERT(sizeof(FStalledFiber) == sizeof(uintptr_t) * 2);
    STATIC_ASSERT(sizeof(STaskCounter) == sizeof(uintptr_t));
    next->AttachWakeUpCallback([self, pcounter{ STaskCounter{ handle._counter } }]() {
        pcounter->Queue(self);
    });

    if (Unlikely(handle.Finished())) {
        // task already finished while we were preparing :
        next->OnWakeUp.Reset();
        _fibers.ReleaseFiber(next);
    }
    else {
        // task is still running, yield execution to next fiber :
        self.Stalled()->YieldFiber(next, false/* this fiber will be resumed later */);
    }
}
//----------------------------------------------------------------------------
void FTaskManagerImpl::RunAndWaitFor(const TMemoryView<FTaskFunc>& rtasks, ETaskPriority priority, ITaskContext* resume) {
    FTaskWaitHandle handle;
    Run(&handle, rtasks, priority);
    WaitFor(handle, resume, priority);
}
//----------------------------------------------------------------------------
void FTaskManagerImpl::RunAndWaitFor(const TMemoryView<const FTaskFunc>& tasks, ETaskPriority priority, ITaskContext* resume) {
    FTaskWaitHandle handle;
    Run(&handle, tasks, priority);
    WaitFor(handle, resume, priority);
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
void FTaskManagerImpl::Consume(size_t workerIndex, FTaskQueued* task) {
    Assert(task);

    _scheduler.Consume(workerIndex, task);
}
//----------------------------------------------------------------------------
void FTaskManagerImpl::ReleaseMemory() {
#if USE_PPE_LOGGER
    size_t reserved, used;
    _fibers.UsageStats(&reserved, &used);

    LOG(Task, Debug, L"before release memory in task manager <{0}>, using {1} fibers / {2} reserved (max {3} used for stack)",
        _manager.Name(),
        Fmt::CountOfElements(used),
        Fmt::CountOfElements(reserved),
        Fmt::SizeInBytes(reserved * FTaskFiberPool::ReservedStackSize()));
#endif

    _fibers.ReleaseMemory(); // get most memory at the top

    BroadcastAndWaitFor(
        [](ITaskContext&) {
            // defragment malloc TLS caches
            malloc_release_cache_memory();
            // defragment fiber pool by getting releasing current fiber and getting a new one
            FWorkerContext_::Get().SetPostTaskDelegate([](ITaskContext&) {
                FTaskFiberPool::CurrentHandleRef()->YieldFiber(nullptr, true);
                });
        },
        ETaskPriority::High/* highest priority, to avoid block waiting for all the jobs queued before */);

    _fibers.ReleaseMemory(); // release defragmented blocks

#if USE_PPE_LOGGER
    _fibers.UsageStats(&reserved, &used);

    LOG(Task, Debug, L"after release memory in task manager <{0}>, using {1} fibers / {2} reserved (max {3} used for stack)",
        _manager.Name(),
        Fmt::CountOfElements(used),
        Fmt::CountOfElements(reserved),
        Fmt::SizeInBytes(reserved * FTaskFiberPool::ReservedStackSize()));
#endif
}
//----------------------------------------------------------------------------
FTaskCounter* FTaskManagerImpl::MakeCounterIFN_(FTaskWaitHandle* phandle, size_t n) {
    if (nullptr == phandle) {
        return nullptr;
    }
    else {
        if (Likely(not phandle->Valid()))
            *phandle = FTaskWaitHandle{ FWorkerContext_::Get().StartCounter(n) };

        return phandle->_counter.get();
    }
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
            if (task.Counter)
                Decrement_ResumeWaitingTasksIfZero(task.Counter);
        }

        // this loop is thigh to a fiber, *NOT A THREAD*
        // so we need to ask for the current thread again here
        FWorkerContext_::PostWork();
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FTaskCounter::Valid() const {
    Assert_NoAssume(CheckCanary_());

    return (0 <= _taskCount);
}
//----------------------------------------------------------------------------
bool FTaskCounter::Finished() const {
    Assert_NoAssume(CheckCanary_());
    Assert(-1 <= _taskCount);

    return (-1 == _taskCount);
}
//----------------------------------------------------------------------------
void FTaskCounter::Start(size_t count) {
    Assert_NoAssume(CheckCanary_());
    Assert(-1 == _taskCount);
    Assert_NoAssume(not _parent);

    _taskCount = checked_cast<i32>(count);
}
//----------------------------------------------------------------------------
bool FTaskCounter::Queue(const FStalledFiber& waiting) {
    Assert_NoAssume(CheckCanary_());
    Assert(waiting.Stalled());
    Assert_NoAssume(waiting.Stalled() != FTaskFiberPool::CurrentHandleRef()); // should be done from another fiber to avoid synchronization issues

    {
        const FAtomicSpinLock::FScope scopedLock(_barrier);

        if (_taskCount > 0) {
            _queue.Emplace(waiting);
            return true;
        }
    }

    // queue for immediate resume if the counter is already exhausted (won't steal execution from current fiber)
    FWorkerContext_::ResumeStalledFiber(waiting);

    return false;
}
//----------------------------------------------------------------------------
void FTaskCounter::AttachTo(const PTaskCounter& parent) {
    Assert(parent);
    Assert_NoAssume(CheckCanary_());

    const FAtomicSpinLock::FScope scopedLock(_barrier);
    AssertRelease(not _parent);

    _parent.reset(parent.get());
    _parent->_taskCount += _taskCount.load();
}
//----------------------------------------------------------------------------
void FTaskCounter::Detach() {
    Assert_NoAssume(CheckCanary_());
    Assert_NoAssume(Finished());

    _parent.reset();
}
//----------------------------------------------------------------------------
NO_INLINE void FTaskCounter::ResumeStalledFibers_() {
    Assert_NoAssume(CheckCanary_());

    TAllocaBlock<FStalledFiber> localq; // can use alloca since won't switch from thread here
    {
        // simply copy the queue with the lock acquired
        // avoid reading from this again !

        const FAtomicSpinLock::FScope scopedLock(_barrier);
        Assert_NoAssume(0 == _taskCount);

        localq.RelocateIFP(_queue.size());
        std::uninitialized_move(_queue.begin(), _queue.end(), localq.MakeView().begin());

        _queue.Clear_ReleaseMemory();
        _taskCount = -1;
    }

    Assert_NoAssume(Finished()); // from now on we work only on the stack :

    if (localq.Count) {
        const TMemoryView<FStalledFiber> resume = localq.MakeView();

        // sort all queued fibers by priority before resuming and outside of the lock
        std::stable_sort(resume.begin(), resume.end(),
            [](const FStalledFiber& lhs, const FStalledFiber& rhs) {
                return (lhs.Priority() < rhs.Priority());
            });

        // stalled fibers are resumed through a task to let the current fiber dispatch
        // all jobs to every worker thread before yielding (won't steal execution from current fiber)
        FWorkerContext_::ResumeStalledFibers(resume);
    }
}
//----------------------------------------------------------------------------
void Decrement_ResumeWaitingTasksIfZero(STaskCounter& saferef) {
    Assert(saferef);

    // need a local copy of the counter to decrement after reseting
    FTaskCounter* const pcounter = saferef.get();
    Assert(pcounter->Valid());

    STaskCounter parent{ pcounter->_parent };

    // need to reset just before decrementing,
    // lifetime should be guaranteed by owner until _taskCount == 0
    saferef.reset();

    // resume stalled fibers, this won't steal the execution from the current fiber
    const i32 n = (--pcounter->_taskCount);
    if (Unlikely(n == 0))
        pcounter->ResumeStalledFibers_();
    else
        Assert_NoAssume(n > 0);

    // can't access pcounter anymore ! if n == 0 lifetime isn't guaranteed anymore

    // safely recurse to the parent afterwards
    if (parent)
        Decrement_ResumeWaitingTasksIfZero(parent);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTaskWaitHandle::FTaskWaitHandle() NOEXCEPT
:   _counter(nullptr) {}
//----------------------------------------------------------------------------
FTaskWaitHandle::FTaskWaitHandle(PTaskCounter&& counter) NOEXCEPT
:   _counter(std::move(counter)) {}
//----------------------------------------------------------------------------
FTaskWaitHandle::~FTaskWaitHandle() {
    Reset();
}
//----------------------------------------------------------------------------
FTaskWaitHandle::FTaskWaitHandle(FTaskWaitHandle&& rvalue) NOEXCEPT {
    operator =(std::move(rvalue));
}
//----------------------------------------------------------------------------
FTaskWaitHandle& FTaskWaitHandle::operator =(FTaskWaitHandle&& rvalue) NOEXCEPT {
    AssertRelease(not _counter.valid()); // should call DestroyCounter() to pool the counter

    _counter = std::move(rvalue._counter);
    Assert_NoAssume(not rvalue._counter.valid());

    return *this;
}
//----------------------------------------------------------------------------
bool FTaskWaitHandle::Finished() const {
    Assert_NoAssume(_counter.valid());

    return _counter->Finished();
}
//----------------------------------------------------------------------------
void FTaskWaitHandle::AttachTo(FTaskWaitHandle& parent) {
    Assert(_counter);
    Assert(parent._counter);

    _counter->AttachTo(parent._counter);
}
//----------------------------------------------------------------------------
void FTaskWaitHandle::Reset() {
    if (_counter) {
        AssertRelease(_counter->Finished());
        FWorkerContext_::Get().DestroyCounter(_counter);
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
void FTaskManager::RunAndWaitFor(const TMemoryView<FTaskFunc>& rtasks, ETaskPriority priority /* = ETaskPriority::Normal */, ITaskContext* resume /* = nullptr */) const {
    Assert_NoAssume(not rtasks.empty());
    Assert(nullptr != _pimpl);

    if (FFiber::IsInFiber()) {
        _pimpl->RunAndWaitFor(rtasks, priority, resume);
    }
    else {
        Assert_NoAssume(not resume);

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
void FTaskManager::RunAndWaitFor(const TMemoryView<FTaskFunc>& rtasks, const FTaskFunc& whileWaiting, ETaskPriority priority/* = ETaskPriority::Normal */, ITaskContext* resume /* = nullptr */) const {
    Assert_NoAssume(not FFiber::IsInFiber());
    Assert_NoAssume(not rtasks.empty());
    Assert(whileWaiting);
    Assert(nullptr != _pimpl);

    if (FFiber::IsInFiber()) {
        FTaskWaitHandle wait;
        _pimpl->Run(&wait, rtasks, priority);

        whileWaiting(*_pimpl);

        _pimpl->WaitFor(wait, resume, priority);
    }
    else {
        Assert_NoAssume(not resume);

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
void FTaskManager::RunAndWaitFor(const TMemoryView<const FTaskFunc>& tasks, ETaskPriority priority /* = ETaskPriority::Normal */, ITaskContext* resume /* = nullptr */) const {
    Assert_NoAssume(not tasks.empty());
    Assert(nullptr != _pimpl);

    if (FFiber::IsInFiber()) {
        _pimpl->RunAndWaitFor(tasks, priority, resume);
    }
    else {
        Assert_NoAssume(not resume);

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
void FTaskManager::ReleaseMemory() {
    if (_pimpl)
        _pimpl->ReleaseMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ITaskContext& CurrentTaskContext() {
    // need to be called from a worker thread ! (asserted by FWorkerContext_)
    const FTaskManager& manager = FWorkerContext_::Get().Manager();
    FTaskManagerImpl* const pimpl = manager.Pimpl();
    Assert(pimpl);
    return *pimpl;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
