#include "stdafx.h"

#include "Malloc.h"

// Lowest level to hook or replace default allocator

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
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
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
NOALIAS
__declspec(restrict)
void*   (aligned_malloc)(size_t size, size_t alignment) {
    return _aligned_malloc(size, alignment);
}
//----------------------------------------------------------------------------
NOALIAS
void    aligned_free(void *ptr) {
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
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
