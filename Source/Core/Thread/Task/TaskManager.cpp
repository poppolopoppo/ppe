#include "stdafx.h"

#include "TaskManager.h"

#include "Allocator/Alloca.h"
#include "Container/RingBuffer.h"
#include "Container/Vector.h"
#include "Diagnostic/Logger.h"
#include "IO/Format.h"
#include "Meta/Delegate.h"
#include "Meta/PointerWFlags.h"
#include "Meta/ThreadResource.h"

#include "Thread/AtomicSpinLock.h"
#include "Thread/Fiber.h"
#include "Thread/ThreadContext.h"

#define WITH_CORE_NONBLOCKING_TASKPRIORITYQUEUE 0 // Turning on will cause the worker thread to busy wait, wasting CPU resources

#if WITH_CORE_NONBLOCKING_TASKPRIORITYQUEUE
#   include "Thread/MPMCBoundedQueue.h"
#else
#   include "Thread/ConcurrentQueue.h"
#endif

#include <thread>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
STATIC_CONST_INTEGRAL(size_t, FiberQueueCapacity_, 2048);
//----------------------------------------------------------------------------
class FFiberQueued_ {
public:
    FFiberQueued_() {
        _taskContextAndPriority.Reset(nullptr, uintptr_t(ETaskPriority::Normal));
    }
    FFiberQueued_(ITaskContext* taskContext, FFiber&& halted, ETaskPriority priority)
    :   _halted(std::move(halted)) {
        Assert(taskContext);
        _taskContextAndPriority.Reset(taskContext, uintptr_t(priority));
    }

    FFiberQueued_(const FFiberQueued_& ) = delete;
    FFiberQueued_& operator =(const FFiberQueued_& ) = delete;

    FFiberQueued_(FFiberQueued_&& ) = default;
    FFiberQueued_& operator =(FFiberQueued_&& ) = default;

    ITaskContext* TaskContext() const { return _taskContextAndPriority.Get(); }
    FFiber& Halted() { return _halted; }
    ETaskPriority Priority() { return ETaskPriority(_taskContextAndPriority.Flag01()); }

private:
    STATIC_ASSERT(uintptr_t(ETaskPriority::High) == 0);
    STATIC_ASSERT(uintptr_t(ETaskPriority::Normal) == 1);
    STATIC_ASSERT(uintptr_t(ETaskPriority::Low) == 2);
    Meta::TPointerWFlags<ITaskContext> _taskContextAndPriority;

    FFiber _halted;
};
STATIC_ASSERT(sizeof(FFiberQueued_) == 2 * sizeof(uintptr_t));
//----------------------------------------------------------------------------
static constexpr size_t GFiberQueueCapacity_ = 8;
typedef TFixedSizeRingBuffer<FFiberQueued_, GFiberQueueCapacity_> FFiberQueue_;
STATIC_ASSERT(sizeof(FFiberQueue_) == (4 + 2 * GFiberQueueCapacity_) * sizeof(uintptr_t));
//----------------------------------------------------------------------------
struct FTaskQueued_ {
    FTaskDelegate Pending;
    STaskCounter Counter;
};
STATIC_ASSERT(sizeof(FTaskQueued_) == 3 * sizeof(uintptr_t));
#if WITH_CORE_NONBLOCKING_TASKPRIORITYQUEUE
typedef TMPMCPriorityQueue<
    TMPMCBoundedQueue<FTaskQueued_>,
    size_t(ETaskPriority::_Count)
>   TaskPriorityQueue_;
#else
typedef CONCURRENT_PRIORITY_QUEUE(Task, FTaskQueued_) FTaskPriorityQueue_;
#endif
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
    bool WaitFor(FFiberQueued_&& waiting);
    void Clear();

    friend void Decrement_ResumeWaitingTasksIfZero(STaskCounter& saferef);

    OVERRIDE_CLASS_ALLOCATOR(ALIGNED_ALLOCATOR(Task, FTaskCounter, CACHELINE_SIZE))

private:
    mutable FAtomicSpinLock _lock;

    i32 _count;
    FFiberQueue_ _queue;
};
STATIC_ASSERT(Meta::IsAligned(CACHELINE_SIZE, sizeof(FTaskCounter)));
//----------------------------------------------------------------------------
class FTaskManagerImpl : public ITaskContext {
public:
    struct FTaskScope {
        FTaskManagerImpl* Pimpl;
        FTaskScope(FTaskManagerImpl* pimpl) : Pimpl(pimpl) {
            ++Pimpl->_taskRunningCount;
        }
        ~FTaskScope() {
            const int n = --Pimpl->_taskRunningCount;
            Assert(n >= 0);
        }
    };

    explicit FTaskManagerImpl(FTaskManager& manager);
    ~FTaskManagerImpl();

    FTaskPriorityQueue_& Queue() { return _queue; }
    TMemoryView<std::thread> Threads() { return MakeView(_threads); }
    size_t TaskRunningCount() const { return checked_cast<size_t>(_taskRunningCount); }

    void Start(const TMemoryView<const size_t>& threadAffinities);
    void Shutdown();

    virtual void Run(FTaskWaitHandle* phandle, const TMemoryView<const FTaskDelegate>& tasks, ETaskPriority priority) override final;
    virtual void WaitFor(FTaskWaitHandle& handle, ITaskContext* resume) override final;
    virtual void RunAndWaitFor(const TMemoryView<const FTaskDelegate>& tasks, ETaskPriority priority, ITaskContext* resume) override final;

private:
    FTaskPriorityQueue_ _queue;
    FTaskManager& _manager;
    VECTOR(Task, std::thread) _threads;

    std::atomic<int> _taskRunningCount;

#ifdef WITH_CORE_ASSERT
public: // public since this is complicated to friend FWorkerContext_ in an anonymous namespace ...
    std::atomic<int> _countersInUse;
    std::atomic<int> _fibersInUse;
#endif
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

    void CreateFiber(FFiber* pFiber);
    void DestroyFiber(FFiber& fiber);
    void ClearFibers();

    void ReleaseFiber();
    void ReleaseFiberIFP();

    void QueueResumeFiber(FFiber& fiber);
    void ResumeFiberIFP();

    void YieldFiber();

    static FWorkerContext_& Instance();

    static void ResumeFiberTask(ITaskContext& ctx, void* fiber);
    static void ExitWorkerTask(ITaskContext& ctx);

private:
    STATIC_CONST_INTEGRAL(size_t, CacheSize, 32); // 32 kb
    STATIC_CONST_INTEGRAL(size_t, StackSize, 1024<<10); // 1 mb

    FFiber _resumingFiber;
    FFiber _releasedFiber;

    TFixedSizeRingBuffer<PTaskCounter, CacheSize> _counters;
    TFixedSizeRingBuffer<FFiber, CacheSize> _fibers;

    FTaskManager& _manager;
    const size_t _workerIndex;

    static void STDCALL WorkerEntryPoint_(void* pArg);

    static THREAD_LOCAL FWorkerContext_* _gInstanceTLS;
};
THREAD_LOCAL FWorkerContext_* FWorkerContext_::_gInstanceTLS = nullptr;
//----------------------------------------------------------------------------
FWorkerContext_::FWorkerContext_(FTaskManager* pmanager, size_t workerIndex)
:   _manager(*pmanager)
,   _workerIndex(workerIndex) {
    Assert(pmanager);
    Assert(nullptr == _gInstanceTLS);

    _gInstanceTLS = this;
}
//----------------------------------------------------------------------------
FWorkerContext_::~FWorkerContext_() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(this == _gInstanceTLS);
    Assert(nullptr == _resumingFiber.Pimpl());
    Assert(nullptr == _releasedFiber.Pimpl());

    _gInstanceTLS = nullptr;

    ClearFibers();
    ClearCounters();
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
    Assert(pcounter->SafeRefCount() == 0);
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
void FWorkerContext_::CreateFiber(FFiber* pFiber) {
    Assert(pFiber);
    THIS_THREADRESOURCE_CHECKACCESS();

#ifdef WITH_CORE_ASSERT
    {
        FTaskManagerImpl* pimpl = _manager.Pimpl();
        ++pimpl->_fibersInUse;
    }
#endif

    if (not _fibers.pop_back(pFiber))
        pFiber->Create(&WorkerEntryPoint_, &_manager, StackSize);
}
//----------------------------------------------------------------------------
void FWorkerContext_::DestroyFiber(FFiber& fiber) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(fiber);

#ifdef WITH_CORE_ASSERT
    {
        FTaskManagerImpl* pimpl = _manager.Pimpl();
        --pimpl->_fibersInUse;
        Assert(0 <= pimpl->_fibersInUse);
    }
#endif

    _fibers.push_back_OverflowIFN(nullptr, std::move(fiber));

    Assert(nullptr == fiber.Pimpl());
}
//----------------------------------------------------------------------------
void FWorkerContext_::ClearFibers() {
    THIS_THREADRESOURCE_CHECKACCESS();

    FFiber fiber;
    while (_fibers.Dequeue(&fiber))
        fiber.Destroy();

    Assert(0 == _fibers.size());
}
//----------------------------------------------------------------------------
void FWorkerContext_::ReleaseFiber() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(_releasedFiber);
    Assert(_releasedFiber.Pimpl() != FFiber::RunningFiber());

    DestroyFiber(_releasedFiber);
}
//----------------------------------------------------------------------------
void FWorkerContext_::ReleaseFiberIFP() {
    THIS_THREADRESOURCE_CHECKACCESS();

    if (_releasedFiber)
        ReleaseFiber();
}
//----------------------------------------------------------------------------
void FWorkerContext_::QueueResumeFiber(FFiber& fiber) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(nullptr != fiber.Pimpl());
    Assert(nullptr == _resumingFiber);

    _resumingFiber = std::move(fiber);
}
//----------------------------------------------------------------------------
void FWorkerContext_::ResumeFiberIFP() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(nullptr == _releasedFiber);

    if (!_resumingFiber)
        return;

    _releasedFiber = FFiber::RunningFiber();
    Assert(_resumingFiber != _releasedFiber);

    FFiber fiber;
    fiber.Swap(_resumingFiber);

    fiber.Resume(); // switch to another fiber, next lines will be executed later !
    fiber.Reset(); // forget fiber pimpl to prevent destruction (the fiber is already recycled or destroyed)
}
//----------------------------------------------------------------------------
void FWorkerContext_::YieldFiber() {
    THIS_THREADRESOURCE_CHECKACCESS();

    // current fiber is not released, assuming it was queued in a counter or is the thread fiber !

    FFiber fiber;
    CreateFiber(&fiber);

    fiber.Resume(); // switch to another fiber, next lines will be executed later !
    fiber.Reset(); // forget fiber pimpl to prevent destruction (the fiber is already recycled or destroyed)
}
//----------------------------------------------------------------------------
NO_INLINE FWorkerContext_& FWorkerContext_::Instance() {
    Assert(FFiber::IsInFiber());
    Assert(nullptr != _gInstanceTLS);
    THREADRESOURCE_CHECKACCESS(_gInstanceTLS);
    return (*_gInstanceTLS);
}
//----------------------------------------------------------------------------
void FWorkerContext_::ResumeFiberTask(ITaskContext& ctx, void* fiber) {
    UNUSED(ctx);
    FFiber resume(fiber);
    Instance().QueueResumeFiber(resume);
    resume.Reset();
}
//----------------------------------------------------------------------------
void FWorkerContext_::ExitWorkerTask(ITaskContext& ctx) {
    UNUSED(ctx);
    FFiber resume(FFiber::ThreadFiber());
    Instance().QueueResumeFiber(resume);
    resume.Reset();
}
//----------------------------------------------------------------------------
void STDCALL FWorkerContext_::WorkerEntryPoint_(void* pArg) {
    Assert(FFiber::IsInFiber());
    Assert(FFiber::RunningFiber() != FFiber::ThreadFiber());
    Assert(nullptr != pArg);

    const FTaskManager& manager = *static_cast<const FTaskManager*>(pArg);

    Assert(manager.Pimpl());
    FTaskManagerImpl& impl = *manager.Pimpl();

    FTaskQueued_ task;
    while (true) {
        Instance().ReleaseFiberIFP();
        Instance().ResumeFiberIFP();

        malloc_release_pending_blocks();

#if WITH_CORE_NONBLOCKING_TASKPRIORITYQUEUE
        while (not impl.Queue().Consume(&task))
            std::this_thread::yield();
#else
        impl.Queue().Consume(&task);
#endif

        Assert(task.Pending.Valid());
        {
            const FTaskManagerImpl::FTaskScope taskScope(&impl);

            task.Pending.Invoke(impl);

            if (task.Counter)
                Decrement_ResumeWaitingTasksIfZero(task.Counter);
        }
    }
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

    char workerName[256];
    Format(workerName, "{0}_Worker#{1}", pmanager->Name(), workerIndex);
    const FThreadContextStartup threadStartup(workerName, pmanager->ThreadTag());

    threadStartup.Context().SetAffinityMask(affinityMask);
    threadStartup.Context().SetPriority(pmanager->Priority());

    FWorkerContext_ workerContext(pmanager, workerIndex);
    {
        const FFiber::FThreadScope fiberScope;
        workerContext.YieldFiber();
        workerContext.ReleaseFiber();
    }
}
//----------------------------------------------------------------------------
struct FRunAndWaitFor_ {
    TMemoryView<const FTaskDelegate> Tasks;
    ETaskPriority Priority;
    bool Available;

    std::mutex Barrier;
    std::condition_variable OnFinished;

    static void Task(ITaskContext& context, FRunAndWaitFor_* pArgs) {
        Assert(pArgs);

        context.RunAndWaitFor(pArgs->Tasks, pArgs->Priority);
        {
            std::unique_lock<std::mutex> scopeLock(pArgs->Barrier);
            Assert(false == pArgs->Available);
            pArgs->Available = true;
            pArgs->OnFinished.notify_all();
        }
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

        FTaskDelegate task = TDelegate(&Noop, nullptr);
        FTaskManagerImpl& impl = *FWorkerContext_::Instance().Manager().Pimpl();

        while (impl.TaskRunningCount() > 1) {
            FTaskWaitHandle waitHandle;
            context.RunOne(&waitHandle, task, ETaskPriority::_Reserved);
            context.WaitFor(waitHandle);
        }

        {
            std::unique_lock<std::mutex> scopeLock(pArgs->Barrier);
            Assert(false == pArgs->Available);
            pArgs->Available = true;
            pArgs->OnFinished.notify_all();
        }
    }
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTaskManagerImpl::FTaskManagerImpl(FTaskManager& manager)
:   _queue(FiberQueueCapacity_)
,   _manager(manager)
,   _taskRunningCount(0)
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
    Assert(_threads.capacity() == _manager.WorkerCount());
    Assert(0 == _taskRunningCount);

#ifdef WITH_CORE_ASSERT
    Assert(0 == _countersInUse);
    Assert(0 == _fibersInUse);
#endif
}
//----------------------------------------------------------------------------
void FTaskManagerImpl::Start(const TMemoryView<const size_t>& threadAffinities) {
    const size_t n = _manager.WorkerCount();
    AssertRelease(threadAffinities.size() == n);
    Assert(_threads.capacity() == n);
    Assert(_threads.empty());

    forrange(i, 0, n) {
        _threads.emplace_back(&WorkerThreadLaunchpad_, &_manager, i, threadAffinities[i]);
        Assert(_threads.back().joinable());
    }
}
//----------------------------------------------------------------------------
void FTaskManagerImpl::Shutdown() {
    const size_t n = _manager.WorkerCount();
    AssertRelease(n == _threads.size());

    {
        STACKLOCAL_POD_ARRAY(FTaskDelegate, exitTasks, n);
        forrange(i, 0, n) {
            exitTasks[i] = TDelegate(&FWorkerContext_::ExitWorkerTask, nullptr);
        }

        Run(nullptr, exitTasks, ETaskPriority::_Reserved);
    }

    forrange(i, 0, n) {
        Assert(_threads[i].joinable());
        _threads[i].join();
    }

    _threads.clear();
}
//----------------------------------------------------------------------------
void FTaskManagerImpl::Run(FTaskWaitHandle* phandle, const TMemoryView<const FTaskDelegate>& tasks, ETaskPriority priority /* = ETaskPriority::Normal */) {
    FTaskCounter* pcounter = nullptr;

    if (phandle) {
        Assert(not phandle->Valid());

        *phandle = FTaskWaitHandle(priority, FWorkerContext_::Instance().CreateCounter());
        Assert(phandle->Valid());
        Assert(phandle->Counter());

        pcounter = const_cast<FTaskCounter*>(phandle->Counter());
        pcounter->Start(tasks.size());
    }

#if WITH_CORE_NONBLOCKING_TASKPRIORITYQUEUE
    for (const FTaskDelegate& task : tasks) {
        FTaskQueued_ queued{ task, pcounter };

        while (false == _queue.Produce(size_t(priority), std::move(queued)))
            ::_mm_pause();
    }
#else
    _queue.Produce(
        int(priority),
        tasks.size(),
        _threads.size(),
        [&tasks, pcounter](size_t i) {
            return FTaskQueued_{ tasks[i], pcounter };
        });
#endif
}
//----------------------------------------------------------------------------
void FTaskManagerImpl::WaitFor(FTaskWaitHandle& handle, ITaskContext* resume) {
    Assert(FFiber::IsInFiber());
    Assert(handle.Valid());

    FFiberQueued_ waiting(resume ? resume : this, FFiber::RunningFiber(), handle.Priority());
    if (handle._counter->WaitFor(std::move(waiting)))
        FWorkerContext_::Instance().YieldFiber();
}
//----------------------------------------------------------------------------
void FTaskManagerImpl::RunAndWaitFor(const TMemoryView<const FTaskDelegate>& tasks, ETaskPriority priority, ITaskContext* resume) {
    FTaskWaitHandle handle;
    Run(&handle, tasks, priority);
    WaitFor(handle, resume);
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
bool FTaskCounter::WaitFor(FFiberQueued_&& waiting) {
    const FAtomicSpinLock::FScope scopedLock(_lock);
    if (0 == _count) {
        Assert(_queue.empty());
        return false;
    }
    else {
        _queue.push_back(std::move(waiting));
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
        FFiberQueued_ waiting;
        while (saferef->_queue.pop_front(&waiting)) {
            const FTaskDelegate task(TDelegate(&FWorkerContext_::ResumeFiberTask, waiting.Halted().Pimpl()));
            waiting.TaskContext()->RunOne(nullptr, task, waiting.Priority());
            waiting.Halted().Reset();
        }
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
    Assert(nullptr == _counter);

    _priority = rvalue._priority;
    _counter = std::move(rvalue._counter);

    rvalue._priority = ETaskPriority::Normal;
    Assert(nullptr == rvalue._counter);

    return *this;
}
//----------------------------------------------------------------------------
bool FTaskWaitHandle::Finished() const {
    Assert(nullptr != _counter);
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

    LOG(Info, L"[Task] Start manager #{0} with {1} workers and tag <{2}> ...", _name, _workerCount, _threadTag);

    _pimpl.reset(new FTaskManagerImpl(*this));
    _pimpl->Start(threadAffinities);
}
//----------------------------------------------------------------------------
void FTaskManager::Shutdown() {
    Assert(nullptr != _pimpl);

    LOG(Info, L"[Task] Shutdown manager #{0} with {1} workers and tag <{2}> ...", _name, _workerCount, _threadTag);

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
void FTaskManager::Run(const TMemoryView<const FTaskDelegate>& tasks, ETaskPriority priority /* = ETaskPriority::Normal */) const {
    Assert(not FFiber::IsInFiber());
    Assert(not tasks.empty());
    Assert(nullptr != _pimpl);

    _pimpl->Run(nullptr, tasks, priority);
}
//----------------------------------------------------------------------------
void FTaskManager::RunAndWaitFor(const TMemoryView<const FTaskDelegate>& tasks, ETaskPriority priority /* = ETaskPriority::Normal */) const {
    Assert(not FFiber::IsInFiber());
    Assert(not tasks.empty());
    Assert(nullptr != _pimpl);

    FRunAndWaitFor_ args{ tasks, priority, false };
    const FTaskDelegate utility = TDelegate(&FRunAndWaitFor_::Task, &args);
    {
        std::unique_lock<std::mutex> scopeLock(args.Barrier);
        _pimpl->RunOne(nullptr, utility, priority);
        args.OnFinished.wait(scopeLock, [&args]() { return args.Available; });
    }
}
//----------------------------------------------------------------------------
void FTaskManager::RunAndWaitFor(const TMemoryView<FTask* const>& tasks, ETaskPriority priority /* = ETaskPriority::Normal */) const {
    STACKLOCAL_POD_ARRAY(FTaskDelegate, delegates, tasks.size());
    forrange(i, 0, tasks.size())
        delegates[i] = *tasks[i];

    RunAndWaitFor(delegates.AddConst(), priority);
}
//----------------------------------------------------------------------------
void FTaskManager::WaitForAll() const {
    Assert(nullptr != _pimpl);

    FWaitForAll_ args{ false };
    const FTaskDelegate utility = TDelegate(&FWaitForAll_::Task, &args);
    {
        std::unique_lock<std::mutex> scopeLock(args.Barrier);
        _pimpl->RunOne(nullptr, utility, ETaskPriority::_Reserved);
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
