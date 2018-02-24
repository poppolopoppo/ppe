#pragma once

#include "Core/Core.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
CORE_API NOALIAS RESTRICT
void*   (malloc)(size_t size);
//----------------------------------------------------------------------------
CORE_API NOALIAS
void    (free)(void *ptr);
//----------------------------------------------------------------------------
CORE_API NOALIAS RESTRICT
void*   (calloc)(size_t nmemb, size_t size);
//----------------------------------------------------------------------------
CORE_API NOALIAS RESTRICT
void*   (realloc)(void *ptr, size_t size);
//----------------------------------------------------------------------------
CORE_API NOALIAS RESTRICT
void*   (aligned_malloc)(size_t size, size_t alignment);
//----------------------------------------------------------------------------
CORE_API NOALIAS
void    (aligned_free)(void *ptr);
//----------------------------------------------------------------------------
CORE_API NOALIAS RESTRICT
void*   (aligned_calloc)(size_t nmemb, size_t size, size_t alignment);
//----------------------------------------------------------------------------
CORE_API NOALIAS RESTRICT
void*   (aligned_realloc)(void *ptr, size_t size, size_t alignment);
//----------------------------------------------------------------------------
CORE_API NOALIAS
void    (malloc_release_pending_blocks)();
//----------------------------------------------------------------------------
CORE_API NOALIAS
size_t  (malloc_snap_size)(size_t size);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE NOALIAS RESTRICT
void* (malloc)(size_t size, Meta::TEnableIf< Meta::TIsNaturalyAligned<_Alignment>::value >* = 0) {
    void* const p = (Core::malloc)(size);
    Assert(Meta::IsAligned(_Alignment, p));
    return p;
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE NOALIAS
void (free)(void *ptr, Meta::TEnableIf< Meta::TIsNaturalyAligned<_Alignment>::value >* = 0) {
    Assert(Meta::IsAligned(_Alignment, ptr));
    (Core::free)(ptr);
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE NOALIAS RESTRICT
void* (calloc)(size_t nmemb, size_t size, Meta::TEnableIf< Meta::TIsNaturalyAligned<_Alignment>::value >* = 0) {
    void* const p = (Core::calloc)(nmemb, size);
    Assert(Meta::IsAligned(_Alignment, p));
    return p;
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE NOALIAS RESTRICT
void* (realloc)(void *ptr, size_t size, Meta::TEnableIf< Meta::TIsNaturalyAligned<_Alignment>::value >* = 0) {
    void* const p = (Core::realloc)(ptr, size);
    Assert(Meta::IsAligned(_Alignment, p));
    return p;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE NOALIAS RESTRICT
void* (malloc)(size_t size, Meta::TEnableIf< !Meta::TIsNaturalyAligned<_Alignment>::value >* = 0) {
    return (Core::aligned_malloc)(size, _Alignment);
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE NOALIAS
void (free)(void *ptr, Meta::TEnableIf< !Meta::TIsNaturalyAligned<_Alignment>::value >* = 0) {
    (Core::aligned_free)(ptr);
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE NOALIAS RESTRICT
void* (calloc)(size_t nmemb, size_t size, Meta::TEnableIf< !Meta::TIsNaturalyAligned<_Alignment>::value >* = 0) {
    return (Core::aligned_calloc)(nmemb, size, _Alignment);
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE NOALIAS RESTRICT
void* (realloc)(void *ptr, size_t size, Meta::TEnableIf< !Meta::TIsNaturalyAligned<_Alignment>::value >* = 0) {
    return (Core::aligned_realloc)(ptr, size, _Alignment);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifndef FINAL_RELEASE
//----------------------------------------------------------------------------
class ALIGN(16) FCallstack;
CORE_API bool FetchMemoryBlockDebugInfos(void* ptr, FCallstack* pCallstack, size_t* pSizeInBytes, bool raw = false);
//----------------------------------------------------------------------------
template <typename T>
class TMemoryView;
CORE_API bool FetchMemoryAllocationHistogram(TMemoryView<const size_t>* classes, TMemoryView<const size_t>* allocations, TMemoryView<const size_t>* totalBytes);
//----------------------------------------------------------------------------
#endif //!FINAL_RELEASE
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
