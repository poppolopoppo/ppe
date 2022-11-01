#include "stdafx.h"

#include "Allocator/TrackingMalloc.h"

#include "Allocator/Malloc.h"

#include "HAL/PlatformMemory.h"
#include "Memory/MemoryDomain.h"
#include "Memory/MemoryTracking.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
#if USE_PPE_MEMORYDOMAINS
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

    static CONSTEXPR CONSTF size_t SystemSize(size_t userSize) {
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
#endif //!USE_PPE_MEMORYDOMAINS
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void* (tracking_malloc)(FMemoryTracking& trackingData, size_t size) {
#if USE_PPE_MEMORYDOMAINS
    return tracking_malloc_for_new(trackingData, size);
#else
    Unused(trackingData);
    return PPE::malloc(size);
#endif
}
//----------------------------------------------------------------------------
void* (tracking_malloc_for_new)(FMemoryTracking& trackingData, size_t size) {
#if USE_PPE_MEMORYDOMAINS
    if (0 == size)
        return nullptr;

    const FMemoryTracking::FThreadScope threadTracking{ trackingData };

    void* const ptr = PPE::malloc_for_new(FBlockTracking_::SystemSize(size));

    return FBlockTracking_::MakeAlloc(trackingData, ptr, size);
#else
    Unused(trackingData);
    return PPE::malloc_for_new(size);
#endif
}
//----------------------------------------------------------------------------
void  (tracking_free)(void *ptr) NOEXCEPT {
#if USE_PPE_MEMORYDOMAINS
    if (nullptr == ptr)
        return;

    FBlockTracking_* const pblock = FBlockTracking_::BlockFromPtr(ptr);
    return tracking_free_for_delete(ptr, pblock->UserSize);
#else
    PPE::free(ptr);
#endif
}
//----------------------------------------------------------------------------
void  (tracking_free_for_delete)(void* ptr, size_t size) NOEXCEPT {
#if USE_PPE_MEMORYDOMAINS
    if (nullptr == ptr) {
        Assert_NoAssume(0 == size);
        return;
    }

    FBlockTracking_* const pblock = FBlockTracking_::ReleaseAlloc(ptr);
    Assert_NoAssume(size == pblock->UserSize);

    const FMemoryTracking::FThreadScope threadTracking{ *pblock->TrackingData };

    PPE::free_for_delete(pblock, FBlockTracking_::SystemSize(size));
#else
    PPE::free_for_delete(ptr, size);
#endif
}
//----------------------------------------------------------------------------
void* (tracking_calloc)(FMemoryTracking& trackingData, size_t nmemb, size_t size) {
    void* ptr = tracking_malloc(trackingData, nmemb * size);
    FPlatformMemory::Memzero(ptr, nmemb * size);
    return ptr;
}
//----------------------------------------------------------------------------
void* (tracking_realloc)(FMemoryTracking& trackingData, void* ptr, size_t size) {
#if USE_PPE_MEMORYDOMAINS
    if (nullptr != ptr) {
        FBlockTracking_* const pblock = FBlockTracking_::BlockFromPtr(ptr);
        return tracking_realloc_for_new(trackingData, ptr, size, pblock->UserSize);
    }
    else {
        return tracking_realloc_for_new(trackingData, ptr, size, 0);
    }
#else
    Unused(trackingData);
    return PPE::realloc(ptr, size);
#endif
}
//----------------------------------------------------------------------------
void* (tracking_realloc_for_new)(FMemoryTracking& trackingData, void* ptr, size_t size, size_t old) {
#if USE_PPE_MEMORYDOMAINS
    const FMemoryTracking::FThreadScope threadTracking{ trackingData };

    FBlockTracking_* pblock = nullptr;
    if (nullptr != ptr) {
        Assert_NoAssume(old > 0);
        pblock = FBlockTracking_::ReleaseAlloc(ptr);
        Assert_NoAssume(&trackingData == pblock->TrackingData);
        Assert_NoAssume(pblock->UserSize == old);
    }

    void* const newp = PPE::realloc_for_new(pblock, FBlockTracking_::SystemSize(size), FBlockTracking_::SystemSize(old));

    if (newp)
        return FBlockTracking_::MakeAlloc(trackingData, newp, size);

    Assert_NoAssume(size == 0);
    return nullptr;
#else
    Unused(trackingData);
    return PPE::realloc_for_new(ptr, size, old);
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
