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

#define WITH_CORE_NONBLOCKING_TASKPRIORITYQUEUE 0

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
struct FiberQueued_ {
    Fiber Halted;
    TaskPriority Priority;
};
typedef FixedSizeRingBuffer<FiberQueued_, 8> FiberQueue_;
//----------------------------------------------------------------------------
struct TaskQueued_ {
    TaskDelegate Pending;
    PTaskCounter Counter;
};
#if WITH_CORE_NONBLOCKING_TASKPRIORITYQUEUE
typedef MPMCPriorityQueue<
    MPMCBoundedQueue<TaskQueued_>,
    size_t(TaskPriority::_Count)
>   TaskPriorityQueue_;
#else
typedef CONCURRENT_PRIORITY_QUEUE(Task, TaskQueued_) TaskPriorityQueue_;
#endif
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class TaskCounter : public RefCountable {
public:
    TaskCounter() : _count(0) {}
    ~TaskCounter() { Assert(0 == _count); }

    TaskCounter(const TaskCounter& ) = delete;
    TaskCounter& operator =(const TaskCounter& ) = delete;

    bool Finished() const { return (0 == _count); }

    void Reset(size_t count);
    void Decrement_ResumeWaitingTasksIfZero(TaskManagerImpl& impl);

    void WaitFor(FiberQueued_&& waiting);

    ALIGNED_ALLOCATED_DEF(TaskCounter, CACHELINE_SIZE);

private:
    AtomicSpinLock _lock;
    std::atomic<u32> _count;
    FiberQueue_ _queue;
};
//----------------------------------------------------------------------------
class TaskManagerImpl : public ITaskContext {
public:
    explicit TaskManagerImpl(TaskManager& manager);
    ~TaskManagerImpl();

    TaskPriorityQueue_& Queue() { return _queue; }
    MemoryView<std::thread> Threads() { return MakeView(_threads); }

    void Start(const MemoryView<const size_t>& threadAffinities);
    void Shutdown();

    virtual void Run(TaskWaitHandle* phandle, const MemoryView<const TaskDelegate>& tasks, TaskPriority priority) override;
    virtual void WaitFor(TaskWaitHandle& handle) override;
    virtual void RunAndWaitFor(const MemoryView<const TaskDelegate>& tasks, TaskPriority priority) override;

private:
    TaskPriorityQueue_ _queue;
    TaskManager& _manager;
    VECTOR(Task, std::thread) _threads;

#ifdef WITH_CORE_ASSERT
public: // public since this is complicated to friend WorkerContext_ in an anonymous namespace ...
    std::atomic<int> _countersInUse = 0;
    std::atomic<int> _fibersInUse = 0;
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
class WorkerContext_ : Meta::ThreadResource {
public:
    explicit WorkerContext_(TaskManager* pmanager, size_t workerIndex);
    ~WorkerContext_();

    WorkerContext_(const WorkerContext_& ) = delete;
    WorkerContext_& operator =(const WorkerContext_& ) = delete;

    const TaskManager& Manager() const { return _manager; }
    size_t WorkerIndex() const { return _workerIndex; }

    PTaskCounter CreateCounter();
    void DestroyCounter(PTaskCounter& counter);
    void ClearCounters();

    void CreateFiber(Fiber* pFiber);
    void DestroyFiber(Fiber& fiber);
    void ClearFibers();

    void ReleaseFiber();
    void ReleaseFiberIFP();
    void ResumeFiber(Fiber& fiber);
    void YieldFiber();

    static WorkerContext_& Instance();

    static void ResumeFiberTask(ITaskContext& ctx, void* fiber);
    static void ExitWorkerTask(ITaskContext& ctx);

private:
    STATIC_CONST_INTEGRAL(size_t, CacheSize, 32);
    STATIC_CONST_INTEGRAL(size_t, StackSize, 1024<<10);

    Fiber _releasedFiber;

    FixedSizeRingBuffer<PTaskCounter, CacheSize> _counters;
    FixedSizeRingBuffer<Fiber, CacheSize> _fibers;

    TaskManager& _manager;
    const size_t _workerIndex;

    static void STDCALL WorkerEntryPoint_(void* pArg);

    static THREAD_LOCAL WorkerContext_* _gInstanceTLS;
};
THREAD_LOCAL WorkerContext_* WorkerContext_::_gInstanceTLS = nullptr;
//----------------------------------------------------------------------------
WorkerContext_::WorkerContext_(TaskManager* pmanager, size_t workerIndex)
:   _manager(*pmanager)
,   _workerIndex(workerIndex) {
    Assert(pmanager);
    Assert(nullptr == _gInstanceTLS);

    _gInstanceTLS = this;
}
//----------------------------------------------------------------------------
WorkerContext_::~WorkerContext_() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(this == _gInstanceTLS);
    Assert(nullptr == _releasedFiber.Pimpl());

    _gInstanceTLS = nullptr;

    ClearFibers();
    ClearCounters();
}
//----------------------------------------------------------------------------
PTaskCounter WorkerContext_::CreateCounter() {
    THIS_THREADRESOURCE_CHECKACCESS();

    PTaskCounter result;
    if (not _counters.pop_back(&result))
        result = new TaskCounter();

#ifdef WITH_CORE_ASSERT
    {
        TaskManagerImpl* pimpl = _manager.Pimpl();
        ++pimpl->_countersInUse;
    }
#endif

    Assert(nullptr != result);
    Assert(result->Finished());
    return result;
}
//----------------------------------------------------------------------------
void WorkerContext_::DestroyCounter(PTaskCounter& counter) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(counter);

#ifdef WITH_CORE_ASSERT
    {
        TaskManagerImpl* pimpl = _manager.Pimpl();
        --pimpl->_countersInUse;
        Assert(0 <= pimpl->_countersInUse);
    }
#endif

    TaskCounter* const pcounter = RemoveRef_AssertReachZero_KeepAlive(counter);
    _counters.push_back_OverflowIFN(pcounter);
}
//----------------------------------------------------------------------------
void WorkerContext_::ClearCounters() {
    THIS_THREADRESOURCE_CHECKACCESS();

    PTaskCounter counter;
    while (_counters.Dequeue(&counter))
        RemoveRef_AssertReachZero(counter);

    Assert(0 == _counters.size());
}
//----------------------------------------------------------------------------
void WorkerContext_::CreateFiber(Fiber* pFiber) {
    Assert(pFiber);
    THIS_THREADRESOURCE_CHECKACCESS();

#ifdef WITH_CORE_ASSERT
    {
        TaskManagerImpl* pimpl = _manager.Pimpl();
        ++pimpl->_fibersInUse;
    }
#endif

    if (not _fibers.pop_back(pFiber))
        pFiber->Create(&WorkerEntryPoint_, &_manager);
}
//----------------------------------------------------------------------------
void WorkerContext_::DestroyFiber(Fiber& fiber) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(fiber);

#ifdef WITH_CORE_ASSERT
    {
        TaskManagerImpl* pimpl = _manager.Pimpl();
        --pimpl->_fibersInUse;
        Assert(0 <= pimpl->_fibersInUse);
    }
#endif

    _fibers.push_back_OverflowIFN(std::move(fiber));

    Assert(nullptr == fiber.Pimpl());
}
//----------------------------------------------------------------------------
void WorkerContext_::ClearFibers() {
    THIS_THREADRESOURCE_CHECKACCESS();

    Fiber fiber;
    while (_fibers.Dequeue(&fiber))
        fiber.Destroy();

    Assert(0 == _fibers.size());
}
//----------------------------------------------------------------------------
void WorkerContext_::ReleaseFiber() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(_releasedFiber);
    Assert(_releasedFiber.Pimpl() != Fiber::RunningFiber());

    DestroyFiber(_releasedFiber);
}
//----------------------------------------------------------------------------
void WorkerContext_::ReleaseFiberIFP() {
    THIS_THREADRESOURCE_CHECKACCESS();

    if (_releasedFiber)
        ReleaseFiber();
}
//----------------------------------------------------------------------------
void WorkerContext_::ResumeFiber(Fiber& fiber) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(nullptr != fiber.Pimpl());
    Assert(nullptr == _releasedFiber);

    _releasedFiber = Fiber::RunningFiber();
    Assert(fiber != _releasedFiber);

    fiber.Resume(); // switch to another fiber, next lines will be executed later !
    fiber.Reset(); // forget fiber pimpl to prevent destruction (the fiber is already recycled or destroyed)
}
//----------------------------------------------------------------------------
void WorkerContext_::YieldFiber() {
    THIS_THREADRESOURCE_CHECKACCESS();

    // current fiber is not released, assuming it was queued in a counter or is the thread fiber !

    Fiber fiber;
    CreateFiber(&fiber);

    fiber.Resume(); // switch to another fiber, next lines will be executed later !
    fiber.Reset(); // forget fiber pimpl to prevent destruction (the fiber is already recycled or destroyed)
}
//----------------------------------------------------------------------------
WorkerContext_& WorkerContext_::Instance() {
    Assert(Fiber::IsInFiber());
    Assert(nullptr != _gInstanceTLS);
    THREADRESOURCE_CHECKACCESS(_gInstanceTLS);
    return (*_gInstanceTLS);
}
//----------------------------------------------------------------------------
void WorkerContext_::ResumeFiberTask(ITaskContext& ctx, void* fiber) {
    UNUSED(ctx);
    Fiber resume(fiber);
    Instance().ResumeFiber(resume);
    resume.Reset();
}
//----------------------------------------------------------------------------
void WorkerContext_::ExitWorkerTask(ITaskContext& ctx) {
    UNUSED(ctx);
    Fiber resume(Fiber::ThreadFiber());
    Instance().ResumeFiber(resume);
    resume.Reset();
}
//----------------------------------------------------------------------------
void STDCALL WorkerContext_::WorkerEntryPoint_(void* pArg) {
    Assert(Fiber::IsInFiber());
    Assert(Fiber::RunningFiber() != Fiber::ThreadFiber());
    Assert(nullptr != pArg);

    const TaskManager& manager = *static_cast<const TaskManager*>(pArg);

    Assert(manager.Pimpl());
    TaskManagerImpl& impl = *manager.Pimpl();

    TaskQueued_ task;
    while (true) {
        Instance().ReleaseFiberIFP();


#if WITH_CORE_NONBLOCKING_TASKPRIORITYQUEUE
        while (not impl.Queue().Consume(&task))
            std::this_thread::yield();
#else
        impl.Queue().Consume(&task);
#endif

        Assert(task.Pending.Valid());

        task.Pending.Invoke(impl);

        if (task.Counter) {
            task.Counter->Decrement_ResumeWaitingTasksIfZero(impl);
        }

        task = TaskQueued_(); // reset to release reference to counter
    }
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static void WorkerThreadLaunchpad_(TaskManager* pmanager, size_t workerIndex, size_t affinityMask) {
    Assert(pmanager);

    char workerName[256];
    Format(workerName, "{0}_Worker#{1}", pmanager->Name(), workerIndex);
    const ThreadContextStartup threadStartup(workerName, pmanager->ThreadTag());

    threadStartup.Context().SetAffinityMask(affinityMask);

    WorkerContext_ workerContext(pmanager, workerIndex);
    {
        const Fiber::ThreadScope fiberScope;
        workerContext.YieldFiber();
        workerContext.ReleaseFiber();
    }
}
//----------------------------------------------------------------------------
struct RunAndWaitForArgs_ {
    MemoryView<const TaskDelegate> Tasks;
    TaskPriority Priority;
    bool Available;

    std::mutex Barrier;
    std::condition_variable OnFinished;
};

static void RunAndWaitForTask_(ITaskContext& context, RunAndWaitForArgs_* pArgs) {
    Assert(pArgs);

    context.RunAndWaitFor(pArgs->Tasks, pArgs->Priority);
    {
        std::unique_lock<std::mutex> scopeLock(pArgs->Barrier);
        Assert(false == pArgs->Available);
        pArgs->Available = true;
        pArgs->OnFinished.notify_all();
    }
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
TaskManagerImpl::TaskManagerImpl(TaskManager& manager)
:   _queue(FiberQueueCapacity_)
,   _manager(manager){
    _threads.reserve(_manager.WorkerCount());
}
//----------------------------------------------------------------------------
TaskManagerImpl::~TaskManagerImpl() {
    Assert(_threads.empty());
    Assert(_threads.capacity() == _manager.WorkerCount());

#ifdef WITH_CORE_ASSERT
    Assert(0 == _countersInUse);
    Assert(0 == _fibersInUse);
#endif
}
//----------------------------------------------------------------------------
void TaskManagerImpl::Start(const MemoryView<const size_t>& threadAffinities) {
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
void TaskManagerImpl::Shutdown() {
    const size_t n = _manager.WorkerCount();
    AssertRelease(n == _threads.size());

    {
        STACKLOCAL_POD_ARRAY(TaskDelegate, exitTasks, n);
        forrange(i, 0, n) {
            exitTasks[i] = Delegate(&WorkerContext_::ExitWorkerTask, nullptr);
        }

        Run(nullptr, exitTasks, TaskPriority::Low);
    }

    forrange(i, 0, n) {
        Assert(_threads[i].joinable());
        _threads[i].join();
    }

    _threads.clear();
}
//----------------------------------------------------------------------------
void TaskManagerImpl::Run(TaskWaitHandle* phandle, const MemoryView<const TaskDelegate>& tasks, TaskPriority priority /* = TaskPriority::Normal */) {
    PTaskCounter counter;

    if (phandle) {
        Assert(not phandle->Valid());

        counter = WorkerContext_::Instance().CreateCounter();
        counter->Reset(tasks.size());

        *phandle = TaskWaitHandle(priority, counter.get());
        Assert(phandle->Valid());
    }

    for (const TaskDelegate& task : tasks) {
        TaskQueued_ queued{ task, counter };

#if WITH_CORE_NONBLOCKING_TASKPRIORITYQUEUE
        while (false == _queue.Produce(size_t(priority), std::move(queued)))
            ::_mm_pause();
#else
        _queue.Produce(int(priority), std::move(queued));
#endif
    }
}
//----------------------------------------------------------------------------
void TaskManagerImpl::WaitFor(TaskWaitHandle& handle) {
    Assert(Fiber::IsInFiber());
    Assert(handle.Valid());

    FiberQueued_ waiting;
    waiting.Halted = Fiber::RunningFiber();
    waiting.Priority = handle.Priority();

    handle._counter->WaitFor(std::move(waiting));

    WorkerContext_::Instance().YieldFiber();
}
//----------------------------------------------------------------------------
void TaskManagerImpl::RunAndWaitFor(const MemoryView<const TaskDelegate>& tasks, TaskPriority priority /* = TaskPriority::Normal */) {
    TaskWaitHandle handle;
    Run(&handle, tasks, priority);
    WaitFor(handle);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void TaskCounter::Reset(size_t count) {
    const AtomicSpinLock::Scope scopedLock(_lock);
    Assert(0 == _count);
    _count = checked_cast<u32>(count);
}
//----------------------------------------------------------------------------
void TaskCounter::Decrement_ResumeWaitingTasksIfZero(TaskManagerImpl& impl) {
    const AtomicSpinLock::Scope scopedLock(_lock);
    const u32 previousCount = _count.fetch_sub(1);
    Assert(previousCount > 0);

    if (1 == previousCount) {
        Assert(0 == _count);

        if (_queue.empty())
            return;

        FiberQueued_ waiting;
        while (_queue.pop_front(&waiting)) {
            const TaskDelegate task(Delegate(&WorkerContext_::ResumeFiberTask, waiting.Halted.Pimpl()));
            impl.RunOne(nullptr, task, waiting.Priority);
            waiting.Halted.Reset();
        }
    }
}
//----------------------------------------------------------------------------
void TaskCounter::WaitFor(FiberQueued_&& waiting) {
    const AtomicSpinLock::Scope scopedLock(_lock);
    Assert(0 < _count);
    _queue.push_back(std::move(waiting));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
TaskWaitHandle::TaskWaitHandle()
:   _priority(TaskPriority::Normal)
,   _counter(nullptr) {}
//----------------------------------------------------------------------------
TaskWaitHandle::TaskWaitHandle(TaskPriority priority, TaskCounter* counter)
:   _priority(priority)
,   _counter(counter) {}
//----------------------------------------------------------------------------
TaskWaitHandle::~TaskWaitHandle() {
    if (_counter)
        WorkerContext_::Instance().DestroyCounter(_counter);
}
//----------------------------------------------------------------------------
TaskWaitHandle::TaskWaitHandle(TaskWaitHandle&& rvalue) {
    operator =(std::move(rvalue));
}
//----------------------------------------------------------------------------
TaskWaitHandle& TaskWaitHandle::operator =(TaskWaitHandle&& rvalue) {
    Assert(nullptr == _counter);

    _priority = rvalue._priority;
    _counter = std::move(rvalue._counter);

    rvalue._priority = TaskPriority::Normal;
    Assert(nullptr == rvalue._counter);

    return *this;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
TaskManager::TaskManager(const char *name, size_t threadTag, size_t workerCount)
:   _name(name)
,   _threadTag(threadTag)
,   _workerCount(workerCount) {
    Assert(nullptr != _name);
    Assert(_workerCount > 0);
}
//----------------------------------------------------------------------------
TaskManager::~TaskManager() {
    Assert(nullptr == _pimpl);
}
//----------------------------------------------------------------------------
void TaskManager::Start(const MemoryView<const size_t>& threadAffinities) {
    Assert(nullptr == _pimpl);

    LOG(Info, L"[Task] Start manager #{0} with {1} workers and tag <{2}> ...", _name, _workerCount, _threadTag);

    _pimpl.reset(new TaskManagerImpl(*this));
    _pimpl->Start(threadAffinities);
}
//----------------------------------------------------------------------------
void TaskManager::Shutdown() {
    Assert(nullptr != _pimpl);

    LOG(Info, L"[Task] Shutdown manager #{0} with {1} workers and tag <{2}> ...", _name, _workerCount, _threadTag);

    _pimpl->Shutdown();
    _pimpl.reset();
}
//----------------------------------------------------------------------------
void TaskManager::Run(const MemoryView<const TaskDelegate>& tasks, TaskPriority priority /* = TaskPriority::Normal */) const {
    Assert(not Fiber::IsInFiber());
    Assert(not tasks.empty());
    Assert(nullptr != _pimpl);

    _pimpl->Run(nullptr, tasks, priority);
}
//----------------------------------------------------------------------------
void TaskManager::RunAndWaitFor(const MemoryView<const TaskDelegate>& tasks, TaskPriority priority /* = TaskPriority::Normal */) const {
    Assert(not Fiber::IsInFiber());
    Assert(not tasks.empty());
    Assert(nullptr != _pimpl);

    RunAndWaitForArgs_ args{ tasks, priority, false };
    const TaskDelegate utility = Delegate(&RunAndWaitForTask_, &args);
    {
        std::unique_lock<std::mutex> scopeLock(args.Barrier);
        _pimpl->RunOne(nullptr, utility, priority);
        args.OnFinished.wait(scopeLock, [&args]() { return args.Available; });
    }
}
//----------------------------------------------------------------------------
void TaskManager::RunAndWaitFor(const MemoryView<Task* const>& tasks, TaskPriority priority /* = TaskPriority::Normal */) const {
    STACKLOCAL_POD_ARRAY(TaskDelegate, delegates, tasks.size());
    forrange(i, 0, tasks.size())
        delegates[i] = *tasks[i];

    RunAndWaitFor(delegates.AddConst(), priority);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ITaskContext& CurrentTaskContext() {
    // need to be called from a worker thread ! (asserted by WorkerContext_)
    const TaskManager& manager = WorkerContext_::Instance().Manager();
    TaskManagerImpl* const pimpl = manager.Pimpl();
    Assert(pimpl);
    return *pimpl;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core