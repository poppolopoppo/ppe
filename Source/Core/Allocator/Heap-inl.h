#pragma once

#include "Core/Allocator/Heap.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE
void* FHeap::Malloc(size_t size, Meta::TEnableIf< Meta::TIsNaturalyAligned<_Alignment>::value >*) {
    void* const ptr = this->Malloc(size);
    Assert(Meta::IsAligned(_Alignment, ptr));
    return ptr;
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE
void FHeap::Free(void *ptr, Meta::TEnableIf< Meta::TIsNaturalyAligned<_Alignment>::value >*) {
    this->Free(ptr);
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE
void* FHeap::Calloc(size_t nmemb, size_t size, Meta::TEnableIf< Meta::TIsNaturalyAligned<_Alignment>::value >*) {
    void* const ptr = this->Calloc(nmemb, size);
    Assert(Meta::IsAligned(_Alignment, ptr));
    return ptr;
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE
void* FHeap::Realloc(void *ptr, size_t size, Meta::TEnableIf< Meta::TIsNaturalyAligned<_Alignment>::value >*) {
    void* const newp = this->Realloc(ptr, size);
    Assert(Meta::IsAligned(_Alignment, newp));
    return newp;
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE
void* FHeap::Malloc(size_t size, Meta::TEnableIf< !Meta::TIsNaturalyAligned<_Alignment>::value >*) {
    return this->AlignedMalloc(size, _Alignment);
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE
void FHeap::Free(void *ptr, Meta::TEnableIf< !Meta::TIsNaturalyAligned<_Alignment>::value >*) {
    this->AlignedFree(ptr);
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE
void* FHeap::Calloc(size_t nmemb, size_t size, Meta::TEnableIf< !Meta::TIsNaturalyAligned<_Alignment>::value >*) {
    return this->AlignedCalloc(nmemb, size, _Alignment);
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE
void* FHeap::Realloc(void *ptr, size_t size, Meta::TEnableIf< !Meta::TIsNaturalyAligned<_Alignment>::value >*) {
    return this->AlignedRealloc(ptr, size, _Alignment);
}
//----------------------------------------------------------------------------
inline void FHeap::Swap(FHeap& other) {
    std::swap(other._handle, _handle);
}
//----------------------------------------------------------------------------
inline void swap(FHeap& lhs, FHeap& rhs) {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
