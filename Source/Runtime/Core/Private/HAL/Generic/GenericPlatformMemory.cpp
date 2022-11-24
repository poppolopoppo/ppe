// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "HAL/Generic/GenericPlatformMemory.h"

#include "Memory/MemoryDomain.h"
#include "Memory/MemoryTracking.h"

#include <malloc.h>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// system allocator
//----------------------------------------------------------------------------
void* FGenericPlatformMemory::SystemMalloc(size_t s) {
    void* const result = ::malloc(s);

#if USE_PPE_MEMORYDOMAINS
    const size_t sys = ::_msize(result);
    MEMORYDOMAIN_TRACKING_DATA(SystemMalloc).Allocate(sys, sys);
#endif

    return result;
}
//----------------------------------------------------------------------------
NODISCARD void* FGenericPlatformMemory::SystemRealloc(void* p, size_t s) {
#if USE_PPE_MEMORYDOMAINS
    if (p) {
        const size_t sys = ::_msize(p);
        MEMORYDOMAIN_TRACKING_DATA(SystemMalloc).Deallocate(sys, sys);
    }
#endif

    void* const result = ::realloc(p, s);
    Assert(0 == s || !!result);

#if USE_PPE_MEMORYDOMAINS
    if (result) {
        const size_t sys = ::_msize(result);
        MEMORYDOMAIN_TRACKING_DATA(SystemMalloc).Allocate(sys, sys);
    }
#endif

    return result;
}
//----------------------------------------------------------------------------,
void FGenericPlatformMemory::SystemFree(void* p) {
    Assert(p);

#if USE_PPE_MEMORYDOMAINS
    const size_t sys = ::_msize(p);
    MEMORYDOMAIN_TRACKING_DATA(SystemMalloc).Deallocate(sys, sys);
#endif

    ::free(p);
}
//----------------------------------------------------------------------------
// system allocator aligned
//----------------------------------------------------------------------------
NODISCARD void* FGenericPlatformMemory::SystemAlignedMalloc(size_t s, size_t boundary) {
    void* const result = ::_aligned_malloc(s, boundary);
    Assert(result);

#if USE_PPE_MEMORYDOMAINS
    const size_t sys = ::_aligned_msize(result, boundary, 0);
    MEMORYDOMAIN_TRACKING_DATA(SystemMalloc).Allocate(sys, sys);
#endif

    return result;
}
//----------------------------------------------------------------------------
NODISCARD void* FGenericPlatformMemory::SystemAlignedRealloc(void* p, size_t s, size_t boundary) {
#if USE_PPE_MEMORYDOMAINS
    if (p) {
        const size_t sys = ::_aligned_msize(p, boundary, 0);
        MEMORYDOMAIN_TRACKING_DATA(SystemMalloc).Deallocate(sys, sys);
    }
#endif

    void* const result = ::_aligned_realloc(p, s, boundary);
    Assert(0 == s || !!result);

#if USE_PPE_MEMORYDOMAINS
    if (result) {
        const size_t sys = ::_aligned_msize(result, boundary, 0);
        MEMORYDOMAIN_TRACKING_DATA(SystemMalloc).Allocate(sys, sys);
    }
#endif

    return result;
}
//----------------------------------------------------------------------------
void FGenericPlatformMemory::SystemAlignedFree(void* p, size_t boundary) {
    Assert(p);

#if USE_PPE_MEMORYDOMAINS
    const size_t sys = ::_aligned_msize(p, boundary, 0);
    MEMORYDOMAIN_TRACKING_DATA(SystemMalloc).Deallocate(sys, sys);
#else
    Unused(boundary);
#endif

    ::_aligned_free(p);
}
//----------------------------------------------------------------------------
#if !USE_PPE_FINAL_RELEASE
size_t FGenericPlatformMemory::SystemAlignedRegionSize(void* p, size_t boundary) {
    Assert(p);
    return ::_aligned_msize(p, boundary, 0);
}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE