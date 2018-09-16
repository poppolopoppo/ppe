#pragma once

#include "Core.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_CORE_API NOALIAS RESTRICT
void*   (malloc)(size_t size);
//----------------------------------------------------------------------------
PPE_CORE_API NOALIAS
void    (free)(void *ptr);
//----------------------------------------------------------------------------
PPE_CORE_API NOALIAS RESTRICT
void*   (calloc)(size_t nmemb, size_t size);
//----------------------------------------------------------------------------
PPE_CORE_API NOALIAS RESTRICT
void*   (realloc)(void *ptr, size_t size);
//----------------------------------------------------------------------------
PPE_CORE_API NOALIAS RESTRICT
void*   (aligned_malloc)(size_t size, size_t alignment);
//----------------------------------------------------------------------------
PPE_CORE_API NOALIAS
void    (aligned_free)(void *ptr);
//----------------------------------------------------------------------------
PPE_CORE_API NOALIAS RESTRICT
void*   (aligned_calloc)(size_t nmemb, size_t size, size_t alignment);
//----------------------------------------------------------------------------
PPE_CORE_API NOALIAS RESTRICT
void*   (aligned_realloc)(void *ptr, size_t size, size_t alignment);
//----------------------------------------------------------------------------
PPE_CORE_API NOALIAS
void    (malloc_release_cache_memory)();
//----------------------------------------------------------------------------
PPE_CORE_API NOALIAS
void    (malloc_release_pending_blocks)();
//----------------------------------------------------------------------------
PPE_CORE_API NOALIAS
size_t  (malloc_snap_size)(size_t size);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE NOALIAS RESTRICT
void* (malloc)(size_t size, Meta::TEnableIf< Meta::TIsNaturalyAligned<_Alignment>::value >* = 0) {
    void* const p = (PPE::malloc)(size);
    Assert(Meta::IsAligned(_Alignment, p));
    return p;
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE NOALIAS
void (free)(void *ptr, Meta::TEnableIf< Meta::TIsNaturalyAligned<_Alignment>::value >* = 0) {
    Assert(Meta::IsAligned(_Alignment, ptr));
    (PPE::free)(ptr);
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE NOALIAS RESTRICT
void* (calloc)(size_t nmemb, size_t size, Meta::TEnableIf< Meta::TIsNaturalyAligned<_Alignment>::value >* = 0) {
    void* const p = (PPE::calloc)(nmemb, size);
    Assert(Meta::IsAligned(_Alignment, p));
    return p;
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE NOALIAS RESTRICT
void* (realloc)(void *ptr, size_t size, Meta::TEnableIf< Meta::TIsNaturalyAligned<_Alignment>::value >* = 0) {
    void* const p = (PPE::realloc)(ptr, size);
    Assert(Meta::IsAligned(_Alignment, p));
    return p;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE NOALIAS RESTRICT
void* (malloc)(size_t size, Meta::TEnableIf< !Meta::TIsNaturalyAligned<_Alignment>::value >* = 0) {
    return (PPE::aligned_malloc)(size, _Alignment);
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE NOALIAS
void (free)(void *ptr, Meta::TEnableIf< !Meta::TIsNaturalyAligned<_Alignment>::value >* = 0) {
    (PPE::aligned_free)(ptr);
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE NOALIAS RESTRICT
void* (calloc)(size_t nmemb, size_t size, Meta::TEnableIf< !Meta::TIsNaturalyAligned<_Alignment>::value >* = 0) {
    return (PPE::aligned_calloc)(nmemb, size, _Alignment);
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE NOALIAS RESTRICT
void* (realloc)(void *ptr, size_t size, Meta::TEnableIf< !Meta::TIsNaturalyAligned<_Alignment>::value >* = 0) {
    return (PPE::aligned_realloc)(ptr, size, _Alignment);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifndef FINAL_RELEASE
template <typename T>
class TMemoryView;
PPE_CORE_API void StartLeakDetector();
PPE_CORE_API void ShutdownLeakDetector();
PPE_CORE_API bool SetLeakDetectorWhiteListed(bool ignoreleaks);
PPE_CORE_API void DumpMemoryLeaks(bool onlyNonDeleters = false);
PPE_CORE_API bool FetchMemoryAllocationHistogram(
    TMemoryView<const size_t>* classes,
    TMemoryView<const i64>* allocations,
    TMemoryView<const i64>* totalBytes );
struct FLeakDetectorWhiteListScope {
    const bool WasIgnoringLeaks;
    FLeakDetectorWhiteListScope(): WasIgnoringLeaks(SetLeakDetectorWhiteListed(true)) {}
    ~FLeakDetectorWhiteListScope() { SetLeakDetectorWhiteListed(WasIgnoringLeaks); }
};
#   define PPE_LEAKDETECTOR_WHITELIST_SCOPE() const ::PPE::FLeakDetectorWhiteListScope ANONYMIZE(_leakDetectorWhiteListScope)
#else
#   define PPE_LEAKDETECTOR_WHITELIST_SCOPE() NOOP()
#endif //!FINAL_RELEASE
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
