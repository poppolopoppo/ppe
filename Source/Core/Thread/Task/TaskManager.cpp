#include "stdafx.h"

#include "TaskManager.h"

#include "Allocator/Alloca.h"
#include "Container/BitMask.h"
#include "Container/RingBuffer.h"
#include "Container/Vector.h"
#include "Diagnostic/Logger.h"
#include "IO/Format.h"
#include "IO/TextWriter.h"
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
LOG_CATEGORY(CORE_API, Task)
STATIC_CONST_INTEGRAL(size_t, GFiberStackSize, ALLOCATION_GRANULARITY);
STATIC_CONST_INTEGRAL(size_t, GFiberPoolCapacity, 128);
STATIC_CONST_INTEGRAL(size_t, GStalledFiberCapacity, 8);
STATIC_CONST_INTEGRAL(size_t, GTaskManagerQueueCapacity, 128);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
// Helper class which carefully tracks usage of a fixed pool of fibers
//----------------------------------------------------------------------------
class FFiberPool_ {
public:
    using FCallback = Meta::TFunction<void()>;

    class FHandle { // opaque handle, can't be manipulated directly
    private:
        friend class FFiberPool_;

#ifdef WITH_CORE_ASSERT
        ~FHandle() {
            Assert(INDEX_NONE == _index);
            Assert(nullptr == _owner);
        }
#endif

        void WakeUp_() const;

        size_t _index;
        FFiberPool_* _owner;
        mutable FFiber _fiber;
        mutable const FHandle* _released;

#ifdef WITH_CORE_ASSERT
        STATIC_CONST_INTEGRAL(size_t, GCanary, CODE3264(0x7337BEEFul, 0x7337BEEF7337BEEFull));
        const size_t _canary = GCanary;
        bool CheckCanary_() const { return (GCanary == _canary); }
#endif
    };

    using FHandleRef = const FHandle*;

    FFiberPool_(FCallback&& callback);
    ~FFiberPool_();

    FFiberPool_(const FFiberPool_&) = delete;
    FFiberPool_& operator =(const FFiberPool_&) = delete;

    FFiberPool_(FFiberPool_&&) = delete;
    FFiberPool_& operator =(FFiberPool_&&) = delete;

    FHandleRef AcquireFiber();
    void ReleaseFiber(FHandleRef handle);
    void YieldCurrentFiber(FHandleRef to, bool release);
    void StartThread();

    static FHandleRef CurrentHandleRef() {
        auto* h = (FHandleRef)FFiber::CurrentFiberData(); // @FHandle is passed down as each fiber data
        Assert(h);
        Assert_NoAssume(h->CheckCanary_());
        return h;
    }

private:
    STATIC_CONST_INTEGRAL(size_t, GStackSize, GFiberStackSize); // 64 kb
    STATIC_CONST_INTEGRAL(size_t, GCapacity, GFiberPoolCapacity); // <=> 128 * 64 kb = 8 mb

    STATIC_ASSERT(Meta::IsPow2(GCapacity));

    STATIC_CONST_INTEGRAL(size_t, GCapacityMask, GCapacity - 1);
    STATIC_CONST_INTEGRAL(size_t, GBitSetSize, GCapacity / FBitMask::GBitCount);

    static constexpr size_t Elmt_(size_t index) { return (index & FBitMask::GBitMask); }
    static constexpr size_t Word_(size_t index) { return (index / FBitMask::GBitCount); }

    FAtomicSpinLock _barrier;
    FBitMask _available[GBitSetSize];
#ifdef WITH_CORE_ASSERT
    FBitMask _hibernated[GBitSetSize];
    size_t _numFibersInUse;
#endif
    FHandle _handles[GCapacity];

    const FCallback _callback;
    static void STDCALL FiberEntryPoint_(void* arg);
};
//----------------------------------------------------------------------------
FFiberPool_::FFiberPool_(FCallback&& callback)
    : _callback(std::move(callback))
#ifdef WITH_CORE_ASSERT
    , _numFibersInUse(0)
#endif
{
    Assert(_callback);

    // reset states to : all free / all awake
    forrange(i, 0, GBitSetSize) {
        _available[i].Data = FBitMask::GAllMask;
#ifdef WITH_CORE_ASSERT
        _hibernated[i].Data = 0;
#endif
    }

    // creates all fiber handles
    forrange(i, 0, GCapacity) {
        FHandle& h = _handles[i];
        Assert_NoAssume(h.CheckCanary_());

        h._owner = this;
        h._index = i;
        h._released = nullptr;
        h._fiber.Create(&FiberEntryPoint_, &h, GFiberStackSize);
    }
}
//----------------------------------------------------------------------------
FFiberPool_::~FFiberPool_() {
    Assert_NoAssume(0 == _numFibersInUse);

    const FAtomicSpinLock::FScope scopeLock(_barrier);

#ifdef WITH_CORE_ASSERT
    // check that all fibers are released and awake
    forrange(i, 0, GBitSetSize) {
        Assert(FBitMask::GAllMask == _available[i]);
        Assert(0 == _hibernated[i].Data);
        Assert(_handles[i]._fiber);
    }
#endif

    // destroys all fiber handles
    forrange(i, 0, GCapacity) {
        FHandle& h = _handles[i];
        Assert_NoAssume(h.CheckCanary_());
        Assert(this == h._owner);
        Assert(nullptr == h._released); // should have been consumed !

        ONLY_IF_ASSERT(h._owner = nullptr);
        ONLY_IF_ASSERT(h._index = INDEX_NONE);

        h._fiber.Destroy(GFiberStackSize);
    }
}
//----------------------------------------------------------------------------
auto FFiberPool_::AcquireFiber() -> FHandleRef {
    const FAtomicSpinLock::FScope scopeLock(_barrier);

    forrange(i, 0, GBitSetSize) {
        if (size_t rel = _available[i].PopFront()) {
            const size_t index = (i * FBitMask::GBitCount + rel - 1);
            const FHandle& h = _handles[index];
            Assert_NoAssume(h.CheckCanary_());
            Assert(nullptr == h._released);
            Assert(not _available[i][rel - 1]);
            Assert_NoAssume(not _hibernated[i][rel - 1]);
            Assert_NoAssume(_numFibersInUse < GCapacity);
            ONLY_IF_ASSERT(_numFibersInUse++);
            return (&h);
        }
    }

    AssertNotReached();
    return nullptr;
}
//----------------------------------------------------------------------------
void FFiberPool_::ReleaseFiber(FHandleRef handle) {
    Assert(handle);
    const FHandle& h = *handle;

    Assert_NoAssume(h.CheckCanary_());
    Assert(this == h._owner);
    Assert(&_handles[h._index] == &h);

    Assert(nullptr == h._released); // shoulda been consumed already !

    const size_t w = Word_(h._index);
    const size_t e = Elmt_(h._index);

    const FAtomicSpinLock::FScope scopeLock(_barrier);

    Assert(not _available[w][e]);
    Assert_NoAssume(not _hibernated[w][e]);

    _available[w].SetTrue(e);

    Assert_NoAssume(_numFibersInUse);
    ONLY_IF_ASSERT(--_numFibersInUse);
}
//----------------------------------------------------------------------------
void FFiberPool_::YieldCurrentFiber(FHandleRef to, bool release) {
    Assert_NoAssume(_numFibersInUse);

    // prepare data for next fiber
    FHandleRef const current = CurrentHandleRef();
    Assert(this == current->_owner);

    ONLY_IF_ASSERT(const size_t w = Word_(current->_index));
    ONLY_IF_ASSERT(const size_t e = Elmt_(current->_index));

    if (nullptr == to)
        to = AcquireFiber();

    Assert(to);
    Assert_NoAssume(to->CheckCanary_());
    Assert(to->_fiber);
    Assert(this == to->_owner);
    Assert(nullptr == to->_released);

    if (release)
        to->_released = current;
#ifdef WITH_CORE_ASSERT
    else {
        const FAtomicSpinLock::FScope scopeLock(_barrier);

        Assert(not _available[w][e]);
        Assert(not _hibernated[w][e]);

        _hibernated[w].SetTrue(e);
    }
#endif

    // <---- yield to another fiber
    to->_fiber.Resume();
    // ----> paused or released fiber gets resumed

    // wake up, you've been resumed
    Assert(current == CurrentHandleRef());

#ifdef WITH_CORE_ASSERT
    {
        const FAtomicSpinLock::FScope scopeLock(_barrier);

        Assert(not _available[w][e]);

        if (release)
            Assert(not _hibernated[w][e]);
        else {
            Assert(_hibernated[w][e]);
            _hibernated[w].SetFalse(e);
        }
    }
#endif

    current->WakeUp_();

    // resume what the fiber was doing before being interrupted
}
//----------------------------------------------------------------------------
void FFiberPool_::StartThread() {
    Assert(FFiber::RunningFiber() == FFiber::ThreadFiber());

    AcquireFiber()->_fiber.Resume();
}
//----------------------------------------------------------------------------
void FFiberPool_::FHandle::WakeUp_() const {
    Assert(CurrentHandleRef() == this);

    if (_released) {
        Assert_NoAssume(_released->CheckCanary_());
        Assert(_released != this);
        Assert(_released->_owner == _owner);

        _owner->ReleaseFiber(_released);
        _released = nullptr;
    }
}
//----------------------------------------------------------------------------
void STDCALL FFiberPool_::FiberEntryPoint_(void* arg) {
    FHandleRef const self = CurrentHandleRef();
    self->WakeUp_(); // potentially need to do something with previous thread
    self->_owner->_callback();
    AssertNotReached(); // a pooled fiber should never exit, or it won't be reusable !
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
class FStalledFiber_ {
public:
    FStalledFiber_()
    :   _stalled(nullptr) {
        _taskContextAndPriority.Reset(nullptr, uintptr_t(ETaskPriority::Normal));
    }

    FStalledFiber_(ITaskContext* taskContext, FFiberPool_::FHandleRef stalled, ETaskPriority priority)
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

    void Resume(void (*resume)(FFiberPool_::FHandleRef)) {
        Assert(_stalled);

        FFiberPool_::FHandleRef stalled = _stalled;
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

    FFiberPool_::FHandleRef _stalled;
};
typedef TFixedSizeRingBuffer<FStalledFiber_, GStalledFiberCapacity> FStalledFiberQueue_;
//----------------------------------------------------------------------------
struct FTaskQueued_ {
    FTaskFunc Pending;
    STaskCounter Counter;
};
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

    FTaskPriorityQueue_& Queue() { return _queue; }
    TMemoryView<std::thread> Threads() { return MakeView(_threads); }
    FFiberPool_& Fibers() { return _fibers; }

    void Start(const TMemoryView<const size_t>& threadAffinities);
    void Shutdown();

    virtual void Run(FTaskWaitHandle* phandle, FTaskFunc&& rtask, ETaskPriority priority) override final;
    virtual void Run(FTaskWaitHandle* phandle, const TMemoryView<const FTaskFunc>& tasks, ETaskPriority priority) override final;
    virtual void WaitFor(FTaskWaitHandle& handle, ITaskContext* resume) override final;
    virtual void RunAndWaitFor(const TMemoryView<const FTaskFunc>& tasks, ETaskPriority priority, ITaskContext* resume) override final;

private:
    FTaskManager& _manager;
    FTaskPriorityQueue_ _queue;
    VECTOR(Task, std::thread) _threads;
    FFiberPool_ _fibers;

#ifdef WITH_CORE_ASSERT
public: // public since this is complicated to friend FWorkerContext_ due to anonymous namespace ...
    std::atomic<int> _countersInUse;
    std::atomic<int> _fibersInUse;
#endif

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

    static FWorkerContext_& Instance();

    static void ResumeFiberTask(FFiberPool_::FHandleRef resume);
    static void ExitWorkerTask(ITaskContext& ctx);

private:
    STATIC_CONST_INTEGRAL(size_t, CacheSize, 32); // 32 kb
    STATIC_CONST_INTEGRAL(size_t, StackSize, 1024<<10); // 1 mb

    TFixedSizeRingBuffer<PTaskCounter, CacheSize> _counters;

    FTaskManager& _manager;
    FFiberPool_& _fibers;
    const size_t _workerIndex;
    FFiberPool_::FHandleRef _fiberToRelease;

    static void WorkerEntryPoint_(void* pArg);
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
NO_INLINE FWorkerContext_& FWorkerContext_::Instance() {
    Assert(FFiber::IsInFiber());
    Assert(nullptr != _gInstanceTLS);
    THREADRESOURCE_CHECKACCESS(_gInstanceTLS);
    return (*_gInstanceTLS);
}
//----------------------------------------------------------------------------
void FWorkerContext_::ResumeFiberTask(FFiberPool_::FHandleRef resume) {
    Instance()._fibers.YieldCurrentFiber(resume, true/* release this fiber in the pool */);
}
//----------------------------------------------------------------------------
void FWorkerContext_::ExitWorkerTask(ITaskContext&) {
    Assert(FFiber::ThreadFiber() != FFiber::RunningFiber());

    FFiberPool_::FHandleRef self = FFiberPool_::CurrentHandleRef();
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
    TMemoryView<const FTaskFunc> Tasks;
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

        FTaskFunc noop = [](ITaskContext&){};
        FTaskManagerImpl& impl = *FWorkerContext_::Instance().Manager().Pimpl();

        do {
            FTaskWaitHandle waitHandle;
            context.RunOne(&waitHandle, noop, ETaskPriority::Internal);
            context.WaitFor(waitHandle);

        } while (not impl.Queue().empty());

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
,   _queue(GTaskManagerQueueCapacity)
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
    const size_t n = _manager.WorkerCount();
    AssertRelease(n == _threads.size());

    {
        STACKLOCAL_ASSUMEPOD_ARRAY(FTaskFunc, exitTasks, n);
        forrange(i, 0, n)
            new ((void*)&exitTasks[i]) FTaskFunc(&FWorkerContext_::ExitWorkerTask);

        Run(nullptr, exitTasks, ETaskPriority::Internal);
    }

    forrange(i, 0, n) {
        Assert(_threads[i].joinable());
        _threads[i].join();
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

#if WITH_CORE_NONBLOCKING_TASKPRIORITYQUEUE
    FTaskQueued_ queued{ std::move(rtask), pcounter };
    while (false == _queue.Produce(size_t(priority), std::move(queued)))
        ::_mm_pause();
#else
    _queue.Produce(u32(priority), FTaskQueued_{ std::move(rtask), pcounter } );
#endif
}
//----------------------------------------------------------------------------
void FTaskManagerImpl::Run(FTaskWaitHandle* phandle, const TMemoryView<const FTaskFunc>& tasks, ETaskPriority priority) {
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
    _queue.Produce(u32(priority), tasks.size(), _threads.size(),
        [&tasks, pcounter](size_t i) {
            return FTaskQueued_{ tasks[i], pcounter };
        });
#endif
}
//----------------------------------------------------------------------------
void FTaskManagerImpl::WaitFor(FTaskWaitHandle& handle, ITaskContext* resume) {
    Assert(FFiber::IsInFiber());
    Assert(handle.Valid());

    FStalledFiber_ stalled(resume ? resume : this, FFiberPool_::CurrentHandleRef(), handle.Priority());
    if (handle._counter->WaitFor(std::move(stalled)))
        _fibers.YieldCurrentFiber(nullptr, false/* this fiber will be resumed later */);
}
//----------------------------------------------------------------------------
void FTaskManagerImpl::RunAndWaitFor(const TMemoryView<const FTaskFunc>& tasks, ETaskPriority priority, ITaskContext* resume) {
    FTaskWaitHandle handle;
    Run(&handle, tasks, priority);
    WaitFor(handle, resume);
}
//----------------------------------------------------------------------------
void FTaskManagerImpl::WorkerLoop_() {
    Assert(FFiber::IsInFiber());
    Assert(FFiber::RunningFiber() != FFiber::ThreadFiber());

    while (true) {
        {
            // need to destroy FTaskFunc immediately :
            // keeping it alive outside the loop could hold resources until a next
            // task is processed on this worker, which could not happen until task manager destruction !
            FTaskQueued_ task;

#if WITH_CORE_NONBLOCKING_TASKPRIORITYQUEUE
            while (not _queue.Consume(&task))
                std::this_thread::yield();
#else
            _queue.Consume(&task);
#endif

            Assert(task.Pending.Valid());
            task.Pending.Invoke(*this);

            // Decrement the counter and resume waiting tasks if any.
            // Won't steal the thread from this fiber, so the loop can safely finish
            if (task.Counter)
                Decrement_ResumeWaitingTasksIfZero(task.Counter);
        }

        CurrentThreadContext().DutyCycle(); // some house keeping between tasks
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
void FTaskManager::Run(const TMemoryView<const FTaskFunc>& tasks, ETaskPriority priority /* = ETaskPriority::Normal */) const {
    Assert(not tasks.empty());
    Assert(nullptr != _pimpl);

    _pimpl->Run(nullptr, tasks, priority);
}
//----------------------------------------------------------------------------
void FTaskManager::RunAndWaitFor(const TMemoryView<const FTaskFunc>& tasks, ETaskPriority priority /* = ETaskPriority::Normal */) const {
    Assert(not FFiber::IsInFiber());
    Assert(not tasks.empty());
    Assert(nullptr != _pimpl);

    FRunAndWaitFor_ args{ tasks, priority, false };
    const FTaskFunc utility = [&args](ITaskContext& ctx) {
        FRunAndWaitFor_::Task(ctx, &args);
    };
    {
        std::unique_lock<std::mutex> scopeLock(args.Barrier);
        _pimpl->RunOne(nullptr, utility, priority);
        args.OnFinished.wait(scopeLock, [&args]() { return args.Available; });
        Assert(args.Available);
    }
}
//----------------------------------------------------------------------------
void FTaskManager::RunAndWaitFor(const TMemoryView<const FTaskFunc>& tasks, const FTaskFunc& whileWaiting, ETaskPriority priority/* = ETaskPriority::Normal */) const {
    Assert(not FFiber::IsInFiber());
    Assert(not tasks.empty());
    Assert(whileWaiting);
    Assert(nullptr != _pimpl);

    FRunAndWaitFor_ args{ tasks, priority, false };
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
