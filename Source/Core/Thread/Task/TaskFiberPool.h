#pragma once

#include "Core/Core.h"

#include "Core/Container/BitMask.h"
#include "Core/Misc/Function.h"
#include "Core/Thread/AtomicSpinLock.h"
#include "Core/Thread/Fiber.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Helper class which carefully tracks usage of a fixed pool of fibers
//----------------------------------------------------------------------------
class FTaskFiberPool {
public:
    class FHandle;

    using FCallback = TFunction<void()>;
    using FHandleRef = const FHandle*;

    class FHandle { // opaque handle, can't be manipulated directly
    public:
        void AttachWakeUpCallback(FCallback&& onWakeUp) const {
            Assert(onWakeUp);
            Assert(not _onWakeUp);

            _onWakeUp = std::move(onWakeUp);
        }

        void YieldFiber(FHandleRef to, bool release) const {
            _owner->YieldCurrentFiber(this, to, release);
        }

    private:
        friend class FTaskFiberPool;

#ifdef WITH_CORE_ASSERT
        ~FHandle() {
            Assert(INDEX_NONE == _index);
            Assert(nullptr == _owner);
        }
#endif

        void WakeUp_() const;

        size_t _index;
        FTaskFiberPool* _owner;
        mutable FFiber _fiber;
        mutable FCallback _onWakeUp;

#ifdef WITH_CORE_ASSERT
        STATIC_CONST_INTEGRAL(size_t, GCanary, CODE3264(0x7337BEEFul, 0x7337BEEF7337BEEFull));
        const size_t _canary = GCanary;
        bool CheckCanary_() const { return (GCanary == _canary); }
#endif
    };

    FTaskFiberPool(FCallback&& callback);
    ~FTaskFiberPool();

    FTaskFiberPool(const FTaskFiberPool&) = delete;
    FTaskFiberPool& operator =(const FTaskFiberPool&) = delete;

    FTaskFiberPool(FTaskFiberPool&&) = delete;
    FTaskFiberPool& operator =(FTaskFiberPool&&) = delete;

    FHandleRef AcquireFiber();
    void ReleaseFiber(FHandleRef handle);
    void YieldCurrentFiber(FHandleRef self, FHandleRef to, bool release);
    void StartThread();

    static FHandleRef CurrentHandleRef() {
        auto* h = (FHandleRef)FFiber::CurrentFiberData(); // @FHandle is passed down as each fiber data
        Assert(h);
        Assert_NoAssume(h->CheckCanary_());
        return h;
    }

private:
    STATIC_CONST_INTEGRAL(size_t, GStackSize, ALLOCATION_GRANULARITY); // 64 kb
    STATIC_CONST_INTEGRAL(size_t, GCapacity, 128); // <=> 128 * 64 kb = 8 mb !!!

    STATIC_ASSERT(Meta::IsPow2(GCapacity));

    STATIC_CONST_INTEGRAL(size_t, GCapacityMask, GCapacity - 1);
    STATIC_CONST_INTEGRAL(size_t, GBitSetSize, GCapacity / FBitMask::GBitCount);

    static constexpr size_t Elmt_(size_t index) { return (index & FBitMask::GBitMask); }
    static constexpr size_t Word_(size_t index) { return (index / FBitMask::GBitCount); }

    const FCallback _callback;

    FAtomicSpinLock _barrier;
    FBitMask _available[GBitSetSize];
#ifdef WITH_CORE_ASSERT
    FBitMask _hibernated[GBitSetSize];
    size_t _numFibersInUse;
#endif
    FHandle _handles[GCapacity];

    static void STDCALL FiberEntryPoint_(void* arg);
};
//----------------------------------------------------------------------------
FTaskFiberPool::FTaskFiberPool(FCallback&& callback)
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
        h._fiber.Create(&FiberEntryPoint_, &h, GStackSize);
    }
}
//----------------------------------------------------------------------------
FTaskFiberPool::~FTaskFiberPool() {
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
        Assert(not h._onWakeUp); // should have been consumed !

        ONLY_IF_ASSERT(h._owner = nullptr);
        ONLY_IF_ASSERT(h._index = INDEX_NONE);

        h._fiber.Destroy(GStackSize);
    }
}
//----------------------------------------------------------------------------
auto FTaskFiberPool::AcquireFiber() -> FHandleRef {
    const FAtomicSpinLock::FScope scopeLock(_barrier);

    forrange(i, 0, GBitSetSize) {
        if (size_t rel = _available[i].PopFront()) {
            const size_t index = (i * FBitMask::GBitCount + rel - 1);
            const FHandle& h = _handles[index];
            Assert_NoAssume(h.CheckCanary_());
            Assert(not h._onWakeUp);
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
void FTaskFiberPool::ReleaseFiber(FHandleRef handle) {
    Assert(handle);
    const FHandle& h = *handle;

    Assert_NoAssume(h.CheckCanary_());
    Assert(this == h._owner);
    Assert(&_handles[h._index] == &h);

    Assert(not h._onWakeUp); // shoulda been consumed already !

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
void FTaskFiberPool::YieldCurrentFiber(FHandleRef self, FHandleRef to, bool release) {
    Assert(self);
    Assert(self == CurrentHandleRef());
    Assert_NoAssume(_numFibersInUse);

    // prepare data for next fiber
    FHandleRef const current = CurrentHandleRef();
    Assert(this == current->_owner);

    ONLY_IF_ASSERT(const size_t w = Word_(current->_index));
    ONLY_IF_ASSERT(const size_t e = Elmt_(current->_index));

    if (nullptr == to) {
        to = AcquireFiber();
        Assert(not to->_onWakeUp);
    }

    Assert(to);
    Assert_NoAssume(to->CheckCanary_());
    Assert(to->_fiber);
    Assert(this == to->_owner);

    if (release) {
        Assert(not to->_onWakeUp);
        to->_onWakeUp = [this, released{ current }]() {
            Assert_NoAssume(released->CheckCanary_());
            ReleaseFiber(released);
        };
    }
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
void FTaskFiberPool::StartThread() {
    Assert(FFiber::RunningFiber() == FFiber::ThreadFiber());

    AcquireFiber()->_fiber.Resume();
}
//----------------------------------------------------------------------------
void FTaskFiberPool::FHandle::WakeUp_() const {
    Assert(CurrentHandleRef() == this);

    if (_onWakeUp)
        _onWakeUp.FireAndForget();
}
//----------------------------------------------------------------------------
void STDCALL FTaskFiberPool::FiberEntryPoint_(void* arg) {
    FHandleRef const self = CurrentHandleRef();
    Assert(self == arg);
    self->WakeUp_(); // need potentially to do something with previous thread
    self->_owner->_callback();
    AssertNotReached(); // a pooled fiber should never exit, or it won't be reusable !
    NOOP(arg);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
