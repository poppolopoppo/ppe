#pragma once

#include "Core/Core.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
NOALIAS RESTRICT
void*   CORE_API (malloc)(size_t size);
//----------------------------------------------------------------------------
NOALIAS
void    CORE_API (free)(void *ptr);
//----------------------------------------------------------------------------
NOALIAS RESTRICT
void*   CORE_API (calloc)(size_t nmemb, size_t size);
//----------------------------------------------------------------------------
NOALIAS RESTRICT
void*   CORE_API (realloc)(void *ptr, size_t size);
//----------------------------------------------------------------------------
NOALIAS RESTRICT
void*   CORE_API (aligned_malloc)(size_t size, size_t alignment);
//----------------------------------------------------------------------------
NOALIAS
void    CORE_API (aligned_free)(void *ptr);
//----------------------------------------------------------------------------
NOALIAS RESTRICT
void*   CORE_API (aligned_calloc)(size_t nmemb, size_t size, size_t alignment);
//----------------------------------------------------------------------------
NOALIAS RESTRICT
void*   CORE_API (aligned_realloc)(void *ptr, size_t size, size_t alignment);
//----------------------------------------------------------------------------
NOALIAS
void    CORE_API (malloc_release_pending_blocks)();
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE NOALIAS RESTRICT
void* (malloc)(size_t size, typename std::enable_if< TIsNaturalyAligned<_Alignment>::value >::type* = 0) {
    void* const p = (Core::malloc)(size);
    Assert(IS_ALIGNED(_Alignment, p));
    return p;
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE NOALIAS
void (free)(void *ptr, typename std::enable_if< TIsNaturalyAligned<_Alignment>::value >::type* = 0) {
    Assert(IS_ALIGNED(_Alignment, ptr));
    (Core::free)(ptr);
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE NOALIAS RESTRICT
void* (calloc)(size_t nmemb, size_t size, typename std::enable_if< TIsNaturalyAligned<_Alignment>::value >::type* = 0) {
    void* const p = (Core::calloc)(nmemb, size);
    Assert(IS_ALIGNED(_Alignment, p));
    return p;
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE NOALIAS RESTRICT
void* (realloc)(void *ptr, size_t size, typename std::enable_if< TIsNaturalyAligned<_Alignment>::value >::type* = 0) {
    void* const p = (Core::realloc)(ptr, size);
    Assert(IS_ALIGNED(_Alignment, p));
    return p;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE NOALIAS RESTRICT
void* (malloc)(size_t size, typename std::enable_if< !TIsNaturalyAligned<_Alignment>::value >::type* = 0) {
    return (Core::aligned_malloc)(size, _Alignment);
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE NOALIAS
void (free)(void *ptr, typename std::enable_if< !TIsNaturalyAligned<_Alignment>::value >::type* = 0) {
    (Core::aligned_free)(ptr);
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE NOALIAS RESTRICT
void* (calloc)(size_t nmemb, size_t size, typename std::enable_if< !TIsNaturalyAligned<_Alignment>::value >::type* = 0) {
    return (Core::aligned_calloc)(nmemb, size, _Alignment);
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE NOALIAS RESTRICT
void* (realloc)(void *ptr, size_t size, typename std::enable_if< !TIsNaturalyAligned<_Alignment>::value >::type* = 0) {
    return (Core::aligned_realloc)(ptr, size, _Alignment);
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
#ifdef WITH_CORE_ASSERT
bool FetchMemoryBlockDebugInfos(void* ptr, class FDecodedCallstack* pCallstack, size_t* pSizeInBytes, bool raw = false);
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
