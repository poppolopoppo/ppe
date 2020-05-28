#pragma once

#include "Core_fwd.h"

#include "HAL/PlatformDebug.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_CORE_API NOALIAS RESTRICT PPE_DECLSPEC_ALLOCATOR()
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
size_t  (malloc_snap_size)(size_t size) NOEXCEPT;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t _Alignment>
using unaligned_alloc_t = Meta::TEnableIf<not Meta::need_alignment_v<_Alignment>>;
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE NOALIAS RESTRICT
void* (malloc)(size_t size, unaligned_alloc_t<_Alignment>* = 0) {
    void* const p = (PPE::malloc)(size);
    Assert(Meta::IsAligned(_Alignment, p));
    return p;
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE NOALIAS
void (free)(void *ptr, unaligned_alloc_t<_Alignment>* = 0) {
    Assert(Meta::IsAligned(_Alignment, ptr));
    (PPE::free)(ptr);
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE NOALIAS RESTRICT
void* (calloc)(size_t nmemb, size_t size, unaligned_alloc_t<_Alignment>* = 0) {
    void* const p = (PPE::calloc)(nmemb, size);
    Assert(Meta::IsAligned(_Alignment, p));
    return p;
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE NOALIAS RESTRICT
void* (realloc)(void *ptr, size_t size, unaligned_alloc_t<_Alignment>* = 0) {
    void* const p = (PPE::realloc)(ptr, size);
    Assert(Meta::IsAligned(_Alignment, p));
    return p;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t _Alignment>
using aligned_alloc_t = Meta::TEnableIf<Meta::need_alignment_v<_Alignment>>;
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE NOALIAS RESTRICT
void* (malloc)(size_t size, aligned_alloc_t<_Alignment>* = 0) {
    return (PPE::aligned_malloc)(size, _Alignment);
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE NOALIAS
void (free)(void *ptr, aligned_alloc_t<_Alignment>* = 0) {
    (PPE::aligned_free)(ptr);
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE NOALIAS RESTRICT
void* (calloc)(size_t nmemb, size_t size, aligned_alloc_t<_Alignment>* = 0) {
    return (PPE::aligned_calloc)(nmemb, size, _Alignment);
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE NOALIAS RESTRICT
void* (realloc)(void *ptr, size_t size, aligned_alloc_t<_Alignment>* = 0) {
    return (PPE::aligned_realloc)(ptr, size, _Alignment);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if !USE_PPE_FINAL_RELEASE
template <typename T>
class TMemoryView;
struct FMallocDebug {
public: // leak detector
    static PPE_CORE_API void StartLeakDetector();
    static PPE_CORE_API void ShutdownLeakDetector();
    static PPE_CORE_API bool SetLeakDetectorWhiteListed(bool ignoreleaks);
    static PPE_CORE_API void DumpMemoryLeaks(bool onlyNonDeleters = false);
public: // statistics
    static PPE_CORE_API bool FetchAllocationHistogram(
        TMemoryView<const size_t>* classes,
        TMemoryView<const i64>* allocations,
        TMemoryView<const i64>* totalBytes );
    static PPE_CORE_API bool FetchMediumMips(
        void** vspace,
        size_t* numCommited,
        size_t* numReserved,
        size_t* mipSizeInBytes,
        TMemoryView<const u32>* mipMasks );
    static PPE_CORE_API bool FetchLargeMips(
        void** vspace,
        size_t* numCommited,
        size_t* numReserved,
        size_t* mipSizeInBytes,
        TMemoryView<const u32>* mipMasks );
};
struct FLeakDetectorWhiteListScope {
    const bool WasIgnoringLeaks;
    FLeakDetectorWhiteListScope(): WasIgnoringLeaks(FMallocDebug::SetLeakDetectorWhiteListed(true)) {}
    ~FLeakDetectorWhiteListScope() { FMallocDebug::SetLeakDetectorWhiteListed(WasIgnoringLeaks); }
};
#   define PPE_LEAKDETECTOR_WHITELIST_SCOPE() const ::PPE::FLeakDetectorWhiteListScope ANONYMIZE(_leakDetectorWhiteListScope)
#else
#   define PPE_LEAKDETECTOR_WHITELIST_SCOPE() NOOP()
#endif //!USE_PPE_FINAL_RELEASE
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
