#include "stdafx.h"

#include "Allocator/BitmapHeap.h"

#include "Allocator/InitSegAllocator.h"
#include "Memory/MemoryDomain.h"
#include "Memory/VirtualMemory.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
using FBitmapPageGlobalCache_ = TVirtualMemoryPool<TBitmapPage<ALLOCATION_GRANULARITY>>;
static FBitmapPageGlobalCache_& BitmapPageGlobalCache_() NOEXCEPT {
    using wrapper_t = TInitSegAlloc<FBitmapPageGlobalCache_>;
#if USE_PPE_MEMORYDOMAINS
    ONE_TIME_INITIALIZE(wrapper_t, GInstance, MEMORYDOMAIN_TRACKING_DATA(BitmapCache) );
#else
    ONE_TIME_DEFAULT_INITIALIZE(wrapper_t, GInstance);
#endif
    return GInstance;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FBitmapPageCache::ReleaseCacheMemory() NOEXCEPT {
    BitmapPageGlobalCache_().ReleaseCacheMemory(false);
}
//----------------------------------------------------------------------------
void* FBitmapPageCache::GragPage_(size_t sz) NOEXCEPT {
    Assert(FBitmapPageGlobalCache_::BlockSize == sz);
    UNUSED(sz);
    return BitmapPageGlobalCache_().Allocate();
}
//----------------------------------------------------------------------------
void FBitmapPageCache::ReleasePage_(void* ptr, size_t sz) NOEXCEPT {
    Assert(FBitmapPageGlobalCache_::BlockSize == sz);
    UNUSED(sz);
    BitmapPageGlobalCache_().Deallocate(ptr);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
