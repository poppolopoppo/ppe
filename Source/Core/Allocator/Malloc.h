#pragma once

#include "Core/Core.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
NOALIAS RESTRICT
void*   (malloc)(size_t size);
//----------------------------------------------------------------------------
NOALIAS
void    (free)(void *ptr);
//----------------------------------------------------------------------------
NOALIAS RESTRICT
void*   (calloc)(size_t nmemb, size_t size);
//----------------------------------------------------------------------------
NOALIAS RESTRICT
void*   (realloc)(void *ptr, size_t size);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
NOALIAS RESTRICT
void*   (aligned_malloc)(size_t size, size_t alignment);
//----------------------------------------------------------------------------
NOALIAS
void    (aligned_free)(void *ptr);
//----------------------------------------------------------------------------
NOALIAS RESTRICT
void*   (aligned_calloc)(size_t nmemb, size_t size, size_t alignment);
//----------------------------------------------------------------------------
NOALIAS RESTRICT
void*   (aligned_realloc)(void *ptr, size_t size, size_t alignment);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE NOALIAS RESTRICT
void* (malloc)(size_t size, typename std::enable_if< IsNaturalyAligned<_Alignment>::value >::type* = 0) {
    void* const p = malloc(size);
    Assert(IS_ALIGNED(_Alignment, p));
    return p;
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE NOALIAS
void (free)(void *ptr, typename std::enable_if< IsNaturalyAligned<_Alignment>::value >::type* = 0) {
    Assert(IS_ALIGNED(_Alignment, ptr));
    free(ptr);
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE NOALIAS RESTRICT
void* (calloc)(size_t nmemb, size_t size, typename std::enable_if< IsNaturalyAligned<_Alignment>::value >::type* = 0) {
    void* const p = calloc(nmemb, size);
    Assert(IS_ALIGNED(_Alignment, p));
    return p;
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE NOALIAS RESTRICT
void* (realloc)(void *ptr, size_t size, typename std::enable_if< IsNaturalyAligned<_Alignment>::value >::type* = 0) {
    void* const p = realloc(ptr, size);
    Assert(IS_ALIGNED(_Alignment, p));
    return p;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE NOALIAS RESTRICT
void* (malloc)(size_t size, typename std::enable_if< !IsNaturalyAligned<_Alignment>::value >::type* = 0) {
    return aligned_malloc(size, _Alignment);
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE NOALIAS
void (free)(void *ptr, typename std::enable_if< !IsNaturalyAligned<_Alignment>::value >::type* = 0) {
    aligned_free(ptr);
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE NOALIAS RESTRICT
void* (calloc)(size_t nmemb, size_t size, typename std::enable_if< !IsNaturalyAligned<_Alignment>::value >::type* = 0) {
    return aligned_calloc(nmemb, size, _Alignment);
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE NOALIAS RESTRICT
void* (realloc)(void *ptr, size_t size, typename std::enable_if< !IsNaturalyAligned<_Alignment>::value >::type* = 0) {
    return aligned_realloc(ptr, size, _Alignment);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define ALIGNED_ALLOCATED_DEF(_Type, _Alignment) \
public: \
    void* operator new(size_t size) { \
        return Core::malloc<_Alignment>(sizeof(_Type)); \
    } \
    void* operator new(size_t, void* ptr) { \
        Assert(ptr); \
        Likely(ptr); \
        return ptr; \
    } \
    \
    void operator delete(void* ptr) { \
        Core::free<_Alignment>(ptr); \
    } \
    void operator delete(void* ptr, size_t) { operator delete(ptr); } \
    void operator delete(void*, void*) {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
