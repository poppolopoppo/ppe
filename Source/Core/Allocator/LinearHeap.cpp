#include "stdafx.h"

#include "LinearHeap.h"

#include "Memory/MemoryDomain.h"
#include "Memory/VirtualMemory.h"
#include "Thread/AtomicSpinLock.h"
#include "Thread/ThreadContext.h"

//----------------------------------------------------------------------------
// Turn to 1 to disable linear heap allocations (useful for memory debugging) :
//----------------------------------------------------------------------------
#define WITH_CORE_LINEARHEAP_FALLBACK_TO_MALLOC (USE_CORE_MEMORY_DEBUGGING) //%__NOCOMMIT%

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
struct FLinearHeapSmallBlock_ {
    STATIC_CONST_INTEGRAL(size_t, AllocationSize, ALLOCATION_GRANULARITY);
    STATIC_CONST_INTEGRAL(CODE3264(u64, u32), DefaultCanary, CODE3264(0xAABBDDEEAABBDDEEull, 0xAABBDDEEul));

    FLinearHeapSmallBlock_* Next = 0;
    u16 Offset = 0;
    u16 Last = 0;
    CODE3264(u64, u32) Canary = DefaultCanary; // serves as padding to align on 16
};
STATIC_ASSERT(sizeof(FLinearHeapSmallBlock_) == ALLOCATION_BOUNDARY);
STATIC_CONST_INTEGRAL(size_t, GLinearHeapSmallBlocksMaxSize, 32736);
STATIC_CONST_INTEGRAL(size_t, GLinearHeapSmallBlocksCapacity, FLinearHeapSmallBlock_::AllocationSize - sizeof(FLinearHeapSmallBlock_));
//----------------------------------------------------------------------------
struct FLinearHeapLargeBlock_ {
    void* Ptr;
    size_t SizeInBytes;
};
STATIC_CONST_INTEGRAL(size_t, GLinearHeapLargeBlocksAllocationSize, FLinearHeapSmallBlock_::AllocationSize);
STATIC_CONST_INTEGRAL(size_t, GLinearHeapLargeBlocksCapacity, GLinearHeapLargeBlocksAllocationSize / sizeof(FLinearHeapLargeBlock_));
//----------------------------------------------------------------------------
class FLinearHeapVMCache_ {
public:
    STATIC_CONST_INTEGRAL(size_t, VMCacheBlocks, 32);
    STATIC_CONST_INTEGRAL(size_t, VMCacheSizeInBytes, 16 * 1024 * 1024); // <=> 16 mo global cache for large blocks

#if !WITH_CORE_LINEARHEAP_FALLBACK_TO_MALLOC
    static FLinearHeapVMCache_& Instance() {
        static FLinearHeapVMCache_ GInstance;
        return GInstance;
    }

    static FLinearHeapVMCache_& GInstance;
#endif

    NO_INLINE static void* AllocateSmallBlock() {
#if WITH_CORE_LINEARHEAP_FALLBACK_TO_MALLOC
        return Core::malloc(FLinearHeapSmallBlock_::AllocationSize);
#else
        FLinearHeapVMCache_& Cache = GInstance;
        const FAtomicSpinLock::FScope scopeLock(Cache._barrier);
        return (FLinearHeapSmallBlock_*)Cache._vm.Allocate(FLinearHeapSmallBlock_::AllocationSize);
#endif
    }

    static void ReleaseSmallBlock(void* smallBlock) {
        Assert(smallBlock);
#if WITH_CORE_LINEARHEAP_FALLBACK_TO_MALLOC
        Core::free(smallBlock);
#else
        FLinearHeapVMCache_& Cache = GInstance;
        const FAtomicSpinLock::FScope scopeLock(Cache._barrier);
        Cache._vm.Free(smallBlock, FLinearHeapSmallBlock_::AllocationSize);
#endif
    }

    NO_INLINE static void AllocateLargeBlock(FLinearHeapLargeBlock_& largeBlock, size_t sizeInBytes, size_t alignment) {
        Assert(nullptr == largeBlock.Ptr);
        Assert(0 == largeBlock.SizeInBytes);
#if WITH_CORE_LINEARHEAP_FALLBACK_TO_MALLOC
        largeBlock.Ptr = Core::aligned_malloc(sizeInBytes, alignment);
#else
        FLinearHeapVMCache_& Cache = GInstance;
        const FAtomicSpinLock::FScope scopeLock(Cache._barrier);
        largeBlock.Ptr = Cache._vm.Allocate(sizeInBytes);
#endif
        largeBlock.SizeInBytes = sizeInBytes;

        Assert(largeBlock.Ptr);
        Assert(Meta::IsAligned(alignment, largeBlock.Ptr));
    }

    static void ReleaseLargeBlock(FLinearHeapLargeBlock_& largeBlock) {
        Assert(largeBlock.Ptr);
        Assert(largeBlock.SizeInBytes);

#if WITH_CORE_LINEARHEAP_FALLBACK_TO_MALLOC
        Core::aligned_free(largeBlock.Ptr);
#else
        FLinearHeapVMCache_& Cache = GInstance;
        const FAtomicSpinLock::FScope scopeLock(Cache._barrier);
        Cache._vm.Free(largeBlock.Ptr, largeBlock.SizeInBytes);
#endif

        ONLY_IF_ASSERT(largeBlock.Ptr = nullptr);
        ONLY_IF_ASSERT(largeBlock.SizeInBytes = 0);
    }

private:
#if !WITH_CORE_LINEARHEAP_FALLBACK_TO_MALLOC
    FAtomicSpinLock _barrier;
    VIRTUALMEMORYCACHE(LinearHeap, VMCacheBlocks, VMCacheSizeInBytes) _vm;
#endif
};
#if !WITH_CORE_LINEARHEAP_FALLBACK_TO_MALLOC
FLinearHeapVMCache_& FLinearHeapVMCache_::GInstance = FLinearHeapVMCache_::Instance();
#endif
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FLinearHeap::FLinearHeap(const char* name)
    : _smallBlocksTLS{ 0 }
    , _largeBlocks(nullptr)
    , _largeBlocksCount(0)
#ifdef WITH_CORE_ASSERT
    , _frozen{ false }
#endif
#ifdef USE_MEMORY_DOMAINS
    , _trackingData(name, &MEMORY_DOMAIN_TRACKING_DATA(LinearHeap)) {
    RegisterAdditionalTrackingData(&_trackingData);
#else
    {
#endif

    // Use a small block to store large allocations : 64k / 16 = 4096 entries max on x64, 8192 on x86
    _largeBlocks = FLinearHeapVMCache_::AllocateSmallBlock();
    ONLY_IF_ASSERT(::memset(_largeBlocks, 0, GLinearHeapLargeBlocksAllocationSize));
}
//----------------------------------------------------------------------------
FLinearHeap::~FLinearHeap() {
    ReleaseAll();

    Assert(_largeBlocks);
    Assert(0 == _largeBlocksCount);
    FLinearHeapVMCache_::ReleaseSmallBlock(_largeBlocks);
    ONLY_IF_ASSERT(_largeBlocks = nullptr); // crash on necrophilia

#ifdef USE_MEMORY_DOMAINS
    UnregisterAdditionalTrackingData(&_trackingData);
#endif
}
//----------------------------------------------------------------------------
void* FLinearHeap::Allocate(size_t sizeInBytes, size_t alignment/* = ALLOCATION_BOUNDARY */) {
    Assert(sizeInBytes);
    Assert(alignment >= sizeof(intptr_t));
    Assert_NoAssume(!_frozen);

    void* result;

#if !WITH_CORE_LINEARHEAP_FALLBACK_TO_MALLOC
    if (Likely(sizeInBytes <= GLinearHeapSmallBlocksMaxSize)) {

        result = SmallAllocate_(sizeInBytes, alignment);
    }
    else
#endif
    {
        result = LargeAllocate_(sizeInBytes, alignment);
    }

    Assert(Meta::IsAligned(alignment, result));
    Assert_NoAssume(!_frozen);

#   ifdef USE_MEMORY_DOMAINS
    _trackingData.Allocate(1, sizeInBytes);
#   endif

    return result;
}
//----------------------------------------------------------------------------
void* FLinearHeap::Relocate(void* ptr, size_t oldSize, size_t newSize, size_t alignment/* = ALLOCATION_BOUNDARY */) {
    Assert(ptr);
    Assert(newSize);
    Assert(oldSize);
    Assert(alignment >= sizeof(intptr_t));
    Assert_NoAssume(!_frozen);

#if !WITH_CORE_LINEARHEAP_FALLBACK_TO_MALLOC
    auto*& smallBlockTLS = (FLinearHeapSmallBlock_*&)_smallBlocksTLS[GCurrentThreadIndex];
    Assert(nullptr == smallBlockTLS || FLinearHeapSmallBlock_::DefaultCanary == smallBlockTLS->Canary);

    bool releaseSmallBlockTLS = false;

    // check if ptr was the last block allocated
    if (((u8*)(smallBlockTLS + 1) + smallBlockTLS->Last) == ptr) {
        if (smallBlockTLS->Last + newSize <= GLinearHeapSmallBlocksCapacity) {
            // enough space available, grow/shrink the last allocation
            smallBlockTLS->Offset = checked_cast<u16>(smallBlockTLS->Last + newSize);

#   ifdef USE_MEMORY_DOMAINS
            _trackingData.Deallocate(1, oldSize);
            _trackingData.Allocate(1, newSize);
#   endif

            return ptr;
        }
        else {
            // can't allocate here, but dispose of the space afterwards
            releaseSmallBlockTLS = true;
        }
    }

    // check if ptr is a large block
    if (Meta::IsAligned(ALLOCATION_GRANULARITY, ptr)) {
        // check if enough space thanks to allocation granularity
        if (ROUND_TO_NEXT_64K(newSize) <= ROUND_TO_NEXT_64K(oldSize))
            return ptr;
#else
    {
#endif
        // look for the large block descriptor and relocate
        auto* largeBlocks = (FLinearHeapLargeBlock_*)_largeBlocks;
        forrange(plarge, largeBlocks, largeBlocks + _largeBlocksCount) {
            if (plarge->Ptr == ptr) {
                Assert(plarge->SizeInBytes >= oldSize);

                FLinearHeapLargeBlock_ newBlock;
                FLinearHeapVMCache_::AllocateLargeBlock(newBlock, ROUND_TO_NEXT_64K(newSize), alignment);

                ::memcpy(newBlock.Ptr, plarge->Ptr, Min(newSize, oldSize));

                FLinearHeapVMCache_::ReleaseLargeBlock(*plarge);
                *plarge = newBlock;

#   ifdef USE_MEMORY_DOMAINS
                _trackingData.Deallocate(1, oldSize);
                _trackingData.Allocate(1, newSize);
#   endif

                return newBlock.Ptr;
            }
        }

        AssertNotReached(); // this pointer don't belong to this linear heap !
    }

#if !WITH_CORE_LINEARHEAP_FALLBACK_TO_MALLOC
    // fall back to slow path
    void* const newPtr = Allocate(newSize, alignment);
    ::memcpy(newPtr, ptr, Min(newSize, oldSize));

    if (releaseSmallBlockTLS) {
        // release previously occupied space in TLS small block
        Assert(((u8*)(smallBlockTLS + 1) + smallBlockTLS->Last) == ptr);
        smallBlockTLS->Offset = smallBlockTLS->Last;

#   ifdef USE_MEMORY_DOMAINS
        _trackingData.Deallocate(1, oldSize);
        // _trackingData.Allocate() already done in previous Allocate() upthere
#   endif
    }
    else {
        // ptr is "leaked", only released by ReleaseAll() later
    }

    return newPtr;
#else
    AssertNotReached();
    return nullptr;
#endif
}
//----------------------------------------------------------------------------
void FLinearHeap::Deallocate(void* ptr) {
    Assert(nullptr != ptr);

#if !WITH_CORE_LINEARHEAP_FALLBACK_TO_MALLOC
    if (Meta::IsAligned(ALLOCATION_GRANULARITY, ptr)) {
#else
    {
#endif
        LargeDeallocate_(ptr);
    }
}
//----------------------------------------------------------------------------
void FLinearHeap::ReleaseAll() {
    Assert_NoAssume(_frozen); // guarantee that nobody is touching the heap ?

    // Small blocks
    {
        forrange(t, 0, lengthof(_smallBlocksTLS)) {
            auto*& smallBlockTLS = (FLinearHeapSmallBlock_*&)_smallBlocksTLS[t];
#if WITH_CORE_LINEARHEAP_FALLBACK_TO_MALLOC
            AssertRelease(nullptr == smallBlockTLS);
#else
            FLinearHeapSmallBlock_* psmall = smallBlockTLS;
            smallBlockTLS = nullptr;

            while (psmall) {
                auto* pnext = psmall->Next;
                FLinearHeapVMCache_::ReleaseSmallBlock(psmall);
                psmall = pnext;
            }
#endif
        }
    }

    // Large blocks
    {
        auto* largeBlocks = (FLinearHeapLargeBlock_*)_largeBlocks;
        const size_t largeBlocksCount = _largeBlocksCount.exchange(0);
        forrange(plarge, largeBlocks, largeBlocks + largeBlocksCount)
            if (plarge->Ptr) // holes can appear when calling Deallocate()
                FLinearHeapVMCache_::ReleaseLargeBlock(*plarge);
            else
                Assert(0 == plarge->SizeInBytes);
    }

#ifdef USE_MEMORY_DOMAINS
    _trackingData.ReleaseAll();
#endif
}
//----------------------------------------------------------------------------
void FLinearHeap::Freeze() {
    Assert_NoAssume(!_frozen);
    ONLY_IF_ASSERT(_frozen = true);
}
//----------------------------------------------------------------------------
void FLinearHeap::Unfreeze() {
    Assert_NoAssume(_frozen);
    ONLY_IF_ASSERT(_frozen = false);
}
//----------------------------------------------------------------------------
size_t FLinearHeap::SnapSize(size_t size) {
    return (size <= GLinearHeapSmallBlocksMaxSize ? size : ROUND_TO_NEXT_64K(size));
}
//----------------------------------------------------------------------------
void* FLinearHeap::SmallAllocate_(size_t sizeInBytes, size_t alignment) {
#if WITH_CORE_LINEARHEAP_FALLBACK_TO_MALLOC
    AssertNotReached();
    return nullptr;

#else
    Assert(sizeInBytes <= GLinearHeapSmallBlocksMaxSize);
    Assert(GCurrentThreadIndex < lengthof(_smallBlocksTLS));

    auto*& smallBlockTLS = (FLinearHeapSmallBlock_*&)_smallBlocksTLS[GCurrentThreadIndex];
    Assert(nullptr == smallBlockTLS || FLinearHeapSmallBlock_::DefaultCanary == smallBlockTLS->Canary);

    // try to allocate from TLS small block, can fail if no block or not enough space available
    if (Unlikely(nullptr == smallBlockTLS || Meta::RoundToNext(smallBlockTLS->Offset, alignment) + sizeInBytes > GLinearHeapSmallBlocksCapacity)) {
        // allocate a new block from dedicated virtual memory cache
        auto* newBlock = new (FLinearHeapVMCache_::AllocateSmallBlock()) FLinearHeapSmallBlock_();
        newBlock->Next = smallBlockTLS;
        smallBlockTLS = newBlock;
    }

    // Increment the offset of the current small block
    Assert(FLinearHeapSmallBlock_::DefaultCanary == smallBlockTLS->Canary);
    const size_t alignedOffset = Meta::RoundToNext(smallBlockTLS->Offset, alignment);
    smallBlockTLS->Offset = checked_cast<u16>(alignedOffset + sizeInBytes);
    smallBlockTLS->Last = u16(alignedOffset);

    return ((u8*)(smallBlockTLS + 1) + alignedOffset);
#endif
}
//----------------------------------------------------------------------------
NO_INLINE void* FLinearHeap::LargeAllocate_(size_t sizeInBytes, size_t alignment) {
#if !WITH_CORE_LINEARHEAP_FALLBACK_TO_MALLOC
    Assert(sizeInBytes > GLinearHeapSmallBlocksMaxSize);
#endif
    STATIC_ASSERT(ALLOCATION_GRANULARITY == 64 * 1024);

    // allocates large blocks from VM cache
    FLinearHeapLargeBlock_ newBlock;
    FLinearHeapVMCache_::AllocateLargeBlock(newBlock, ROUND_TO_NEXT_64K(sizeInBytes), ALLOCATION_GRANULARITY);

    Assert(Meta::IsAligned(ALLOCATION_GRANULARITY, newBlock.Ptr));

    // atomically register large blocks for later deletion
    for (;;) {
        size_t largeBlockIndex = _largeBlocksCount;
        if (_largeBlocksCount.compare_exchange_weak(largeBlockIndex, largeBlockIndex + 1, std::memory_order_relaxed)) {
            auto* plarge = ((FLinearHeapLargeBlock_*)_largeBlocks + largeBlockIndex);
            Assert(nullptr == plarge->Ptr);
            Assert(0 == plarge->SizeInBytes);
            *plarge = newBlock;
            break;
        }
    }

    return newBlock.Ptr;
}
//----------------------------------------------------------------------------
NO_INLINE void FLinearHeap::LargeDeallocate_(void* ptr) {
#if !WITH_CORE_LINEARHEAP_FALLBACK_TO_MALLOC
    Assert(Meta::IsAligned(ALLOCATION_GRANULARITY, ptr));
#endif

    // look for the large block descriptor and relocate
    auto* largeBlocks = (FLinearHeapLargeBlock_*)_largeBlocks;
    forrange(plarge, largeBlocks, largeBlocks + _largeBlocksCount) {
        if (plarge->Ptr == ptr) {
#ifdef USE_MEMORY_DOMAINS
            _trackingData.Deallocate(1, plarge->SizeInBytes);
#endif

            FLinearHeapVMCache_::ReleaseLargeBlock(*plarge);
            plarge->Ptr = nullptr; // needed for release all
            plarge->SizeInBytes = 0;

            return;
        }
    }

    AssertNotReached(); // this pointer don't belong to this linear heap !
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
