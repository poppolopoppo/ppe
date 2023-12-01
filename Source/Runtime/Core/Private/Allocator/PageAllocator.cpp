// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Allocator/PageAllocator.h"

#include "Allocator/InitSegAllocator.h"
#include "Container/Array.h"
#include "Container/BitMask.h"
#include "HAL/PlatformMemory.h"
#include "Memory/MemoryDomain.h"
#include "Memory/VirtualMemory.h"
#include "Meta/Utility.h"
#include "Thread/CriticalSection.h"

#include <atomic>

#define USE_PPE_VIRTUALPAGE_CACHE (!USE_PPE_SANITIZER)

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
#if USE_PPE_VIRTUALPAGE_CACHE
PRAGMA_MSVC_WARNING_PUSH()
PRAGMA_MSVC_WARNING_DISABLE(4324) // 'XXX' structure was padded due to alignment
//----------------------------------------------------------------------------
struct CACHELINE_ALIGNED FVirtualPageCache_ : Meta::FNonCopyableNorMovable {
    STATIC_CONST_INTEGRAL(size_t, PageSize, FPageAllocator::PageSize);

    struct ALIGN(PageSize) FFreePage {
        FFreePage* NextPage;
    };

    struct FBlock {
        STATIC_CONST_INTEGRAL(u32, BitMask, UINT16_MAX);
        u8* Packed;

        NODISCARD u8* Alloc() const {
            u8* const alloc = Packed - (reinterpret_cast<uintptr_t>(Packed) & BitMask);
            Assert_NoAssume(Meta::IsAligned(FPlatformMemory::AllocationGranularity, alloc));
            return alloc;
        }

        NODISCARD void* PageAddress(u32 index) const {
            Assert_NoAssume(UsedPages().Get(index));
            return Alloc() + (PageSize * index);
        }

        NODISCARD TBitMask<u32> UsedPages() const {
            return { static_cast<u32>(reinterpret_cast<uintptr_t>(Packed)& BitMask) };
        }

        void SetUsedPages(const TBitMask<u32>& mask) {
            Assert_NoAssume((mask.Data & BitMask) == mask.Data);
            const auto addr = reinterpret_cast<uintptr_t>(Packed);
            Packed = (Packed - (addr & BitMask)) + mask.Data;
        }

        void Reset(void* alloc, size_t sizeInBytes) {
            Assert(alloc);
            Assert(Meta::IsAlignedPow2(BitMask + 1, alloc));
            Assert_NoAssume(Meta::IsAligned(FPlatformMemory::AllocationGranularity, alloc));
            Assert_NoAssume(sizeInBytes / PageSize <= BitMask);
            const auto usedPages = TBitMask<u32>::SetFirstN(static_cast<u32>(sizeInBytes / PageSize));
            Packed = static_cast<u8*>(alloc) + usedPages.Data/* all pages are used */;
            Assert_NoAssume(Alloc() == alloc);
        }
    };

    struct FChunk {
        FChunk* NextChunk{ nullptr };
        size_t NumBlocks{ 0 };
        STATIC_CONST_INTEGRAL(u32, Capacity, (PageSize - sizeof(NextChunk) - sizeof(NumBlocks)) / sizeof(FBlock));
        TStaticArray<FBlock, Capacity> Blocks{};

        NODISCARD TMemoryView<const FBlock> MakeView() const {
            return Blocks.MakeView().CutBefore(NumBlocks);
        }
    };

    STATIC_CONST_INTEGRAL(size_t, ChunkSize, sizeof(FChunk));
    STATIC_ASSERT(ChunkSize == PageSize);
    STATIC_ASSERT(sizeof(FFreePage) == PageSize);

#if USE_PPE_MEMORYDOMAINS
    NODISCARD static FMemoryTracking& TrackingData() NOEXCEPT {
        return MEMORYDOMAIN_TRACKING_DATA(PageAllocator);
    }
#endif

    FCriticalSection Barrier;
    FChunk* Chunks{ nullptr };

    std::atomic<FFreePage*> FreePages{ nullptr };

#if USE_PPE_ASSERT
    std::atomic<i32> NumFreePages{ 0 };
    std::atomic<i32> NumUsedPages{ 0 };
#endif

    NODISCARD static FVirtualPageCache_& Get() {
        ONE_TIME_INITIALIZE(TInitSegAlloc<FVirtualPageCache_>, GInstance, 200);
        return GInstance;
    }

    NO_INLINE ~FVirtualPageCache_() {
        ReleaseAll();
    }

    NODISCARD void* Allocate() {
        void* freePage = FindFreePage_();
        if (Unlikely(nullptr == freePage))
            freePage = AllocateNewPage_();

        ONLY_IF_MEMORYDOMAINS(TrackingData().AllocateUser(PageSize));
        return freePage;
    }

    void Deallocate(void* p) {
        Assert(p);
        ONLY_IF_MEMORYDOMAINS(TrackingData().DeallocateUser(PageSize));
        ReleaseUsedPages_(p, p);
    }

    void ReleaseAll() {
        const FCriticalScope scopeLock{ &Barrier };

        // first need to acquire all the queue
        FFreePage* freePages = nullptr;
        for (freePages = FreePages.load(std::memory_order_relaxed);;) {
            if (FreePages.compare_exchange_weak(freePages, nullptr,
                std::memory_order_release, std::memory_order_relaxed))
                break;
        }

        Assert_NoAssume(NumUsedPages == 0);

        // optional sanity checks on free list
#if USE_PPE_ASSERT
        for (FFreePage* p = freePages; p; p = p->NextPage)
            Assert_Lightweight(ValidatePage_(p));
#endif

        // then collect all blocks to release (because chunks are allocated inside)
        FFreePage* chunkBlocks = nullptr;
        for (const FChunk* chunk = Chunks; chunk; chunk = chunk->NextChunk) {
            for (const FBlock& block : chunk->MakeView()) {
#if USE_PPE_ASSERT
                const i32 numPagesReleased = checked_cast<i32>(block.UsedPages().Count());
                Assert_NoAssume(NumFreePages.fetch_add(-numPagesReleased, std::memory_order_relaxed) >= numPagesReleased);
#endif

                if (Likely(Meta::IsPow2(block.UsedPages().Count()))) {
                    BlockFree_(block.Alloc());
                    continue;
                }

                auto* const chunkBlock = reinterpret_cast<FFreePage*>(block.Alloc());
                chunkBlock->NextPage = chunkBlocks;
                chunkBlocks = chunkBlock;
            }
        }

        Chunks = nullptr;
        Assert_NoAssume(NumFreePages.load(std::memory_order_relaxed) == 0);

        // finally release all memory hold by chunks
        while (chunkBlocks) {
            ONLY_IF_MEMORYDOMAINS(TrackingData().DeallocateUser(PageSize));
            auto* const nextBlock = chunkBlocks->NextPage;
            BlockFree_(chunkBlocks);
            chunkBlocks = nextBlock;
        }
    }

private:
    NODISCARD static void* BlockAlloc_() {
        return FVirtualMemory::InternalAlloc(FPlatformMemory::AllocationGranularity ARGS_IF_MEMORYDOMAINS(TrackingData()));
    }

    static void BlockFree_(void* p) {
        return FVirtualMemory::InternalFree(p, FPlatformMemory::AllocationGranularity ARGS_IF_MEMORYDOMAINS(TrackingData()));
    }

    NODISCARD FORCE_INLINE void* FindFreePage_() {
        i32 backoff = 0;
        for (FFreePage* p = FreePages.load(std::memory_order_relaxed); p;) {
            if (FreePages.compare_exchange_weak(p, p->NextPage,
                    std::memory_order_release, std::memory_order_relaxed)) {
                Assert_NoAssume(NumFreePages.fetch_add(-1, std::memory_order_relaxed) > 0);
                ONLY_IF_ASSERT(NumUsedPages.fetch_add(1, std::memory_order_relaxed));
                return p;
            }

            FPlatformProcess::SleepForSpinning(backoff);
        }

        return nullptr;
    }

    NODISCARD FORCE_INLINE void ReleaseUsedPages_(void* pfirst, void* plast) {
        auto* const first = static_cast<FFreePage*>(pfirst);
        auto* const last = static_cast<FFreePage*>(plast);

        i32 backoff = 0;
        for (last->NextPage = FreePages.load(std::memory_order_relaxed);;) {
            if (FreePages.compare_exchange_weak(last->NextPage, static_cast<FFreePage*>(first),
                    std::memory_order_release, std::memory_order_relaxed)) {
                ONLY_IF_ASSERT(const i32 numPages = checked_cast<i32>((last + 1) - first));
                Assert_NoAssume(NumUsedPages.fetch_add(-numPages, std::memory_order_relaxed) >= numPages);
                ONLY_IF_ASSERT(NumFreePages.fetch_add(numPages, std::memory_order_relaxed));
                break;
            }

            FPlatformProcess::SleepForSpinning(backoff);
        }
    }

    NODISCARD NO_INLINE void* AllocateNewPage_() {
        const FCriticalScope scopeLock{&Barrier};

        // another thread might have allocated a block already
        if (void* const freePage = FindFreePage_())
            return freePage;

        Assert_NoAssume(NumFreePages.load(std::memory_order_relaxed) == 0);

        // get a new virtual memory block
        FBlock newBlock{};
        newBlock.Reset(BlockAlloc_(), FPlatformMemory::AllocationGranularity);

        auto newPages = newBlock.UsedPages();
        Assert_NoAssume(Meta::IsPow2(newPages.Count()));

        // check if we need a new chunk for the new block
        if (Unlikely(!Chunks || Chunks->NumBlocks == FChunk::Capacity)) {
            Verify(0 == newPages.PopFront_AssumeNotEmpty());
            ONLY_IF_MEMORYDOMAINS(TrackingData().AllocateUser(PageSize));
            Assert_NoAssume(not Meta::IsPow2(newPages.Count()));

            auto* const newChunk = INPLACE_NEW(newBlock.Alloc(), FChunk);
            newChunk->NextChunk = Chunks;
            Chunks = newChunk;
        }

        // register new block in current chunk
        Assert(Chunks);
        Assert(Chunks->NumBlocks < FChunk::Capacity);
        newBlock.SetUsedPages(newPages);
        Chunks->Blocks[Chunks->NumBlocks++] = newBlock;

        // reserve first page for callee
        ONLY_IF_ASSERT(NumUsedPages.fetch_add(newPages.Count(), std::memory_order_relaxed));
        void* const result = newBlock.PageAddress(newPages.PopFront_AssumeNotEmpty());

        // register all remaining free pages from new block
        auto* const first = static_cast<FFreePage*>(newBlock.PageAddress(newPages.FirstBitSet_Unsafe()));
        auto* const last = first + (newPages.Count() - 1);

        for (auto* page = first; page != last; ++page) {
            Assert_NoAssume(Meta::IsAligned(PageSize, page));
#if USE_PPE_MEMORY_DEBUGGING
            Assert_Lightweight(ValidatePage_(page));
#endif
            page->NextPage = page + 1;
        }

        last->NextPage = nullptr;
#if USE_PPE_MEMORY_DEBUGGING
        Assert_NoAssume(ValidatePage_(last));
#endif

        ReleaseUsedPages_(first, last);

        // finally return a new free block
        return result;
    }


    NODISCARD bool ValidatePage_(const FFreePage* const page) const {
        const FCriticalScope scopeLock{ &Barrier };

        bool bValidPage = false;

        for (FChunk* chunk = Chunks; !bValidPage && chunk; chunk = chunk->NextChunk) {
            reverseforrange(b, 0, chunk->NumBlocks) {
                const FBlock& block = chunk->Blocks[b];
                const u32 first = block.UsedPages().FirstBitSet_AssumeNotEmpty();
                const u32 count = block.UsedPages().Count();
                Assert_NoAssume((first + count) * PageSize == FPlatformMemory::AllocationGranularity);

                if (FPlatformMemory::Memaliases(chunk->Blocks[b].Alloc() + first * PageSize, count * PageSize, page)) {
                    bValidPage = true;
                    break;
                }
            }
        }


        return bValidPage;
    }

};
//----------------------------------------------------------------------------
PRAGMA_MSVC_WARNING_POP() // Padding
#endif //!USE_PPE_VIRTUALPAGE_CACHE
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FAllocatorBlock FPageAllocator::Allocate(size_t s) {
    Assert(PageSize == s);

    FAllocatorBlock blk;
    blk.SizeInBytes = s;
#if USE_PPE_VIRTUALPAGE_CACHE
    blk.Data = FVirtualPageCache_::Get().Allocate();
#else
    blk.Data = FPlatformMemory::SystemAlignedMalloc(PageSize, PageSize);
#endif

    Assert(blk.Data);
    Assert_NoAssume(Meta::IsAlignedPow2(PageSize, blk.Data));
    return blk;
}
//----------------------------------------------------------------------------
void FPageAllocator::Deallocate(FAllocatorBlock blk) {
    Assert(blk.Data);
    Assert(PageSize == blk.SizeInBytes);
    Assert_NoAssume(Meta::IsAlignedPow2(PageSize, blk.Data));

#if USE_PPE_VIRTUALPAGE_CACHE
    FVirtualPageCache_::Get().Deallocate(blk.Data);
#else
    FPlatformMemory::SystemAlignedFree(blk.Data, PageSize);
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
