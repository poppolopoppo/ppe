#include "stdafx.h"

#include "TaskPool.h"

#include "Thread/Task/Task.h"
#include "Thread/Task/TaskPriorityQueue.h"

#include "Thread/Fiber.h"
#include "Thread/MRUCache.h"
#include "Thread/ThreadContext.h"

#include "Allocator/PoolAllocator.h"
#include "Allocator/PoolAllocator-impl.h"
#include "Diagnostic/Logger.h"
#include "IO/Format.h"

#include <atomic>
#include <thread>

#ifdef OS_WINDOWS
#   include <windows.h>
#else
#   error "OS not yet supported"
#endif

#define CORE_TASKPOOL_MRU_CACHESIZE     32
#define CORE_TASKPOOL_QUEUE_CAPACITY    (4096)

#define WITH_CORE_TASKPOOL_MRU_FIBER_CACHE_TLS      1
#define WITH_CORE_TASKPOOL_MRU_COUNTER_CACHE_TLS    1

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
struct FiberQueued;
}
//----------------------------------------------------------------------------
class TaskCounter {
public:
    explicit TaskCounter(size_t count);
    ~TaskCounter();

    bool Finished();
    void Decrement();

    void Reset(size_t count);

    SINGLETON_POOL_ALLOCATED_DECL(TaskCounter);

private:
    std::atomic<int> _count;
}; 
SINGLETON_POOL_ALLOCATED_DEF(TaskCounter, );
//----------------------------------------------------------------------------
TaskCounter::TaskCounter(size_t count)
:   _count(int(count)) {
    Assert(count > 0);
}
//----------------------------------------------------------------------------
TaskCounter::~TaskCounter() {
    Assert(Finished());
}
//----------------------------------------------------------------------------
bool TaskCounter::Finished() {
    Assert(_count >= 0);
    return (0 == _count);
}
//----------------------------------------------------------------------------
void TaskCounter::Decrement() {
    --_count;
}
//----------------------------------------------------------------------------
void TaskCounter::Reset(size_t count) {
    Assert(Finished());
    Assert(count > 0);
    _count = int(count);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static void __stdcall TaskFiberLoop_(void *arg);
//----------------------------------------------------------------------------
struct FiberQueued {
    Fiber Preempted;
    TaskCounter *Counter;
    Fiber Released;
};
//----------------------------------------------------------------------------
using WaitingFibers = MPMCBoundedQueue<FiberQueued *>;
using MRUFibersCache = MRUCache<void, CORE_TASKPOOL_MRU_CACHESIZE>;
using MRUCountersCache = MRUCache<TaskCounter, CORE_TASKPOOL_MRU_CACHESIZE>;
//----------------------------------------------------------------------------
struct TaskThreadContext {
    FiberQueued *WaitingForPFiber = nullptr;
    void *FiberToReleaseBeforeExit = nullptr;

#if WITH_CORE_TASKPOOL_MRU_FIBER_CACHE_TLS
    MRUFibersCache MRUFibersTLS;
#endif

#if WITH_CORE_TASKPOOL_MRU_COUNTER_CACHE_TLS
    MRUCountersCache MRUCountersTLS;
#endif

    size_t WorkerIndex;
};
static void TaskContextStartup_(TaskThreadContext& ctx, TaskPool *ppool);
static void TaskContextShutdown_(TaskThreadContext& ctx, TaskPool *ppool);
//----------------------------------------------------------------------------
static THREAD_LOCAL TaskThreadContext *gTaskThreadContextTLS = nullptr;
static NO_INLINE TaskThreadContext& CurrentTaskThreadContext() {
    Assert(gTaskThreadContextTLS);
    return *gTaskThreadContextTLS;
}
//----------------------------------------------------------------------------
static void TaskThreadStartup_(TaskPool *ppool, size_t workerIndex);
static void TaskThreadShutdown_(TaskPool *ppool);
struct TaskThreadScope : Fiber::ThreadScope, TaskThreadContext {
    TaskPool *Pool;

    NO_INLINE TaskThreadScope(TaskPool *pool, size_t workerIndex) : Pool(pool) { 
        Assert(!gTaskThreadContextTLS);
        gTaskThreadContextTLS = this;
        TaskContextStartup_(*gTaskThreadContextTLS, Pool);
        TaskThreadStartup_(Pool, workerIndex); 
    }

    NO_INLINE ~TaskThreadScope() { 
        Assert(this == gTaskThreadContextTLS);
        TaskThreadShutdown_(Pool); 
        TaskContextShutdown_(*gTaskThreadContextTLS, Pool);
        gTaskThreadContextTLS = nullptr;
    }
};
//----------------------------------------------------------------------------
static void TaskThreadLaunchpad_(TaskPool *ppool, size_t workerIndex) {
    Assert(ppool);

    char workerName[256];
    Format(workerName, "{0}_Worker#{1}", ppool->Name(), workerIndex);
    const ThreadContextStartup context(workerName, WORKER_THREADTAG);
    
    const TaskThreadScope taskThreadScope(ppool, workerIndex);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class TaskPoolImpl : TaskThreadContext {
public:
    TaskPoolImpl(TaskPool *pool, size_t queueCapacity, size_t workerCount);
    ~TaskPoolImpl();

    size_t WorkerCount() const { return _workerCount; }

    bool SignalExit() const { return _signalExit; }

    TaskPriorityQueue& Queue() { return _queue; }
    FiberFactory& Fibers() { return _fibers; }

    WaitingFibers& Waiting() { return *_pwaiting; }

    void Start();
    void Shutdown();

private:
    std::atomic<bool> _signalExit;

    TaskPriorityQueue _queue;
    WaitingFibers *_pwaiting;
    FiberFactory _fibers;

    std::thread *_threads;
    size_t _workerCount;
};
//----------------------------------------------------------------------------
TaskPoolImpl::TaskPoolImpl(TaskPool *pool, size_t queueCapacity, size_t workerCount) 
:   _signalExit(true) 
,   _queue(queueCapacity)
,   _pwaiting(nullptr)
,   _fibers(&TaskFiberLoop_, pool)
,   _threads(nullptr)
,   _workerCount(workerCount) {
    Assert(_workerCount > 0);
}
//----------------------------------------------------------------------------
TaskPoolImpl::~TaskPoolImpl() {
    Assert(nullptr == _threads);
    Assert(nullptr == _pwaiting);
}
//----------------------------------------------------------------------------
void TaskPoolImpl::Start() {
    Assert(nullptr == _threads);
    Assert(nullptr == _pwaiting);
    Assert(_signalExit);

    _signalExit = false;
    _pwaiting = new WaitingFibers(_queue.capacity());

    _threads = new std::thread[_workerCount];
    TaskPool *const ppool = reinterpret_cast<TaskPool *>(_fibers.Arg());

    Assert(nullptr == gTaskThreadContextTLS);
    gTaskThreadContextTLS = this;
    TaskContextStartup_(*gTaskThreadContextTLS, ppool);

    for (size_t i = 0; i < _workerCount; ++i) {
        Assert(!_threads[i].joinable());
        _threads[i] = std::thread(&TaskThreadLaunchpad_, ppool, i);
    }
}
//----------------------------------------------------------------------------
void TaskPoolImpl::Shutdown() {
    Assert(nullptr != _threads);
    Assert(nullptr != _pwaiting);
    Assert(!_signalExit);

    _signalExit = true;

    for (size_t i = 0; i < _workerCount; ++i) {
        Assert(_threads[i].joinable());
        _threads[i].join();
    }

    Assert(_queue.empty());
    Assert(_pwaiting->empty());

    delete[](_threads);
    _threads = nullptr;

    delete(_pwaiting);
    _pwaiting = nullptr;

    TaskPool *const ppool = reinterpret_cast<TaskPool *>(_fibers.Arg());

    Assert(this == gTaskThreadContextTLS);
    TaskContextShutdown_(*gTaskThreadContextTLS, ppool);
    gTaskThreadContextTLS = nullptr;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static void TaskContextStartup_(TaskThreadContext& ctx, TaskPool *ppool) {
    Assert(ppool);
    Assert(nullptr == ctx.WaitingForPFiber);
    Assert(nullptr == ctx.FiberToReleaseBeforeExit);

#if WITH_CORE_TASKPOOL_MRU_FIBER_CACHE_TLS
    Assert(ctx.MRUFibersTLS.empty());
#endif

#if WITH_CORE_TASKPOOL_MRU_COUNTER_CACHE_TLS
    Assert(ctx.MRUCountersTLS.empty());
#endif
}
//----------------------------------------------------------------------------
static void TaskContextShutdown_(TaskThreadContext& ctx, TaskPool *ppool) {
    Assert(ppool);
    Assert(nullptr == ctx.WaitingForPFiber);

#if WITH_CORE_TASKPOOL_MRU_FIBER_CACHE_TLS
    {
        FiberFactory& fibers = ppool->Pimpl()->Fibers();
        void *fiber = nullptr;
        while (!ctx.MRUFibersTLS.Get_ReturnIfEmpty(&fiber)) {
            Assert(fiber);
            Fiber toRelease(fiber);
            fibers.Release(toRelease);
            fiber = nullptr;
        }
        ctx.MRUFibersTLS.Clear_AssumeCacheDestroyed();
    }
#endif

#if WITH_CORE_TASKPOOL_MRU_COUNTER_CACHE_TLS
    {
        TaskCounter *counter = nullptr;
        while (!ctx.MRUCountersTLS.Get_ReturnIfEmpty(&counter)) {
            Assert(counter);
            delete(counter);
            counter = nullptr;
        }
        ctx.MRUCountersTLS.Clear_AssumeCacheDestroyed();
    }
#endif
}
//----------------------------------------------------------------------------
static void TaskThreadStartup_(TaskPool *ppool, size_t workerIndex) {
    Assert(ppool);
    Assert(Fiber::RunningFiber() == Fiber::ThreadFiber());

    TaskThreadContext& ctx = CurrentTaskThreadContext();
    ctx.WorkerIndex = workerIndex;

    LOG(Information, L"[Tasks] Starting task worker with index #{0} from pool \"{1}\" ...",
        workerIndex, ppool->Name() );

    Fiber newFiber = ppool->Pimpl()->Fibers().Create();
    newFiber.Resume();
}
//----------------------------------------------------------------------------
static void TaskThreadShutdown_(TaskPool *ppool) {
    Assert(ppool);
    Assert(Fiber::RunningFiber() == Fiber::ThreadFiber());

    TaskThreadContext& ctx = CurrentTaskThreadContext();

    LOG(Information, L"[Tasks] Stopping task worker with index #{0} from pool \"{1}\" ...",
        ctx.WorkerIndex, ppool->Name() );

    Assert(nullptr != ctx.FiberToReleaseBeforeExit);
    Fiber toRelease = ctx.FiberToReleaseBeforeExit;
    ctx.FiberToReleaseBeforeExit = nullptr;
    ppool->Pimpl()->Fibers().Release(toRelease);
}
//----------------------------------------------------------------------------
static bool EnqueueCurrentThreadWaitingFiberIFN_(TaskPoolImpl *pimpl) {
    TaskThreadContext& ctx = CurrentTaskThreadContext();
    if (ctx.WaitingForPFiber) {
        FiberQueued *const waitingFor = ctx.WaitingForPFiber;
        ctx.WaitingForPFiber = nullptr;
        if (!pimpl->Waiting().Enqueue(waitingFor))
            throw std::exception("waiting queue not big enough");
        return true;
    }
    return false;
}
//----------------------------------------------------------------------------
static void EnqueueCurrentThreadWaitingFiber_(TaskPoolImpl *pimpl) {
    if (!EnqueueCurrentThreadWaitingFiberIFN_(pimpl))
        Assert(false);
}
//----------------------------------------------------------------------------
static void __stdcall TaskFiberLoop_(void *arg) {
    Assert(arg);
    TaskPool *const ppool = reinterpret_cast<TaskPool *>(arg);
    TaskPoolImpl *const pimpl = ppool->Pimpl();

    TaskPriorityQueue& queue = pimpl->Queue();
    WaitingFibers& waiting = pimpl->Waiting();

    EnqueueCurrentThreadWaitingFiberIFN_(pimpl);

    TaskQueued toRun;
    FiberQueued *toResume = nullptr;

    for (;;) {
        Assert(nullptr == CurrentTaskThreadContext().WaitingForPFiber);

        bool nothingDone = true;

        if (queue.Dequeue(&toRun)) {
            Assert(toRun.Task.Valid());

            toRun.Task(*ppool);

            if (toRun.Counter)
                toRun.Counter->Decrement();

            nothingDone = false;
        }

        if (waiting.Dequeue(&toResume)) {
            Assert(toResume);
            if (toResume->Counter->Finished()) {
                Assert(!toResume->Released);
                toResume->Released = Fiber::RunningFiber();
                Assert(toResume->Released != toResume->Preempted);

                {
                    Fiber preempted = toResume->Preempted;
                    toResume = nullptr;
                    std::atomic_thread_fence(std::memory_order_seq_cst);
                    preempted.Resume();
                }

#if WITH_CORE_TASKPOOL_MRU_FIBER_CACHE_TLS
                EnqueueCurrentThreadWaitingFiber_(pimpl);
#else
                Assert(false);
#endif
            }
            else {
                if (!waiting.Enqueue(toResume))
                    throw std::exception("waiting queue not big enough");
            }

            nothingDone = false;
        }

        if (nothingDone) {
            if (pimpl->SignalExit())
                break;
            else
                std::this_thread::yield();
        }
    }
    Assert(pimpl->SignalExit());
    {
        TaskThreadContext& ctx = CurrentTaskThreadContext();
        Assert(nullptr == ctx.FiberToReleaseBeforeExit);
        Fiber threadFiber = Fiber::ThreadFiber();
        ctx.FiberToReleaseBeforeExit = Fiber::RunningFiber();
        Assert(ctx.FiberToReleaseBeforeExit != threadFiber);
        std::atomic_thread_fence(std::memory_order_seq_cst);
        threadFiber.Resume();
    }
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
TaskPool::TaskPool(const char *name, size_t workerCount)
:   _name(name)
,   _workerCount(workerCount) {
    Assert(nullptr == _pimpl);
    Assert(nullptr != _name);
    Assert(0 < _workerCount);
}
//----------------------------------------------------------------------------
TaskPool::~TaskPool() {
    Assert(nullptr == _pimpl);
}
//----------------------------------------------------------------------------
void TaskPool::Run(const Task *ptasks, size_t count, TaskCounter **pcounter/* = nullptr */, TaskPriority priority/* = TaskPriority::Normal */) const {
    Assert(_pimpl);
    Assert(ptasks);
    Assert(count > 0);

    TaskCounter *counter = nullptr;
    if (pcounter) {
        Assert(nullptr == *pcounter);
#if WITH_CORE_TASKPOOL_MRU_COUNTER_CACHE_TLS
        TaskThreadContext& ctx = CurrentTaskThreadContext();
        if (ctx.MRUCountersTLS.Get_ReturnIfEmpty(&counter))
            counter = new TaskCounter(count);
        else
            counter->Reset(count);
#else
        counter = new TaskCounter(count);
#endif
        Assert(counter);
        *pcounter = counter;
    }

    TaskPriorityQueue& queue = _pimpl->Queue();

    for (size_t i = 0; i < count; ++i) {
        const TaskQueued taskQueued { ptasks[i], counter };
        Assert(taskQueued.Task.Valid());
        if (!queue.Enqueue(taskQueued, priority))
            throw std::exception("task queue not big enough");
    }
}
//----------------------------------------------------------------------------
void TaskPool::WaitFor(TaskCounter **pcounter) const {
    Assert(_pimpl);
    Assert(pcounter);
    Assert(*pcounter);

    if (!(*pcounter)->Finished()) {
        const Fiber runningFiber = Fiber::RunningFiberIFP();

        if (runningFiber) {
            FiberQueued waitFor = { runningFiber, *pcounter, Fiber() };
            Assert(!waitFor.Released);

            Fiber newFiber;
            Assert(!newFiber);
            {
                TaskThreadContext& ctx = CurrentTaskThreadContext();
                Assert(!ctx.WaitingForPFiber);
                ctx.WaitingForPFiber = &waitFor;

#if WITH_CORE_TASKPOOL_MRU_FIBER_CACHE_TLS
                if (ctx.MRUFibersTLS.Get_ReturnIfEmpty(&newFiber._pimpl))
                    newFiber = _pimpl->Fibers().Create();
#else
                newFiber = _pimpl->Fibers().Create();
#endif
            }
            Assert(newFiber);

            std::atomic_thread_fence(std::memory_order_seq_cst);
            newFiber.Resume();

            Assert(waitFor.Counter->Finished());
            Assert(waitFor.Preempted == Fiber::RunningFiber());
            Assert(waitFor.Released);
            Assert(waitFor.Released != waitFor.Preempted);

            {
                TaskThreadContext& ctx = CurrentTaskThreadContext();
                Assert(!ctx.WaitingForPFiber);

#if WITH_CORE_TASKPOOL_MRU_FIBER_CACHE_TLS
                if (ctx.MRUFibersTLS.Release_ReturnIfFull(&waitFor.Released._pimpl))
                    _pimpl->Fibers().Release(waitFor.Released);
#else
                _pimpl->Fibers().Release(waitFor.Released);
#endif
                Assert(!waitFor.Released);
            }
        }
        else {
            while (!(*pcounter)->Finished())
                std::this_thread::yield();
        }
    }

    Assert((*pcounter)->Finished());
#if WITH_CORE_TASKPOOL_MRU_COUNTER_CACHE_TLS
    {
        TaskThreadContext& ctx = CurrentTaskThreadContext();
        if (ctx.MRUCountersTLS.Release_ReturnIfFull(pcounter)) {
            delete(*pcounter);
            *pcounter = nullptr;
        }
    }
#else
    {
        delete(*pcounter);
        *pcounter = nullptr;
    }
#endif
    Assert(nullptr == *pcounter);
}
//----------------------------------------------------------------------------
void TaskPool::RunAndWaitFor(const Task *ptasks, size_t count, TaskPriority priority/* = TaskPriority::Normal */) const {
    TaskCounter *counter = nullptr;
    Run(ptasks, count, &counter, priority);
    WaitFor(&counter);
}
//----------------------------------------------------------------------------
void TaskPool::Start() {
    Assert(!_pimpl);
    
    LOG(Information, L"[Tasks] Starting task pool \"{0}\" with {1} workers ...",
        _name, _workerCount );

    _pimpl.reset(new TaskPoolImpl(this, CORE_TASKPOOL_QUEUE_CAPACITY, _workerCount));
    _pimpl->Start();
}
//----------------------------------------------------------------------------
void TaskPool::Shutdown() {
    Assert(_pimpl);

    LOG(Information, L"[Tasks] Stopping task pool \"{0}\" with {1} workers ...",
        _name, _workerCount );

    _pimpl->Shutdown();
    _pimpl.reset();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
