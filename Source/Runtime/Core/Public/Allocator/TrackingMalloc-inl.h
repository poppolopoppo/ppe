#pragma once

#include "Allocator/TrackingMalloc.h"
#include "Memory/MemoryDomain.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _MemoryDomain>
inline void* tracking_malloc(size_t size) {
#if USE_PPE_MEMORYDOMAINS
    return (tracking_malloc)(_MemoryDomain::TrackingData(), size);
#else
    return (malloc)(size);
#endif
}
//----------------------------------------------------------------------------
template <typename _MemoryDomain>
void* tracking_calloc(size_t nmemb, size_t size) {
#if USE_PPE_MEMORYDOMAINS
    return (tracking_calloc)(_MemoryDomain::TrackingData(), nmemb, size);
#else
    return (calloc)(nmemb, size);
#endif
}
//----------------------------------------------------------------------------
template <typename _MemoryDomain>
void* tracking_realloc(void *ptr, size_t size) {
#if USE_PPE_MEMORYDOMAINS
    return (tracking_realloc)(_MemoryDomain::TrackingData(), ptr, size);
#else
    return (realloc)(ptr, size);
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
