#include "stdafx.h"

#include "Allocator/LinearHeap.h"

#include "HAL/PlatformMemory.h"
#include "Memory/MemoryTracking.h"
#include "Memory/VirtualMemory.h"
#include "Meta/Utility.h" // unlikely
#include "Thread/AtomicSpinLock.h"
#include "Thread/ThreadContext.h"

#include "Diagnostic/Logger.h"
#ifdef USE_DEBUG_LOGGER
#   include "IO/FormatHelpers.h"
#endif

//----------------------------------------------------------------------------
// Turn to 1 to disable linear heap allocations (useful for memory debugging) :
//----------------------------------------------------------------------------
#define USE_PPE_LINEARHEAP_DEBUG_FALLBACK (USE_PPE_MEMORY_DEBUGGING) //%_NOCOMMIT%

//----------------------------------------------------------------------------
// Turn to 0 to disable linear heap pooling (useful for memory debugging) :
//----------------------------------------------------------------------------
#define USE_PPE_LINEARHEAP_POOLING (!USE_PPE_LINEARHEAP_DEBUG_FALLBACK && 1) //%_NOCOMMIT%

#if USE_PPE_LINEARHEAP_DEBUG_FALLBACK
#   include "Allocator/TrackingMalloc.h"
#   include "Container/IntrusiveList.h"
#endif

namespace PPE {
EXTERN_LOG_CATEGORY(PPE_CORE_API, MemoryDomain)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
STATIC_CONST_INTEGRAL(size_t, GLinearHeapGranularity_, ALLOCATION_GRANULARITY); // 64k
STATIC_CONST_INTEGRAL(size_t, GLinearHeapAllocationSize, GLinearHeapGranularity_);
STATIC_CONST_INTEGRAL(size_t, GLinearHeapAllocationMask, GLinearHeapAllocationSize - 1);
STATIC_CONST_INTEGRAL(size_t, GLinearHeapBlockOffset, CODE3264(16, 32));
STATIC_CONST_INTEGRAL(size_t, GLinearHeapBlockCapacity, GLinearHeapAllocationSize - GLinearHeapBlockOffset);
STATIC_CONST_INTEGRAL(size_t, GLinearHeapMaxSize, GLinearHeapBlockCapacity);
//----------------------------------------------------------------------------
#if USE_PPE_ASSERT
static void FillUninitializedBlock_(void* ptr, size_t sizeInBytes) { FPlatformMemory::Memset(ptr, 0xCC, sizeInBytes); }
static void FillDeletedBlock_(void* ptr, size_t sizeInBytes) { FPlatformMemory::Memset(ptr, 0xDD, sizeInBytes); }
#endif
//----------------------------------------------------------------------------
PRAGMA_MSVC_WARNING_PUSH()
PRAGMA_MSVC_WARNING_DISABLE(4324) // structure was padded due to alignement
struct ALIGN(ALLOCATION_BOUNDARY) FLinearHeapBlock_ {

    size_t SizeInBytes{ 0 };
    TIntrusiveListNode<FLinearHeapBlock_> Node;

#if USE_PPE_LINEARHEAP_DEBUG_FALLBACK
    static FLinearHeapBlock_* FromData(void* data) {
        return reinterpret_cast<FLinearHeapBlock_*>((u8*)data - GLinearHeapBlockOffset);
    }
#else
    static FLinearHeapBlock_* FromData(void* data, size_t* offset) {
        *offset = (uintptr_t(data) & GLinearHeapAllocationMask) - GLinearHeapBlockOffset;
        return reinterpret_cast<FLinearHeapBlock_*>(uintptr_t(data) & ~GLinearHeapAllocationMask);
    }
#endif

    u8* Data() const { return ((u8*)this + GLinearHeapBlockOffset); }

    static FLinearHeapBlock_*& Head(void*& blocks) {
        return reinterpret_cast<FLinearHeapBlock_*&>(blocks);
    }

};
STATIC_ASSERT(sizeof(FLinearHeapBlock_) == GLinearHeapBlockOffset);
PRAGMA_MSVC_WARNING_POP()
//----------------------------------------------------------------------------
using FLinearHeapBlockList_ = INTRUSIVELIST_ACCESSOR(&FLinearHeapBlock_::Node);
//----------------------------------------------------------------------------
#if !USE_PPE_LINEARHEAP_DEBUG_FALLBACK
class FLinearHeapVMCache_ {
public:

#if USE_PPE_MEMORYDOMAINS
    static void Reserve(FLinearHeapBlock_** phead, size_t* off, size_t sz, FMemoryTracking& trackingData) {
#else
    static void Reserve(FLinearHeapBlock_** phead, size_t* off, size_t sz) {
#endif
        Assert(phead);
        Assert(sz);
        Assert(off);
        Assert_NoAssume(sz <= GLinearHeapBlockCapacity);

        if (*phead) {
            (*phead)->SizeInBytes = *off;
            Assert_NoAssume((*phead)->SizeInBytes + sz > GLinearHeapBlockCapacity);

            for (FLinearHeapBlock_* blk = (*phead)->Node.Next; blk; blk = blk->Node.Next) {
                if (blk->SizeInBytes + sz <= GLinearHeapBlockCapacity) {
                    *off = blk->SizeInBytes;
                    FLinearHeapBlockList_::PokeHead(phead, nullptr, blk);
                    return;
                }
            }
        }

        Get().NewBlock(phead);
        Assert_NoAssume(phead);
        Assert_NoAssume(0 == (*phead)->SizeInBytes);

#if USE_PPE_MEMORYDOMAINS
        trackingData.AllocateSystem(GLinearHeapAllocationSize);
        MEMORYDOMAIN_TRACKING_DATA(LinearHeap).AllocateUser(GLinearHeapAllocationSize);
#endif

        *off = 0;
    }

#if USE_PPE_MEMORYDOMAINS
    static void FreeBlock(FLinearHeapBlock_* blk, FMemoryTracking& trackingData) {
#else
    static void FreeBlock(FLinearHeapBlock_* blk) {
#endif
        Assert(blk);

#if USE_PPE_MEMORYDOMAINS
        trackingData.DeallocateSystem(GLinearHeapAllocationSize);
        MEMORYDOMAIN_TRACKING_DATA(LinearHeap).DeallocateUser(GLinearHeapAllocationSize);
#endif

        Get()._vm.Free(blk, GLinearHeapAllocationSize);
    }

#if USE_PPE_MEMORYDOMAINS
    static NO_INLINE void ReleaseEmptyBlock(FLinearHeapBlock_** phead, FLinearHeapBlock_* blk, FMemoryTracking& trackingData) {
#else
    static NO_INLINE void ReleaseEmptyBlock(FLinearHeapBlock_ * *phead, FLinearHeapBlock_ * blk) {
#endif
        Assert(phead);
        Assert(blk);
        Assert_NoAssume(0 == blk->SizeInBytes);

        FLinearHeapBlockList_::Erase(phead, nullptr, blk);

#if USE_PPE_MEMORYDOMAINS
        FreeBlock(blk, trackingData);
#else
        FreeBlock(blk);
#endif
    }

    static void ReleaseAll() {
        Get()._vm.ReleaseAll();
    }

private:
    STATIC_CONST_INTEGRAL(size_t, VMCacheBlocks, 32);
    STATIC_CONST_INTEGRAL(size_t, VMCacheSizeInBytes, 2 * 1024 * 1024); // <=> 2 mo global cache for large blocks
    VIRTUALMEMORYCACHE(LinearHeap, VMCacheBlocks, VMCacheSizeInBytes) _vm;

    static FLinearHeapVMCache_& Get() {
        static FLinearHeapVMCache_ GInstance;
        return GInstance;
    }

    void NewBlock(FLinearHeapBlock_** phead) {
        void* const ptr = _vm.Allocate(GLinearHeapAllocationSize);
        FLinearHeapBlock_* const blk = INPLACE_NEW(ptr, FLinearHeapBlock_);
        FLinearHeapBlockList_::PushHead(phead, nullptr, blk);
    }

    FLinearHeapVMCache_() = default; // private constructor
};
#endif //!!USE_PPE_LINEARHEAP_DEBUG_FALLBACK
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const size_t FLinearHeap::MinBlockSize = ALLOCATION_BOUNDARY;
const size_t FLinearHeap::MaxBlockSize = GLinearHeapMaxSize;
//----------------------------------------------------------------------------
#if USE_PPE_MEMORYDOMAINS
FLinearHeap::FLinearHeap(FMemoryTracking& parent) NOEXCEPT
#else
FLinearHeap::FLinearHeap() NOEXCEPT
#endif
    : _offset(0)
    , _blocks(nullptr)
#if USE_PPE_MEMORYDOMAINS
    , _trackingData("LinearHeap", &parent) {
    Assert(parent.IsChildOf(FMemoryTracking::UsedMemory()));
    RegisterTrackingData(&_trackingData);
}
#else
{}
#endif
//----------------------------------------------------------------------------
FLinearHeap::~FLinearHeap() {
    ReleaseAll();
#if USE_PPE_MEMORYDOMAINS
    Assert_NoAssume(_trackingData.empty());
    UnregisterTrackingData(&_trackingData);
#endif
}
//----------------------------------------------------------------------------
void* FLinearHeap::Allocate(size_t size) {
    Assert(size);

    const size_t userSize = size;
#if USE_PPE_LINEARHEAP_DEBUG_FALLBACK
    size = SnapSize(size + GLinearHeapBlockOffset);
#else
    size = SnapSize(size);
#endif

    void* ptr;

#if USE_PPE_LINEARHEAP_DEBUG_FALLBACK
    AssertRelease(userSize <= MaxBlockSize);

    FLinearHeapBlock_*& head = FLinearHeapBlock_::Head(_blocks);

    FLinearHeapBlock_* const block = (FLinearHeapBlock_*)PPE::malloc(size);
    block->SizeInBytes = userSize;

#   if USE_PPE_MEMORYDOMAINS
    _trackingData.Allocate(userSize, size);
#   endif

    FLinearHeapBlockList_::PushHead(&head, nullptr, block);

    ptr = block->Data();

#else
    AssertRelease(size <= MaxBlockSize);

    FLinearHeapBlock_*& head = FLinearHeapBlock_::Head(_blocks);

    if (Unlikely((!head) | (_offset + size > MaxBlockSize))) {
#   if USE_PPE_MEMORYDOMAINS
        FLinearHeapVMCache_::Reserve(&head, &_offset, size, _trackingData);
#   else
        FLinearHeapVMCache_::Reserve(&head, &_offset, size);
#   endif
    }

    Assert(head);
    Assert_NoAssume(_offset + size <= MaxBlockSize);

    ptr = (head->Data() + _offset);
    _offset += size;

#   if USE_PPE_MEMORYDOMAINS
    _trackingData.AllocateUser(userSize);
#   endif

#   if USE_PPE_ASSERT
    {
        size_t offset2;
        FLinearHeapBlock_* const blk = FLinearHeapBlock_::FromData(ptr, &offset2);
        Assert_NoAssume(blk == head);
        Assert_NoAssume(offset2 + size == _offset);
    }
#   else
    UNUSED(userSize);
#   endif

#endif

    Assert(ptr);
    Assert_NoAssume(Meta::IsAligned(ALLOCATION_BOUNDARY, ptr));

    ONLY_IF_ASSERT(FillUninitializedBlock_(ptr, userSize));

    return ptr;
}
//----------------------------------------------------------------------------
void* FLinearHeap::Reallocate(void* ptr, size_t newSize, size_t oldSize) {
    Assert(newSize);
    AssertRelease(newSize <= MaxBlockSize);

    if (nullptr == ptr) {
        Assert(0 == oldSize);
        return Allocate(newSize);
    }

    Assert_NoAssume(Meta::IsAligned(ALLOCATION_BOUNDARY, ptr));
    Assert_NoAssume(AliasesToHeap(ptr));

#if USE_PPE_LINEARHEAP_DEBUG_FALLBACK

    const size_t userOldSize = oldSize;
    const size_t userNewSize = newSize;

    oldSize = SnapSize(oldSize + GLinearHeapBlockOffset);
    newSize = SnapSize(newSize + GLinearHeapBlockOffset);

    FLinearHeapBlock_* const blk = FLinearHeapBlock_::FromData(ptr);
    AssertRelease(userOldSize == blk->SizeInBytes);

    if (oldSize == newSize) {
        blk->SizeInBytes = userNewSize;

#   if USE_PPE_MEMORYDOMAINS
        // user size might be different
        _trackingData.Deallocate(userOldSize, oldSize);
        _trackingData.Allocate(userNewSize, newSize);
#   endif

        return ptr;
    }
    else {
        return Reallocate_AssumeSlow(ptr, userNewSize, userOldSize);
    }

#else

    size_t off;
    FLinearHeapBlock_* const blk = FLinearHeapBlock_::FromData(ptr, &off);

    if ((blk == _blocks) & (off + SnapSize(oldSize) == _offset))
        return Reallocate_AssumeLast(ptr, newSize, oldSize);
    else
        return Reallocate_AssumeSlow(ptr, newSize, oldSize);

#endif
}
//----------------------------------------------------------------------------
void FLinearHeap::Deallocate(void* ptr, size_t size) {
    Assert(ptr);
    Assert(size);
    AssertRelease(size <= MaxBlockSize);
    Assert_NoAssume(Meta::IsAligned(ALLOCATION_BOUNDARY, ptr));
    Assert_NoAssume(AliasesToHeap(ptr));

    ONLY_IF_ASSERT(FillDeletedBlock_(ptr, size));

#if USE_PPE_MEMORYDOMAINS
    const size_t userSize = size;
#endif
#if USE_PPE_LINEARHEAP_DEBUG_FALLBACK
    size = SnapSize(size + GLinearHeapBlockOffset);
#else
    size = SnapSize(size);
#endif

#if USE_PPE_LINEARHEAP_DEBUG_FALLBACK

    FLinearHeapBlock_* const blk = FLinearHeapBlock_::FromData(ptr);
    Assert_NoAssume(userSize == blk->SizeInBytes);

    FLinearHeapBlock_*& head = FLinearHeapBlock_::Head(_blocks);
    FLinearHeapBlockList_::Erase(&head, nullptr, blk);

#   if USE_PPE_MEMORYDOMAINS
    _trackingData.Deallocate(userSize, size);
#   endif

    PPE::free(blk);

#else

#   if USE_PPE_MEMORYDOMAINS
    _trackingData.DeallocateUser(userSize);
#   endif

    size_t offset;
    FLinearHeapBlock_* const blk = FLinearHeapBlock_::FromData(ptr, &offset);

    if ((blk == _blocks) & (offset + size == _offset))
        _offset = offset;
    else if (offset + size == blk->SizeInBytes) {
        blk->SizeInBytes = offset;

        if (Unlikely(0 == offset)) {
            // release free blocks only when they're not at head
#   if USE_PPE_MEMORYDOMAINS
            FLinearHeapVMCache_::ReleaseEmptyBlock(&FLinearHeapBlock_::Head(_blocks), blk, _trackingData);
#   else
            FLinearHeapVMCache_::ReleaseEmptyBlock(&FLinearHeapBlock_::Head(_blocks), blk);
#   endif
        }
    }
    // else this block is definitely lost and won't be reusable, use a pool ahead of the heap !

#endif
}
//----------------------------------------------------------------------------
void* FLinearHeap::Reallocate_AssumeLast(void* ptr, size_t newSize, size_t oldSize) {
    Assert(ptr);
    Assert(newSize);
    Assert(oldSize);
    AssertRelease(newSize <= MaxBlockSize);
    Assert_NoAssume(Meta::IsAligned(ALLOCATION_BOUNDARY, ptr));
    Assert_NoAssume(AliasesToHeap(ptr));

#if USE_PPE_LINEARHEAP_DEBUG_FALLBACK

    FLinearHeapBlock_* const blk = FLinearHeapBlock_::FromData(ptr);
    AssertRelease(blk == _blocks); // last block allocated

    return Reallocate(ptr, newSize, oldSize);

#else

    const size_t userNewSize = newSize;
    const size_t userOldSize = oldSize;

    newSize = SnapSize(newSize);
    oldSize = SnapSize(oldSize);

    size_t offset;
    Verify(FLinearHeapBlock_::FromData(ptr, &offset) == _blocks);
    Assert_NoAssume(offset + oldSize == _offset);

    if (Likely(offset + newSize <= MaxBlockSize)) {
#if USE_PPE_MEMORYDOMAINS
        _trackingData.DeallocateUser(userOldSize);
        _trackingData.AllocateUser(userNewSize);
#endif

        _offset = offset + newSize;
        return ptr;
    }
    else {
        return Reallocate_AssumeSlow(ptr, userNewSize, userOldSize);
    }

#endif
}
//----------------------------------------------------------------------------
void FLinearHeap::Deallocate_AssumeLast(void* ptr, size_t size) {
    Assert(ptr);
    Assert(size);
    AssertRelease(size <= MaxBlockSize);
    Assert_NoAssume(Meta::IsAligned(ALLOCATION_BOUNDARY, ptr));
    Assert_NoAssume(AliasesToHeap(ptr));

#if USE_PPE_LINEARHEAP_DEBUG_FALLBACK

    FLinearHeapBlock_* const blk = FLinearHeapBlock_::FromData(ptr);
    AssertRelease(blk == _blocks); // last block allocated

    return Deallocate(ptr, size);

#else

#if USE_PPE_MEMORYDOMAINS
    _trackingData.DeallocateUser(size);
#endif

    size = SnapSize(size);

    size_t offset;
    Verify(FLinearHeapBlock_::FromData(ptr, &offset) == _blocks);
    Assert_NoAssume(offset + size == _offset);

    _offset = offset;

#endif
}
//----------------------------------------------------------------------------
void* FLinearHeap::Reallocate_AssumeSlow(void* ptr, size_t newSize, size_t oldSize) {
    Assert(ptr);
    Assert(newSize);
    Assert(oldSize);

    void* const newp = Allocate(newSize);
    FPlatformMemory::MemcpyLarge(newp, ptr, Min(SnapSize(newSize), SnapSize(oldSize))/* using aligned memcpy */);
    Deallocate(ptr, oldSize);

    return newp;
}
//----------------------------------------------------------------------------
void FLinearHeap::ReleaseAll() {
    if (nullptr == _blocks) {
        Assert_NoAssume(0 == _offset);
        return; // nothing to do
    }

#if USE_PPE_LINEARHEAP_DEBUG_FALLBACK
    for (FLinearHeapBlock_*& head = FLinearHeapBlock_::Head(_blocks); head;)
        Deallocate(head->Data(), head->SizeInBytes);

#else
    FLinearHeapBlock_*& head = FLinearHeapBlock_::Head(_blocks);

#   if USE_PPE_MEMORYDOMAINS
    _trackingData.ReleaseAllUser();
#   endif

    while (FLinearHeapBlock_* const blk = FLinearHeapBlockList_::PopHead(&head, nullptr)) {
#   if USE_PPE_MEMORYDOMAINS
        FLinearHeapVMCache_::FreeBlock(blk, _trackingData);
#   else
        FLinearHeapVMCache_::FreeBlock(blk);
#   endif
    }

    _offset = 0;
    _blocks = nullptr;

#endif
}
//----------------------------------------------------------------------------
bool FLinearHeap::IsLastBlock(void* ptr, size_t size) const NOEXCEPT {
    Assert(ptr);
    Assert(size);

#if USE_PPE_LINEARHEAP_DEBUG_FALLBACK

    FLinearHeapBlock_* const blk = FLinearHeapBlock_::FromData(ptr);
    Assert_NoAssume(blk->SizeInBytes == SnapSize(size));

    return (blk == ptr);

#else

    size_t offset;
    if (FLinearHeapBlock_::FromData(ptr, &offset) == _blocks)
        return (offset + SnapSize(size) == _offset);
    else
        return false;

#endif
}
//----------------------------------------------------------------------------
#if !USE_PPE_FINAL_RELEASE
bool FLinearHeap::AliasesToHeap(void* ptr) const NOEXCEPT {
    Assert(ptr);
    Assert_NoAssume(Meta::IsAligned(ALLOCATION_BOUNDARY, ptr));

    // !!! SLOW !!! (but safe)

#if USE_PPE_LINEARHEAP_DEBUG_FALLBACK

    for (FLinearHeapBlock_* blk = (FLinearHeapBlock_*)_blocks; blk; blk = blk->Node.Next) {
        const u8* pData = (const u8*)blk->Data();
        if ((pData <= ptr) & (pData + blk->SizeInBytes >= ptr))
            return true;
    }

#else

    for (auto* blk = static_cast<const FLinearHeapBlock_*>(_blocks); blk; blk = blk->Node.Next) {
        const size_t off = (blk == _blocks ? _offset : blk->SizeInBytes);
        if ((blk->Data() <= (u8*)ptr) & (blk->Data() + off > ptr))
            return true;
    }

#endif

    return false;
}
#endif //!#if !USE_PPE_FINAL_RELEASE
//----------------------------------------------------------------------------
size_t FLinearHeap::SnapSize(size_t sizeInBytes) NOEXCEPT {
#if USE_PPE_LINEARHEAP_DEBUG_FALLBACK
    return PPE::malloc_snap_size(sizeInBytes);
#else
    STATIC_ASSERT(16 == ALLOCATION_BOUNDARY);
    return ROUND_TO_NEXT_16(sizeInBytes);
#endif
}
//----------------------------------------------------------------------------
void FLinearHeap::FlushVirtualMemoryCache() {
#if !USE_PPE_LINEARHEAP_DEBUG_FALLBACK
    FLinearHeapVMCache_::ReleaseAll();
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
struct FPoolLinearHeapBlock_ {
    size_t SizeInBytes;
    TIntrusiveSingleListNode<FPoolLinearHeapBlock_> Node;
};
//----------------------------------------------------------------------------
using FPoolLinearHeapList_ = INTRUSIVESINGLELIST_ACCESSOR(&FPoolLinearHeapBlock_::Node);
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_MEMORYDOMAINS
FPooledLinearHeap::FPooledLinearHeap(FMemoryTracking& parent) NOEXCEPT
:   _heap(parent)
,   _pools{0}
{}
#else
FPooledLinearHeap::FPooledLinearHeap() NOEXCEPT
:   _pools{0}
{}
#endif
//----------------------------------------------------------------------------
void* FPooledLinearHeap::Allocate(size_t size) {
    Assert(size);

#if USE_PPE_LINEARHEAP_POOLING
    const size_t cls = FMallocBinned::SizeClass(size);

    if (Likely((cls < lengthof(_pools)) && _pools[cls])) {
        auto* blk = static_cast<FPoolLinearHeapBlock_*>(_pools[cls]);
        Assert_NoAssume(FMallocBinned::SizeClasses[cls] == blk->SizeInBytes);
        _pools[cls] = blk->Node.Next;

#   if USE_PPE_MEMORYDOMAINS
        _heap._trackingData.AllocateUser(FMallocBinned::SizeClasses[cls]);
#   endif

        return blk;
    }
    else {
        return _heap.Allocate(
            (cls < lengthof(_pools)
                ? FMallocBinned::SizeClasses[cls]
                : FLinearHeap::SnapSize(size) ));
    }

#else
    return _heap.Allocate(size);

#endif
}
//----------------------------------------------------------------------------
void* FPooledLinearHeap::Reallocate(void* ptr, size_t newSize, size_t oldSize) {
#if USE_PPE_LINEARHEAP_POOLING
    if (0 == oldSize) {
        Assert_NoAssume(nullptr == ptr);
        return Allocate(newSize);
    }

    Assert(ptr);
    Assert(oldSize);

    newSize = SnapSize(newSize);
    oldSize = SnapSize(oldSize);

    if (_heap.IsLastBlock(ptr, oldSize)) {
        return _heap.Reallocate_AssumeLast(ptr, newSize, oldSize);
    }
    else {
        void* const newp = Allocate(newSize);
        FPlatformMemory::MemcpyLarge(newp, ptr, Min(oldSize, newSize));
        Deallocate(ptr, oldSize);
        return newp;
    }

#else
    return _heap.Reallocate(ptr, newSize, oldSize);

#endif
}
//----------------------------------------------------------------------------
void FPooledLinearHeap::Deallocate(void* ptr, size_t size) {
    Assert(ptr);
    Assert(size);

#if USE_PPE_LINEARHEAP_POOLING
    size = SnapSize(size);

    if (Likely(_heap.IsLastBlock(ptr, size) == false)) {
        Assert_NoAssume(sizeof(FPoolLinearHeapBlock_) <= size);

        const size_t cls = FMallocBinned::SizeClass(size);
        if (cls < lengthof(_pools)) {
            Assert_NoAssume(FMallocBinned::SizeClasses[cls] == size);

            auto* blk = static_cast<FPoolLinearHeapBlock_*>(ptr);
            blk->SizeInBytes = size; // stores the size only for trimming
            blk->Node.Next = static_cast<FPoolLinearHeapBlock_*>(_pools[cls]);

#   if USE_PPE_MEMORYDOMAINS
            _heap._trackingData.DeallocateUser(size);
#   endif

            _pools[cls] = blk;
        }
        else {
            _heap.Deallocate(ptr, size);
        }
    }
    else {
        _heap.Deallocate_AssumeLast(ptr, size);
    }

#else
    _heap.Deallocate(ptr, size);
#endif
}
//----------------------------------------------------------------------------
void FPooledLinearHeap::ReleaseAll() {
    for (void*& p : _pools)
        p = nullptr;

    _heap.ReleaseAll();
}
//----------------------------------------------------------------------------
void FPooledLinearHeap::TrimPools() {
#if USE_PPE_LINEARHEAP_POOLING
    // sort the pooled blocks to reclaim as mush memory as possible.
    // using insertion sort to avoid allocation by reusing the pool list.

    struct FSortByEndAddress_ {
        bool operator ()(const FPoolLinearHeapBlock_& a, const FPoolLinearHeapBlock_& b) const NOEXCEPT {
            return ((u8*)& a + a.SizeInBytes > (u8*)& b + b.SizeInBytes);
        }
    };

    FPoolLinearHeapBlock_* sorted = nullptr;

    forrange(cls, 0, FMallocBinned::NumSizeClasses) {
        for (auto* blk = static_cast<FPoolLinearHeapBlock_*>(_pools[cls]); blk; ) {
            FPoolLinearHeapBlock_* const b = blk;
            blk = blk->Node.Next;
            FPoolLinearHeapList_::Insert(&sorted, b, FSortByEndAddress_{});
        }

        _pools[cls] = nullptr;
    }

    // then yield to internal heap once sorted, still not guaranteed to be complete
    // so don't use Deallocate_AssumeLast()

    while (sorted) {
        if (_heap.IsLastBlock(sorted, sorted->SizeInBytes) == false)
            break; // don't lose blocks, stop if we have a hole

        FPoolLinearHeapBlock_* const blk = FPoolLinearHeapList_::PopHead(&sorted);

#   if USE_PPE_MEMORYDOMAINS
        _heap._trackingData.AllocateUser(blk->SizeInBytes); // to keep consistency with tracking inside Deallocate()
#   endif

        _heap.Deallocate(blk, blk->SizeInBytes);
    }

    // finally reconstruct the free lists if we found a hole and stopped the process

    while (FPoolLinearHeapBlock_* const blk = FPoolLinearHeapList_::PopHead(&sorted)) {
        const size_t cls = FMallocBinned::SizeClass(blk->SizeInBytes);
        blk->Node.Next = static_cast<FPoolLinearHeapBlock_*>(_pools[cls]);
        _pools[cls] = blk;
    }

#endif //!USE_PPE_LINEARHEAP_POOLING
}
//----------------------------------------------------------------------------
size_t FPooledLinearHeap::SnapSize(size_t sizeInBytes) NOEXCEPT {
#if USE_PPE_LINEARHEAP_POOLING
    const size_t cls = FMallocBinned::SizeClass(sizeInBytes);
    return (cls < lengthof(_pools)
        ? FMallocBinned::SizeClasses[cls]
        : FLinearHeap::SnapSize(sizeInBytes) );
#else
    return FLinearHeap::SnapSize(sizeInBytes);
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
