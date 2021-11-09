#pragma once

#include "Allocator/MipMapAllocator.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline bool FMipmapPage::AliasesToMipMap(const FPaging& page, const void* ptr) const NOEXCEPT {
    Assert(ptr);
    Assert(page.SizeInBytes);
    return ((uintptr_t)ptr >= (uintptr_t)vAddressSpace &&
            (uintptr_t)ptr < (uintptr_t)vAddressSpace + page.SizeInBytes * NumMips * MaxBlocks);
}
//----------------------------------------------------------------------------
inline size_t FMipmapPage::AllocationSize(const FPaging& page, const void* ptr) const NOEXCEPT {
    Assert(ptr);
    Assert(page.SizeInBytes);
    Assert_NoAssume(AliasesToMipMap(page, ptr));

    const uintptr_t vptr = ((u8*)ptr - (u8*)vAddressSpace);
    const u32 mip = checked_cast<u32>(vptr / (page.SizeInBytes * NumMips));
    const FBlock& block = Blocks[mip];

    const u32 idx = checked_cast<u32>((vptr - mip * (page.SizeInBytes * NumMips)) / page.SizeInBytes);
    Assert_NoAssume(FPlatformMaths::popcnt64(block.Size & SizeMasks[idx]) == 1);

    const u32 bit = FirstBitSet(block.Size & SizeMasks[idx]);
    const u32 lvl = FPlatformMaths::FloorLog2(bit + 1);

    return (size_t(page.SizeInBytes * NumMips) / (size_t(1) << lvl));
}
//----------------------------------------------------------------------------
inline auto FMipmapPage::MemoryStats(const FPaging& page) const NOEXCEPT -> FMemoryStats {
    FMemoryStats mem{};

    forrange(blk, 0, NumBlocks) {
        mem.TotalNumAllocations += checked_cast<u32>(FPlatformMaths::popcnt(Blocks[blk].Size.load(std::memory_order_relaxed)));
        mem.TotalAvailableSize += AvailableMips(blk);
    }

    mem.TotalCommittedSize = NumBlocks * NumMips * page.SizeInBytes;
    mem.TotalAvailableSize *= page.SizeInBytes;
    mem.TotalAllocatedSize = mem.TotalCommittedSize - mem.TotalAllocatedSize;

    return mem;
}
//----------------------------------------------------------------------------
inline u32 FMipmapPage::AvailableMips(u32 block) const NOEXCEPT {
    Assert(block < NumBlocks);
    return checked_cast<u32>(FPlatformMaths::popcnt(
        Blocks[block].Mips.load(std::memory_order_relaxed) & LevelMasks[lengthof(LevelMasks) - 1]) );
}
//----------------------------------------------------------------------------
inline bool FMipmapPage::AvailableBlock_AssumeLocked(u32* pblock, const FPaging& page, u32 select) {
    // first try without locking
    FBitMask mask{ LiveSet.load(std::memory_order_acquire) & select };
    if (Likely(mask.AnyTrue())) {
        // use an already committed block
        *pblock = checked_cast<u32>(mask.FirstBitSet_AssumeNotEmpty());
        Assert_NoAssume(*pblock < NumBlocks);
        return true;
    }
    // ok now we might need to commit *ONE* new page
    else {
        select &= ~mask; // disable pages already tested

        const FAtomicSpinLock::FScope scopeLock(GrowthBarrier);

        // first check that nobody allocated a new page before us
        mask = FBitMask{ LiveSet.load(std::memory_order_relaxed) & select };
        if (Likely(mask.AnyTrue())) {
            // use an already committed block
            *pblock = checked_cast<u32>(mask.FirstBitSet_AssumeNotEmpty());
            Assert_NoAssume(*pblock < NumBlocks);
            return true;
        }

        // now only one page will be committed for every worker
        return Meta::unlikely([this, &page, pblock]() -> bool {
            Assert_NoAssume(CheckSanity());
            if (NumBlocks < MaxBlocks) {
                // commit a new block if any available
                if (Likely(NumBlocks < MaxBlocks)) {
                    const u32 blk = NumBlocks;
                    FBlock& block = Blocks[blk];
                    block.Mips.store(EmptyMips, std::memory_order_relaxed);
                    block.Size.store(EmptySize, std::memory_order_relaxed);

                    page.fCommit((u8*)vAddressSpace + blk * (NumMips * page.SizeInBytes), NumMips * page.SizeInBytes);

                    ++NumBlocks;
                    ++NumUnused;
                    EnableBlock(blk);

                    *pblock = blk;
                    return true;
                }
            }
            return false; // OOM
        });
    }
}
//----------------------------------------------------------------------------
inline void* FMipmapPage::Allocate(const FPaging& page, std::atomic<i32>* pUnusedPages, u32 sizeInBytes, u32 blockHint/* = 0 */) {
    Assert(pUnusedPages);
    Assert(sizeInBytes);
    Assert_NoAssume(sizeInBytes <= NumMips * page.SizeInBytes);
    AssertRelease_NoAssume(Meta::IsAlignedPow2(page.SizeInBytes, sizeInBytes));

    const u32 lvl = MipLevel(sizeInBytes, page.SizeInBytes);
    const u32 fst = MipOffset(lvl);
    const u64 msk = LevelMasks[lvl];

    for (u32 select = FLiveMask::AllMask, blk = blockHint;;) {
        if (Likely(blk < NumBlocks)) {
            FBlock& block = Blocks[blk];
            Assert_NoAssume(DeletedMask != block.Mips);

            for (i32 backoff = 0;;) {
                u64 mip = block.Mips.load(std::memory_order_relaxed);
                Assert_NoAssume(DeletedMask != mip);

                const u64 avail = mip & msk;
                if (Unlikely(not avail))
                    break;

                const u32 bit = FirstBitSet(avail);
                Assert_NoAssume(mip & ~SetMasks[bit]);
                Assert_NoAssume(mip != (mip & SetMasks[bit]));

                const u64 upd = mip & SetMasks[bit];
                if (block.Mips.compare_exchange_weak(mip, upd,
                    std::memory_order_release, std::memory_order_relaxed)) {
                    Assert_NoAssume((block.Mips & ~SetMasks[bit]) == 0);
                    Assert_NoAssume((block.Size & (u64(1) << bit)) == 0);

                    block.Size |= (u64(1) << bit);

                    // book-keeping for full/empty mipmaps

                    if (Unlikely(FullMips == upd))
                        DisableBlock(blk);
                    else if (Unlikely(EmptyMips == mip)) {
                        const u32 numUnused = NumUnused--;
                        if (Unlikely(numUnused == NumBlocks))
                            --*pUnusedPages;
                        else
                            Assert_NoAssume(numUnused <= NumBlocks);
                    }

                    const u32 off = (bit - fst) * u32((NumMips * page.SizeInBytes) / (u32(1) << lvl));
                    Assert_NoAssume(FPlatformMaths::popcnt64(block.Size & SizeMasks[off / page.SizeInBytes]) == 1);

                    ONLY_IF_MEMORYDOMAINS(page.TrackingData.AllocateUser(size_t(page.SizeInBytes * NumMips) / (size_t(1) << lvl)));

                    return ((u8*)vAddressSpace + (blk * (NumMips * page.SizeInBytes) + off));
                }

                FPlatformProcess::SleepForSpinning(backoff);
            }

            select &= ~(u32(1) << blk);
        }

        if (Unlikely(not AvailableBlock_AssumeLocked(&blk, page, select)))
            break;
    }

    return nullptr;
}
//----------------------------------------------------------------------------
inline bool FMipmapPage::Free_ReturnIfUnused(const FPaging& page, void* ptr) {
    Assert(ptr);
    Assert_NoAssume(AliasesToMipMap(page, ptr));

    const intptr_t rel = ((u8*)ptr - (u8*)vAddressSpace);
    const u32 blk = checked_cast<u32>(rel / (NumMips * page.SizeInBytes));
    const u32 idx = checked_cast<u32>((rel - blk * (NumMips * page.SizeInBytes)) / page.SizeInBytes);

    FBlock& block = Blocks[blk];

    Assert(blk < NumBlocks);
    Assert_NoAssume(FPlatformMaths::popcnt64(block.Size & SizeMasks[idx]) == 1);

    const u32 bit = FirstBitSet(block.Size & SizeMasks[idx]);
    Assert_NoAssume((block.Mips & ~SetMasks[bit]) == 0);
    Assert_NoAssume(block.Size & (u64(1) << bit));

    // first clear the allocated bit from the SizeMask,
    // so we can't allocate from MipMask when SizeMask is still set
    block.Size &= ~(u64(1) << bit);

    u64 mip = block.Mips.load(std::memory_order_relaxed); // <== updated by compare_exchange_weak
    for (i32 backoff = 0;;) {
        Assert_NoAssume((mip & ~SetMasks[bit]) == 0);

        const u64 upd = UnsetMask(mip, bit);
        Assert_NoAssume(mip != upd);

        if (block.Mips.compare_exchange_weak(mip, upd,
            std::memory_order_release, std::memory_order_relaxed)) {

            // enable mips again if it was full previously
            bool runGCAfterFree = false;
            if (Unlikely(mip == FullMips))
                EnableBlock(blk);
            // garbage collect free mips if more than 4 are completely empty
            else if (Unlikely(upd == EmptyMips)) {
                const u32 numUnused = ++NumUnused;
                runGCAfterFree = (numUnused == NumBlocks);
                Assert_NoAssume(numUnused <= NumBlocks);
            }

#if USE_PPE_MEMORYDOMAINS
            const u32 lvl = FPlatformMaths::FloorLog2(bit + 1);
            page.TrackingData.DeallocateUser(size_t(page.SizeInBytes * NumMips) / (size_t(1) << lvl));
#endif

            return runGCAfterFree;
        }

        FPlatformProcess::SleepForSpinning(backoff);
    }
}
//----------------------------------------------------------------------------
inline void* FMipmapPage::Resize(const FPaging& page, void* ptr, u32 sizeInBytes) NOEXCEPT {
Assert(ptr);
    Assert(sizeInBytes);
    Assert_NoAssume(AliasesToMipMap(page, ptr));
    Assert_NoAssume(sizeInBytes <= NumMips * page.SizeInBytes);
    Assert_NoAssume(Meta::IsAlignedPow2(page.SizeInBytes, sizeInBytes));

    const intptr_t rel = ((u8*)ptr - (u8*)vAddressSpace);
    const u32 blk = checked_cast<u32>(rel / (NumMips * page.SizeInBytes));
    const u32 idx = checked_cast<u32>((rel - blk * (NumMips * page.SizeInBytes)) / page.SizeInBytes);

    const u32 nlvl = MipLevel(sizeInBytes, page.SizeInBytes);
    const u64 nmsk = (LevelMasks[nlvl] & SizeMasks[idx]); // don't move the pointer
    if (Unlikely(0 == nmsk))
        return nullptr; // can't grow this block (by design)

    Assert(blk < NumBlocks);
    FBlock& block = Blocks[blk];

    Assert_NoAssume(FPlatformMaths::popcnt64(block.Size & SizeMasks[idx]) == 1);

    const u32 obit = FirstBitSet(block.Size & SizeMasks[idx]);
    Assert_NoAssume((block.Mips & ~SetMasks[obit]) == 0);
    Assert_NoAssume(block.Size & (u64(1) << obit));

    const u32 olvl = FPlatformMaths::FloorLog2(obit + 1);
    if (Unlikely(olvl == nlvl))
        return ptr; // same mip level, no need to reallocate

    // first clear the allocated bit from the SizeMask,
    // so we can't allocate from MipMask when SizeMask is still set
    block.Size &= ~(u64(1) << obit);

    for (i32 backoff = 0;;) {
        u64 mip = block.Mips.load(std::memory_order_relaxed);
        Assert_NoAssume(DeletedMask != mip);

        // reset old block mask
        u64 upd = UnsetMask(mip, obit);

        // try to allocate the new block (shrink or growth)
        const u64 avail = upd & nmsk;
        if (Unlikely(not avail))
            break; // not enough space

        const u32 nbit = FirstBitSet(avail);
        Assert(nbit != obit);
        Assert_NoAssume(upd & ~SetMasks[nbit]);
        Assert_NoAssume(upd != (upd & SetMasks[nbit]));

        upd &= SetMasks[nbit];
        if (block.Mips.compare_exchange_weak(mip, upd,
            std::memory_order_release, std::memory_order_relaxed)) {
            Assert_NoAssume((block.Mips & ~SetMasks[nbit]) == 0);
            Assert_NoAssume((block.Size & (u64(1) << nbit)) == 0);

            block.Size |= (u64(1) << nbit);

            // book-keeping for full/empty mipmaps
            if (Unlikely(FullMips == upd))
                DisableBlock(blk);
            Assert_NoAssume(EmptyMips != mip);

#if USE_PPE_ASSERT
            const u32 nfst = MipOffset(nlvl);
            const u32 noff = (nbit - nfst) * u32((NumMips * page.SizeInBytes) / (u32(1) << nlvl));
            Assert_NoAssume(FPlatformMaths::popcnt64(block.Size & SizeMasks[noff / page.SizeInBytes]) == 1);
            Assert_NoAssume(((u8*)vAddressSpace + (blk * page.SizeInBytes  * NumMips + noff)) == ptr);
#endif

            ONLY_IF_MEMORYDOMAINS(page.TrackingData.DeallocateUser(size_t(page.SizeInBytes * NumMips) / (size_t(1) << olvl)));
            ONLY_IF_MEMORYDOMAINS(page.TrackingData.AllocateUser(size_t(page.SizeInBytes * NumMips) / (size_t(1) << nlvl)));

            return ptr;
        }

        FPlatformProcess::SleepForSpinning(backoff);
    }

    Assert_NoAssume(((NumMips * page.SizeInBytes) / (u32(1) << olvl)) < sizeInBytes); // verify we fail only when growing
    block.Size |= (u64(1) << obit); // restore previous size mask before leaving
    return nullptr; // failed to grow
}
//----------------------------------------------------------------------------
inline bool FMipmapPage::GarbageCollect_AssumeLocked(const FPaging& page) {
    // will release mip maps ATE of committed space IFP
    // this is locking all other threads attempting to allocate
    // sadly due to fragmentation blocks maybe too scattered to release a
    // substantial amount of virtual memory

    const u32 blockSize = NumMips * page.SizeInBytes;

    u32 numDecommittedBlocks = 0;
    for (u32 blk = (NumBlocks - 1); blk; --blk) {
        FBlock& block = Blocks[blk];
        if (block.Mips.load(std::memory_order_relaxed) == EmptyMips) {
            Assert_NoAssume(0 == block.Size);
#if USE_PPE_DEBUG
            block.Mips = DeletedMask;
            block.Size = DeletedMask;
#endif

            page.fDecommit((u8*)vAddressSpace + blk * blockSize, blockSize);

            DisableBlock(blk);
            ++numDecommittedBlocks;
        }
        else {
            break;
        }
    }

    if (numDecommittedBlocks) {
        NumBlocks -= numDecommittedBlocks;
        Assert_NoAssume(NumBlocks <= MaxBlocks/* unsigned overflow */);
        if ((NumUnused -= numDecommittedBlocks) == NumBlocks)
            return true;

        Assert_NoAssume(NumUnused <= MaxBlocks/* unsigned overflow */);
    }

    return (NumUnused == NumBlocks);
}
//----------------------------------------------------------------------------
template <typename _Each>
u32 FMipmapPage::EachBlock(const FPaging& page, const _Each& each) const NOEXCEPT {
    // used for fragmentation diagnostic:

    // convert the mip mask to a 32 bits mask (ie lowest mip)
    const u32 n = NumBlocks;
    forrange(blk, 0, NumBlocks) {
        const u64 mipMask = Blocks[blk].Mips.load(std::memory_order_relaxed);
        const u64 sizeMask = Blocks[blk].Size.load(std::memory_order_relaxed);

        u32 allocationMask = 0;
        forrange(mip, 0, NumMips)
            allocationMask |= (sizeMask & SizeMasks[mip] ? 1 : 0) << mip;

        u32 committedMask = ~u32(mipMask >> u64(31));

        u32 lowestFreeLevel = lengthof(LevelMasks);
        forrange(lvl, 0, u32(lengthof(LevelMasks))) {
            if (mipMask & LevelMasks[lvl])
                lowestFreeLevel = Min(lowestFreeLevel, lvl);
        }

        u32 largestFreeBlockSize = (lowestFreeLevel < lengthof(LevelMasks)
            ? size_t(page.SizeInBytes * NumMips) / (size_t(1) << lowestFreeLevel)
            : 0 );

        each(blk, largestFreeBlockSize, allocationMask, committedMask);
    }

    return n;
}
//----------------------------------------------------------------------------
inline u32 FMipmapPage::SnapSize(u32 sizeInBytes, u32 mipSize) NOEXCEPT {
    Assert(sizeInBytes);

    const u32 lvl = MipLevel(sizeInBytes, mipSize);
    const u32 snapped = ((NumMips * mipSize) / (u32(1) << lvl));

    Assert_NoAssume(snapped >= sizeInBytes);
    return snapped;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_MEMORYDOMAINS
template <typename _Traits>
TMipMapAllocator2<_Traits>::TMipMapAllocator2() NOEXCEPT
:   Pages(traits_type::TrackingData())
{}
#endif
//----------------------------------------------------------------------------
template <typename _Traits>
TMipMapAllocator2<_Traits>::~TMipMapAllocator2() {
    const FReadWriteLock::FScopeLockWrite scopeWrite(RWLock);
    Pages.Foreach([](uintptr_t key, uintptr_t value) {
        UNUSED(key);

        FMipmapPage* const page = reinterpret_cast<FMipmapPage*>(value);
        Assert_NoAssume(uintptr_t(page->vAddressSpace) == key);
        Assert_NoAssume(0 == page->NumBlocks);

        traits_type::PageRelease(page->vAddressSpace, PageSize);
        FMipmapPage::ReleaseBusyPage(page);
    });
}
//----------------------------------------------------------------------------
template <typename _Traits>
FMipmapPage* TMipMapAllocator2<_Traits>::AliasingMipMap(const void* ptr) const NOEXCEPT {
    const FReadWriteLock::FScopeLockRead scopeRead(RWLock);
    return AliasingMipMap(Pages, ptr);
}
//----------------------------------------------------------------------------
template <typename _Traits>
size_t TMipMapAllocator2<_Traits>::AllocationSize(const void* ptr) const NOEXCEPT {
    Assert(ptr);

    const FReadWriteLock::FScopeLockRead scopeRead(RWLock);
    FMipmapPage* const page = AliasingMipMap(Pages, ptr);
    AssertRelease(page);

    return page->AllocationSize(MakePaging(), ptr);
}
//----------------------------------------------------------------------------
template <typename _Traits>
auto TMipMapAllocator2<_Traits>::MemoryStats() const NOEXCEPT -> FMemoryStats {
    FMemoryStats mem{};

    const FReadWriteLock::FScopeLockRead scopeRead(RWLock);
    Pages.Foreach([&mem](uintptr_t, uintptr_t value) {
        auto* const page = reinterpret_cast<FMipmapPage*>(value);
        FMemoryStats tmp = page->MemoryStats(MakePaging());
        mem.TotalNumAllocations += tmp.TotalNumAllocations;
        mem.TotalAllocatedSize += tmp.TotalAllocatedSize;
        mem.TotalAvailableSize += tmp.TotalAvailableSize;
        mem.TotalCommittedSize += tmp.TotalCommittedSize;
    });

    return mem;
}
//----------------------------------------------------------------------------
template <typename _Traits>
void* TMipMapAllocator2<_Traits>::Allocate(size_t sizeInBytes) {
    static THREAD_LOCAL void* HintTLS{ nullptr };
    return Allocate(sizeInBytes, &HintTLS);
}
//----------------------------------------------------------------------------
template <typename _Traits>
void* TMipMapAllocator2<_Traits>::Allocate(size_t sizeInBytes, void** phint) {
    Assert(sizeInBytes);
    Assert(phint);
    Assert_NoAssume(Meta::IsAlignedPow2(traits_type::Granularity, sizeInBytes));

    // first try to fetch from hint or from a page already allocated
    {
        const void* mipmapHint = *phint;

        const FReadWriteLock::FScopeLockRead scopeRead(RWLock);

        // hint is generally the last pointer allocated
        FMipmapPage* __restrict const page = AliasingMipMap(Pages, mipmapHint);
        if (Likely(page)) {
            const u32 blockHint = checked_cast<u32>((uintptr_t(mipmapHint) - uintptr_t(page->vAddressSpace)) / BlockSize);
            void* const ptr = page->Allocate(MakePaging(), &NumUnused, checked_cast<u32>(sizeInBytes), blockHint);
            if (Likely(ptr)) {
                *phint = ptr;
                return ptr;
            }
        }

        // then look for a page with free space if the hint didn't work
        void* const ptr = AllocateFromPages(Pages, NumUnused, checked_cast<u32>(sizeInBytes));
        if (Likely(ptr)) {
            *phint = ptr;
            return ptr;
        }
    }

    // otherwise we might need to commit a new page
    return Meta::unlikely([this, sizeInBytes, phint]() -> void* {
        const FReadWriteLock::FScopeLockWrite scopeWrite(RWLock);

        // first look again for free space now that we have exclusive access
        void* ptr = AllocateFromPages(Pages, NumUnused, checked_cast<u32>(sizeInBytes));

        // secondly we search for a free mipmap page if still OOM
        if (Likely(nullptr == ptr)) {
            Assert_NoAssume(0 == NumUnused.load(std::memory_order_relaxed));

            FMipmapPage* freePage = nullptr;
            std::swap(FreePage, freePage);

            // lastly we try to reserve a new page if everything failed
            if (not freePage) {
                freePage = FMipmapPage::GrabFreePage();
                Assert_NoAssume(0 == freePage->NumBlocks);
                Assert_NoAssume(0 == freePage->LiveSet);
                Assert_NoAssume(0 == freePage->NumUnused);
                freePage->NumBlocks = freePage->NumUnused = freePage->LiveSet = 0;
                freePage->vAddressSpace = traits_type::PageReserve(PageSize);
                AssertRelease(freePage->vAddressSpace);
                Assert_NoAssume(Meta::IsAlignedPow2(PageSize, freePage->vAddressSpace));
            }

            // at this point we should have a new page, without any older allocations
            Assert(freePage);
            Assert_NoAssume(freePage->NumBlocks == freePage->NumUnused);

            ++NumUnused;
            ptr = freePage->Allocate(MakePaging(), &NumUnused, checked_cast<u32>(sizeInBytes));
            Assert(ptr);

            // share the new page only *after* taking our share
            Pages.Insert(freePage->vAddressSpace, freePage);
        }

        *phint = ptr;
        return ptr;
    });
}
//----------------------------------------------------------------------------
template <typename _Traits>
void TMipMapAllocator2<_Traits>::Free(void* ptr, FMipmapPage* page/* = nullptr */) {
    // first release block and check if we need to run GC
    {
        const FReadWriteLock::FScopeLockRead scopeRead(RWLock);

        if (nullptr == page)
            page = AliasingMipMap(Pages, ptr);
        AssertRelease(page);

        if (Unlikely(page->Free_ReturnIfUnused(MakePaging(), ptr))) {
            if (nullptr == page->NextPage) {
                for (i32 backoff = 0;;) {
                    page->NextPage = GCList.load(std::memory_order_relaxed);
                    Assert(page->NextPage);
                    if (GCList.compare_exchange_weak(page->NextPage, page, std::memory_order_release, std::memory_order_relaxed))
                        break;

                    FPlatformProcess::SleepForSpinning(backoff);
                }
            }

            ++NumUnused;
        }
    }
    // then check if we have too much unused pages (always keep 1 ready)
    if (Unlikely(NumUnused.load(std::memory_order_relaxed) > 1))
        GarbageCollect();
}
//----------------------------------------------------------------------------
template <typename _Traits>
void* TMipMapAllocator2<_Traits>::Resize(void* ptr, size_t sizeInBytes, FMipmapPage* page/* = nullptr */) NOEXCEPT {
    const FReadWriteLock::FScopeLockRead scopeRead(RWLock);

    if (nullptr == page)
        page = AliasingMipMap(Pages, ptr);
    AssertRelease(page);

    return page->Resize(MakePaging(), ptr, checked_cast<u32>(sizeInBytes));
}
//----------------------------------------------------------------------------
template <typename _Traits>
void TMipMapAllocator2<_Traits>::GarbageCollect() {
    if (Likely(RWLock.TryLockWrite())) {
        Assert_NoAssume(NumUnused > 0);
        Assert_NoAssume(GCList != GDummyPage);

        for (FMipmapPage* page = GCList.load(std::memory_order_relaxed); page != GDummyPage;) {
            FMipmapPage* const pnext = page->NextPage;
            page->NextPage = nullptr;

            if (Likely(page->GarbageCollect_AssumeLocked(MakePaging()))) {
                ReleaseUnusedPage(&FreePage, NumUnused, page);
                Pages.Erase(page->vAddressSpace);
            }

            page = pnext;
        }

        Assert_NoAssume(0 == NumUnused);
        GCList.store(GDummyPage, std::memory_order_release);

        RWLock.UnlockWrite();
    }
}
//----------------------------------------------------------------------------
template <typename _Traits>
void TMipMapAllocator2<_Traits>::ForceGarbageCollect() {
    const FReadWriteLock::FScopeLockWrite scopeWrite(RWLock);

    // run GC on every chunk to trim unused committed memory
    Pages.DeleteIf([this](uintptr_t, uintptr_t value) -> bool {
        auto* __restrict const page = reinterpret_cast<FMipmapPage*>(value);
        page->NextPage = nullptr;

        if (page->GarbageCollect_AssumeLocked(MakePaging())) {
            ReleaseUnusedPage(&FreePage, NumUnused, page);
            return true;
        }
        return false;
    });

    GCList.store(GDummyPage, std::memory_order_release);
}
//----------------------------------------------------------------------------
template <typename _Traits>
template <typename _Each>
size_t TMipMapAllocator2<_Traits>::EachMipmapBlock(_Each&& each) const NOEXCEPT {
    const FReadWriteLock::FScopeLockRead scopeRead(RWLock);

    size_t numBlocks = 0;
    Pages.Foreach([&each, &numBlocks](uintptr_t, uintptr_t value) NOEXCEPT {
        numBlocks += reinterpret_cast<FMipmapPage*>(value)->EachBlock(MakePaging(), each);
    });

    return numBlocks;
}
//----------------------------------------------------------------------------
template <typename _Traits>
template <typename _Each>
size_t TMipMapAllocator2<_Traits>::EachMipmapPage(_Each&& each) const NOEXCEPT {
    const FReadWriteLock::FScopeLockRead scopeRead(RWLock);

    size_t numPages = 0;
    Pages.Foreach([&each, &numPages](uintptr_t, uintptr_t value) NOEXCEPT {
        each(reinterpret_cast<FMipmapPage*>(value));
        ++numPages;
    });

    return numPages;
}
//----------------------------------------------------------------------------
template <typename _Traits>
FMipmapPage* TMipMapAllocator2<_Traits>::AliasingMipMap(const FCompressedRadixTrie& pages, const void* ptr) NOEXCEPT {
    const void* const vAddressSpace = Meta::RoundToPrevPow2(ptr, PageSize);

    FMipmapPage* page;
    if (pages.Find(reinterpret_cast<void**>(&page), vAddressSpace)) {
        Assert(page);
        Assert_NoAssume(page->AliasesToMipMap(MakePaging(), ptr));
        return page;
    }

    return nullptr;
}
//----------------------------------------------------------------------------
template <typename _Traits>
void* TMipMapAllocator2<_Traits>::AllocateFromPages(const FCompressedRadixTrie& pages, std::atomic<i32>& numUnused, u32 sizeInBytes) NOEXCEPT {
    void* ptr;
    const bool found = pages.Where([&numUnused, sizeInBytes, &ptr](uintptr_t, uintptr_t value) NOEXCEPT{
        auto* const page = reinterpret_cast<FMipmapPage*>(value);
        ptr = page->Allocate(MakePaging(), &numUnused, sizeInBytes);
        return (!!ptr);
    });

    return (found ? ptr : nullptr);
}
//----------------------------------------------------------------------------
template <typename _Traits>
void TMipMapAllocator2<_Traits>::ReleaseUnusedPage(FMipmapPage** pfreePages, std::atomic<i32>& numUnused, FMipmapPage* page) {
    Assert(pfreePages);
    Assert_NoAssume(page->NextPage == nullptr);

    Assert(numUnused > 0);
    numUnused.fetch_sub(1, std::memory_order_relaxed);

    // always one page in buffer when empty for hysteresis
    FMipmapPage* freePage = page;
    std::swap(*pfreePages, freePage);

    // if we have more than one free page then we release it
    if (freePage) {
        AssertRelease_NoAssume(freePage->NumUnused == freePage->NumBlocks);
        Assert(freePage != page);
        Assert_NoAssume(freePage->NumBlocks < 2);
        Assert_NoAssume(freePage->NextPage == nullptr);
        Assert_NoAssume(freePage->LiveSet == (u32(1) << freePage->NumBlocks) - 1);

        // decommit blocks still committed
        forrange(blk, 0, freePage->NumBlocks)
            traits_type::PageDecommit(reinterpret_cast<u8*>(freePage->vAddressSpace) + blk * BlockSize, BlockSize);

        freePage->NumBlocks = 0;
        freePage->NumUnused.store(0, std::memory_order_relaxed);
        freePage->LiveSet.store(0, std::memory_order_relaxed);

        traits_type::PageRelease(freePage->vAddressSpace, PageSize);
#if USE_PPE_ASSERT
        freePage->vAddressSpace = nullptr;
#endif

        FMipmapPage::ReleaseBusyPage(freePage);
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
