#include "stdafx.h"

#include "TaskManager.h"

#include "Allocator/Alloca.h"
#include "Container/RingBuffer.h"
#include "Container/Vector.h"
#include "Diagnostic/Logger.h"
#include "IO/Format.h"
#include "Meta/Delegate.h"
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
    FFiberQueued_() : _taskContext(nullptr), _priority(ETaskPriority::Normal) {}
    FFiberQueued_(ITaskContext* taskContext, FFiber&& halted, ETaskPriority priority)
    :   _taskContext(taskContext)
    ,   _halted(std::move(halted))
    ,   _priority(priority) {
        Assert(_taskContext);
    }

    FFiberQueued_(const FFiberQueued_& ) = delete;
    FFiberQueued_& operator =(const FFiberQueued_& ) = delete;

    FFiberQueued_(FFiberQueued_&& ) = default;
    FFiberQueued_& operator =(FFiberQueued_&& ) = default;

    ITaskContext* TaskContext() const { return _taskContext; }
    FFiber& Halted() { return _halted; }
    ETaskPriority Priority() { return _priority; }

private:
    ITaskContext* _taskContext;
    FFiber _halted;
    ETaskPriority _priority;
};
typedef TFixedSizeRingBuffer<FFiberQueued_, 8> FiberQueue_;
//----------------------------------------------------------------------------
struct FTaskQueued_ {
    FTaskDelegate Pending;
    PTaskCounter Counter;
};
#if WITH_CORE_NONBLOCKING_TASKPRIORITYQUEUE
typedef TMPMCPriorityQueue<
    TMPMCBoundedQueue<FTaskQueued_>,
    size_t(ETaskPriority::_Count)
>   TaskPriorityQueue_;
#else
typedef CONCURRENT_PRIORITY_QUEUE(Task, FTaskQueued_) TaskPriorityQueue_;
#endif
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FTaskCounter : public FRefCountable {
public:
    FTaskCounter() : _count(0) {}
    ~FTaskCounter() { Assert(0 == _count); }

    FTaskCounter(const FTaskCounter& ) = delete;
    FTaskCounter& operator =(const FTaskCounter& ) = delete;

    bool Finished() const { return (0 == _count); }

    void Reset(size_t count);
    void Decrement_ResumeWaitingTasksIfZero();

    void WaitFor(FFiberQueued_&& waiting);

    ALIGNED_ALLOCATED_DEF(FTaskCounter, CACHELINE_SIZE);

private:
    FAtomicSpinLock _lock;
    std::atomic<u32> _count;
    FiberQueue_ _queue;
};
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

    TaskPriorityQueue_& Queue() { return _queue; }
    TMemoryView<std::thread> Threads() { return MakeView(_threads); }
    size_t TaskRunningCount() const { return checked_cast<size_t>(_taskRunningCount); }

    void Start(const TMemoryView<const size_t>& threadAffinities);
    void Shutdown();

    virtual void Run(FTaskWaitHandle* phandle, const TMemoryView<const FTaskDelegate>& tasks, ETaskPriority priority) override final;
    virtual void WaitFor(FTaskWaitHandle& handle, ITaskContext* resume) override final;
    virtual void RunAndWaitFor(const TMemoryView<const FTaskDelegate>& tasks, ETaskPriority priority, ITaskContext* resume) override final;

private:
    TaskPriorityQueue_ _queue;
    FTaskManager& _manager;
    VECTOR(Task, std::thread) _threads;

    std::atomic<int> _taskRunningCount;

#ifdef WITH_CORE_ASSERT
public: // public since this is complicated to friend FWorkerContext_ in an anonymous namespace ...
    std::atomic<int> _countersInUse = 0;
    std::atomic<int> _fibersInUse = 0;
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
    void DestroyCounter(PTaskCounter& counter);
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
    STATIC_CONST_INTEGRAL(size_t, CacheSize, 32);
    STATIC_CONST_INTEGRAL(size_t, StackSize, 1024<<10);

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
    if (not _counters.pop_back(&result))
        result = new FTaskCounter();

#ifdef WITH_CORE_ASSERT
    {
        FTaskManagerImpl* pimpl = _manager.Pimpl();
        ++pimpl->_countersInUse;
    }
#endif

    Assert(nullptr != result);
    Assert(result->Finished());
    return result;
}
//----------------------------------------------------------------------------
void FWorkerContext_::DestroyCounter(PTaskCounter& counter) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(counter);

#ifdef WITH_CORE_ASSERT
    {
        FTaskManagerImpl* pimpl = _manager.Pimpl();
        --pimpl->_countersInUse;
        Assert(0 <= pimpl->_countersInUse);
    }
#endif

    FTaskCounter* const pcounter = RemoveRef_AssertReachZero_KeepAlive(counter);
    _counters.push_back_OverflowIFN(pcounter);
}
//----------------------------------------------------------------------------
void FWorkerContext_::ClearCounters() {
    THIS_THREADRESOURCE_CHECKACCESS();

    PTaskCounter counter;
    while (_counters.Dequeue(&counter))
        RemoveRef_AssertReachZero(counter);

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
        pFiber->Create(&WorkerEntryPoint_, &_manager);
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

    _fibers.push_back_OverflowIFN(std::move(fiber));

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
FWorkerContext_& FWorkerContext_::Instance() {
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

            if (task.Counter) {
                task.Counter->Decrement_ResumeWaitingTasksIfZero();
            }

            task = FTaskQueued_(); // reset to release reference to counter
        }

        Instance().ResumeFiberIFP();
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
,   _taskRunningCount(0) {
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
    PTaskCounter counter;

    if (phandle) {
        Assert(not phandle->Valid());

        counter = FWorkerContext_::Instance().CreateCounter();
        counter->Reset(tasks.size());

        *phandle = FTaskWaitHandle(priority, counter.get());
        Assert(phandle->Valid());
    }

    for (const FTaskDelegate& task : tasks) {
        FTaskQueued_ queued{ task, counter };

#if WITH_CORE_NONBLOCKING_TASKPRIORITYQUEUE
        while (false == _queue.Produce(size_t(priority), std::move(queued)))
            ::_mm_pause();
#else
        _queue.Produce(int(priority), std::move(queued));
#endif
    }
}
//----------------------------------------------------------------------------
void FTaskManagerImpl::WaitFor(FTaskWaitHandle& handle, ITaskContext* resume) {
    Assert(FFiber::IsInFiber());
    Assert(handle.Valid());

    FFiberQueued_ waiting(resume ? resume : this, FFiber::RunningFiber(), handle.Priority());
    handle._counter->WaitFor(std::move(waiting));

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
void FTaskCounter::Reset(size_t count) {
    const FAtomicSpinLock::FScope scopedLock(_lock);
    Assert(0 == _count);
    _count = checked_cast<u32>(count);
}
//----------------------------------------------------------------------------
void FTaskCounter::Decrement_ResumeWaitingTasksIfZero() {
    const FAtomicSpinLock::FScope scopedLock(_lock);
    const u32 previousCount = _count.fetch_sub(1);
    Assert(previousCount > 0);

    if (1 == previousCount) {
        Assert(0 == _count);

        if (_queue.empty())
            return;

        FFiberQueued_ waiting;
        while (_queue.pop_front(&waiting)) {
            const FTaskDelegate task(TDelegate(&FWorkerContext_::ResumeFiberTask, waiting.Halted().Pimpl()));
            waiting.TaskContext()->RunOne(nullptr, task, waiting.Priority());
            waiting.Halted().Reset();
        }
    }
}
//----------------------------------------------------------------------------
void FTaskCounter::WaitFor(FFiberQueued_&& waiting) {
    const FAtomicSpinLock::FScope scopedLock(_lock);
    Assert(0 < _count);
    _queue.push_back(std::move(waiting));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTaskWaitHandle::FTaskWaitHandle()
:   _priority(ETaskPriority::Normal)
,   _counter(nullptr) {}
//----------------------------------------------------------------------------
FTaskWaitHandle::FTaskWaitHandle(ETaskPriority priority, FTaskCounter* counter)
:   _priority(priority)
,   _counter(counter) {}
//----------------------------------------------------------------------------
FTaskWaitHandle::~FTaskWaitHandle() {
    if (_counter)
        FWorkerContext_::Instance().DestroyCounter(_counter);
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
