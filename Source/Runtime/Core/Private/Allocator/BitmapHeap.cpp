#include "stdafx.h"

#include "Allocator/BitmapHeap.h"

#include "Allocator/InitSegAllocator.h"
#include "Allocator/SystemPageAllocator.h"
#include "HAL/PlatformMemory.h"
#include "Memory/MemoryDomain.h"
#include "Thread/CriticalSection.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
struct FBitmapPagePool_ : FSystemPageAllocator {
    using allocator_traits = TAllocatorTraits<FSystemPageAllocator>;

    STATIC_CONST_INTEGRAL(u32, BlockSize, sizeof(FBitmapBasicPage));
    STATIC_CONST_INTEGRAL(u32, ChunkSize, FSystemPageAllocator::PageSize);
    STATIC_CONST_INTEGRAL(u32, NumBlocksPerChunk, ChunkSize / BlockSize - 1/* first block stores a FChunk */);

    struct FChunk {
        FChunk* NextChunk{ nullptr };
        void** FreePages{ nullptr };
        u32 NumUsedPages{ 0 };
        u32 HighestOffset{ BlockSize/* first block is used by (*this) */ };
        byte* Data() { return reinterpret_cast<byte*>(this); }
    };
    STATIC_ASSERT(sizeof(FChunk) <= BlockSize);

    FCriticalSection Barrier;
    FChunk* UsedChunks{ nullptr };
    u32 NumFreePages{ 0 };

    static FBitmapPagePool_& Get() NOEXCEPT {
        ONE_TIME_INITIALIZE(TInitSegAlloc<FBitmapPagePool_>, GInstance, 1000);
        return GInstance;
    }

#if USE_PPE_MEMORYDOMAINS
    static FMemoryTracking& TrackingData() {
        return MEMORYDOMAIN_TRACKING_DATA(BitmapPage);
    }
#endif

    FBitmapPagePool_() = default;

    ~FBitmapPagePool_() {
        const FCriticalScope scopeLock(&Barrier);
        while (UsedChunks)
            ReleaseUnusedChunk_(UsedChunks, nullptr);
    }

    FBitmapBasicPage* Allocate() {
        const FCriticalScope scopeLock(&Barrier);

        void* result;
        if (UsedChunks && UsedChunks->HighestOffset + BlockSize <= ChunkSize) {
            result = (UsedChunks->Data() + UsedChunks->HighestOffset);
            UsedChunks->HighestOffset += BlockSize;
            ++UsedChunks->NumUsedPages;
        }
        else {
            result = nullptr;

            for (FChunk* ch = UsedChunks; ch; ch = ch->NextChunk) {
                if (ch->FreePages) {
                    result = ch->FreePages;
                    ch->FreePages = (void**)*ch->FreePages;
                    ++ch->NumUsedPages;
                    break;
                }
            }

            if (Unlikely(nullptr == result))
                result = AllocateFromNewChunk_();
        }

        Assert(NumFreePages > 0);
        --NumFreePages;

        Assert(Meta::IsAlignedPow2(ALLOCATION_BOUNDARY, result));
        ONLY_IF_MEMORYDOMAINS(TrackingData().AllocateUser(BlockSize));
        ONLY_IF_ASSERT(FPlatformMemory::Memuninitialized(result, BlockSize));

        return INPLACE_NEW(result, FBitmapBasicPage);
    }

    void Deallocate(FBitmapBasicPage* p) {
        Assert(Meta::IsAlignedPow2(ALLOCATION_BOUNDARY, p));
        const FCriticalScope scopeLock(&Barrier);

        Assert_NoAssume(u64(-1) == p->Pages);
        Assert_NoAssume(0 == p->Sizes);
        Assert_NoAssume(nullptr == p->vAddressSpace);

        Assert(UsedChunks);
        ONLY_IF_MEMORYDOMAINS(TrackingData().DeallocateUser(BlockSize));
        ONLY_IF_ASSERT(FPlatformMemory::Memdeadbeef(p, BlockSize));

        ++NumFreePages;

        FChunk* const ch = (FChunk*)Meta::RoundToPrevPow2(reinterpret_cast<uintptr_t>(p), ChunkSize);
        Assert(ch->NumUsedPages > 0);

        *(void**)p = ch->FreePages;
        ch->FreePages = (void**)p;

        if (Unlikely(--ch->NumUsedPages == 0 && NumFreePages > NumBlocksPerChunk))
            ReleaseUnusedChunk_(ch);
    }

    void ReleaseCacheMemory() {
        const FCriticalScope scopeLock(&Barrier);

        for (FChunk* prv = nullptr, *ch = UsedChunks; ch; ) {
            FChunk* const nxt = ch->NextChunk;
            if (0 == ch->NumUsedPages) // release any unused chunk
                ReleaseUnusedChunk_(ch, prv);
            else
                prv = ch;
            ch = nxt;
        }
    }

private:
    NO_INLINE void* AllocateFromNewChunk_() {
        FChunk* const ch = INPLACE_NEW(
            allocator_traits::Allocate(*this, ChunkSize).Data,
            FChunk );
        Assert(Meta::IsAlignedPow2(ChunkSize, ch));
        ch->NextChunk = UsedChunks;
        UsedChunks = ch;

        ONLY_IF_MEMORYDOMAINS(TrackingData().AllocateSystem(ChunkSize));

        Assert(ch->HighestOffset + BlockSize < ChunkSize);
        Assert(0 == ch->NumUsedPages);
        void* const result = (ch->Data() + ch->HighestOffset);
        ch->HighestOffset += BlockSize;
        ch->NumUsedPages = 1;

        NumFreePages += NumBlocksPerChunk; // will be decremented in callee
        return result;
    }

    void ReleaseUnusedChunk_(FChunk* unused, FChunk* prev) {
        Assert(0 == unused->NumUsedPages);
        Assert(NumFreePages >= NumBlocksPerChunk);

        ONLY_IF_MEMORYDOMAINS(TrackingData().DeallocateSystem(ChunkSize));

        NumFreePages -= NumBlocksPerChunk;

        if (prev) {
            Assert(unused == prev->NextChunk);
            prev->NextChunk = unused->NextChunk;
        }
        else {
            Assert(UsedChunks == unused);
            UsedChunks = unused->NextChunk;
        }

        Meta::Destroy(unused);
        allocator_traits::Deallocate(*this, FAllocatorBlock{ unused, ChunkSize });
    }

    NO_INLINE void ReleaseUnusedChunk_(FChunk* unused) {
        Assert(unused);

        // need to look for chunk predecessor
        for (FChunk* prv = nullptr, *ch = UsedChunks; ch; prv = ch, ch = ch->NextChunk) {
            if (ch == unused) {
                ReleaseUnusedChunk_(unused, prv);
                return;
            }
        }

        AssertNotReached(); // this chunk doesn't belong to this pool!?
    }
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FBitmapBasicPage* FBitmapBasicPage::Allocate() {
    return FBitmapPagePool_::Get().Allocate();
}
//----------------------------------------------------------------------------
void FBitmapBasicPage::Deallocate(FBitmapBasicPage* p) NOEXCEPT {
    Assert(p);
    FBitmapPagePool_::Get().Deallocate(p);
}
//----------------------------------------------------------------------------
void FBitmapBasicPage::ReleaseCacheMemory() NOEXCEPT {
    FBitmapPagePool_::Get().ReleaseCacheMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
