#include "stdafx.h"

#include "Allocator/MallocStomp.h"

#ifdef WITH_PPE_MALLOCSTOMP

// #TODO : should be using a pool of 64k chunks and commit 4 pages out of it
// this would substantially increase usability on 32 bits builds (overhead falling from 64k to 4k for 16 bytes)

#   include "HAL/PlatformMemory.h"
#   include "Memory/MemoryTracking.h"

#   define PPE_MALLOCSTOMP_CHECK_OVERRUN 1 // set to 0 to check for under-runs
#   define PPE_MALLOCSTOMP_DELAY_DELETES 1 // set to 1 to check for necrophilia
#   define PPE_MALLOCSTOMP_BLOCK_OVERLAP 0 // need PPE_MALLOCSTOMP_DELAY_DELETES

#   if PPE_MALLOCSTOMP_DELAY_DELETES
#       include "Container/RingBuffer.h"
#       include "Thread/AtomicSpinLock.h"
        PRAGMA_INITSEG_COMPILER
#   endif

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Stomp allocator : try to find memory corruptions
//  - Allocate an additional page with read and write access disabled
//  - Every access to this area will summon a page fault exception by OS
//  - Check for overruns in wasted memory implied by alignment
//  - Check for underruns in each allocation payload data
//
//   / \   Consumes a large amount of memory
//  / ! \  Should turn off every special memory allocation schemes (pools)
//
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static constexpr u32 GStompAlignment        = 16;
static constexpr u32 GStompPageSize         = PAGE_SIZE;
static constexpr u32 GStompDefaultCanary    = 0xBEEFDEADul;
static constexpr size_t GStompLongCanary    = CODE3264(0xBEEFDEADul, 0xBEEFDEADBEEFDEADull);
//----------------------------------------------------------------------------
struct FStompPayload_ {
    const u32 AllocationSize;
    const u32 UserOffset;
    const u32 UserSize;
    const u32 Canary = GStompDefaultCanary;

    FStompPayload_(size_t allocationSize, size_t UserOffset, size_t userSize)
        : AllocationSize(checked_cast<u32>(allocationSize))
        , UserOffset(checked_cast<u32>(UserOffset))
        , UserSize(checked_cast<u32>(userSize))
    {}

    void CheckCanary() const {
        AssertRelease("Corrupted payload !", GStompDefaultCanary == Canary);
    }
};
STATIC_ASSERT(sizeof(FStompPayload_) == ALLOCATION_BOUNDARY);
//----------------------------------------------------------------------------
static void StompFillPadding_(const void* pbegin, const void* pend) {
    constexpr size_t word_size = sizeof(GStompLongCanary);
    if (pbegin >= pend)
        return;

    u8* const p0 = (u8*)pbegin;
    u8* const p3 = (u8*)pend;

    u8* const p1 = Meta::RoundToNext(p0, word_size);
    u8* const p2 = Meta::RoundToPrev(p3, word_size);

    forrange(p, p0, p1)
        *p = 0xAA;

    forrange(p, (size_t*)p1, (size_t*)p2)
        *p = GStompLongCanary;

    forrange(p, p2, p3)
        *p = 0xAA;
}
//----------------------------------------------------------------------------
static void StompCheckPadding_(const void* pbegin, const void* pend) {
    Assert(pbegin <= pend);

    constexpr size_t word_size = sizeof(GStompLongCanary);

    const u8* const p0 = (u8*)pbegin;
    const u8* const p3 = (u8*)pend;

    const u8* const p1 = Meta::RoundToNext(p0, word_size);
    const u8* const p2 = Meta::RoundToPrev(p3, word_size);

    bool succeed = true;

    forrange(p, p0, p1)
        succeed &= (0xAA == *p);

    forrange(p, (const size_t*)p1, (const size_t*)p2)
        succeed &= (GStompLongCanary == *p);

    forrange(p, p2, p3)
        succeed &= (0xAA == *p);

    AssertRelease("Corrupted memory block !", succeed);
}
//----------------------------------------------------------------------------
static const FStompPayload_* StompGetPayload_(const void* userPtr) {
    const FStompPayload_* const payload = (reinterpret_cast<const FStompPayload_*>(userPtr) - 1);

    payload->CheckCanary();

    const u8* allocationPtr = ((u8*)userPtr - payload->UserOffset);

#if PPE_MALLOCSTOMP_CHECK_OVERRUN
    StompCheckPadding_(allocationPtr, payload); // padding before
    StompCheckPadding_((u8*)userPtr + payload->UserSize, allocationPtr + payload->AllocationSize - GStompPageSize); // padding after

#else
    StompCheckPadding_((u8*)userPtr + payload->UserSize, allocationPtr + payload->AllocationSize); // padding after

#endif

    return payload;
}
//----------------------------------------------------------------------------
#if PPE_MALLOCSTOMP_DELAY_DELETES
class FStompDelayedDeletes_ {
public:
    static FStompDelayedDeletes_& Get() {
        static FStompDelayedDeletes_ GLocalInstance;
        return GLocalInstance;
    }

    static FStompDelayedDeletes_& GInstance;

    void Delete(void* ptr, size_t sizeInBytes) {
        Assert(ptr);
        Assert(Meta::IsAligned(GStompPageSize, sizeInBytes));

        // blocks too large are immediately freed to limit memory consumption
        if (sizeInBytes > MaxBlockSizeInBytes) {
            FPlatformMemory::PageFree(ptr, sizeInBytes);
            return;
        }

        // construct the deleted block to be delayed
        const FDeletedBlock_ delayed{ ptr, sizeInBytes };

        // can't read or write any of this deleted block now
        FPlatformMemory::PageProtect(ptr, sizeInBytes, false, false);

        // need thread safety from here
        const FAtomicSpinLock::FScope scopeLock(_barrier);

        // track total delayed memory size
        _delayedSizeInBytes += sizeInBytes;

        // check if the block overlaps with something in the list
#if PPE_MALLOCSTOMP_BLOCK_OVERLAP
        forrange(i, 0, _delayeds.size()) {
            const FDeletedBlock_& deleted = _delayeds[i];
            AssertRelease(not delayed.Overlaps(deleted));
        }
#endif

        // queue this block for delayed deletion and possibly delete an older one
        FDeletedBlock_ toDelete;
        if (_delayeds.push_back_OverflowIFN(&toDelete, delayed)) {
            ReleaseDelayedDelete_(toDelete);
        }

        // try to cap memory usage
        while (_delayedSizeInBytes > MaxDelayedSizeInBytes) {
            if (not _delayeds.pop_back(&toDelete))
                AssertNotReached(); // memory still used <=> at least one pending block to delete

            ReleaseDelayedDelete_(toDelete);
        }
    }

private:
    struct FDeletedBlock_ {
        void* Ptr;
        size_t SizeInBytes;

#if PPE_MALLOCSTOMP_BLOCK_OVERLAP
        // return true if this block intersects the memory mapped by other
        bool Overlaps(const FDeletedBlock_& other) const {
            u8* const c0 = (u8*)Ptr + (SizeInBytes >> 1);
            u8* const c1 = (u8*)other.Ptr + (other.SizeInBytes >> 1);
            return (size_t(std::abs(c0 - c1)) * 2 < (SizeInBytes + other.SizeInBytes));
        }
#endif
    };

#ifdef ARCH_X64
    // 64 bits can afford to let a large amount of memory temporarly leaking
    static constexpr size_t MaxDelayedDeletes       = ((2048ul<<10) / sizeof(FDeletedBlock_)); // maximum blocks delayed (2mb storage)
    static constexpr size_t MaxDelayedSizeInBytes   = 2000ul<<20; // 2gb of ram
    static constexpr size_t MaxBlockSizeInBytes     = 200ul<<20; // 200mb maximum for delayed deletion
#else
    // 32 bits processes have a much more limited virtual memory space
    static constexpr size_t MaxDelayedDeletes       = ((256ul<<10) / sizeof(FDeletedBlock_)); // maximum blocks delayed (256kb storage)
    static constexpr size_t MaxDelayedSizeInBytes   = 512ul<<20; // 512mb of ram
    static constexpr size_t MaxBlockSizeInBytes     = 50ul<<20; // 50mb maximum for delayed deletion
#endif

    FAtomicSpinLock _barrier;
    TRingBuffer<FDeletedBlock_> _delayeds;
    size_t _delayedSizeInBytes;

    FStompDelayedDeletes_()
        : _delayeds((FDeletedBlock_*)FPlatformMemory::VirtualAlloc(sizeof(FDeletedBlock_) * MaxDelayedDeletes, true), MaxDelayedDeletes)
        , _delayedSizeInBytes(0)
    {}

    ~FStompDelayedDeletes_() {
        // need thread safety from here
        const FAtomicSpinLock::FScope scopeLock(_barrier);

        // flush remaining blocks to prevent from leaking the program
        FDeletedBlock_ toDelete;
        while (_delayeds.pop_back(&toDelete))
            ReleaseDelayedDelete_(toDelete);

        Assert(0 == _delayedSizeInBytes);

        FPlatformMemory::VirtualFree(_delayeds.data(), sizeof(FDeletedBlock_) * MaxDelayedDeletes, true);
    }

    void ReleaseDelayedDelete_(const FDeletedBlock_& toDelete) {
        Assert(toDelete.SizeInBytes <= _delayedSizeInBytes);
        _delayedSizeInBytes -= toDelete.SizeInBytes;

        FPlatformMemory::PageFree(toDelete.Ptr, toDelete.SizeInBytes);
    }
};
FStompDelayedDeletes_& FStompDelayedDeletes_::GInstance = FStompDelayedDeletes_::Get();
#endif //!PPE_MALLOCSTOMP_DELAY_DELETES
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void* FMallocStomp::Malloc(size_t size) {
    return FMallocStomp::AlignedMalloc(size, GStompAlignment);
}
//----------------------------------------------------------------------------
void  FMallocStomp::Free(void* ptr) {
    return FMallocStomp::AlignedFree(ptr);
}
//----------------------------------------------------------------------------
void* FMallocStomp::Realloc(void* ptr, size_t size) {
    return FMallocStomp::AlignedRealloc(ptr, size, GStompAlignment);
}
//----------------------------------------------------------------------------
void* FMallocStomp::AlignedMalloc(size_t size, size_t alignment) {
    if (0 == size)
        return nullptr;

    Assert(Meta::IsPow2(alignment));

    const size_t paddingSize = Meta::RoundToNext(size, alignment) - size;
    const size_t pageAlignedSize = Meta::RoundToNext(size + paddingSize + sizeof(FStompPayload_), GStompPageSize);
    const size_t allocationSize = (pageAlignedSize + GStompPageSize); // extra page that will be read/write protected

    u8* const allocationPtr = (u8*)FPlatformMemory::PageAlloc(allocationSize);

#if PPE_MALLOCSTOMP_CHECK_OVERRUN
    u8* const protectedPtr = (allocationPtr + allocationSize - GStompPageSize);
    u8* const userPtr = (protectedPtr - paddingSize - size);

    StompFillPadding_(allocationPtr, userPtr - sizeof(FStompPayload_)); // padding before
    StompFillPadding_(userPtr + size, allocationPtr + pageAlignedSize); // padding after

#else // CHECK UNDERRUNS
    u8* const protectedPtr = allocationPtr;
    u8* const userPtr = (allocationPtr + GStompPageSize + sizeof(FStompPayload_));

    StompFillPadding_(userPtr + size, allocationPtr + allocationSize); // padding after

#endif //!PPE_MALLOCSTOMP_CHECK_OVERRUN

    // fill payload data to retrieve needed allocation properties
    const u32 userOffset = checked_cast<u32>(userPtr - (u8*)allocationPtr);
    INPLACE_NEW(userPtr - sizeof(FStompPayload_), FStompPayload_){ allocationSize, userOffset, size };
    ONLY_IF_ASSERT(StompGetPayload_(userPtr));

    // can't read or write AFTER user data
    FPlatformMemory::PageProtect(protectedPtr, GStompPageSize, false, false);

    Assert(userPtr);
    Assert(Meta::IsAligned(alignment, userPtr));
    return userPtr;
}
//----------------------------------------------------------------------------
void  FMallocStomp::AlignedFree(void* ptr) {
    if (nullptr == ptr)
        return;

    const FStompPayload_* const payload = StompGetPayload_(ptr);
    u8* const allocationPtr = ((u8*)ptr - payload->UserOffset);

#if PPE_MALLOCSTOMP_DELAY_DELETES
    FStompDelayedDeletes_::GInstance.Delete(allocationPtr, payload->AllocationSize);
#else
    StompVMFree_(allocationPtr, payload->AllocationSize);
#endif
}
//----------------------------------------------------------------------------
void* FMallocStomp::AlignedRealloc(void* ptr, size_t size, size_t alignment) {
    if (0 == size) {
        FMallocStomp::AlignedFree(ptr);
        return nullptr;
    }
    else if (nullptr == ptr) {
        return FMallocStomp::AlignedMalloc(size, alignment);
    }

    Assert(Meta::IsAligned(alignment, ptr));

#if 1 // much faster path, alas less secure
    if (ptr) {
        const FStompPayload_* const payload = StompGetPayload_(ptr);
        u8* const allocationPtr = ((u8*)ptr - payload->UserOffset);

        Assert(Meta::IsAligned(alignment, ptr));

        const size_t oldUserSize = payload->UserSize;
        const size_t paddingSize = Meta::RoundToNext(size, alignment) - size;
        const size_t pageAlignedSize = Meta::RoundToNext(size + paddingSize + sizeof(FStompPayload_), GStompPageSize);
        const size_t allocationSize = (pageAlignedSize + GStompPageSize); // extra page that will be read/write protected

        if (allocationSize == payload->AllocationSize) {
#if PPE_MALLOCSTOMP_CHECK_OVERRUN
            u8* const protectedPtr = (allocationPtr + allocationSize - GStompPageSize);
            u8* const userPtr = (protectedPtr - paddingSize - size);
#else // CHECK UNDERRUNS
            u8* const protectedPtr = allocationPtr;
            u8* const userPtr = (allocationPtr + GStompPageSize + sizeof(FStompPayload_));
#endif //!PPE_MALLOCSTOMP_CHECK_OVERRUN

            // keep data, use memmove because of overlapping
            const size_t moveSize = Min(size, oldUserSize);
            FPlatformMemory::Memmove(userPtr, ptr, moveSize);

#if PPE_MALLOCSTOMP_CHECK_OVERRUN
            StompFillPadding_(allocationPtr, userPtr - sizeof(FStompPayload_)); // padding before
            StompFillPadding_(userPtr + size, allocationPtr + pageAlignedSize); // padding after
#else // CHECK UNDERRUNS
            StompFillPadding_(userPtr + size, allocationPtr + allocationSize); // padding after
#endif //!PPE_MALLOCSTOMP_CHECK_OVERRUN

            // fill payload data to retrieve needed allocation properties
            const u32 userOffset = checked_cast<u32>(userPtr - (u8*)allocationPtr);
            INPLACE_NEW(userPtr - sizeof(FStompPayload_), FStompPayload_){ allocationSize, userOffset, size };
            ONLY_IF_ASSERT(StompGetPayload_(userPtr));

            Assert(userPtr);
            Assert(Meta::IsAligned(alignment, userPtr));
            return userPtr;
        }
    }
#endif

    // slow/safe path :
    void* const newPtr = FMallocStomp::AlignedMalloc(size, alignment);

    const FStompPayload_* payload = StompGetPayload_(ptr);
    FPlatformMemory::Memcpy(newPtr, ptr, Min(payload->UserSize, size));

    FMallocStomp::AlignedFree(ptr);

    return newPtr;
}
//----------------------------------------------------------------------------
size_t FMallocStomp::RegionSize(void* ptr) {
    Assert(ptr);
    return StompGetPayload_(ptr)->UserSize;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!WITH_PPE_MALLOCSTOMP
