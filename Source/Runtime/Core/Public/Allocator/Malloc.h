#pragma once

#include "Core_fwd.h"

#include "Allocator/AllocatorBlock.h"
#include "HAL/PlatformDebug.h"

#if !USE_PPE_FINAL_RELEASE
#   include "IO/TextWriter_fwd.h"
#endif

// C++ Sized Deallocation
// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3536.html
// #TODO: ENABLE WHEN ALL DELETE() CALLS WILL USE SIZED OVERLOADS
#define USE_PPE_SIZED_DEALLOCATION (PPE_HAS_CXX14 && 0)

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
FAllocatorBlock (malloc_for_new)(size_t size);
//----------------------------------------------------------------------------
PPE_CORE_API NOALIAS
FAllocatorBlock (realloc_for_new)(FAllocatorBlock blk, size_t size);
//----------------------------------------------------------------------------
PPE_CORE_API NOALIAS
void    (free_for_delete)(FAllocatorBlock blk);
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
#if !USE_PPE_FINAL_RELEASE
class FMemoryTracking;
template <typename T>
class TMemoryView;
struct FMallocDebug {
public: // leak detector
    static PPE_CORE_API void StartLeakDetector();
    static PPE_CORE_API void ShutdownLeakDetector();
    static PPE_CORE_API bool SetLeakDetectorWhiteListed(bool ignoreleaks);
    static PPE_CORE_API void DumpMemoryLeaks(bool onlyNonDeleters = false);
    static PPE_CORE_API size_t RegionSize(void* ptr);
public: // statistics
    static PPE_CORE_API bool FetchAllocationHistogram(
        TMemoryView<const u32>* sizeClasses,
        TMemoryView<const FMemoryTracking>* bins );
    static PPE_CORE_API void DumpMemoryInfo(FTextWriter& oss);
    static PPE_CORE_API void DumpMemoryInfo(FWTextWriter& oss);
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
