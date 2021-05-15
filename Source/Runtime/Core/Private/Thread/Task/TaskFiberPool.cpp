#include "stdafx.h"

#include "Thread/Task/TaskFiberPool.h"

#include "Allocator/Alloca.h" // for debug only
#include "Allocator/TrackingMalloc.h"
#include "Container/IntrusiveList.h"
#include "Memory/MemoryTracking.h"
#include "Meta/Singleton.h"
#include "Meta/Utility.h"
#include "Misc/Function_fwd.h"
#include "Thread/AtomicPool.h"

PRAGMA_MSVC_WARNING_PUSH()
PRAGMA_MSVC_WARNING_DISABLE(4324) // 'XXX' structure was padded due to alignment

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FTaskFiberPool;
class FTaskFiberChunk : Meta::FNonCopyableNorMovable {
    using atomic_pool_t = TAtomicPool<FTaskFiberPool::FHandle>;
public:
#if USE_PPE_ASSERT
    // debug programs often use far more stack without the optimizer :
    STATIC_CONST_INTEGRAL(u32, StackSize, 4 * ALLOCATION_GRANULARITY); // 256 kb
#else
    STATIC_CONST_INTEGRAL(u32, StackSize, 2 * ALLOCATION_GRANULARITY); // 128 kb
#endif
    STATIC_CONST_INTEGRAL(u32, Capacity, atomic_pool_t::Capacity); // <=> 64 * 128 kb = 8 mb (16 for debug),

    using FCallback = FTaskFiberPool::FCallback;
    using FHandle = FTaskFiberPool::FHandle;
    using FHandleRef = FTaskFiberPool::FHandleRef;

    FTaskFiberChunk() NOEXCEPT;
    ~FTaskFiberChunk();

    bool Idle() const { return (_free.NumFreeBlocks() == Capacity); }
    bool Saturated() const { return (_free.NumFreeBlocks() == 0); }
    size_t NumAvailable() const { return _free.NumCreatedAndFreeBlocks(); }

    const TIntrusiveListNode<FTaskFiberChunk>& Node() const { return _node; }

    bool AliasesToChunk(FHandleRef handle) const;
    void TakeOwnerShip(FTaskFiberPool* pool);

    FHandleRef AcquireFiber();
    void ReleaseFiber(FHandleRef handle);
    void ReleaseMemory();

    static bool ReleaseChunk(FTaskFiberChunk** head, FTaskFiberChunk* chunk);

private:
    friend class FTaskFiberPool;

    atomic_pool_t _free;
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
    Assert_NoAssume(Chunk->_pool);
    Assert_NoAssume(not OnWakeUp);

    OnWakeUp = std::move(onWakeUp);
}
//----------------------------------------------------------------------------
void FTaskFiberPool::FHandle::YieldFiber(FHandleRef to, bool release) const {
    Assert(to != this);

    Assert_NoAssume(Chunk->_pool);
    Chunk->_pool->YieldCurrentFiber(this, to, release);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTaskFiberChunk::FTaskFiberChunk() NOEXCEPT
:   _pool{ nullptr }
,   _node{ nullptr, nullptr }
{}
//----------------------------------------------------------------------------
FTaskFiberChunk::~FTaskFiberChunk() {
    Assert_NoAssume(nullptr == _pool);
    Assert_NoAssume(Idle());

    ReleaseMemory();
}
//----------------------------------------------------------------------------
bool FTaskFiberChunk::AliasesToChunk(FHandleRef handle) const {
    return _free.Aliases(handle);
}
//----------------------------------------------------------------------------
void FTaskFiberChunk::TakeOwnerShip(FTaskFiberPool* pool) {
    Assert_NoAssume(Idle());

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
    return _free.Allocate([this](FHandle* h) {
        Meta::Construct(h);
        h->Chunk = this;
        h->Fiber.Create(&FiberEntryPoint_, h, StackSize);
    });
}
//----------------------------------------------------------------------------
void FTaskFiberChunk::ReleaseFiber(FHandleRef handle) {
    Assert(handle);
    Assert_NoAssume(AliasesToChunk(handle));
    Assert_NoAssume(handle->Chunk == this);
    Assert_NoAssume(not handle->OnWakeUp);

    _free.Release(handle);
}
//----------------------------------------------------------------------------
void FTaskFiberChunk::ReleaseMemory() {
    _free.Clear_ReleaseMemory([ARG0_IF_ASSERT(this)](FHandle* h) {
        Assert_NoAssume(h->Chunk == this);
        h->Fiber.Destroy(StackSize);
    });
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
    auto* h = static_cast<FHandleRef>(arg);
    Assert_NoAssume(FTaskFiberPool::CurrentHandleRef() == h);

    if (h->OnWakeUp)
        h->OnWakeUp.FireAndForget();

    h->Chunk->_callback();

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
    const FCriticalSection::FScopeLock scopeLock(&_barrierCS);

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
    return (handle->Chunk->_pool == this);
}
//----------------------------------------------------------------------------
void FTaskFiberPool::ReleaseFiber(FHandleRef handle) {
    Assert(handle);
    Assert_NoAssume(OwnsFiber(handle));
    Assert_NoAssume(not handle->OnWakeUp);

    FTaskFiberChunk* const chunk = handle->Chunk;

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
        const FCriticalSection::FScopeLock scopeLock(&_barrierCS);

        for (FTaskFiberChunk* ch = _chunks; ch; ) {
            FTaskFiberChunk* const nxt = ch->_node.Next;

            // release potentially idle chunks
            if (not FTaskFiberChunk::ReleaseChunk(&_chunks, ch)) {
                // release unused fibers
                ch->ReleaseMemory();
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
        const FCriticalSection::FScopeLock scopeLock(&_barrierCS);

        for (FTaskFiberChunk* ch = _chunks; ch; ch = ch->Node().Next) {
            r += FTaskFiberChunk::Capacity;
            u += FTaskFiberChunk::Capacity - ch->_free.NumFreeBlocks();
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
    const FCriticalSection::FScopeLock scopeLock(&_barrierCS);

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
