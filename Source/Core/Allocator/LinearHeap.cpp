#include "stdafx.h"

#include "LinearHeap.h"

#include "Memory/MemoryDomain.h"
#include "Memory/VirtualMemory.h"
#include "Thread/AtomicSpinLock.h"
#include "Thread/ThreadContext.h"

#include "Diagnostic/Logger.h"
#ifdef USE_DEBUG_LOGGER
#   include "IO/FormatHelpers.h"
#endif

//----------------------------------------------------------------------------
// Turn to 1 to disable linear heap allocations (useful for memory debugging) :
//----------------------------------------------------------------------------
#define WITH_CORE_LINEARHEAP_FALLBACK_TO_MALLOC (USE_CORE_MEMORY_DEBUGGING) //%__NOCOMMIT%

namespace Core {
EXTERN_LOG_CATEGORY(CORE_API, MemoryTracking)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
STATIC_CONST_INTEGRAL(size_t, GLinearHeapGranularity_, ALLOCATION_GRANULARITY); // 64k
STATIC_CONST_INTEGRAL(size_t, GLinearHeapAllocationSize, GLinearHeapGranularity_);
STATIC_CONST_INTEGRAL(size_t, GLinearHeapBlockCapacity, GLinearHeapAllocationSize - ALLOCATION_BOUNDARY);
STATIC_CONST_INTEGRAL(size_t, GLinearHeapMaxSize, GLinearHeapBlockCapacity);
//----------------------------------------------------------------------------
#ifdef WITH_CORE_ASSERT
#   define AssertCheckCanary(_ITEM) Assert_NoAssume((_ITEM).CheckCanary())
static void FillDeletedBlock_(void* ptr, size_t sizeInBytes) { ::memset(ptr, 0xDD, sizeInBytes); }
#else
#   define AssertCheckCanary(_ITEM) NOOP(_ITEM)
#endif
//----------------------------------------------------------------------------
#if !WITH_CORE_LINEARHEAP_FALLBACK_TO_MALLOC
struct FLinearHeapBlock_ {
    FLinearHeapBlock_* Next;
    u16 Offset;
    u16 Last;

#   ifdef WITH_CORE_ASSERT
    STATIC_CONST_INTEGRAL(CODE3264(u64, u32), DefaultCanary, CODE3264(0xAABBDDEEAABBDDEEull, 0xAABBDDEEul));
    CODE3264(u64, u32) Canary = DefaultCanary; // serves as padding to align on 16
    bool CheckCanary() const { return (DefaultCanary == Canary); }

    static bool Contains(const FLinearHeapBlock_* head, const FLinearHeapBlock_* blk) {
        for (; head; head = head->Next) {
            AssertCheckCanary(*head);
            if (head == blk)
                return true;
        }
        return false;
    }

#   else
    CODE3264(u64, u32) _Padding;
#   endif

    size_t OffsetFromPtr(void* ptr) {
        const size_t off = ((u8*)ptr - (u8*)(this + 1));
        Assert(off < GLinearHeapBlockCapacity);
        return off;
    }

    static FLinearHeapBlock_* BlockFromPtr(void* ptr) {
        Assert(not Meta::IsAligned(GLinearHeapAllocationSize, ptr));
        auto* blk = static_cast<FLinearHeapBlock_*>(Meta::RoundToPrev(ptr, GLinearHeapAllocationSize));
        AssertCheckCanary(*blk);
        Assert(blk->Last >= blk->OffsetFromPtr(ptr));
        return blk;
    }
};
STATIC_ASSERT(sizeof(FLinearHeapBlock_) + GLinearHeapBlockCapacity == GLinearHeapGranularity_);
#else
struct FLinearHeapBlock_ {
    FLinearHeapBlock_* Next;
    u32 SizeInBytes;
#ifdef WITH_CORE_ASSERT
    STATIC_CONST_INTEGRAL(CODE3264(u64, u32), DefaultCanary, CODE3264(0xAABBDDEEAABBDDEEull, 0xAABBDDEEul));
    CODE3264(u64, u32) Canary = DefaultCanary; // serves as padding to align on 16
    bool CheckCanary() const { return (DefaultCanary == Canary); }
#else
    CODE3264(u64, u32) _Padding;
#endif
};
#endif //!!WITH_CORE_LINEARHEAP_FALLBACK_TO_MALLOC
STATIC_ASSERT(sizeof(FLinearHeapBlock_) == ALLOCATION_BOUNDARY);
//----------------------------------------------------------------------------
#if !WITH_CORE_LINEARHEAP_FALLBACK_TO_MALLOC
class FLinearHeapVMCache_ {
public:
    static NO_INLINE FLinearHeapBlock_* AllocateBlock(FLinearHeapBlock_* next) {
        void* const ptr = Instance_().Allocate_(GLinearHeapAllocationSize);
        FLinearHeapBlock_* blk = new (ptr) FLinearHeapBlock_;
        blk->Offset = blk->Last = 0;
        blk->Next = next;
#ifdef WITH_CORE_ASSERT
        ::memset(blk + 1, 0xCC, GLinearHeapBlockCapacity);
#endif
        return blk;
    }

    static FLinearHeapBlock_* ReleaseBlock(FLinearHeapBlock_* blk) {
        AssertCheckCanary(*blk);
        Assert(blk->Last <= blk->Offset);
        Assert(blk->Offset <= GLinearHeapBlockCapacity);
        FLinearHeapBlock_* const next = blk->Next;
        Instance_().Free_(blk, GLinearHeapAllocationSize);
        return next;
    }

    static void ReleaseAll() {
        Instance_().ReleaseAll_();
    }

private:
    STATIC_CONST_INTEGRAL(size_t, VMCacheBlocks, 32);
    STATIC_CONST_INTEGRAL(size_t, VMCacheSizeInBytes, 2 * 1024 * 1024); // <=> 2 mo global cache for large blocks

    FAtomicSpinLock _barrier;
    VIRTUALMEMORYCACHE(LinearHeap, VMCacheBlocks, VMCacheSizeInBytes) _vm;

    static FLinearHeapVMCache_& Instance_() {
        static FLinearHeapVMCache_ GInstance;
        return GInstance;
    }

    void* Allocate_(size_t sizeInBytes) {
        const FAtomicSpinLock::FScope scopeLock(_barrier);
        return _vm.Allocate(sizeInBytes);
    }

    void Free_(void* ptr, size_t sizeInBytes) {
        const FAtomicSpinLock::FScope scopeLock(_barrier);
        _vm.Free(ptr, sizeInBytes);
    }

    void ReleaseAll_() {
        const FAtomicSpinLock::FScope scopeLock(_barrier);
        _vm.ReleaseAll();
    }
};
#endif //!!WITH_CORE_LINEARHEAP_FALLBACK_TO_MALLOC
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const size_t FLinearHeap::MaxBlockSize = GLinearHeapMaxSize;
//----------------------------------------------------------------------------
#ifdef USE_MEMORY_DOMAINS
FLinearHeap::FLinearHeap(FMemoryTracking* parent)
#else
FLinearHeap::FLinearHeap()
#endif
    : _blocks(nullptr)
#ifdef USE_MEMORY_DOMAINS
    , _trackingData("LinearHeap", parent) {
    RegisterAdditionalTrackingData(&_trackingData);
}
#else
{}
#endif
//----------------------------------------------------------------------------
FLinearHeap::~FLinearHeap() {
#if defined(USE_DEBUG_LOGGER) && defined(USE_MEMORY_DOMAINS)
    size_t totalBlockCount = 0;
    size_t totalSizeInBytes = 0;
    for (auto* blk = static_cast<FLinearHeapBlock_*>(_blocks);
        blk; blk = blk->Next) {
        totalBlockCount++;
        totalSizeInBytes += GLinearHeapBlockCapacity;
    }

    LOG(MemoryTracking, Debug, L"linear heap <{0}> allocated {1} blocks using {2} pages, ie {3:f2} / {4:f2} allocated ({5}, max:{6})",
        _trackingData.Parent()
            ? MakeCStringView(_trackingData.Parent()->Name())
            : MakeCStringView(_trackingData.Name()),
        Fmt::CountOfElements(_trackingData.BlockCount()),
        totalBlockCount,
        Fmt::SizeInBytes(_trackingData.TotalSizeInBytes()),
        Fmt::SizeInBytes(totalSizeInBytes),
        Fmt::Percentage(_trackingData.TotalSizeInBytes(), totalSizeInBytes),
        Fmt::Percentage(_trackingData.MaxTotalSizeInBytes(), totalSizeInBytes) );
#endif

    ReleaseAll();

#ifdef USE_MEMORY_DOMAINS
    UnregisterAdditionalTrackingData(&_trackingData);
#endif
}
//----------------------------------------------------------------------------
void* FLinearHeap::Allocate(size_t size, size_t alignment/* = ALLOCATION_BOUNDARY */) {
    Assert(size);
    Assert(alignment && alignment <= 16);
    AssertRelease(size <= MaxBlockSize);

    alignment = ROUND_TO_NEXT_16(alignment); // force alignment to 16

    void* ptr;
#if WITH_CORE_LINEARHEAP_FALLBACK_TO_MALLOC
    auto* blk = new (Core::aligned_malloc(sizeof(FLinearHeapBlock_) + size, alignment)) FLinearHeapBlock_;
    blk->SizeInBytes = checked_cast<u32>(size);
    blk->Next = static_cast<FLinearHeapBlock_*>(_blocks);
    AssertCheckCanary(*blk);

    _blocks = blk;
    ptr = (blk + 1);

#else
    auto* blk = static_cast<FLinearHeapBlock_*>(_blocks);
    if (Unlikely(nullptr == blk || Meta::RoundToNext(blk->Offset, alignment) + size > GLinearHeapBlockCapacity)) {
        FLinearHeapBlock_* const prev = blk;
#if 0
        // try to look in previous blocks in there's enough space there
        if (blk)
            for (blk = blk->Next; blk; blk = blk->Next) {
                if (Meta::RoundToNext(blk->Offset, alignment) + size <= GLinearHeapBlockCapacity)
                    break; // yeah fits here !
            }
        // allocate a new block if didn't find
        if (nullptr == blk)
#endif
        _blocks = blk = FLinearHeapVMCache_::AllocateBlock(prev);
    }
    AssertCheckCanary(*blk);

    const size_t off = Meta::RoundToNext(blk->Offset, alignment);
    Assert(off + size <= GLinearHeapBlockCapacity);

    blk->Offset = checked_cast<u16>(off + size);
    blk->Last = u16(off);

    ptr = (&reinterpret_cast<u8*>(blk + 1)[off]);

#endif

    Assert(ptr);
    Assert(Meta::IsAligned(alignment, ptr));
#ifdef USE_MEMORY_DOMAINS
    _trackingData.Allocate(1, size);
#endif

    return ptr;
}
//----------------------------------------------------------------------------
void* FLinearHeap::Relocate(void* ptr, size_t newSize, size_t oldSize, size_t alignment/* = ALLOCATION_BOUNDARY */) {
    Assert(newSize);
    Assert(alignment && alignment <= 16);
    AssertRelease(newSize <= MaxBlockSize);

    if (nullptr == ptr) {
        Assert(0 == oldSize);
        return Allocate(newSize, alignment);
    }

    alignment = ROUND_TO_NEXT_16(alignment); // force alignment to 16

#if WITH_CORE_LINEARHEAP_FALLBACK_TO_MALLOC
    FLinearHeapBlock_* prev = nullptr;
    for (auto* blk = static_cast<FLinearHeapBlock_*>(_blocks); blk; blk = blk->Next) {
        AssertCheckCanary(*blk);
        if (ptr == (blk + 1)) {
            Assert(nullptr != prev || _blocks == ptr);

            if (prev)
                prev->Next = blk->Next;
            else
                _blocks = blk->Next;

            void* const newp = Allocate(newSize, alignment);
            ::memcpy(newp, ptr, Min(oldSize, newSize));

            Core::aligned_free(blk);
#   ifdef USE_MEMORY_DOMAINS
            _trackingData.Deallocate(1, oldSize);
#   endif
            return newp;
        }
        prev = blk;
    }

    AssertNotReached(); // block don't belong to this heap :'(

#else
    if (ptr) {
        FLinearHeapBlock_* blk = FLinearHeapBlock_::BlockFromPtr(ptr);
        Assert_NoAssume(FLinearHeapBlock_::Contains(static_cast<FLinearHeapBlock_*>(_blocks), blk));

        const size_t off = blk->OffsetFromPtr(ptr);
        if (blk->Last == off && blk->Last + newSize <= GLinearHeapBlockCapacity) {
            blk->Offset = checked_cast<u16>(blk->Last + newSize);
#ifdef USE_MEMORY_DOMAINS
            _trackingData.Deallocate(1, oldSize);
            _trackingData.Allocate(1, newSize);
#endif
            return ptr;
        }
        else {
            void* const newp = Allocate(newSize, alignment);
            ::memcpy(newp, ptr, Min(oldSize, newSize));
            ONLY_IF_ASSERT(FillDeletedBlock_(ptr, oldSize));
            return newp;
        }
    }

    return Allocate(newSize, alignment);

#endif
}
//----------------------------------------------------------------------------
void FLinearHeap::Release(void* ptr, size_t size) {
    Assert(nullptr != ptr);
    Assert(0 != size);
    AssertRelease(size <= MaxBlockSize);

#if WITH_CORE_LINEARHEAP_FALLBACK_TO_MALLOC
    FLinearHeapBlock_* prev = nullptr;
    for (auto* blk = static_cast<FLinearHeapBlock_*>(_blocks); blk; blk = blk->Next) {
        AssertCheckCanary(*blk);
        if (ptr == (blk + 1)) {
            Assert(nullptr != prev || _blocks == ptr);
            Assert(size == blk->SizeInBytes);

            if (prev)
                prev->Next = blk->Next;
            else
                _blocks = blk->Next;

#   ifdef USE_MEMORY_DOMAINS
            _trackingData.Deallocate(1, size);
#   endif

            Core::aligned_free(blk);
            return;
        }
        prev = blk;
    }

    AssertNotReached();

#else
    FLinearHeapBlock_* blk = FLinearHeapBlock_::BlockFromPtr(ptr);
    Assert_NoAssume(FLinearHeapBlock_::Contains(static_cast<FLinearHeapBlock_*>(_blocks), blk));

    const size_t off = blk->OffsetFromPtr(ptr);
    if (off == blk->Last) {
        Assert(off < blk->Offset);
        // reclaim memory :
        blk->Offset = blk->Last;
#   ifdef USE_MEMORY_DOMAINS
        _trackingData.Deallocate(1, size);
#   endif
    }

    ONLY_IF_ASSERT(FillDeletedBlock_(ptr, size));

#endif
}
//----------------------------------------------------------------------------
void FLinearHeap::ReleaseAll() {
    auto* blk = static_cast<FLinearHeapBlock_*>(_blocks);

#if WITH_CORE_LINEARHEAP_FALLBACK_TO_MALLOC
    while (blk) {
        AssertCheckCanary(*blk);

#   ifdef USE_MEMORY_DOMAINS
        _trackingData.Deallocate(1, blk->SizeInBytes);
#   endif

        FLinearHeapBlock_* const nxt = blk->Next;
        Core::aligned_free(blk);
        blk = nxt;
    }

    Assert_NoAssume(_trackingData.AllocationCount() == 0);

#else
    while (blk)
        blk = FLinearHeapVMCache_::ReleaseBlock(blk);

    _blocks = nullptr;
#   ifdef USE_MEMORY_DOMAINS
    _trackingData.ReleaseAll();
#   endif

#endif
}
//----------------------------------------------------------------------------
void FLinearHeap::FlushVirtualMemoryCache() {
    FLinearHeapVMCache_::ReleaseAll();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#undef AssertCheckCanary
