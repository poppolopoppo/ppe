#include "stdafx.h"

#include "TrackingMalloc.h"

#include "Malloc.h"

#include "Allocator/ThreadLocalHeap.h"
#include "Memory/MemoryDomain.h"
#include "Memory/MemoryTracking.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
#ifdef USE_MEMORY_DOMAINS
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
};
STATIC_ASSERT(sizeof(FBlockTracking_) == ALLOCATION_BOUNDARY);
#endif //!USE_MEMORY_DOMAINS
//----------------------------------------------------------------------------
template <typename _Malloc>
static void* tracking_malloc_(FMemoryTracking& trackingData, size_t size, _Malloc mallocF) {
#ifdef USE_MEMORY_DOMAINS
    if (0 == size)
        return nullptr;

    trackingData.Allocate(1, size);
    void* const ptr = mallocF(size + sizeof(FBlockTracking_));

    auto* const pblock = reinterpret_cast<FBlockTracking_*>(ptr);
    pblock->TrackingData = &trackingData;
    pblock->SizeInBytes = checked_cast<u32>(size);
    pblock->Canary = FBlockTracking_::CanaryValue;

    return (pblock + 1);
#else
    return mallocF(size);
#endif
}
//----------------------------------------------------------------------------
template <typename _Free>
static void tracking_free_(FMemoryTracking& trackingData, void *ptr, _Free freeF) {
#ifdef USE_MEMORY_DOMAINS
    if (nullptr == ptr)
        return;

    auto* const pblock = reinterpret_cast<FBlockTracking_*>(ptr) - 1;
    Assert(&trackingData == pblock->TrackingData);
    Assert(FBlockTracking_::CanaryValue == pblock->CanaryValue);

    trackingData.Deallocate(1, pblock->SizeInBytes);
    freeF(pblock);
#else
    freeF(ptr);
#endif
}
//----------------------------------------------------------------------------
template <typename _Calloc>
static void* tracking_calloc_(FMemoryTracking& trackingData, size_t nmemb, size_t size, _Calloc callocF) {
#ifdef USE_MEMORY_DOMAINS
    void* ptr = tracking_malloc_(trackingData, nmemb * size, [callocF](size_t sz) {
        return callocF(sz, 1);
    });
    ::memset(ptr, 0, nmemb * size);
    return ptr;
#else
    return callocF(nmemb, size);
#endif
}
//----------------------------------------------------------------------------
template <typename _Realloc>
static void* tracking_realloc_(FMemoryTracking& trackingData, void *ptr, size_t size, _Realloc reallocF) {
#ifdef USE_MEMORY_DOMAINS
    FBlockTracking_* pblock = nullptr;
    if (nullptr != ptr) {
        pblock = reinterpret_cast<FBlockTracking_*>(ptr) - 1;
        Assert(&trackingData == pblock->TrackingData);
        Assert(FBlockTracking_::CanaryValue == pblock->CanaryValue);

        trackingData.Deallocate(1, pblock->SizeInBytes);
    }

    if (0 == size && pblock) {
        void* p = reallocF(pblock, 0); // fake free() with realloc(0)
        Assert(nullptr == p); // memory need to be freed !
        return nullptr;
    }
    else {
        trackingData.Allocate(1, size);
        ptr = reallocF(pblock, size + sizeof(FBlockTracking_));

        pblock = reinterpret_cast<FBlockTracking_*>(ptr);
        pblock->TrackingData = &trackingData;
        pblock->SizeInBytes = checked_cast<u32>(size);
        pblock->Canary = FBlockTracking_::CanaryValue;

        return (pblock + 1);
    }
#else
    return reallocF(ptr, size);
#endif
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void* (malloc)(FMemoryTracking& trackingData, size_t size) {
    return tracking_malloc_(trackingData, size, [](size_t sz) { return Core::malloc(sz); });
}
//----------------------------------------------------------------------------
void  (free)(FMemoryTracking& trackingData, void *ptr) {
    tracking_free_(trackingData, ptr, [](void* p) { Core::free(p); });
}
//----------------------------------------------------------------------------
void* (calloc)(FMemoryTracking& trackingData, size_t nmemb, size_t size) {
    return tracking_calloc_(trackingData, nmemb, size, [](size_t n, size_t s) { return Core::calloc(n, s); });
}
//----------------------------------------------------------------------------
void* (realloc)(FMemoryTracking& trackingData, void *ptr, size_t size) {
    return tracking_realloc_(trackingData, ptr, size, [](void* p, size_t s) { return Core::realloc(p, s); });
}
//----------------------------------------------------------------------------
void* (malloc_thread_local)(FMemoryTracking& trackingData, size_t size) {
    return tracking_malloc_(trackingData, size, [](size_t sz) { return GetThreadLocalHeap().Malloc(sz); });
}
//----------------------------------------------------------------------------
void  (free_thread_local)(FMemoryTracking& trackingData, void *ptr) {
    tracking_free_(trackingData, ptr, [](void* p) { GetThreadLocalHeap().Free(p); });
}
//----------------------------------------------------------------------------
void* (calloc_thread_local)(FMemoryTracking& trackingData, size_t nmemb, size_t size) {
    return tracking_calloc_(trackingData, nmemb, size, [](size_t n, size_t s) { return GetThreadLocalHeap().Calloc(n, s); });
}
//----------------------------------------------------------------------------
void* (realloc_thread_local)(FMemoryTracking& trackingData, void *ptr, size_t size) {
    return tracking_realloc_(trackingData, ptr, size, [](void* p, size_t s) { return GetThreadLocalHeap().Realloc(p, s); });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
