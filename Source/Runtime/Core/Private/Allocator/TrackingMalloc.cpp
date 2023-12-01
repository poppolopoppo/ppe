// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Allocator/TrackingMalloc.h"

#include "Allocator/Malloc.h"
#include "Container/Hash.h"
#include "Memory/MemoryDomain.h"
#include "Memory/MemoryTracking.h"

#include "HAL/PlatformMemory.h"

#define USE_PPE_TRACKINGMALLOC USE_PPE_MEMORYDOMAINS

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
#if USE_PPE_TRACKINGMALLOC
struct FBlockTracking_ {
    FMemoryTracking* TrackingData;
    u32 UserSize;
    // adapt canary size to preserve alignment on 16 :
#ifdef ARCH_X86
    using canary_type = u64;
    STATIC_CONST_INTEGRAL(u64, CanarySeed, 0xBADC0FFEE0DDF00Dull);
#else
    using canary_type = u32;
    STATIC_CONST_INTEGRAL(u32, CanarySeed, 0xABADCAFEul);
#endif

    canary_type Canary;

    canary_type MakeCanary() const {
        return static_cast<canary_type>(hash_tuple(
            hash_ptr(this),
            hash_ptr(TrackingData),
            UserSize,
            CanarySeed));
    }

    bool CheckCanary() const { return (MakeCanary() == Canary); }

    static CONSTEXPR CONSTF size_t UserSizeFromSystem(size_t systemSize) {
        Assert(systemSize > sizeof(FBlockTracking_));
        return systemSize - sizeof(FBlockTracking_);
    }
    static CONSTEXPR CONSTF size_t SystemSizeFromUser(size_t userSize) {
        return userSize + sizeof(FBlockTracking_);
    }

    static FBlockTracking_* BlockFromPtr(void* ptr) NOEXCEPT {
        return (reinterpret_cast<FBlockTracking_*>(ptr) - 1);
    }

    static void* MakeAlloc(FMemoryTracking& tracking, void* ptr, size_t userSize) NOEXCEPT {
        auto* const pblock = reinterpret_cast<FBlockTracking_*>(ptr);
        pblock->TrackingData = &tracking;
        pblock->UserSize = checked_cast<u32>(userSize);
        pblock->Canary = pblock->MakeCanary();

        pblock->TrackingData->Allocate(pblock->UserSize, FMallocDebug::RegionSize(pblock));

        return (pblock + 1);
    }

    static FBlockTracking_* ReleaseAlloc(void* ptr) {
        FBlockTracking_* const pblock = BlockFromPtr(ptr);
        Assert_NoAssume(pblock->CheckCanary());

        pblock->TrackingData->Deallocate(pblock->UserSize, FMallocDebug::RegionSize(pblock));

        return pblock;
    }
};
STATIC_ASSERT(sizeof(FBlockTracking_) == ALLOCATION_BOUNDARY);
#endif //!USE_PPE_TRACKINGMALLOC
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void* (tracking_malloc)(FMemoryTracking& trackingData, size_t size) {
#if USE_PPE_TRACKINGMALLOC
    if (0 == size)
        return nullptr;

    const FMemoryTracking::FThreadScope threadTracking{ trackingData };

    void* const pblock = PPE::malloc(FBlockTracking_::SystemSizeFromUser(size));

    return FBlockTracking_::MakeAlloc(trackingData, pblock, size);
#else
    Unused(trackingData);
    return PPE::malloc(size);
#endif
}
//----------------------------------------------------------------------------
FAllocatorBlock (tracking_malloc_for_new)(FMemoryTracking& trackingData, size_t size) {
#if USE_PPE_TRACKINGMALLOC
    if (0 == size)
        return Default;

    const FMemoryTracking::FThreadScope threadTracking{ trackingData };

    FAllocatorBlock blk = PPE::malloc_for_new(FBlockTracking_::SystemSizeFromUser(size));

    blk.Data = FBlockTracking_::MakeAlloc(trackingData, blk.Data, size);
    blk.SizeInBytes = FBlockTracking_::UserSizeFromSystem(blk.SizeInBytes);

    return blk;
#else
    Unused(trackingData);
    return PPE::malloc_for_new(size);
#endif
}
//----------------------------------------------------------------------------
void  (tracking_free)(void *ptr) NOEXCEPT {
#if USE_PPE_TRACKINGMALLOC
    if (nullptr == ptr)
        return;

    FBlockTracking_* const pblock = FBlockTracking_::ReleaseAlloc(ptr);

    const FMemoryTracking::FThreadScope threadTracking{ *pblock->TrackingData };

    PPE::free(pblock);
#else
    PPE::free(ptr);
#endif
}
//----------------------------------------------------------------------------
void  (tracking_free_for_delete)(FAllocatorBlock blk) NOEXCEPT {
#if USE_PPE_TRACKINGMALLOC
    if (!blk) {
        Assert_NoAssume(0 == blk.SizeInBytes);
        return;
    }

    FBlockTracking_* const pblock = FBlockTracking_::ReleaseAlloc(blk.Data);
    Assert_NoAssume(blk.SizeInBytes >= pblock->UserSize);

    const FMemoryTracking::FThreadScope threadTracking{ *pblock->TrackingData };

    PPE::free_for_delete(FAllocatorBlock(pblock, FBlockTracking_::SystemSizeFromUser(blk.SizeInBytes)));
#else
    PPE::free_for_delete(blk);
#endif
}
//----------------------------------------------------------------------------
void* (tracking_calloc)(FMemoryTracking& trackingData, size_t nmemb, size_t size) {
    void* const ptr = tracking_malloc(trackingData, nmemb * size);

    FPlatformMemory::Memzero(ptr, nmemb * size);

    return ptr;
}
//----------------------------------------------------------------------------
void* (tracking_realloc)(FMemoryTracking& trackingData, void* ptr, size_t size) {
#if USE_PPE_TRACKINGMALLOC
    const FMemoryTracking::FThreadScope threadTracking{ trackingData };

    FBlockTracking_* pblock = nullptr;
    if (!!ptr) {
        pblock = FBlockTracking_::ReleaseAlloc(ptr);
        Assert_NoAssume(&trackingData == pblock->TrackingData);
    }

    void* const nblock = PPE::realloc(pblock, FBlockTracking_::SystemSizeFromUser(size));
    if (Likely(!!nblock))
        return FBlockTracking_::MakeAlloc(trackingData, nblock, size);

    Assert_NoAssume(size == 0);
    return nullptr;
#else
    Unused(trackingData);
    return PPE::realloc(ptr, size);
#endif
}
//----------------------------------------------------------------------------
FAllocatorBlock (tracking_realloc_for_new)(FMemoryTracking& trackingData, FAllocatorBlock blk, size_t size) {
#if USE_PPE_TRACKINGMALLOC
    const FMemoryTracking::FThreadScope threadTracking{ trackingData };

    FBlockTracking_* pblock = nullptr;
    if (!!blk) {
        Assert_NoAssume(blk.SizeInBytes > 0);
        pblock = FBlockTracking_::ReleaseAlloc(blk.Data);
        Assert_NoAssume(&trackingData == pblock->TrackingData);
        Assert_NoAssume(pblock->UserSize == blk.SizeInBytes);
    }

    FAllocatorBlock nblk = PPE::realloc_for_new(FAllocatorBlock(pblock, FBlockTracking_::SystemSizeFromUser(blk.SizeInBytes)), FBlockTracking_::SystemSizeFromUser(size));

    if (Likely(!!nblk)) {
        nblk.Data = FBlockTracking_::MakeAlloc(trackingData, nblk.Data, size);
        nblk.SizeInBytes = FBlockTracking_::UserSizeFromSystem(nblk.SizeInBytes);
        return nblk;
    }

    Assert_NoAssume(size == 0);
    return Default;
#else
    Unused(trackingData);
    return PPE::realloc_for_new(blk, size);
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
