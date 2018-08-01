#include "stdafx.h"

#include "TrackingMalloc.h"

#include "Malloc.h"

#include "HAL/PlatformMemory.h"
#include "Memory/MemoryDomain.h"
#include "Memory/MemoryTracking.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
#if USE_CORE_MEMORYDOMAINS
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
};
STATIC_ASSERT(sizeof(FBlockTracking_) == ALLOCATION_BOUNDARY);
#endif //!USE_CORE_MEMORYDOMAINS
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void* (tracking_malloc)(FMemoryTracking& trackingData, size_t size) {
#if USE_CORE_MEMORYDOMAINS
    if (0 == size)
        return nullptr;

    trackingData.Allocate(1, size);
    void* const ptr = Core::malloc(size + sizeof(FBlockTracking_));

    auto* const pblock = reinterpret_cast<FBlockTracking_*>(ptr);
    pblock->TrackingData = &trackingData;
    pblock->SizeInBytes = checked_cast<u32>(size);
    pblock->Canary = FBlockTracking_::CanaryValue;

    return (pblock + 1);
#else
    NOOP(trackingData);
    return Core::malloc(size);
#endif
}
//----------------------------------------------------------------------------
void  (tracking_free)(void *ptr) {
#if USE_CORE_MEMORYDOMAINS
    if (nullptr == ptr)
        return;

    auto* const pblock = (reinterpret_cast<FBlockTracking_*>(ptr) - 1);
    Assert(pblock->CheckCanary());

    pblock->TrackingData->Deallocate(1, pblock->SizeInBytes);
    Core::free(pblock);
#else
    Core::free(ptr);
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
#if USE_CORE_MEMORYDOMAINS
    FBlockTracking_* pblock = nullptr;
    if (nullptr != ptr) {
        pblock = reinterpret_cast<FBlockTracking_*>(ptr) - 1;
        Assert(pblock->CheckCanary());
        Assert(&trackingData == pblock->TrackingData);

        trackingData.Deallocate(1, pblock->SizeInBytes);
    }

    if (0 == size && pblock) {
        void* p = Core::realloc(pblock, 0); // fake free() with realloc(0)
        Assert(nullptr == p); // memory need to be freed !
        return nullptr;
    }
    else {
        trackingData.Allocate(1, size);
        ptr = Core::realloc(pblock, size + sizeof(FBlockTracking_));

        pblock = reinterpret_cast<FBlockTracking_*>(ptr);
        pblock->TrackingData = &trackingData;
        pblock->SizeInBytes = checked_cast<u32>(size);
        pblock->Canary = FBlockTracking_::CanaryValue;

        return (pblock + 1);
    }
#else
    NOOP(trackingData);
    return Core::realloc(ptr, size);
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
