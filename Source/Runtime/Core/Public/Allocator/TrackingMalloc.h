#pragma once

#include "Core_fwd.h"

#include "Allocator/AllocatorBlock.h"
#include "Allocator/Malloc.h"
#include "Memory/MemoryDomain.h"

#define TRACKING_MALLOC(_DOMAIN, _SIZE) ::PPE::tracking_malloc<MEMORYDOMAIN_TAG(_DOMAIN)>(_SIZE)
#define TRACKING_ALIGNED_MALLOC(_DOMAIN, _SIZE, _ALIGN) ::PPE::tracking_aligned_malloc<MEMORYDOMAIN_TAG(_DOMAIN)>((_SIZE), (_ALIGN))
#define TRACKING_CALLOC(_DOMAIN, _NMEMB, _SIZE) ::PPE::tracking_calloc<MEMORYDOMAIN_TAG(_DOMAIN)>((_NMEMB), (_SIZE))
#define TRACKING_REALLOC(_DOMAIN, _PTR, _SIZE) ::PPE::tracking_realloc<MEMORYDOMAIN_TAG(_DOMAIN)>((_PTR), (_SIZE))
#define TRACKING_MALLOC_FOR_NEW(_DOMAIN, _SIZE) ::PPE::tracking_malloc_for_new<MEMORYDOMAIN_TAG(_DOMAIN)>(_SIZE)
#define TRACKING_REALLOC_FOR_NEW(_DOMAIN, _BLK, _SIZE) ::PPE::tracking_realloc_for_new<MEMORYDOMAIN_TAG(_DOMAIN)>(_BLK, _SIZE)
#define TRACKING_FREE(_DOMAIN, _PTR) ::PPE::tracking_free(_PTR)
#define TRACKING_FREE_FOR_DELETE(_DOMAIN, _BLK) ::PPE::tracking_free_for_delete(_BLK)

#define TRACKING_NEW(_DOMAIN, ...) INPLACE_NEW(::PPE::tracking_new<MEMORYDOMAIN_TAG(_DOMAIN) COMMA __VA_ARGS__>(), __VA_ARGS__)
#define TRACKING_DELETE(_DOMAIN, _PTR) ::PPE::tracking_delete(_PTR)

namespace PPE {
class FMemoryTracking;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API void* (tracking_malloc)(FMemoryTracking& trackingData, size_t size);
//----------------------------------------------------------------------------
PPE_CORE_API void  (tracking_free)(void *ptr) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API void* (tracking_calloc)(FMemoryTracking& trackingData, size_t nmemb, size_t size);
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API void* (tracking_realloc)(FMemoryTracking& trackingData, void *ptr, size_t size);
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API FAllocatorBlock (tracking_malloc_for_new)(FMemoryTracking& trackingData, size_t size);
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API FAllocatorBlock (tracking_realloc_for_new)(FMemoryTracking& trackingData, FAllocatorBlock blk, size_t size);
//----------------------------------------------------------------------------
PPE_CORE_API void (tracking_free_for_delete)(FAllocatorBlock blk) NOEXCEPT;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
NODISCARD inline void* (tracking_aligned_malloc)(FMemoryTracking& trackingData, size_t size, size_t alignment) {
    Unused(alignment);
    void* const p = (tracking_malloc)(trackingData, size);
    Assert_NoAssume(Meta::IsAlignedPow2(alignment, p));
    return p;
}
//----------------------------------------------------------------------------
NODISCARD inline void* (tracking_aligned_realloc)(FMemoryTracking& trackingData, void* ptr, size_t size, size_t alignment) {
    Unused(alignment);
    void* const p = (tracking_realloc)(trackingData, ptr, size);
    Assert_NoAssume(Meta::IsAlignedPow2(alignment, p));
    return p;
}
//----------------------------------------------------------------------------
inline void (tracking_aligned_free)(void* ptr) {
    (tracking_free)(ptr);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _MemoryDomain>
NODISCARD void* tracking_malloc(size_t size) {
#if USE_PPE_MEMORYDOMAINS
    return (tracking_malloc)(_MemoryDomain::TrackingData(), size);
#else
    return (PPE::malloc)(size);
#endif
}
//----------------------------------------------------------------------------
template <typename _MemoryDomain>
NODISCARD FAllocatorBlock tracking_malloc_for_new(size_t size) {
#if USE_PPE_MEMORYDOMAINS
    return (tracking_malloc_for_new)(_MemoryDomain::TrackingData(), size);
#else
    return (PPE::malloc_for_new)(size);
#endif
}
//----------------------------------------------------------------------------
template <typename _MemoryDomain>
NODISCARD void* tracking_aligned_malloc(size_t size, size_t alignment) {
#if USE_PPE_MEMORYDOMAINS
    return (tracking_aligned_malloc)(_MemoryDomain::TrackingData(), size, alignment);
#else
    Unused(alignment); // assume always naturally aligned with target
    void* const p = (PPE::malloc)(size);
    Assert_NoAssume(Meta::IsAlignedPow2(alignment, p));
    return p;
#endif
}
//----------------------------------------------------------------------------
template <typename _MemoryDomain>
NODISCARD void* tracking_calloc(size_t nmemb, size_t size) {
#if USE_PPE_MEMORYDOMAINS
    return (tracking_calloc)(_MemoryDomain::TrackingData(), nmemb, size);
#else
    return (PPE::calloc)(nmemb, size);
#endif
}
//----------------------------------------------------------------------------
template <typename _MemoryDomain>
NODISCARD void* tracking_realloc(void *ptr, size_t size) {
#if USE_PPE_MEMORYDOMAINS
    return (tracking_realloc)(_MemoryDomain::TrackingData(), ptr, size);
#else
    return (PPE::realloc)(ptr, size);
#endif
}
//----------------------------------------------------------------------------
template <typename _MemoryDomain>
NODISCARD FAllocatorBlock tracking_realloc_for_new(FAllocatorBlock blk, size_t size) {
#if USE_PPE_MEMORYDOMAINS
    return (tracking_realloc_for_new)(_MemoryDomain::TrackingData(), blk, size);
#else
    return (PPE::realloc_for_new)(blk, size);
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _MemoryDomain, typename T>
NODISCARD void* tracking_new() {
    return tracking_malloc<_MemoryDomain>(sizeof(T));
}
//----------------------------------------------------------------------------
template <typename T>
void tracking_delete(T* ptr) NOEXCEPT {
    Assert(ptr);
    Meta::Destroy(ptr);
    tracking_free(ptr);
}
//----------------------------------------------------------------------------
template <typename T>
struct TTrackingDeleter {
    void operator ()(T* ptr) NOEXCEPT {
        tracking_delete<T>(ptr);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
