#include "stdafx.h"

#include "Malloc.h"
#include "MallocBinned.h"

// Lowest level to hook or replace default allocator

#define CORE_MALLOC_FORCE_STD 1 //%_NOCOMMIT%

#define CORE_MALLOC_ALLOCATOR_STD       0
#define CORE_MALLOC_ALLOCATOR_BINNED    1

#if     CORE_MALLOC_FORCE_STD
#   define CORE_MALLOC_ALLOCATOR CORE_MALLOC_ALLOCATOR_STD
#else
#   define CORE_MALLOC_ALLOCATOR CORE_MALLOC_ALLOCATOR_BINNED
#endif

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if (CORE_MALLOC_ALLOCATOR == CORE_MALLOC_ALLOCATOR_STD)
//----------------------------------------------------------------------------
NOALIAS
__declspec(restrict)
void*   (malloc)(size_t size) {
    return ::malloc(size);
}
//----------------------------------------------------------------------------
NOALIAS
void    (free)(void *ptr) {
    return ::free(ptr);
}
//----------------------------------------------------------------------------
NOALIAS
__declspec(restrict)
void*   (calloc)(size_t nmemb, size_t size) {
    return ::calloc(nmemb, size);
}
//----------------------------------------------------------------------------
NOALIAS
__declspec(restrict)
void*   (realloc)(void *ptr, size_t size) {
    return ::realloc(ptr, size);
}
//----------------------------------------------------------------------------
NOALIAS
__declspec(restrict)
void*   (aligned_malloc)(size_t size, size_t alignment) {
    return _aligned_malloc(size, alignment);
}
//----------------------------------------------------------------------------
NOALIAS
void    (aligned_free)(void *ptr) {
    _aligned_free(ptr);
}
//----------------------------------------------------------------------------
NOALIAS
__declspec(restrict)
void*   (aligned_calloc)(size_t nmemb, size_t size, size_t alignment) {
    void* const p = _aligned_malloc(size * nmemb, alignment);
    ::memset(p, 0, size * nmemb);
    return p;
}
//----------------------------------------------------------------------------
NOALIAS
__declspec(restrict)
void*   (aligned_realloc)(void *ptr, size_t size, size_t alignment) {
    return _aligned_realloc(ptr, size, alignment);
}
//----------------------------------------------------------------------------
#endif //!(CORE_MALLOC_ALLOCATOR == CORE_MALLOC_ALLOCATOR_STD)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if (CORE_MALLOC_ALLOCATOR == CORE_MALLOC_ALLOCATOR_BINNED)
//----------------------------------------------------------------------------
NOALIAS
__declspec(restrict)
void*   (malloc)(size_t size) {
    return FMallocBinned::Malloc(size);
}
//----------------------------------------------------------------------------
NOALIAS
void    (free)(void *ptr) {
    FMallocBinned::Free(ptr);
}
//----------------------------------------------------------------------------
NOALIAS
__declspec(restrict)
void*   (calloc)(size_t nmemb, size_t size) {
    void* const p = FMallocBinned::Malloc(nmemb * size);
    ::memset(p, 0, nmemb * size);
    return p;
}
//----------------------------------------------------------------------------
NOALIAS
__declspec(restrict)
void*   (realloc)(void *ptr, size_t size) {
    return FMallocBinned::Realloc(ptr, size);
}
//----------------------------------------------------------------------------
NOALIAS
__declspec(restrict)
void*   (aligned_malloc)(size_t size, size_t alignment) {
    return FMallocBinned::AlignedMalloc(size, alignment);
}
//----------------------------------------------------------------------------
NOALIAS
void    (aligned_free)(void *ptr) {
    FMallocBinned::AlignedFree(ptr);
}
//----------------------------------------------------------------------------
NOALIAS
__declspec(restrict)
void*   (aligned_calloc)(size_t nmemb, size_t size, size_t alignment) {
    void* const p = FMallocBinned::AlignedMalloc(nmemb * size, alignment);
    ::memset(p, 0, nmemb * size);
    return p;
}
//----------------------------------------------------------------------------
NOALIAS
__declspec(restrict)
void*   (aligned_realloc)(void *ptr, size_t size, size_t alignment) {
    return FMallocBinned::AlignedRealloc(ptr, size, alignment);
}
//----------------------------------------------------------------------------
#endif //!(CORE_MALLOC_ALLOCATOR == CORE_MALLOC_ALLOCATOR_BINNED)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
