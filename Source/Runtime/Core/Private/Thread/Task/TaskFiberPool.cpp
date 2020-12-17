#include "stdafx.h"

#include "Thread/Task/TaskFiberPool.h"

#include "Allocator/Alloca.h" // for debug only
#include "Allocator/TrackingMalloc.h"
#include "Container/BitMask.h"
#include "Container/IntrusiveList.h"
#include "Memory/MemoryTracking.h"
#include "Meta/Singleton.h"
#include "Meta/Utility.h"
#include "Misc/Function_fwd.h"

PRAGMA_MSVC_WARNING_PUSH()
PRAGMA_MSVC_WARNING_DISABLE(4324) // 'XXX' structure was padded due to alignment

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FTaskFiberPool;
class FTaskFiberChunk : Meta::FNonCopyableNorMovable {
public:
#if USE_PPE_ASSERT
    // debug programs often use far more stack without the optimizer :
    STATIC_CONST_INTEGRAL(u32, StackSize, 4 * ALLOCATION_GRANULARITY); // 256 kb
#else
    STATIC_CONST_INTEGRAL(u32, StackSize, 2 * ALLOCATION_GRANULARITY); // 128 kb
#endif
    STATIC_CONST_INTEGRAL(u32, Capacity, 64); // <=> 64 * 128 kb = 8 mb (16 for debug),
    STATIC_CONST_INTEGRAL(u64, BusyMask, UINT64_MAX); // all bits set <=> all fibers available

    using FCallback = FTaskFiberPool::FCallback;
    using FHandle = FTaskFiberPool::FHandle;
    using FHandleRef = FTaskFiberPool::FHandleRef;

    using bit_mask = TBitMask<u64>;

    FTaskFiberChunk() NOEXCEPT;
    ~FTaskFiberChunk();

    bool Idle() const { return (_free == BusyMask); }
    bool Saturated() const { return (_free == 0); }
    size_t NumAvailable() const { return bit_mask{ _free }.Count(); }

    const TIntrusiveListNode<FTaskFiberChunk>& Node() const { return _node; }

    bool AliasesToChunk(FHandleRef handle) const;
    void TakeOwnerShip(FTaskFiberPool* pool);

    FHandleRef AcquireFiber();
    void ReleaseFiber(FHandleRef handle);

    static bool ReleaseChunk(FTaskFiberChunk** head, FTaskFiberChunk* chunk);

private:
    friend class FTaskFiberPool;

    FHandle _handles[Capacity];

    std::atomic<u64> _free; // one busy flag for each of 64 fibers
    FCallback _callback;
    FTaskFiberPool* _pool;
    TIntrusiveListNode<FTaskFiberChunk> _node;

    using list_t = INTRUSIVELIST_ACCESSOR(&FTaskFiberChunk::_node);

    static void STDCALL FiberEntryPoint_(void* arg);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static FTaskFiberChunk* AllocateTaskFiberChunk_() {
    PPE_LEAKDETECTOR_WHITELIST_SCOPE();
    return TRACKING_NEW(Fibers, FTaskFiberChunk);
}
//----------------------------------------------------------------------------
static void ReleaseTaskFiberChunk_(FTaskFiberChunk* chunk) {
    Assert(chunk);
    return TRACKING_DELETE(Fibers, chunk);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FTaskFiberPool::FHandle::AttachWakeUpCallback(FCallback&& onWakeUp) const {
    Assert(onWakeUp);
    Assert_NoAssume(Chunk()->_pool);
    Assert_NoAssume(not OnWakeUp);

    OnWakeUp = std::move(onWakeUp);
}
//----------------------------------------------------------------------------
void FTaskFiberPool::FHandle::YieldFiber(FHandleRef to, bool release) const {
    Assert(to != this);

    FTaskFiberChunk* const chunk = Chunk();
    Assert_NoAssume(chunk->_pool);

    chunk->_pool->YieldCurrentFiber(this, to, release);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTaskFiberChunk::FTaskFiberChunk() NOEXCEPT
:   _free{ BusyMask }
,   _pool{ nullptr }
,   _node{ nullptr, nullptr } {

    forrange(i, 0, Capacity) {
        FHandle& h = _handles[i];
        h.Index = i;
        h.Fiber.Create(&FiberEntryPoint_, &h, StackSize);
    }
}
//----------------------------------------------------------------------------
FTaskFiberChunk::~FTaskFiberChunk() {
    Assert_NoAssume(nullptr == _pool);
    Assert_NoAssume(BusyMask == _free);

    forrange(i, 0, Capacity) {
        FHandle& h = _handles[i];
        Assert_NoAssume(i == h.Index);
        Assert_NoAssume(not h.OnWakeUp);
        h.Fiber.Destroy(StackSize);
    }
}
//----------------------------------------------------------------------------
bool FTaskFiberChunk::AliasesToChunk(FHandleRef handle) const {
    return (uintptr_t(handle) >= uintptr_t(this) &&
            uintptr_t(handle) <= uintptr_t(&_handles[Capacity - 1]) );
}
//----------------------------------------------------------------------------
void FTaskFiberChunk::TakeOwnerShip(FTaskFiberPool* pool) {
    Assert_NoAssume(BusyMask == _free);

    // #TODO can't share the chunks across all pools because of _callback :
    // those fibers would trigger the callback actually once, and then loop forever inside of it for processing incoming work
    // because of this a fiber started in certain pool will forever use the initial callback, and thus isn't sharable with a different pool

    if (pool) {
        Assert_NoAssume(not _pool);
        _pool = pool;
        _callback = pool->Callback();
    }
    else {
        Assert_NoAssume(_pool);
        _pool = nullptr;
        _callback.Reset();
    }
}
//----------------------------------------------------------------------------
auto FTaskFiberChunk::AcquireFiber() -> FHandleRef {
    for (;;) {
        u64 expected = _free.load(std::memory_order_relaxed);

        if (Likely(expected)) {
            TBitMask<u64> m{ expected };

            ONLY_IF_ASSERT(const size_t n0 = m.Count());
            u64 w = m.PopFront_AssumeNotEmpty();
            Assert_NoAssume(m.Count() + 1 == n0);

            if (Likely(_free.compare_exchange_weak(expected, m.Data,
                std::memory_order_release, std::memory_order_relaxed))) {
                Assert(w < Capacity);
                return (&_handles[w]);
            }
        }
        else {
            return nullptr; // this chunk is empty, break the loop and return nullptr
        }
    }
}
//----------------------------------------------------------------------------
void FTaskFiberChunk::ReleaseFiber(FHandleRef handle) {
    Assert(handle);
    Assert_NoAssume(AliasesToChunk(handle));
    Assert_NoAssume(handle->Chunk() == this);
    Assert_NoAssume(not handle->OnWakeUp);

    for (;;) {
        u64 expected = _free.load(std::memory_order_relaxed);

        TBitMask<u64> m{ expected };

        Assert_NoAssume(m.Data != FTaskFiberChunk::BusyMask); // at least one free fiber
        Assert_NoAssume(handle->Index < Capacity);
        Assert_NoAssume(not m.Get(handle->Index)); // check if it was correctly flagged as busy

        ONLY_IF_ASSERT(const size_t n0 = m.Count());
        m.SetTrue(handle->Index); // set fiber as available
        Assert_NoAssume(m.Count() == n0 + 1);

        if (Likely(_free.compare_exchange_weak(expected, m.Data,
            std::memory_order_release, std::memory_order_relaxed))) {
            return; // all done, this handle was released atomically
        }
    }
}
//----------------------------------------------------------------------------
bool FTaskFiberChunk::ReleaseChunk(FTaskFiberChunk** head, FTaskFiberChunk* chunk) {
    Assert(head);

    if (chunk->Idle()) { // release the chunks when all fibers are idle
        list_t::Erase(head, nullptr, chunk);
        chunk->TakeOwnerShip(nullptr);
        ReleaseTaskFiberChunk_(chunk);

        return true;
    }
    else {
        return false;
    }
}
//----------------------------------------------------------------------------
void STDCALL FTaskFiberChunk::FiberEntryPoint_(void* arg) {
    auto* h = (FHandleRef)arg;
    Assert_NoAssume(FTaskFiberPool::CurrentHandleRef() == h);

    if (h->OnWakeUp)
        h->OnWakeUp.FireAndForget();

    h->Chunk()->_callback();

    AssertNotReached(); // a pooled fiber should never exit, or it won't be reusable !
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTaskFiberPool::FTaskFiberPool(FCallback&& callback) NOEXCEPT
:   _callback(callback)
,   _chunks(nullptr) {
    Assert_NoAssume(_callback);
}
//----------------------------------------------------------------------------
FTaskFiberPool::~FTaskFiberPool() {
    const Meta::FLockGuard scopeLock(_barrier);

    while (_chunks) // all fibers should be idle !
        VerifyRelease(FTaskFiberChunk::ReleaseChunk(&_chunks, _chunks));
}
//----------------------------------------------------------------------------
auto FTaskFiberPool::AcquireFiber() -> FHandleRef {
    for (;;) {
        if (Likely(_chunks)) {
            Assert_NoAssume(this == _chunks->_pool);
            FHandleRef const h = _chunks->AcquireFiber();

            if (Likely(h)) {
                Assert_NoAssume(not h->OnWakeUp);
#if USE_PPE_MEMORYDOMAINS
                MEMORYDOMAIN_TRACKING_DATA(Fibers).AllocateUser(FTaskFiberChunk::StackSize);
#endif
                return h;
            }
        }

        AcquireChunk_();
    }
}
//----------------------------------------------------------------------------
bool FTaskFiberPool::OwnsFiber(FHandleRef handle) const NOEXCEPT {
    Assert(handle);
    return (handle->Chunk()->_pool == this);
}
//----------------------------------------------------------------------------
void FTaskFiberPool::ReleaseFiber(FHandleRef handle) {
    Assert(handle);
    Assert_NoAssume(OwnsFiber(handle));
    Assert_NoAssume(not handle->OnWakeUp);

    FTaskFiberChunk* const chunk = handle->Chunk();

#if USE_PPE_MEMORYDOMAINS
    MEMORYDOMAIN_TRACKING_DATA(Fibers).DeallocateUser(FTaskFiberChunk::StackSize);
#endif

    chunk->ReleaseFiber(handle);

    // we never release owned chunks (#TODO ? looks like this would be quite rare, and we still have ReleaseMemory() available)
}
//----------------------------------------------------------------------------
void FTaskFiberPool::YieldCurrentFiber(FHandleRef self, FHandleRef to, bool release) {
    Assert(self);
    Assert_NoAssume(OwnsFiber(self));
    Assert_NoAssume(self == CurrentHandleRef());
    Assert_NoAssume(AllocaDepth() == 0); // can't switch fibers with live TLS block(s)
    Assert_NoAssume(not self->OnWakeUp);

    // prepare data for next fiber
    if (nullptr == to) {
        to = AcquireFiber();
        Assert_NoAssume(not to->OnWakeUp);
    }

    Assert(to);
    Assert(to->Fiber);

    if (release) {
        Assert_NoAssume(not to->OnWakeUp);
        to->AttachWakeUpCallback([this, released{ self }]() {
            ReleaseFiber(released);
        });
    }

    // <---- yield to another fiber
    to->Fiber.Resume();
    // ----> paused or released fiber gets resumed

    // wake up, you've been resumed
    Assert(self == CurrentHandleRef());

    if (self->OnWakeUp)
        self->OnWakeUp.FireAndForget();

    // resume what the fiber was doing before being interrupted
}
//----------------------------------------------------------------------------
void FTaskFiberPool::StartThread() {
    Assert(FFiber::RunningFiber() == FFiber::ThreadFiber());

    // <---- yield to a new fiber with a worker loop
    AcquireFiber()->Fiber.Resume();
    // ----> leave fiber later when the worker loop broke
}
//----------------------------------------------------------------------------
void FTaskFiberPool::ReleaseMemory() {
    if (Likely(_chunks)) {
        const Meta::FLockGuard scopeLock(_barrier);

        for (FTaskFiberChunk* ch = _chunks; ch; ) {
            FTaskFiberChunk* const nxt = ch->_node.Next;

            // release potentially idle chunks
            if (not FTaskFiberChunk::ReleaseChunk(&_chunks, ch)) {
                // keep the chunk with most available fibers at head
                if ((ch != _chunks) & (ch->NumAvailable() >= _chunks->NumAvailable()))
                    // this will help concentrate future allocations on the first chunk
                    FTaskFiberChunk::list_t::PokeHead(&_chunks, nullptr, ch);
            }

            ch = nxt;
        }
    }
}
//----------------------------------------------------------------------------
#if !USE_PPE_FINAL_RELEASE
void FTaskFiberPool::UsageStats(size_t* reserved, size_t* inUse) {
    Assert(reserved);
    Assert(inUse);

    size_t r = 0;
    size_t u = 0;
    {
        const Meta::FLockGuard scopeLock(_barrier);

        for (FTaskFiberChunk* ch = _chunks; ch; ch = ch->Node().Next) {
            r += FTaskFiberChunk::Capacity;
            u += FTaskFiberChunk::Capacity - FTaskFiberChunk::bit_mask{ ch->_free.load() }.Count();
        }
    }

    *reserved = r;
    *inUse = u;
}
#endif //!!USE_PPE_FINAL_RELEASE
//----------------------------------------------------------------------------
size_t FTaskFiberPool::ReservedStackSize() {
    return FTaskFiberChunk::StackSize;
}
//----------------------------------------------------------------------------
NO_INLINE FTaskFiberChunk* FTaskFiberPool::AcquireChunk_() {
    FTaskFiberChunk* const head = _chunks;

    // need to lock to avoid allocate more than one new chunk
    const Meta::FLockGuard scopeLock(_barrier);

    // another thread might have already allocated a new chunk when we wake up :
    if (head != _chunks)
        return nullptr; // let the callee recurse into AcquireFiber(), extremely likely to find a new fiber

    // first look for a potential chunk with free fibers remaining
    for (FTaskFiberChunk* ch = _chunks; ch; ch = ch->_node.Next) {
        if (not ch->Saturated()) {
            FTaskFiberChunk::list_t::PokeHead(&_chunks, nullptr, ch);
            return nullptr; // don't return that existing chunk directly :
            // /!\ since it was just poked at the front of the chunk it has now be made available
            //     to all other threads, so it might not have a free fiber when finally access it after this return
        }
    }

    // didn't find any free fiber : allocate a new chunk and insert it at the front
    FTaskFiberChunk* const fresh = AllocateTaskFiberChunk_();
    fresh->TakeOwnerShip(this); // *MUST* be initialized *BEFORE* insertion : still not thread safe !
    // /!\  another thread could poke in the pool while we are still inside TakeOwnership(),
    //      so we wait for the new chunk to be completely initialized before insertion

    FTaskFiberChunk::list_t::PushHead(&_chunks, nullptr, fresh);

    return fresh; // new => allocation is guaranteed to succeed
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Meant to be used as a thread local cache to remove contention on FTaskFiberPool
//----------------------------------------------------------------------------
FTaskFiberLocalCache::FTaskFiberLocalCache(FTaskFiberPool& pool) NOEXCEPT
:   _pool(pool)
{}
//----------------------------------------------------------------------------
FTaskFiberLocalCache::~FTaskFiberLocalCache() {
    ReleaseMemory();
}
//----------------------------------------------------------------------------
auto FTaskFiberLocalCache::AcquireFiber() -> FHandleRef {
    THIS_THREADRESOURCE_CHECKACCESS();

    FHandleRef result = nullptr;
    if (not _freed.Pop(&result))
        result = _pool.AcquireFiber();

    Assert(result);
    Assert_NoAssume(_pool.OwnsFiber(result));
    return result;
}
//----------------------------------------------------------------------------
void FTaskFiberLocalCache::ReleaseFiber(FHandleRef handle) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(handle);
    Assert_NoAssume(_pool.OwnsFiber(handle));
    Assert_NoAssume(not handle->OnWakeUp);

    // release all when full, to avoid releasing at every call
    if (Unlikely(_freed.full()))
        ReleaseMemory();

    _freed.Push(handle);
}
//----------------------------------------------------------------------------
void FTaskFiberLocalCache::ReleaseMemory() {
    THIS_THREADRESOURCE_CHECKACCESS();

    FHandleRef fiber = nullptr;
    while (_freed.Pop(&fiber))
        _pool.ReleaseFiber(fiber);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

PRAGMA_MSVC_WARNING_POP()
