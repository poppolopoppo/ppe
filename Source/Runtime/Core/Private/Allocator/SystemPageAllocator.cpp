#include "stdafx.h"

#include "Allocator/SystemPageAllocator.h"

#include "Allocator/InitSegAllocator.h"
#include "Container/Array.h"
#include "Container/Stack.h"
#include "HAL/PlatformMemory.h"
#include "Memory/MemoryDomain.h"
#include "Memory/VirtualMemory.h"
#include "Thread/CriticalSection.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
PRAGMA_MSVC_WARNING_PUSH()
PRAGMA_MSVC_WARNING_DISABLE(4324) // 'XXX' structure was padded due to alignment
struct CACHELINE_ALIGNED FSystemPageCache_ {
    using index_type = CODE3264(u16, u32);
    STATIC_CONST_INTEGRAL(u32, PageSize, FSystemPageAllocator::PageSize);

    struct FFreePage_ {
        FFreePage_* NextPage;
    };

    struct FChunk {
        byte* Address;
        index_type Capacity;
        index_type HighestOffset;

        NODISCARD bool Allocate(FFreePage_** outResult) NOEXCEPT {
            if (Likely(HighestOffset + PageSize <= Capacity)) {
                *outResult = reinterpret_cast<FFreePage_*>(Address + HighestOffset);
                HighestOffset += PageSize;
                return true;
            }
            Assert_NoAssume(HighestOffset == Capacity);
            return false;
        }
    };

    struct FChunkArray {
        FChunkArray* NextArray;
        index_type HighestIndex;
        index_type Reserved;
        STATIC_CONST_INTEGRAL(u32, Capacity, (PageSize - sizeof(NextArray) - sizeof(HighestIndex) - sizeof(Reserved)) / sizeof(FChunk));
        TStaticArray<FChunk, Capacity> Data;

        FChunk& Last() {
            Assert(HighestIndex);
            return Data[HighestIndex - 1];
        }
    };
    STATIC_ASSERT(sizeof(FChunkArray) == PageSize);

    FCriticalSection Barrier;
    FFreePage_* FreePages{ nullptr };
    FChunkArray* UsedChunks{ nullptr };
#if USE_PPE_ASSERT_RELEASE
    u32 NumUserPages{ 0 };
#endif

    static FSystemPageCache_& Get() {
        ONE_TIME_DEFAULT_INITIALIZE(TInitSegAlloc<FSystemPageCache_>, GInstance);
        return GInstance;
    }

    NO_INLINE ~FSystemPageCache_() {
        ReleaseAll();
    }

#if USE_PPE_MEMORYDOMAINS
    static FMemoryTracking& TrackingData() NOEXCEPT {
        return MEMORYDOMAIN_TRACKING_DATA(SystemPage);
    }
#endif

    FFreePage_* Allocate() {
        FFreePage_* result = nullptr;
        {
            const FCriticalScope scopeLock{ &Barrier };

            if (FreePages) {
                result = FreePages;
                FreePages = FreePages->NextPage;
            }
            else if (not (UsedChunks && UsedChunks->Last().Allocate(&result)))
                result = AllocateFromNewChunk_();
        }
        Assert(Meta::IsAligned(PageSize, result));
        ONLY_IF_MEMORYDOMAINS(TrackingData().AllocateUser(PageSize));
        ONLY_IF_ASSERT(FPlatformMemory::Memuninitialized(result, PageSize));
        ONLY_IF_ASSERT_RELEASE(++NumUserPages);
        return result;
    }

    void Deallocate(FFreePage_* p) {
        Assert(Meta::IsAligned(PageSize, p));
        ONLY_IF_ASSERT(FPlatformMemory::Memdeadbeef(p, PageSize));

        const FCriticalScope scopeLock{ &Barrier };
        ONLY_IF_MEMORYDOMAINS(TrackingData().DeallocateUser(PageSize));

        AssertRelease_NoAssume(NumUserPages > 0);
        ONLY_IF_ASSERT_RELEASE(--NumUserPages);

        p->NextPage = FreePages;
        FreePages = p;
    }

    void ReleaseAll() {
        const FCriticalScope scopeLock{ &Barrier };
        AssertReleaseMessage_NoAssume(L"still got system pages in flight!", 0 == NumUserPages);

        while (UsedChunks) {
            FChunkArray* const pnext = UsedChunks->NextArray;
            Assert(UsedChunks->HighestIndex);
            AssertMessage(L"the first chunk should hold the allocation for the current array", FPlatformMemory::Memaliases(UsedChunks->Data[0].Address, UsedChunks->Data[0].Capacity, UsedChunks));

            forrange(i, /* first will be deleted last */1, UsedChunks->HighestIndex) {
                FChunk& ch = UsedChunks->Data[i];
                FVirtualMemory::InternalFree(ch.Address, ch.Capacity ARGS_IF_MEMORYDOMAINS(TrackingData()));
            }

            ONLY_IF_MEMORYDOMAINS(TrackingData().DeallocateUser(sizeof(FChunkArray)));
            const FChunk& first = UsedChunks->Data[0];
            FVirtualMemory::InternalFree(first.Address, first.Capacity ARGS_IF_MEMORYDOMAINS(TrackingData()));

            UsedChunks = pnext;
        }

        FreePages = nullptr;
        ONLY_IF_ASSERT_RELEASE(NumUserPages = 0);
    }

private:
    NO_INLINE FFreePage_* AllocateFromNewChunk_() {
        FChunk newChunk;
        newChunk.HighestOffset = 0;
        newChunk.Capacity = checked_cast<index_type>(FPlatformMemory::AllocationGranularity);
        newChunk.Address = static_cast<byte*>(FVirtualMemory::InternalAlloc(newChunk.Capacity ARGS_IF_MEMORYDOMAINS(TrackingData())));
        Assert(newChunk.Address);

        if (nullptr == UsedChunks || UsedChunks->HighestIndex == FChunkArray::Capacity) {
            ONLY_IF_MEMORYDOMAINS(TrackingData().AllocateUser(sizeof(FChunkArray)));
            FChunkArray* const arr = reinterpret_cast<FChunkArray*>(newChunk.Address);
            STATIC_ASSERT(sizeof(FChunkArray) == PageSize);
            newChunk.HighestOffset += checked_cast<index_type>(sizeof(FChunkArray));
            arr->NextArray = UsedChunks;
            arr->HighestIndex = 0;
            UsedChunks = arr;
        }

        FFreePage_* result = nullptr;
        Verify(newChunk.Allocate(&result));
        Assert(UsedChunks->HighestIndex < FChunkArray::Capacity);
        UsedChunks->Data[UsedChunks->HighestIndex++] = std::move(newChunk);

        return result;
    }
};
PRAGMA_MSVC_WARNING_POP()
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FAllocatorBlock FSystemPageAllocator::Allocate(size_t s) {
    Assert(FSystemPageCache_::PageSize == s);
    FAllocatorBlock blk;
    blk.SizeInBytes = s;
    blk.Data = FSystemPageCache_::Get().Allocate();
    Assert(blk.Data);
    return blk;
}
//----------------------------------------------------------------------------
void FSystemPageAllocator::Deallocate(FAllocatorBlock blk) {
    Assert(blk.Data);
    Assert(FSystemPageCache_::PageSize == blk.SizeInBytes);
    FSystemPageCache_::Get().Deallocate(static_cast<FSystemPageCache_::FFreePage_*>(blk.Data));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
