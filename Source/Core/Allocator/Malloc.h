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
template <typename T>
class TMemoryView;
CORE_API void StartLeakDetector();
CORE_API void ShutdownLeakDetector();
CORE_API bool SetLeakDetectorWhiteListed(bool ignoreleaks);
CORE_API void DumpMemoryLeaks(bool onlyNonDeleters = false);
CORE_API bool FetchMemoryAllocationHistogram(
    TMemoryView<const size_t>* classes,
    TMemoryView<const size_t>* allocations,
    TMemoryView<const size_t>* totalBytes );
struct FLeakDetectorWhiteListScope {
    const bool WasIgnoringLeaks;
    FLeakDetectorWhiteListScope(): WasIgnoringLeaks(SetLeakDetectorWhiteListed(true)) {}
    ~FLeakDetectorWhiteListScope() { SetLeakDetectorWhiteListed(WasIgnoringLeaks); }
};
#   define CORE_LEAKDETECTOR_WHITELIST_SCOPE() const ::Core::FLeakDetectorWhiteListScope ANONYMIZE(_leakDetectorWhiteListScope)
#else
#   define CORE_LEAKDETECTOR_WHITELIST_SCOPE() NOOP()
#endif //!FINAL_RELEASE
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
