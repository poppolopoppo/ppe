#pragma once

#include "Core/Allocator/Heap.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE
void* Heap::malloc(size_t size, MemoryTrackingData& trackingData, typename std::enable_if< IsNaturalyAligned<_Alignment>::value >::type*) {
    void* const ptr = this->malloc(size, trackingData);
    Assert(IS_ALIGNED(_Alignment, ptr));
    return ptr;
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE
void Heap::free(void *ptr, MemoryTrackingData& trackingData, typename std::enable_if< IsNaturalyAligned<_Alignment>::value >::type*) {
    this->free(ptr, trackingData);
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE
void* Heap::calloc(size_t nmemb, size_t size, MemoryTrackingData& trackingData, typename std::enable_if< IsNaturalyAligned<_Alignment>::value >::type*) {
    void* const ptr = this->calloc(nmemb, size, trackingData);
    Assert(IS_ALIGNED(_Alignment, ptr));
    return ptr;
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE
void* Heap::realloc(void *ptr, size_t size, MemoryTrackingData& trackingData, typename std::enable_if< IsNaturalyAligned<_Alignment>::value >::type*) {
    void* const newp = this->realloc(ptr, size, trackingData);
    Assert(IS_ALIGNED(_Alignment, newp));
    return newp;
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE
void* Heap::malloc(size_t size, MemoryTrackingData& trackingData, typename std::enable_if< !IsNaturalyAligned<_Alignment>::value >::type*) {
    return this->aligned_malloc(size, _Alignment, trackingData);
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE
void Heap::free(void *ptr, MemoryTrackingData& trackingData, typename std::enable_if< !IsNaturalyAligned<_Alignment>::value >::type*) {
    this->aligned_free(ptr, trackingData);
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE
void* Heap::calloc(size_t nmemb, size_t size, MemoryTrackingData& trackingData, typename std::enable_if< !IsNaturalyAligned<_Alignment>::value >::type*) {
    return this->aligned_calloc(nmemb, size, _Alignment, trackingData);
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE
void* Heap::realloc(void *ptr, size_t size, MemoryTrackingData& trackingData, typename std::enable_if< !IsNaturalyAligned<_Alignment>::value >::type*) {
    return this->aligned_realloc(ptr, size, trackingData);
}
//----------------------------------------------------------------------------
inline void Heap::Swap(Heap& other) {
    std::swap(other._handle, _handle);
}
//----------------------------------------------------------------------------
inline void swap(Heap& lhs, Heap& rhs) {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
