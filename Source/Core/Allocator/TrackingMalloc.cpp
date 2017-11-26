#include "stdafx.h"

#include "TrackingMalloc.h"

#include "Malloc.h"

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
STATIC_ASSERT(sizeof(FBlockTracking_) == 16);
#endif
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void* (malloc)(FMemoryTracking& trackingData, size_t size) {
#ifdef USE_MEMORY_DOMAINS
    if (0 == size)
        return nullptr;

    trackingData.Allocate(1, size);
    void* const ptr = malloc(size + sizeof(FBlockTracking_));

    auto* const pblock = reinterpret_cast<FBlockTracking_*>(ptr);
    pblock->TrackingData = &trackingData;
    pblock->SizeInBytes = checked_cast<u32>(size);
    pblock->Canary = FBlockTracking_::CanaryValue;

    return (pblock+1);
#else
    return malloc(size);
#endif
}
//----------------------------------------------------------------------------
void (free)(FMemoryTracking& trackingData, void *ptr) {
#ifdef USE_MEMORY_DOMAINS
    if (nullptr == ptr)
        return;

    auto* const pblock = reinterpret_cast<FBlockTracking_*>(ptr) - 1;
    Assert(&trackingData == pblock->TrackingData);
    Assert(FBlockTracking_::CanaryValue == pblock->CanaryValue);

    trackingData.Deallocate(1, pblock->SizeInBytes);
    free(pblock);
#else
    free(ptr);
#endif
}
//----------------------------------------------------------------------------
void* (calloc)(FMemoryTracking& trackingData, size_t nmemb, size_t size) {
#ifdef USE_MEMORY_DOMAINS
    const size_t sizeInBytes = nmemb*size;
    void* const ptr = (malloc)(trackingData, sizeInBytes);
    ::memset(ptr, 0, sizeInBytes);
    return ptr;
#else
    return Core::malloc(size);
#endif
}
//----------------------------------------------------------------------------
void* (realloc)(FMemoryTracking& trackingData, void *ptr, size_t size) {
#ifdef USE_MEMORY_DOMAINS
    if (nullptr != ptr) {
        auto* const pblock = reinterpret_cast<FBlockTracking_*>(ptr) - 1;
        Assert(&trackingData == pblock->TrackingData);
        Assert(FBlockTracking_::CanaryValue == pblock->CanaryValue);

        trackingData.Deallocate(1, pblock->SizeInBytes);
    }

    if (0 == size) {
        free(ptr);
        return nullptr;
    }
    else {
        trackingData.Allocate(1, size);
        ptr = realloc(ptr, size + sizeof(FBlockTracking_));

        auto* const pblock = reinterpret_cast<FBlockTracking_*>(ptr);
        pblock->TrackingData = &trackingData;
        pblock->SizeInBytes = checked_cast<u32>(size);
        pblock->Canary = FBlockTracking_::CanaryValue;

        return (pblock+1);
    }
#else
    return realloc(ptr, size);
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
