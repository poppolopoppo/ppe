#include "stdafx.h"

#include "TaskManager.h"

#include "TaskFiberPool.h"
#include "TaskScheduler.h"

#include "Thread/AtomicSpinLock.h"
#include "Thread/Fiber.h"
#include "Thread/ThreadContext.h"

#include "Allocator/Alloca.h"
#include "Container/BitMask.h"
#include "Container/RingBuffer.h"
#include "Container/Vector.h"
#include "Diagnostic/Logger.h"
#include "HAL/PlatformMemory.h"
#include "IO/Format.h"
#include "IO/TextWriter.h"
#include "Meta/PointerWFlags.h"
#include "Meta/ThreadResource.h"

#include <algorithm>

namespace PPE {
LOG_CATEGORY(PPE_API, Task)
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

#ifdef WITH_PPE_ASSERT
    ~FStalledFiber() {
        Assert(nullptr == _stalled); // should have been resumed !
    }
#endif

    FStalledFiber(const FStalledFiber& ) = delete;
    FStalledFiber& operator =(const FStalledFiber& ) = delete;

    FStalledFiber(FStalledFiber&& rvalue)
    :   FStalledFiber() {
        operator =(std::move(rvalue));
    }
    FStalledFiber& operator =(FStalledFiber&& rvalue) {
        Assert(nullptr == _stalled);
        _taskContextAndPriority = rvalue._taskContextAndPriority;
        _stalled = rvalue._stalled;
        rvalue._stalled = nullptr;
        return (*this);
    }

    ETaskPriority Priority() const {
        Assert(_stalled);

        return ETaskPriority(_taskContextAndPriority.Flag01());
    }

    void Resume(void (*resume)(FTaskFiberRef)) {
        Assert(_stalled);

        FTaskFiberRef stalled = _stalled;
        _stalled = nullptr;

        _taskContextAndPriority->RunOne(nullptr,
            [resume, stalled](ITaskContext& ) {
                resume(stalled);
            },
            ETaskPriority(_taskContextAndPriority.Flag01()) );
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
class CACHELINE_ALIGNED FTaskCounter : public FRefCountable {
public:
    FTaskCounter()
        : _taskCount(-1)
        , _queueSize(0)
    {}

    ~FTaskCounter() {
        Assert_NoAssume(CheckCanary_());
        Assert(-1 == _taskCount);
        Assert(0 == _queueSize);
        Assert_NoAssume(SafeRefCount() == 0);
    }

    FTaskCounter(const FTaskCounter& ) = delete;
    FTaskCounter& operator =(const FTaskCounter& ) = delete;

    bool Valid() const;
    bool Finished() const;
    void Start(size_t count);
    bool Queue(ITaskContext* resume, FTaskFiberRef fiber, ETaskPriority priority);
    void Clear();

    friend void Decrement_ResumeWaitingTasksIfZero(STaskCounter& saferef);

    OVERRIDE_CLASS_ALLOCATOR(ALIGNED_ALLOCATOR(Task, FTaskCounter, CACHELINE_SIZE))

private:
    void ResumeStalledFibers_();

    mutable FAtomicSpinLock _barrier;
    std::atomic<i32> _taskCount;
    u32 _queueSize;

#if USE_PPE_SAFEPTR // _safeCount disapears otherwise
    STATIC_CONST_INTEGRAL(u32, QueueCapacity, CODE3264(13, 14)); // exploit cache line alignment
#else
    STATIC_CONST_INTEGRAL(u32, QueueCapacity, CODE3264(14, 15)); // exploit all cache line alignment
#endif
    FStalledFiber _queue[QueueCapacity];

#if USE_PPE_SAFEPTR // _safeCount disapears otherwise
    const size_t _canary_freeToUse = CODE3264(0x01234567ul, 0x0123456789ABCDEFull);
#   ifdef WITH_PPE_ASSERT
    bool CheckCanary_() const { return (CODE3264(0x01234567ul, 0x0123456789ABCDEFull) == _canary_freeToUse); }
#   endif
#endif
};
STATIC_ASSERT(sizeof(FTaskCounter) == CODE3264(128, 256));
//----------------------------------------------------------------------------
class FTaskManagerImpl : public ITaskContext {
public:
    explicit FTaskManagerImpl(FTaskManager& manager);
    ~FTaskManagerImpl();

    FTaskScheduler& Scheduler() { return _scheduler; }
    TMemoryView<std::thread> Threads() { return MakeView(_threads); }
    FTaskFiberPool& Fibers() { return _fibers; }

    void Start(const TMemoryView<const u64>& threadAffinities);
    void Shutdown();

    virtual void Run(FTaskWaitHandle* phandle, FTaskFunc&& rtask, ETaskPriority priority) override final;
    virtual void Run(FTaskWaitHandle* phandle, const TMemoryView<FTaskFunc>& rtasks, ETaskPriority priority) override final;
    virtual void WaitFor(FTaskWaitHandle& handle, ITaskContext* resume) override final;
    virtual void RunAndWaitFor(const TMemoryView<FTaskFunc>& rtasks, ETaskPriority priority, ITaskContext* resume) override final;

private:
    FTaskManager& _manager;
    FTaskScheduler _scheduler;
    FTaskFiberPool _fibers;
    VECTOR(Task, std::thread) _threads;

#ifdef WITH_PPE_ASSERT
public: // public since this is complicated to friend FWorkerContext_ due to anonymous namespace ...
    std::atomic<int> _countersInUse;
    std::atomic<int> _fibersInUse;
#endif

    using FTaskQueued = FTaskScheduler::FTaskQueued;

    void WorkerLoop_();
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
class FWorkerContext_ : Meta::FThreadResource {
public:
    explicit FWorkerContext_(FTaskManager* pmanager, size_t workerIndex);
    ~FWorkerContext_();

    FWorkerContext_(const FWorkerContext_& ) = delete;
    FWorkerContext_& operator =(const FWorkerContext_& ) = delete;

    const FTaskManager& Manager() const { return _manager; }
    size_t WorkerIndex() const { return _workerIndex; }

    PTaskCounter CreateCounter();
    void DestroyCounter(PTaskCounter& pcounter);
    void ClearCounters();

    void DutyCycle();

    static FWorkerContext_& Get();

    static void ResumeFiberTask(FTaskFiberRef resume);
    static void ExitWorkerTask(ITaskContext& ctx);

private:
    STATIC_CONST_INTEGRAL(size_t, CacheSize, 32); // 32 kb

    FTaskManager& _manager;
    FTaskFiberPool& _fibers;
    const size_t _workerIndex;
    FTaskFiberRef _fiberToRelease;

    size_t _revision = 0;

    TFixedSizeRingBuffer<PTaskCounter, CacheSize> _counters;

    static THREAD_LOCAL FWorkerContext_* _gInstanceTLS;
};
THREAD_LOCAL FWorkerContext_* FWorkerContext_::_gInstanceTLS = nullptr;
//----------------------------------------------------------------------------
FWorkerContext_::FWorkerContext_(FTaskManager* pmanager, size_t workerIndex)
:   _manager(*pmanager)
,   _fibers(_manager.Pimpl()->Fibers())
,   _workerIndex(workerIndex)
,   _fiberToRelease(nullptr) {
    Assert(pmanager);
    Assert(nullptr == _gInstanceTLS);
    Assert(not FFiber::IsInFiber());

    _gInstanceTLS = this;
}
//----------------------------------------------------------------------------
FWorkerContext_::~FWorkerContext_() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(this == _gInstanceTLS);
    Assert(_fiberToRelease); // last fiber used to switch back to thread fiber
    Assert(not FFiber::IsInFiber());

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

#ifdef WITH_PPE_ASSERT
    {
        FTaskManagerImpl* pimpl = _manager.Pimpl();
        ++pimpl->_countersInUse;
    }
#endif

    Assert(not result->Valid());
    return result;
}
//----------------------------------------------------------------------------
void FWorkerContext_::DestroyCounter(PTaskCounter& counter) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(counter);
    Assert(counter->Finished());

#ifdef WITH_PPE_ASSERT
    {
        FTaskManagerImpl* pimpl = _manager.Pimpl();
        --pimpl->_countersInUse;
        Assert(0 <= pimpl->_countersInUse);
    }
#endif

    FTaskCounter* const pcounter = RemoveRef_AssertReachZero_KeepAlive(counter);
    Assert_NoAssume(pcounter->SafeRefCount() == 0);
    pcounter->Clear();

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

    Assert(0 == _counters.size());
}
//----------------------------------------------------------------------------
void FWorkerContext_::DutyCycle() {
    THIS_THREADRESOURCE_CHECKACCESS();

    if (++_revision & 15) // every 16 calls
        CurrentThreadContext().DutyCycle();
}
//----------------------------------------------------------------------------
NO_INLINE FWorkerContext_& FWorkerContext_::Get() {
    Assert(FFiber::IsInFiber());
    Assert(nullptr != _gInstanceTLS);
    return (*_gInstanceTLS);
}
//----------------------------------------------------------------------------
void FWorkerContext_::ResumeFiberTask(FTaskFiberRef resume) {
    FTaskFiberPool::CurrentHandleRef()->YieldFiber(resume, true/* release this fiber in the pool */);
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

#ifndef FINAL_RELEASE
    char workerName[128];
    Format(workerName, "{0}_Worker#{1}", pmanager->Name(), workerIndex);
#else
    const char* workerName = "";
#endif // !FINAL_RELEASE
    const FThreadContextStartup threadStartup(workerName, pmanager->ThreadTag());

    threadStartup.Context().SetAffinityMask(affinityMask);
    threadStartup.Context().SetPriority(pmanager->Priority());

    FWorkerContext_ workerContext(pmanager, workerIndex);
    {
        // switch to a fiber from the pool,
        // and resume when original thread when task manager is destroyed
        const FFiber::FThreadScope fiberScope;
        pmanager->Pimpl()->Fibers().StartThread();
    }
}
//----------------------------------------------------------------------------
struct FRunAndWaitFor_ {
    TMemoryView<FTaskFunc> Tasks;
    ETaskPriority Priority;
    bool Available;

    std::mutex Barrier;
    std::condition_variable OnFinished;

    static void Task(ITaskContext& context, FRunAndWaitFor_* pArgs) {
        Assert(pArgs);

        context.RunAndWaitFor(pArgs->Tasks, pArgs->Priority);

        {
            Meta::FLockGuard scopeLock(pArgs->Barrier);
            Assert(false == pArgs->Available);
            pArgs->Available = true;
        }

        pArgs->OnFinished.notify_all();
    }
};
//----------------------------------------------------------------------------
struct FWaitForAll_ {
    bool Available;

    std::mutex Barrier;
    std::condition_variable OnFinished;

    static void Noop(ITaskContext& ) { NOOP(); }

    static void Task(ITaskContext& context, FWaitForAll_* pArgs) {
        Assert(pArgs);

        FTaskManagerImpl& impl = *FWorkerContext_::Get().Manager().Pimpl();

        do {
            FTaskWaitHandle waitHandle;
            context.RunOne(&waitHandle, [](ITaskContext&) { NOOP(); }, ETaskPriority::Internal);
            context.WaitFor(waitHandle);

        } while (impl.Scheduler().HasPendingTask());

        {
            Meta::FLockGuard scopeLock(pArgs->Barrier);
            Assert(false == pArgs->Available);
            pArgs->Available = true;
        }

        pArgs->OnFinished.notify_all();
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
,   _fibers(MakeFunction(this, &FTaskManagerImpl::WorkerLoop_))
#ifdef WITH_PPE_ASSERT
,   _countersInUse(0)
,   _fibersInUse(0)
#endif
{
    _threads.reserve(_manager.WorkerCount());
}
//----------------------------------------------------------------------------
FTaskManagerImpl::~FTaskManagerImpl() {
    Assert(_threads.empty());

#ifdef WITH_PPE_ASSERT
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
        Assert(_threads.back().joinable());
    }
}
//----------------------------------------------------------------------------
void FTaskManagerImpl::Shutdown() {
    Assert(_threads.size() == _manager.WorkerCount());

    _scheduler.SignalExitToWorkers(&FWorkerContext_::ExitWorkerTask);

    for (std::thread& workerThread : _threads) {
        Assert(workerThread.joinable());
        workerThread.join();
    }

    _threads.clear();
}
//----------------------------------------------------------------------------
void FTaskManagerImpl::Run(FTaskWaitHandle* phandle, FTaskFunc&& rtask, ETaskPriority priority) {
    FTaskCounter* pcounter = nullptr;

    if (phandle) {
        Assert(not phandle->Valid());

        *phandle = FTaskWaitHandle(priority, FWorkerContext_::Get().CreateCounter());
        Assert(phandle->Valid());
        Assert(phandle->Counter());

        pcounter = const_cast<FTaskCounter*>(phandle->Counter());
        pcounter->Start(1);
    }

    _scheduler.Produce(priority, std::move(rtask), pcounter);
}
//----------------------------------------------------------------------------
void FTaskManagerImpl::Run(FTaskWaitHandle* phandle, const TMemoryView<FTaskFunc>& rtasks, ETaskPriority priority) {
    FTaskCounter* pcounter = nullptr;

    if (phandle) {
        Assert(not phandle->Valid());

        *phandle = FTaskWaitHandle(priority, FWorkerContext_::Get().CreateCounter());
        Assert(phandle->Valid());
        Assert(phandle->Counter());

        pcounter = const_cast<FTaskCounter*>(phandle->Counter());
        pcounter->Start(rtasks.size());
    }

    _scheduler.Produce(priority, rtasks, pcounter);
}
//----------------------------------------------------------------------------
void FTaskManagerImpl::WaitFor(FTaskWaitHandle& handle, ITaskContext* resume) {
    Assert(FFiber::IsInFiber());
    Assert(handle.Valid());

    if (Unlikely(handle.Finished()))
        return;

    if (nullptr == resume)
        resume = this;

    FTaskFiberRef const self = FTaskFiberPool::CurrentHandleRef();
    FTaskFiberRef const next = _fibers.AcquireFiber();
    next->AttachWakeUpCallback([resume, self, &handle]() {
        handle._counter->Queue(resume, self, handle.Priority());
    });

    self->YieldFiber(next, false/* this fiber will be resumed later */);
}
//----------------------------------------------------------------------------
void FTaskManagerImpl::RunAndWaitFor(const TMemoryView<FTaskFunc>& rtasks, ETaskPriority priority, ITaskContext* resume) {
    FTaskWaitHandle handle;
    Run(&handle, rtasks, priority);
    WaitFor(handle, resume);
}
//----------------------------------------------------------------------------
void FTaskManagerImpl::WorkerLoop_() {
    Assert(FFiber::IsInFiber());
    Assert(FFiber::RunningFiber() != FFiber::ThreadFiber());

    for (;;) {
        // need to destroy FTaskFunc immediately :
        // keeping it alive outside the loop could hold resources until a next
        // task is processed on this worker, which could not happen until task manager destruction !
        FTaskQueued task;

        _scheduler.Consume(FWorkerContext_::Get().WorkerIndex(), &task);

        Assert(task.Pending.Valid());
        task.Pending.Invoke(*this);

        // Decrement the counter and resume waiting tasks if any.
        // Won't steal the thread from this fiber, so the loop can safely finish
        if (task.Counter)
            Decrement_ResumeWaitingTasksIfZero(task.Counter);

        FWorkerContext_::Get().DutyCycle();
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
    Assert(0 <= _taskCount);
    return (0 == _taskCount);
}
//----------------------------------------------------------------------------
void FTaskCounter::Start(size_t count) {
    Assert_NoAssume(CheckCanary_());
    Assert(-1 == _taskCount);
    _taskCount = checked_cast<i32>(count);
}
//----------------------------------------------------------------------------
bool FTaskCounter::Queue(ITaskContext* resume, FTaskFiberRef fiber, ETaskPriority priority) {
    Assert_NoAssume(CheckCanary_());
    Assert(resume);
    Assert(fiber);
    Assert(fiber != FTaskFiberPool::CurrentHandleRef()); // should be done from another fiber to avoid synchronization issues

    FStalledFiber stalled(resume, fiber, priority);
    {
        const FAtomicSpinLock::FScope scopedLock(_barrier);

        if (Likely(_taskCount)) {
            // will be resumed by counter when exhausted in Decrement_ResumeWaitingTasksIfZero()
            Assert(_queueSize < QueueCapacity);
            _queue[_queueSize++] = std::move(stalled);

            return true;
        }
    }

    // queue for immediate resume if the counter is already exhausted (won't steal execution from current fiber)
    stalled.Resume(&FWorkerContext_::ResumeFiberTask);

    return false;
}
//----------------------------------------------------------------------------
void FTaskCounter::Clear() {
    Assert_NoAssume(CheckCanary_());
    Assert(0 == _taskCount);
    Assert(0 == _queueSize);

    _taskCount = -1;
}
//----------------------------------------------------------------------------
NO_INLINE void FTaskCounter::ResumeStalledFibers_() {
    Assert_NoAssume(CheckCanary_());

    const FAtomicSpinLock::FScope scopedLock(_barrier);

    // sort all queued fibers by priority before resuming
    const TMemoryView<FStalledFiber> fibers(_queue, _queueSize);
    std::stable_sort(fibers.begin(), fibers.end(),
        [](const FStalledFiber& lhs, const FStalledFiber& rhs) {
        return (lhs.Priority() < rhs.Priority());
    });

    // stalled fibers are resumed through a task to let the current fiber dispatch
    // all jobs to every worker thread before yielding
    for (FStalledFiber& fiber : fibers)
        fiber.Resume(&FWorkerContext_::ResumeFiberTask);

    _queueSize = 0;
}
//----------------------------------------------------------------------------
void Decrement_ResumeWaitingTasksIfZero(STaskCounter& saferef) {
    Assert(saferef);

    FTaskCounter* const pcounter = saferef.get();
    Assert_NoAssume(pcounter->CheckCanary_());

    if (Unlikely(0 == --pcounter->_taskCount))
        pcounter->ResumeStalledFibers_();

    Assert(pcounter->Valid());
    saferef.reset();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTaskWaitHandle::FTaskWaitHandle()
:   _priority(ETaskPriority::Normal)
,   _counter(nullptr) {}
//----------------------------------------------------------------------------
FTaskWaitHandle::FTaskWaitHandle(ETaskPriority priority, PTaskCounter&& counter)
:   _priority(priority)
,   _counter(std::move(counter)) {}
//----------------------------------------------------------------------------
FTaskWaitHandle::~FTaskWaitHandle() {
    if (_counter) {
        Assert(_counter->Finished());
        FWorkerContext_::Get().DestroyCounter(_counter);
    }
}
//----------------------------------------------------------------------------
FTaskWaitHandle::FTaskWaitHandle(FTaskWaitHandle&& rvalue) {
    operator =(std::move(rvalue));
}
//----------------------------------------------------------------------------
FTaskWaitHandle& FTaskWaitHandle::operator =(FTaskWaitHandle&& rvalue) {
    Assert(not _counter.valid());

    _priority = rvalue._priority;
    _counter = std::move(rvalue._counter);

    rvalue._priority = ETaskPriority::Normal;
    Assert(not rvalue._counter.valid());

    return *this;
}
//----------------------------------------------------------------------------
bool FTaskWaitHandle::Finished() const {
    Assert(_counter.valid());

    return _counter->Finished();
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
    Assert(not rtasks.empty());
    Assert(nullptr != _pimpl);

    _pimpl->Run(nullptr, rtasks, priority);
}
//----------------------------------------------------------------------------
void FTaskManager::RunAndWaitFor(const TMemoryView<FTaskFunc>& rtasks, ETaskPriority priority /* = ETaskPriority::Normal */) const {
    Assert(not FFiber::IsInFiber());
    Assert(not rtasks.empty());
    Assert(nullptr != _pimpl);

    FRunAndWaitFor_ args{ rtasks, priority, false };
    {
        Meta::FUniqueLock scopeLock(args.Barrier);

        _pimpl->RunOne(nullptr, [&args](ITaskContext& ctx) {
            FRunAndWaitFor_::Task(ctx, &args);
        }, priority );

        args.OnFinished.wait(scopeLock, [&args]() { return args.Available; });
        Assert(args.Available);
    }
}
//----------------------------------------------------------------------------
void FTaskManager::RunAndWaitFor(const TMemoryView<FTaskFunc>& rtasks, const FTaskFunc& whileWaiting, ETaskPriority priority/* = ETaskPriority::Normal */) const {
    Assert(not FFiber::IsInFiber());
    Assert(not rtasks.empty());
    Assert(whileWaiting);
    Assert(nullptr != _pimpl);

    FRunAndWaitFor_ args{ rtasks, priority, false };
    {
        Meta::FUniqueLock scopeLock(args.Barrier);

        _pimpl->RunOne(nullptr, [&args](ITaskContext& ctx) {
            FRunAndWaitFor_::Task(ctx, &args);
        }   , priority);

        whileWaiting(*_pimpl);

        args.OnFinished.wait(scopeLock, [&args]() { return args.Available; });
        Assert(args.Available);
    }
}
//----------------------------------------------------------------------------
void FTaskManager::WaitForAll() const {
    Assert(not FFiber::IsInFiber());
    Assert(nullptr != _pimpl);

    FWaitForAll_ args{ false };
    // trigger a task with lowest priority and wait for it
    // should wait for all other tasks since Internal is reserved
    {
        Meta::FUniqueLock scopeLock(args.Barrier);

        _pimpl->RunOne(nullptr, [&args](ITaskContext& ctx) {
            FWaitForAll_::Task(ctx, &args);
        },  ETaskPriority::Internal/* this priority is reserved for this particular usage */);

        args.OnFinished.wait(scopeLock, [&args]() { return args.Available; });
    }
}
//----------------------------------------------------------------------------
void FTaskManager::WaitForAll(int timeoutMS) const {
    if (FFiber::IsInFiber())
        return;

    Assert(nullptr != _pimpl);

    FWaitForAll_ args{ false };
    // trigger a task with lowest priority and wait for it
    // should wait for all other tasks since Internal is reserved
    {
        Meta::FUniqueLock scopeLock(args.Barrier);

        _pimpl->RunOne(nullptr, [&args](ITaskContext& ctx) {
            FWaitForAll_::Task(ctx, &args);
        }, ETaskPriority::Internal/* this priority is reserved for this particular usage */);

        args.OnFinished.wait_for(scopeLock, std::chrono::milliseconds(timeoutMS),
            [&args]() { return args.Available;
        });
    }
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
