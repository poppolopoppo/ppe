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
    u32 SizeInBytes;
    // adapt canary size to preserve alignment on 16 :
#ifdef ARCH_X86
    u64 Canary;
    STATIC_CONST_INTEGRAL(u64, CanaryValue, 0xBADC0FFEE0DDF00Dull);
#else
    u32 Canary;
    STATIC_CONST_INTEGRAL(u32, CanaryValue, 0xABADCAFEul);
#endif
    bool CheckCanary() const { return (CanaryValue == Canary); }

    static void* MakeAlloc(FMemoryTracking& tracking, void* ptr, size_t size) NOEXCEPT {
        tracking.Allocate(size, malloc_snap_size(size)/* don't account for domain tracking, tracked by reserved memory instead */);

        auto * const pblock = reinterpret_cast<FBlockTracking_*>(ptr);
        pblock->TrackingData = &tracking;
        pblock->SizeInBytes = checked_cast<u32>(size);
        pblock->Canary = FBlockTracking_::CanaryValue;

        return (pblock + 1);
    }

    static void* ReleaseAlloc(void* ptr) {
        auto* const pblock = (reinterpret_cast<FBlockTracking_*>(ptr) - 1);
        Assert(pblock->CheckCanary());

        pblock->TrackingData->Deallocate(pblock->SizeInBytes, malloc_snap_size(pblock->SizeInBytes));

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
    if (0 == size)
        return nullptr;

    void* const ptr = PPE::malloc(size + sizeof(FBlockTracking_));

    return FBlockTracking_::MakeAlloc(trackingData, ptr, size);
#else
    UNUSED(trackingData);
    return PPE::malloc(size);
#endif
}
//----------------------------------------------------------------------------
void  (tracking_free)(void *ptr) NOEXCEPT {
#if USE_PPE_MEMORYDOMAINS
    if (nullptr == ptr)
        return;

    void* const pblock = FBlockTracking_::ReleaseAlloc(ptr);

    PPE::free(pblock);
#else
    PPE::free(ptr);
#endif
}
//----------------------------------------------------------------------------
void* (tracking_calloc)(FMemoryTracking& trackingData, size_t nmemb, size_t size) {
    void* ptr = tracking_malloc(trackingData, nmemb * size);
    FPlatformMemory::Memzero(ptr, nmemb * size);
    return ptr;
}
//----------------------------------------------------------------------------
void* (tracking_realloc)(FMemoryTracking& trackingData, void *ptr, size_t size) {
#if USE_PPE_MEMORYDOMAINS
    FBlockTracking_* pblock = nullptr;
    if (nullptr != ptr) {
        pblock = reinterpret_cast<FBlockTracking_*>(ptr) - 1;
        Assert(pblock->CheckCanary());
        Assert(&trackingData == pblock->TrackingData);

        trackingData.Deallocate(pblock->SizeInBytes, malloc_snap_size(pblock->SizeInBytes));
    }

    if (0 == size && pblock) {
        void* p = PPE::realloc(pblock, 0); // fake free() with realloc(0)
        Assert(nullptr == p); // memory need to be freed !
        return nullptr;
    }
    else {
        trackingData.Allocate(size, malloc_snap_size(size));
        ptr = PPE::realloc(pblock, size + sizeof(FBlockTracking_));

        pblock = reinterpret_cast<FBlockTracking_*>(ptr);
        pblock->TrackingData = &trackingData;
        pblock->SizeInBytes = checked_cast<u32>(size);
        pblock->Canary = FBlockTracking_::CanaryValue;

        return (pblock + 1);
    }
#else
    UNUSED(trackingData);
    return PPE::realloc(ptr, size);
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
