#pragma once

#include "Core/Allocator/Heap.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE
void* Heap::Malloc(size_t size, MemoryTrackingData& trackingData, typename std::enable_if< IsNaturalyAligned<_Alignment>::value >::type*) {
    void* const ptr = this->Malloc(size, trackingData);
    Assert(IS_ALIGNED(_Alignment, ptr));
    return ptr;
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE
void Heap::Free(void *ptr, MemoryTrackingData& trackingData, typename std::enable_if< IsNaturalyAligned<_Alignment>::value >::type*) {
    this->Free(ptr, trackingData);
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE
void* Heap::Calloc(size_t nmemb, size_t size, MemoryTrackingData& trackingData, typename std::enable_if< IsNaturalyAligned<_Alignment>::value >::type*) {
    void* const ptr = this->Calloc(nmemb, size, trackingData);
    Assert(IS_ALIGNED(_Alignment, ptr));
    return ptr;
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE
void* Heap::Realloc(void *ptr, size_t size, MemoryTrackingData& trackingData, typename std::enable_if< IsNaturalyAligned<_Alignment>::value >::type*) {
    void* const newp = this->Realloc(ptr, size, trackingData);
    Assert(IS_ALIGNED(_Alignment, newp));
    return newp;
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE
void* Heap::Malloc(size_t size, MemoryTrackingData& trackingData, typename std::enable_if< !IsNaturalyAligned<_Alignment>::value >::type*) {
    return this->AlignedMalloc(size, _Alignment, trackingData);
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE
void Heap::Free(void *ptr, MemoryTrackingData& trackingData, typename std::enable_if< !IsNaturalyAligned<_Alignment>::value >::type*) {
    this->AlignedFree(ptr, trackingData);
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE
void* Heap::Calloc(size_t nmemb, size_t size, MemoryTrackingData& trackingData, typename std::enable_if< !IsNaturalyAligned<_Alignment>::value >::type*) {
    return this->AlignedCalloc(nmemb, size, _Alignment, trackingData);
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE
void* Heap::Realloc(void *ptr, size_t size, MemoryTrackingData& trackingData, typename std::enable_if< !IsNaturalyAligned<_Alignment>::value >::type*) {
    return this->AlignedRealloc(ptr, size, trackingData);
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
