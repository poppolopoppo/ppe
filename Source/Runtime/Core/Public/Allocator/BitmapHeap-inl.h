#pragma once

#include "Allocator/BitmapHeap.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <u32 _PageSize>
inline bool TBitmapPage<_PageSize>::Aliases(const void* ptr) const NOEXCEPT {
    Assert(ptr);
    return ((uintptr_t)ptr >= (uintptr_t)vAddressSpace &&
            (uintptr_t)ptr < (uintptr_t)vAddressSpace + PageSize * MaxPages);
}
//----------------------------------------------------------------------------
template <u32 _PageSize>
inline u32 TBitmapPage<_PageSize>::Available() const NOEXCEPT {
    return u32(FPlatformMaths::popcnt(Pages.load(std::memory_order_relaxed)) * PageSize);
}
//----------------------------------------------------------------------------
template <u32 _PageSize>
inline u32 TBitmapPage<_PageSize>::Capacity() const NOEXCEPT {
    return (MaxPages * PageSize);
}
//----------------------------------------------------------------------------
template <u32 _PageSize>
inline bool TBitmapPage<_PageSize>::Empty() const NOEXCEPT {
    return (Sizes.load(std::memory_order_relaxed) == 0);
}
//----------------------------------------------------------------------------
template <u32 _PageSize>
inline u32 TBitmapPage<_PageSize>::LargestBlockAvailable() const NOEXCEPT {
    return (FPlatformMaths::ContiguousBits(Pages.load(std::memory_order_relaxed)) * PageSize);
}
//----------------------------------------------------------------------------
template <u32 _PageSize>
inline float TBitmapPage<_PageSize>::ExternalFragmentation() const NOEXCEPT {
    const u32 avail = Available();
    return (avail ? (100.f - (LargestBlockAvailable() * 100.f) / avail) : 0.f);
}
//----------------------------------------------------------------------------
template <u32 _PageSize>
inline size_t TBitmapPage<_PageSize>::RegionSize(const void* ptr) const NOEXCEPT {
    Assert(ptr);
    Assert_NoAssume(Aliases(ptr));

    const u32 off = checked_cast<u32>(((u8*)ptr - (u8*)vAddressSpace) / PageSize);
    Assert(off < MaxPages);

    const u32 numPages = (u32(FPlatformMaths::tzcnt(Sizes >> off)) + 1);
    Assert(numPages < MaxPages);
    Assert_NoAssume((Pages.load(std::memory_order_relaxed) & ((mask_t(-1) >> (MaxPages - numPages))) << off) == 0);

    return (numPages * PageSize);
}
//----------------------------------------------------------------------------
template <u32 _PageSize>
inline auto TBitmapPage<_PageSize>::MemoryStats() const NOEXCEPT -> FBitmapMemoryStats {
    Assert(vAddressSpace);

    const u64 pages = Pages.load(std::memory_order_relaxed);
    const u64 sizes = Sizes.load(std::memory_order_relaxed);
    const u32 numPagesAvailable = u32(FPlatformMaths::popcnt(pages));

    return FBitmapMemoryStats{
        u32(FPlatformMaths::popcnt(sizes)),
        FPlatformMaths::ContiguousBits(pages) * PageSize,
        (MaxPages - numPagesAvailable) * PageSize,
        numPagesAvailable * PageSize,
        MaxPages * PageSize
    };
}
//----------------------------------------------------------------------------
template <u32 _PageSize>
inline void* TBitmapPage<_PageSize>::Allocate(u32 sizeInBytes) NOEXCEPT {
    Assert(sizeInBytes);
    Assert_NoAssume(sizeInBytes < Capacity());
    Assert_NoAssume(Meta::IsAligned(PageSize, sizeInBytes));

    const bool ate = Segregate(sizeInBytes); // reverse search order for larger blocks
    const u32 numPages = ((sizeInBytes + PageSize - 1) / PageSize);
    Assert(numPages <= MaxPages);

    mask_t pages = Pages.load(std::memory_order_relaxed);
    for (;;) {
    RETRY_CAS:
        const mask_t blk = (mask_t(-1) >> (MaxPages - numPages));
        mask_t lookup = pages;
        if (ate) {
            lookup = FPlatformMaths::ReverseBits(lookup);
            Assert_NoAssume(FPlatformMaths::ReverseBits(lookup) == pages);
        }

        for (u32 off = checked_cast<u32>(FPlatformMaths::tzcnt(lookup)); (off != MaxPages) & !!(lookup >> off); ++off) {
            if ((blk & (lookup >> off)) == blk) {
                if (ate)
                    off = MaxPages - off - numPages;
                Assert(off + numPages <= MaxPages);

                const mask_t insert = (pages & ~(blk << off));
                if (Pages.compare_exchange_weak(pages, insert, std::memory_order_release, std::memory_order_relaxed)) {
                    Assert(off + numPages - 1 < MaxPages);
                    Sizes |= (mask_t(1) << (off + numPages - 1));
                    void* const result = (reinterpret_cast<u8*>(vAddressSpace) + PageSize * off);
                    Assert_NoAssume(RegionSize(result) == SnapSize(sizeInBytes));
                    return result;
                }

                FPlatformAtomics::ShortSyncWait();
                goto RETRY_CAS;
            }
        }

        Assert_NoAssume(FPlatformMaths::ContiguousBits(pages) < numPages);
        return nullptr;
    }
}
//----------------------------------------------------------------------------
template <u32 _PageSize>
inline bool TBitmapPage<_PageSize>::Free_ReturnIfUnused(void* ptr) NOEXCEPT {
    Assert(ptr);
    Assert_NoAssume(Aliases(ptr));

    const u32 off = checked_cast<u32>(((u8*)ptr - (u8*)vAddressSpace) / PageSize);
    Assert(off < MaxPages);

    const u32 numPages = u32(FPlatformMaths::tzcnt(Sizes.load(std::memory_order_relaxed) >> off) + 1);
    Assert(off + numPages <= MaxPages);

    mask_t pages = Pages.load(std::memory_order_relaxed);
    for (;;) {
        Assert_NoAssume((pages & ((mask_t(-1) >> (MaxPages - numPages)) << off)) == 0);
        const mask_t remove = (pages | ((mask_t(-1) >> (MaxPages - numPages)) << off));

        if (Pages.compare_exchange_weak(pages, remove, std::memory_order_release, std::memory_order_relaxed)) {
            Sizes &= ~(mask_t(1) << (off + numPages - 1));
            Assert_NoAssume(LargestBlockAvailable() >= numPages * PageSize);
            return (remove == mask_t(-1));
        }

        FPlatformAtomics::ShortSyncWait();
    }
}
//----------------------------------------------------------------------------
template <u32 _PageSize>
inline void* TBitmapPage<_PageSize>::Resize(void* ptr, u32 sizeInBytes) NOEXCEPT {
    Assert(ptr);
    Assert(sizeInBytes);
    Assert_NoAssume(Aliases(ptr));
    Assert_NoAssume(sizeInBytes < Capacity());
    Assert_NoAssume(Meta::IsAligned(PageSize, sizeInBytes));

    const u32 off = checked_cast<u32>(((u8*)ptr - (u8*)vAddressSpace) / PageSize);
    Assert(off < MaxPages);

    const u32 newNumPages = (sizeInBytes / PageSize);
    Assert(newNumPages < MaxPages);
    Assert_NoAssume(newNumPages * PageSize == sizeInBytes);
    const u32 oldNumPages = checked_cast<u32>(FPlatformMaths::tzcnt(Sizes.load(std::memory_order_relaxed) >> off) + 1);
    Assert(oldNumPages < MaxPages);
    Assert(off + oldNumPages <= MaxPages);

    AssertRelease(oldNumPages != newNumPages);
    if (off + newNumPages > MaxPages)
        return nullptr;

    const mask_t oblk = (mask_t(-1) >> (MaxPages - oldNumPages)) << off;
    const mask_t nblk = (mask_t(-1) >> (MaxPages - newNumPages)) << off;

    Sizes &= ~(mask_t(1) << (off + oldNumPages - 1));

    mask_t pages = Pages.load(std::memory_order_relaxed);
    for (;;) {
        Assert_NoAssume((pages & oblk) == 0);
        mask_t resize = (pages | oblk);

        if ((nblk & resize) == nblk) {
            resize &= ~nblk;

            if (Pages.compare_exchange_weak(pages, resize, std::memory_order_release, std::memory_order_relaxed)) {
                Sizes |= (mask_t(1) << (off + newNumPages - 1));
                Assert_NoAssume(RegionSize(ptr) == SnapSize(sizeInBytes));
                return ptr;
            }

            FPlatformAtomics::ShortSyncWait();
        }
        else {
            break;
        }
    }

    Sizes |= (mask_t(1) << (off + oldNumPages - 1));
    return nullptr;
}
//----------------------------------------------------------------------------
template <u32 _PageSize>
inline u32 TBitmapPage<_PageSize>::SnapSize(u32 sz) NOEXCEPT {
    Assert(sz);
    STATIC_ASSERT(Meta::IsPow2(PageSize));
    return ((0 == sz) ? u32(0) : (sz + PageSize - u32(1)) & ~(PageSize - u32(1)));
}
//----------------------------------------------------------------------------
template <u32 _PageSize>
inline void TBitmapPage<_PageSize>::DebugInfo(FBitmapPageInfo* pinfo) const NOEXCEPT {
    Assert(pinfo);

    pinfo->vAddressSpace = vAddressSpace;
    pinfo->Pages = Pages.load(std::memory_order_relaxed);
    pinfo->Sizes = Sizes.load(std::memory_order_relaxed);
    pinfo->Stats = MemoryStats();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_MEMORYDOMAINS
template <typename _Traits>
TBitmapHeap<_Traits>::TBitmapHeap() NOEXCEPT
:   Pages(MEMORYDOMAIN_TRACKING_DATA(Bookkeeping))
{}
#endif
//----------------------------------------------------------------------------
template <typename _Traits>
TBitmapHeap<_Traits>::~TBitmapHeap() {
    const FReadWriteLock::FScopeLockWrite scopeWrite(RWLock);
    Pages.DeleteIf([this](uintptr_t key, uintptr_t value) {
        UNUSED(key);

        auto* const page = reinterpret_cast<page_type*>(value);
        Assert_NoAssume(uintptr_t(page->vAddressSpace) == key); // check for leaks

        ReleaseUnusedPage_AssumeLocked_(page);
        return true;
    });
}
//----------------------------------------------------------------------------
template <typename _Traits>
auto TBitmapHeap<_Traits>::Aliases(const void* ptr) const NOEXCEPT -> page_type* {
    const FReadWriteLock::FScopeLockRead scopeRead(RWLock);
    return AliasingPage_AssumeLocked_(ptr);
}
//----------------------------------------------------------------------------
template <typename _Traits>
size_t TBitmapHeap<_Traits>::AllocationSize(const void* ptr) const NOEXCEPT {
    Assert(ptr);

    const FReadWriteLock::FScopeLockRead scopeRead(RWLock);
    page_type* const page = AliasingPage_AssumeLocked_(ptr);
    AssertRelease(page);

    return page->RegionSize(ptr);
}
//----------------------------------------------------------------------------
template <typename _Traits>
auto TBitmapHeap<_Traits>::MemoryStats() const NOEXCEPT -> FBitmapMemoryStats {
    FBitmapMemoryStats mem{};

    const FReadWriteLock::FScopeLockRead scopeRead(RWLock);
    Pages.Foreach([&mem](uintptr_t, uintptr_t value) {
        auto* const page = reinterpret_cast<page_type*>(value);
        mem = mem + page->MemoryStats();
    });

    return mem;
}
//----------------------------------------------------------------------------
template <typename _Traits>
void* TBitmapHeap<_Traits>::Allocate(size_t sizeInBytes) {
    Assert(sizeInBytes >= MinAllocSize);
    Assert(sizeInBytes <= MaxAllocSize);
    Assert_NoAssume(Meta::IsAligned(Granularity, sizeInBytes));

    FHintTLS& hint = FHintTLS::Get();

    void* result = nullptr;

    // first try to fetch from hint or from a page already allocated
    {
        const FReadWriteLock::FScopeLockRead scopeRead(RWLock);

        // use thread local least-recently-used hint cache
#if 0 // trivial: first-fit
        for (const auto& LRU : hint.LRU) {
            if (Likely((LRU.Revision == Revision) & (!!LRU.Page))) {
                result = LRU.Page->Allocate(checked_cast<u32>(sizeInBytes));
                if (Likely(result))
                    return result;
            }
        }
#elif 0 // segregate: try to isolate small blocks from large blocks
        const bool ate = page_type::Segregate(checked_cast<u32>(sizeInBytes));
        for (u32 c = ate ? FHintTLS::CacheSize : 1; ate ? c : c <= FHintTLS::CacheSize; c = ate ? c - 1 : c + 1) {
            const auto& LRU = hint.LRU[c - 1];
            if (Likely((LRU.Revision == Revision) & (!!LRU.Page))) {
                result = LRU.Page->Allocate(checked_cast<u32>(sizeInBytes));
                if (Likely(result))
                    return result;
            }
        }
#elif 0 // address-first-fit: try to allocate from the lowest block first
        page_type* firstFit = nullptr;
        for (const auto& LRU : hint.LRU) {
            if (Likely((LRU.Revision == Revision) & (!!LRU.Page))) {
                if ((LRU.Page->LargestBlockAvailable() >= sizeInBytes) & ((!firstFit) | (firstFit > LRU.Page)))
                    firstFit = LRU.Page;
            }
        }
        if (Likely(firstFit)) {
            result = firstFit->Allocate(checked_cast<u32>(sizeInBytes));
            if (Likely(result))
                return result;
        }
#elif 1 // address-first-fit+segregate: try to allocate from the lowest block first + reverse order for larger blocks
        const bool ate = page_type::Segregate(checked_cast<u32>(sizeInBytes));
        page_type* firstFit = nullptr;
        for (const auto& LRU : hint.LRU) {
            if (Likely((LRU.Revision == Revision) & (!!LRU.Page))) {
                if ((LRU.Page->LargestBlockAvailable() >= sizeInBytes) & ((!firstFit) | (ate ? firstFit < LRU.Page : firstFit > LRU.Page)))
                    firstFit = LRU.Page;
            }
        }
        if (Likely(firstFit)) {
            result = firstFit->Allocate(checked_cast<u32>(sizeInBytes));
            if (Likely(result))
                return result;
        }
#else // cost: try to minimize fragmentation across LRU cache
        u32 bestCost = UINT32_MAX;
        page_type* bestFit = nullptr;
        for (const auto& LRU : hint.LRU) {
            if (Likely((LRU.Revision == Revision) & (!!LRU.Page))) {
                u32 cost = FPlatformMaths::Abs(i32(sizeInBytes) - i32(LRU.Page->LargestBlockAvailable()));
                if (cost < bestCost) {
                    bestCost = cost;
                    bestFit = LRU.Page;
                }
            }
        }
        if (Likely(bestFit)) {
            result = bestFit->Allocate(checked_cast<u32>(sizeInBytes));
            if (Likely(result))
                return result;
        }
#endif

        // then look for a page with free space if the hint didn't work
        if (Likely(page_type* const page = FreePage_AssumeLocked_(checked_cast<u32>(sizeInBytes)))) {
            hint.Push(page, Revision);
            result = page->Allocate(checked_cast<u32>(sizeInBytes));

            if (Likely(result))
                return result;
        }
    }

    // otherwise we might need to commit a new page
    return Meta::unlikely([this, sizeInBytes, &hint]() -> void* {
        const FReadWriteLock::FScopeLockWrite scopeWrite(RWLock);

        // first look again for free space now that we have exclusive access
        page_type* freePage = FreePage_AssumeLocked_(checked_cast<u32>(sizeInBytes));

        // then we try to reserve a new page if not enough memory is available
        if (Unlikely(nullptr == freePage)) {
#if USE_PPE_ASSERT && 1
            FBitmapMemoryStats mem{};
            Pages.Foreach([&mem](uintptr_t, uintptr_t value) {
                auto* const page = reinterpret_cast<page_type*>(value);
                mem = mem + page->MemoryStats();
            });
            Pages.Foreach([&mem](uintptr_t, uintptr_t value) {
                auto* const page = reinterpret_cast<page_type*>(value);
                char buffer[256];
                FFixedSizeTextWriter oss(buffer);
                using mask_t = typename page_type::mask_t;
                const mask_t pages = page->Pages.load(std::memory_order_relaxed);
                const mask_t sizes = page->Sizes.load(std::memory_order_relaxed);
                char tag = 0;
                forrange(b, 0, mask_t(page_type::MaxPages)) {
                    oss << ((pages & (mask_t(1) << b)) ? '.' : char('A' + tag));
                    if (sizes & (mask_t(1) << b)) tag = (tag + 1) & 1;
                }
                oss << "  " << page->LargestBlockAvailable() << Eol << Eos;
                FPlatformDebug::OutputDebug(buffer);
            });
            const float externalFragmentation = (100.f - mem.LargestFreeBlock * 100.f / mem.TotalSizeAvailable);
            char buffer[256];
            FFixedSizeTextWriter oss(buffer);
            oss << "Ask = " << sizeInBytes << " -> " << (sizeInBytes / Granularity) << " pages" << Eol
                << "External fragmentation = " << externalFragmentation << Eol
                << "Largest block = " << mem.LargestFreeBlock << Eol
                << "Total size available = " << mem.TotalSizeAvailable << Eol
                << "Total size committed = " << mem.TotalSizeCommitted << Eol
                << Eos;
            FPlatformDebug::OutputDebug(buffer);
            //Assert_NoAssume(externalFragmentation < 90.f);
            Assert_NoAssume(mem.TotalSizeCommitted < 400*1024*1024);
#endif

            freePage = FBitmapPageCache::GrabPage<Granularity>();
            Assert(freePage);
            Assert_NoAssume(freePage->Pages.load(std::memory_order_relaxed) == mask_type(-1));
            Assert_NoAssume(0 == freePage->Sizes.load(std::memory_order_relaxed));
            Assert_NoAssume(nullptr == freePage->NextPage);

            freePage->vAddressSpace = traits_type::PageReserve(BitmapSize);

            if (Likely(freePage->vAddressSpace)) {
                Assert_NoAssume(Meta::IsAligned(BitmapSize, freePage->vAddressSpace));
                traits_type::PageCommit(freePage->vAddressSpace, BitmapSize);
                Pages.Insert(freePage->vAddressSpace, freePage);
            }
            else {
                FBitmapPageCache::ReleasePage(freePage);
                freePage = nullptr; // OOM
            }
        }

        if (Likely(freePage )) {
            // at this point we should have a new page, without any older allocations
            void* const ptr = freePage->Allocate(checked_cast<u32>(sizeInBytes));
            AssertRelease(ptr);

            hint.Push(freePage, Revision);
            return ptr;
        }

        return nullptr;
    });
}
//----------------------------------------------------------------------------
template <typename _Traits>
void TBitmapHeap<_Traits>::Free(void* ptr, page_type* page/* = nullptr */) {
    Assert(ptr);

    // first release block and check if we need to run GC
    {
        const FReadWriteLock::FScopeLockRead scopeRead(RWLock);

        if (nullptr == page)
            page = AliasingPage_AssumeLocked_(ptr);
        AssertRelease(page);

        if (Unlikely(page->Free_ReturnIfUnused(ptr) && page->NextPage == nullptr)) {
            for (;;) {
                page->NextPage = FreePage.load(std::memory_order_relaxed);
                if (FreePage.compare_exchange_weak(page->NextPage, page, std::memory_order_release, std::memory_order_relaxed))
                    break;

                FPlatformAtomics::ShortSyncWait();
            }
        }
    }

    // then check if we have too much unused pages (always keep 1 ready)
    page_type* freePage = FreePage.load(std::memory_order_relaxed);
    if (Unlikely(freePage != GDummyPage && freePage->NextPage))
        GarbageCollect();
}
//----------------------------------------------------------------------------
template <typename _Traits>
void* TBitmapHeap<_Traits>::Resize(void* ptr, size_t sizeInBytes, page_type* page/* = nullptr */) NOEXCEPT {
    const FReadWriteLock::FScopeLockRead scopeRead(RWLock);

    if (nullptr == page)
        page = AliasingPage_AssumeLocked_(ptr);
    AssertRelease(page);

    return page->Resize(ptr, checked_cast<u32>(sizeInBytes));
}
//----------------------------------------------------------------------------
template <typename _Traits>
void TBitmapHeap<_Traits>::GarbageCollect() {
    if (Likely(RWLock.TryLockWrite())) {
        for (page_type* page = FreePage.load(std::memory_order_relaxed); page != GDummyPage;) {
            page_type* const pnext = page->NextPage;
            page->NextPage = nullptr;

            if (page->Empty()) {
                Pages.Erase(page->vAddressSpace);
                ReleaseUnusedPage_AssumeLocked_(page);
            }

            page = pnext;
        }

        FreePage.store(GDummyPage, std::memory_order_release);
        RWLock.UnlockWrite();
    }
}
//----------------------------------------------------------------------------
template <typename _Traits>
void TBitmapHeap<_Traits>::ForceGarbageCollect() {
    const FReadWriteLock::FScopeLockWrite scopeWrite(RWLock);

    // run GC on every chunk to trim unused committed memory
    Pages.DeleteIf([this](uintptr_t, uintptr_t value) NOEXCEPT -> bool {
        auto* const page = reinterpret_cast<page_type*>(value);
        page->NextPage = nullptr;

        if (page->Empty()) {
            ReleaseUnusedPage_AssumeLocked_(page);
            return true;
        }

        return false;
    });

    FreePage.store(GDummyPage, std::memory_order_release);
}
//----------------------------------------------------------------------------
template <typename _Traits>
template <typename _Each>
size_t TBitmapHeap<_Traits>::EachPage(_Each&& each) const NOEXCEPT {
    const FReadWriteLock::FScopeLockRead scopeRead(RWLock);

    size_t numBlocks = 0;
    Pages.Foreach([&each, &numBlocks](uintptr_t, uintptr_t value) NOEXCEPT {
        numBlocks++;
        auto* const page = reinterpret_cast<page_type*>(value);
        each(   page->vAddressSpace,
                page->Pages.load(std::memory_order_relaxed),
                page->Sizes.load(std::memory_order_relaxed) );
    });

    return numBlocks;
}
//----------------------------------------------------------------------------
template <typename _Traits>
size_t TBitmapHeap<_Traits>::DebugInfo(FBitmapHeapInfo* pinfo) const NOEXCEPT {
    const FReadWriteLock::FScopeLockRead scopeRead(RWLock);

    if (pinfo) {
        pinfo->BitmapSize = BitmapSize;
        pinfo->Granularity = Granularity;
        pinfo->PagesPerBlock = page_type::MaxPages;
        pinfo->Stats = FBitmapMemoryStats{};
    }

    size_t numPages = 0;
    Pages.Foreach([&pinfo, &numPages](uintptr_t, uintptr_t value) {
        auto* const page = reinterpret_cast<page_type*>(value);

        if (pinfo && numPages < pinfo->Pages.size()) {
            FBitmapPageInfo& pageInfo = pinfo->Pages[numPages];
            page->DebugInfo(&pageInfo);
            pinfo->Stats = pinfo->Stats + pageInfo.Stats;
        }

        numPages++;
    });

    return numPages;
}
//----------------------------------------------------------------------------
template <typename _Traits>
auto TBitmapHeap<_Traits>::AliasingPage_AssumeLocked_(const void* ptr) const NOEXCEPT -> page_type* {
    const void* const vAddressSpace = Meta::RoundToPrev(ptr, BitmapSize);

    FHintTLS& hint = FHintTLS::Get();
    for (const auto& LRU : hint.LRU) {
        if ((LRU.Revision == Revision) & (!!LRU.Page)) {
            if (LRU.Page->vAddressSpace == vAddressSpace)
                return LRU.Page;
        }
    }

    page_type* page;
    if (Pages.Find(reinterpret_cast<void**>(&page), vAddressSpace)) {
        Assert(page);
        Assert_NoAssume(page->Aliases(ptr));
        hint.Push(page, Revision);
        return page;
    }

    return nullptr;
}
//----------------------------------------------------------------------------
template <typename _Traits>
auto TBitmapHeap<_Traits>::FreePage_AssumeLocked_(u32 sizeInBytes) NOEXCEPT -> page_type* {
    Assert(sizeInBytes);
    Assert_NoAssume(Meta::IsAligned(Granularity, sizeInBytes));

    page_type* freePage = nullptr;
    Pages.Where([this, sizeInBytes, &freePage](uintptr_t, uintptr_t value) NOEXCEPT -> bool {
        auto* const page = reinterpret_cast<page_type*>(value);
        if (page->LargestBlockAvailable() >= sizeInBytes) {
            freePage = page;
            return true;
        }
        return false;
    });

    return freePage;
}
//----------------------------------------------------------------------------
template <typename _Traits>
void TBitmapHeap<_Traits>::ReleaseUnusedPage_AssumeLocked_(page_type* page) NOEXCEPT {
    Assert(page);
    Assert(page->vAddressSpace);
    Assert_NoAssume(page->NextPage == nullptr);
    Assert_NoAssume(page->Empty());
    Assert_NoAssume(page->Pages.load(std::memory_order_relaxed) == mask_type(-1));

    ++Revision; // invalidate each TLS cache

    traits_type::PageDecommit(page->vAddressSpace, BitmapSize);
    traits_type::PageRelease(page->vAddressSpace, BitmapSize);

    ONLY_IF_ASSERT(page->vAddressSpace = nullptr);

    FBitmapPageCache::ReleasePage(page);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
