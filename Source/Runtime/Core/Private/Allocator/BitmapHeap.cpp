#include "stdafx.h"

#include "Allocator/BitmapHeap.h"

#include "Allocator/InitSegAllocator.h"
#include "Memory/MemoryDomain.h"
#include "Memory/MemoryTracking.h"
#include "Memory/VirtualMemory.h"

#include <mutex>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
struct FBitmapPageGlobalCache_ {
    STATIC_CONST_INTEGRAL(u32, PageSize, ALLOCATION_GRANULARITY);
    STATIC_CONST_INTEGRAL(u32, DescriptorSize, sizeof(TBitmapPage<PageSize>));

    struct ALIGN(DescriptorSize) FNode {
        FNode* Next;
    };

    std::mutex Barrier;
    FNode* _freeList{ nullptr };
    FNode* _newPageAllocated{ nullptr };

    static FBitmapPageGlobalCache_& Get() NOEXCEPT {
        ONE_TIME_DEFAULT_INITIALIZE(TInitSegAlloc<FBitmapPageGlobalCache_>, GlobalCache);
        return GlobalCache;
    }

    void* Allocate() NOEXCEPT {
        Meta::FUniqueLock scopeLock(Barrier);

        FNode* newNode;
        if (_freeList) {
            _freeList = *(FNode**)(newNode = _freeList);
        }
        else if (_newPageAllocated) {
            newNode = _newPageAllocated;
            if (!((uintptr_t)++_newPageAllocated & (PageSize - 1)))
                _newPageAllocated = ((FNode**)_newPageAllocated)[-1];
        }
        else {
            // !! this memory block will *NEVER* be released !!
            FNode* const newPage = (FNode*)FVirtualMemory::InternalAlloc(PageSize
#if USE_PPE_MEMORYDOMAINS
                , MEMORYDOMAIN_TRACKING_DATA(Bookkeeping)
#endif
            );
            AssertRelease(newPage);

            // in case if other thread also have just allocated a new page
            Assert(((char**)((char*)newPage + PageSize))[-1] == nullptr);
            ((FNode**)((char*)newPage + PageSize))[-1] = _newPageAllocated;

            // eat first block and saves the rest for later insertions
            newNode = newPage;
            _newPageAllocated = newPage + 1;
        }

#if USE_PPE_MEMORYDOMAINS
        MEMORYDOMAIN_TRACKING_DATA(Bookkeeping).AllocateUser(sizeof(FNode));
#endif

        return newNode;
    }

    void Deallocate(void* ptr) NOEXCEPT {
        Assert(ptr);

        Meta::FUniqueLock scopeLock(Barrier);

        FNode* n = reinterpret_cast<FNode*>(ptr);
        *(FNode**)n = _freeList;
        _freeList = n;

#if USE_PPE_MEMORYDOMAINS
        MEMORYDOMAIN_TRACKING_DATA(Bookkeeping).DeallocateUser(sizeof(FNode));
#endif
    }
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void* FBitmapPageCache::GragPage_(size_t sz) NOEXCEPT {
    Assert(FBitmapPageGlobalCache_::DescriptorSize == sz);
    UNUSED(sz);
    return FBitmapPageGlobalCache_::Get().Allocate();
}
//----------------------------------------------------------------------------
void FBitmapPageCache::ReleasePage_(void* ptr, size_t sz) NOEXCEPT {
    Assert(FBitmapPageGlobalCache_::DescriptorSize == sz);
    UNUSED(sz);
    FBitmapPageGlobalCache_::Get().Deallocate(ptr);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
