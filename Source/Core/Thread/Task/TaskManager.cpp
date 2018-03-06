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
#include "IO/Format.h"
#include "IO/TextWriter.h"
#include "Meta/PointerWFlags.h"
#include "Meta/ThreadResource.h"

#include <thread>

namespace Core {
LOG_CATEGORY(CORE_API, Task)
STATIC_CONST_INTEGRAL(size_t, GStalledFiberCapacity, 8);
STATIC_CONST_INTEGRAL(size_t, GTaskManagerQueueCapacity, 128);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
using FTaskFiberRef_ = FTaskFiberPool::FHandleRef;
//----------------------------------------------------------------------------
class FStalledFiber_ {
public:
    FStalledFiber_()
    :   _stalled(nullptr) {
        _taskContextAndPriority.Reset(nullptr, uintptr_t(ETaskPriority::Normal));
    }

    FStalledFiber_(ITaskContext* taskContext, FTaskFiberRef_ stalled, ETaskPriority priority)
    :   _stalled(stalled) {
        Assert(taskContext);
        _taskContextAndPriority.Reset(taskContext, uintptr_t(priority));
    }

#ifdef WITH_CORE_ASSERT
    ~FStalledFiber_() {
        Assert(nullptr == _stalled); // should have been resumed !
    }
#endif

    FStalledFiber_(const FStalledFiber_& ) = delete;
    FStalledFiber_& operator =(const FStalledFiber_& ) = delete;

    FStalledFiber_(FStalledFiber_&& rvalue)
    :   FStalledFiber_() {
        operator =(std::move(rvalue));
    }
    FStalledFiber_& operator =(FStalledFiber_&& rvalue) {
        Assert(nullptr == _stalled);
        _taskContextAndPriority = rvalue._taskContextAndPriority;
        _stalled = rvalue._stalled;
        rvalue._stalled = nullptr;
        return (*this);
    }

    void Resume(void (*resume)(FTaskFiberRef_)) {
        Assert(_stalled);

        FTaskFiberRef_ stalled = _stalled;
        _stalled = nullptr;

        _taskContextAndPriority->RunOne(nullptr,
            [resume, stalled](ITaskContext& ctx) {
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

    FTaskFiberRef_ _stalled;
};
typedef TFixedSizeRingBuffer<FStalledFiber_, GStalledFiberCapacity> FStalledFiberQueue_;
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class CACHELINE_ALIGNED FTaskCounter : public FRefCountable {
public:
    FTaskCounter() : _count(-1) {}
    ~FTaskCounter() { Assert(-1 == _count); }

    FTaskCounter(const FTaskCounter& ) = delete;
    FTaskCounter& operator =(const FTaskCounter& ) = delete;

    bool Valid() const;
    bool Finished() const;
    void Start(size_t count);
    bool WaitFor(FStalledFiber_&& waiting);
    void Clear();

    friend void Decrement_ResumeWaitingTasksIfZero(STaskCounter& saferef);

    OVERRIDE_CLASS_ALLOCATOR(ALIGNED_ALLOCATOR(Task, FTaskCounter, CACHELINE_SIZE))

private:
    mutable FAtomicSpinLock _lock;

    int _count;
    FStalledFiberQueue_ _queue;
};
STATIC_ASSERT(Meta::IsAligned(CACHELINE_SIZE, sizeof(FTaskCounter)));
//----------------------------------------------------------------------------
class FTaskManagerImpl : public ITaskContext {
public:
    explicit FTaskManagerImpl(FTaskManager& manager);
    ~FTaskManagerImpl();

    FTaskScheduler& Scheduler() { return _scheduler; }
    TMemoryView<std::thread> Threads() { return MakeView(_threads); }
    FTaskFiberPool& Fibers() { return _fibers; }

    void Start(const TMemoryView<const size_t>& threadAffinities);
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

#ifdef WITH_CORE_ASSERT
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

    static FWorkerContext_& Instance();

    static void ResumeFiberTask(FTaskFiberRef_ resume);
    static void ExitWorkerTask(ITaskContext& ctx);

private:
    STATIC_CONST_INTEGRAL(size_t, CacheSize, 32); // 32 kb
    STATIC_CONST_INTEGRAL(size_t, StackSize, 1024<<10); // 1 mb

    TFixedSizeRingBuffer<PTaskCounter, CacheSize> _counters;

    FTaskManager& _manager;
    FTaskFiberPool& _fibers;
    const size_t _workerIndex;
    FTaskFiberRef_ _fiberToRelease;

    size_t _revision = 0;

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
    if (not _counters.pop_front(&result))
        result.reset(new FTaskCounter());

#ifdef WITH_CORE_ASSERT
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

#ifdef WITH_CORE_ASSERT
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
NO_INLINE FWorkerContext_& FWorkerContext_::Instance() {
    Assert(FFiber::IsInFiber());
    Assert(nullptr != _gInstanceTLS);
    return (*_gInstanceTLS);
}
//----------------------------------------------------------------------------
void FWorkerContext_::ResumeFiberTask(FTaskFiberRef_ resume) {
    Instance()._fibers.YieldCurrentFiber(resume, true/* release this fiber in the pool */);
}
//----------------------------------------------------------------------------
void FWorkerContext_::ExitWorkerTask(ITaskContext&) {
    Assert(FFiber::ThreadFiber() != FFiber::RunningFiber());

    FTaskFiberRef_ self = FTaskFiberPool::CurrentHandleRef();
    Assert(self);

    FWorkerContext_& workerContext = Instance();
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
static void WorkerThreadLaunchpad_(FTaskManager* pmanager, size_t workerIndex, size_t affinityMask) {
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

        FTaskManagerImpl& impl = *FWorkerContext_::Instance().Manager().Pimpl();

        do {
            FTaskWaitHandle waitHandle;
            context.RunOne(&waitHandle, [](ITaskContext&) { NOOP(); }, ETaskPriority::Internal);
            context.WaitFor(waitHandle);

        } while (not impl.Scheduler().HasPendingTask());

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
,   _fibers(Meta::MakeFunction(this, &FTaskManagerImpl::WorkerLoop_))
#ifdef WITH_CORE_ASSERT
,   _countersInUse(0)
,   _fibersInUse(0)
#endif
{
    _threads.reserve(_manager.WorkerCount());
}
//----------------------------------------------------------------------------
FTaskManagerImpl::~FTaskManagerImpl() {
    Assert(_threads.empty());

#ifdef WITH_CORE_ASSERT
    Assert(0 == _countersInUse);
    Assert(0 == _fibersInUse);
#endif
}
//----------------------------------------------------------------------------
void FTaskManagerImpl::Start(const TMemoryView<const size_t>& threadAffinities) {
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

        *phandle = FTaskWaitHandle(priority, FWorkerContext_::Instance().CreateCounter());
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

        *phandle = FTaskWaitHandle(priority, FWorkerContext_::Instance().CreateCounter());
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

    FStalledFiber_ stalled(resume ? resume : this, FTaskFiberPool::CurrentHandleRef(), handle.Priority());
    if (handle._counter->WaitFor(std::move(stalled)))
        _fibers.YieldCurrentFiber(nullptr, false/* this fiber will be resumed later */);
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

        _scheduler.Consume(FWorkerContext_::Instance().WorkerIndex(), &task);

        Assert(task.Pending.Valid());
        task.Pending.Invoke(*this);

        // Decrement the counter and resume waiting tasks if any.
        // Won't steal the thread from this fiber, so the loop can safely finish
        if (task.Counter)
            Decrement_ResumeWaitingTasksIfZero(task.Counter);

        FWorkerContext_::Instance().DutyCycle();
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FTaskCounter::Valid() const {
    const FAtomicSpinLock::FScope scopedLock(_lock);
    return (0 <= _count);
}
//----------------------------------------------------------------------------
bool FTaskCounter::Finished() const {
    const FAtomicSpinLock::FScope scopedLock(_lock);
#ifdef WITH_CORE_ASSERT
    Assert(0 <= _count);
    if (0 == _count) {
        Assert(SafeRefCount() == 0);
        return true;
    }
    else {
        return false;
    }
#else
    return (0 == _count);
#endif
}
//----------------------------------------------------------------------------
void FTaskCounter::Start(size_t count) {
    const FAtomicSpinLock::FScope scopedLock(_lock);
    Assert(-1 == _count);

    _count = checked_cast<i32>(count);
}
//----------------------------------------------------------------------------
bool FTaskCounter::WaitFor(FStalledFiber_&& stalled) {
    const FAtomicSpinLock::FScope scopedLock(_lock);
    if (0 == _count) {
        Assert(_queue.empty());
        return false;
    }
    else {
        _queue.push_back(std::move(stalled));
        return true;
    }
}
//----------------------------------------------------------------------------
void FTaskCounter::Clear() {
    const FAtomicSpinLock::FScope scopedLock(_lock);
    Assert(0 == _count);
    Assert(_queue.empty());

    _count = -1;
    _queue.clear();
}
//----------------------------------------------------------------------------
NO_INLINE void Decrement_ResumeWaitingTasksIfZero(STaskCounter& saferef) {
    Assert(saferef);
    Assert(saferef->Valid());

    const FAtomicSpinLock::FScope scopedLock(saferef->_lock);

    if (0 == --saferef->_count) {
        // stalled fibers are resumed through a task to let the current fiber dispatch
        // all jobs to every worker thread before yielding

        FStalledFiber_ stalled;
        while (saferef->_queue.pop_front(&stalled))
            stalled.Resume(&FWorkerContext_::ResumeFiberTask);
    }

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
        FWorkerContext_::Instance().DestroyCounter(_counter);
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
    Assert(not _counter.valid());
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
void FTaskManager::Start(const TMemoryView<const size_t>& threadAffinities) {
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
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ITaskContext& CurrentTaskContext() {
    // need to be called from a worker thread ! (asserted by FWorkerContext_)
    const FTaskManager& manager = FWorkerContext_::Instance().Manager();
    FTaskManagerImpl* const pimpl = manager.Pimpl();
    Assert(pimpl);
    return *pimpl;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
