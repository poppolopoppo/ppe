﻿#pragma once

#include "Core.h"

#include "HAL/PlatformAtomics.h"
#include "HAL/PlatformMaths.h"
#include "Thread/AtomicSpinLock.h"
#include "Thread/ReadWriteLock.h"

#include <atomic>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FGenericMipMapVMemTraits {
STATIC_CONST_INTEGRAL(size_t, Granularity, 0);
STATIC_CONST_INTEGRAL(size_t, ReservedSize, 0);
static void* PageReserve(size_t sizeInBytes) = delete;
static void PageCommit(void* ptr, size_t sizeInBytes) = delete;
static void PageDecommit(void* ptr, size_t sizeInBytes) = delete;
static void PageRelease(void* ptr, size_t sizeInBytes) = delete;
};
//----------------------------------------------------------------------------
PRAGMA_MSVC_WARNING_PUSH()
PRAGMA_MSVC_WARNING_DISABLE(4324) // 'XXX' structure was padded due to alignment
template <typename _VMemTraits = FGenericMipMapVMemTraits>
class TMallocMipMap : _VMemTraits {
public:
    using vmem_traits = _VMemTraits;

    STATIC_CONST_INTEGRAL(size_t, NumBottomMips, 32); // hard-coded, like the flags bellow
    STATIC_CONST_INTEGRAL(size_t, BottomMipSize, vmem_traits::Granularity);
    STATIC_CONST_INTEGRAL(size_t, TopMipSize, NumBottomMips * BottomMipSize);
    STATIC_CONST_INTEGRAL(size_t, NumMipMaps, vmem_traits::ReservedSize / TopMipSize);

    TMallocMipMap();
    ~TMallocMipMap();

    TMallocMipMap(const TMallocMipMap&) = delete;
    TMallocMipMap& operator =(const TMallocMipMap&) = delete;

    TMallocMipMap(TMallocMipMap&&) = delete;
    TMallocMipMap& operator =(TMallocMipMap&&) = delete;

    bool AliasesToMipMaps(const void* ptr) const;
    size_t AllocationSize(const void* ptr) const;

    size_t TotalAllocatedSize() const;
    size_t TotalAvailableSize() const;
    size_t TotalCommittedSize() const;

    void* AllocateHintTLS(size_t sizeInBytes);
    void* Allocate(size_t sizeInBytes, size_t* hint);
    void Free(void* ptr);

    void GarbageCollect();

    static size_t SnapSize(size_t sizeInBytes);

private:
    static constexpr u64 GEmptyMipMask_     = 0xFFFFFFFFFFFFFFFFull;
    static constexpr u64 GEmptySizeMask_    = 0x0000000000000000ull;
#if USE_PPE_DEBUG
    static constexpr u64 GDeletedMask_      = 0xDDDDDDDDDDDDDDDDull;
#endif

    struct FMipMap {
#if USE_PPE_DEBUG
        std::atomic<u64> MipMask{ GDeletedMask_ };
        std::atomic<u64> SizeMask{ GDeletedMask_ };
#else
        std::atomic<u64> MipMask;
        std::atomic<u64> SizeMask;
#endif
    };

    FReadWriteLock _GClockRW;

    void* _vSpace;

    FAtomicSpinLock _barrier;
    std::atomic<u32> _numMipMaps;
    std::atomic<u32> _nextFreeMipMap;

    ALIGN(16) FMipMap _mipMaps[NumMipMaps];

    u64 MipAvailableSizeInBytes_(u32 mipIndex) const;

    // These flags allows to manipulate the binary tree (almost) without recursing
    // - We can store 32 blocks of 1 page which can be hierarchically collapsed up to 1 block of 32 pages
    // - Allocating is O(1) and deallocating is O(log2(level)) (but still very fast due to high memory coherency)
    // - Everything is done atomically, but there's contention when a new chunk of 32 blocks must be committed (amortized in time)
    // - There's a maximum reserved size which can't grow, but it's only consumed if needed and reserved only in virtual memory (don't commit the pages initially)
    //   so don't be afraid to put a fair amount of reserve
    // - You should call GarbageCollect() once in a while to release some committed chunks (can only release the last chunk if empty)
    // #NOTE: this is templated since the algorithm could fit well inside a VRAM allocator

    static constexpr u64 GLevelMasks_[6] = {
        0x0000000000000001ull,
        0x0000000000000006ull,
        0x0000000000000078ull,
        0x0000000000007F80ull,
        0x000000007FFF8000ull,
        0x7FFFFFFF80000000ull,
    };

    static constexpr u64 GSetMasks_[64] = {
        0x8000000000000000ull, 0xFFFF80007F807864ull, 0x80007FFF807F879Aull, 0xFFFFFF807FF87E74ull,
        0xFFFF807FFF87F9ECull, 0xFF807FFFF87FE7DAull, 0x807FFFFF87FF9FBAull, 0xFFFFFFF87FFE7F74ull,
        0xFFFFFF87FFF9FEF4ull, 0xFFFFF87FFFE7FDECull, 0xFFFF87FFFF9FFBECull, 0xFFF87FFFFE7FF7DAull,
        0xFF87FFFFF9FFEFDAull, 0xF87FFFFFE7FFDFBAull, 0x87FFFFFF9FFFBFBAull, 0xFFFFFFFE7FFF7F74ull,
        0xFFFFFFF9FFFEFF74ull, 0xFFFFFFE7FFFDFEF4ull, 0xFFFFFF9FFFFBFEF4ull, 0xFFFFFE7FFFF7FDECull,
        0xFFFFF9FFFFEFFDECull, 0xFFFFE7FFFFDFFBECull, 0xFFFF9FFFFFBFFBECull, 0xFFFE7FFFFF7FF7DAull,
        0xFFF9FFFFFEFFF7DAull, 0xFFE7FFFFFDFFEFDAull, 0xFF9FFFFFFBFFEFDAull, 0xFE7FFFFFF7FFDFBAull,
        0xF9FFFFFFEFFFDFBAull, 0xE7FFFFFFDFFFBFBAull, 0x9FFFFFFFBFFFBFBAull, 0xFFFFFFFF7FFF7F74ull,
        0xFFFFFFFEFFFF7F74ull, 0xFFFFFFFDFFFEFF74ull, 0xFFFFFFFBFFFEFF74ull, 0xFFFFFFF7FFFDFEF4ull,
        0xFFFFFFEFFFFDFEF4ull, 0xFFFFFFDFFFFBFEF4ull, 0xFFFFFFBFFFFBFEF4ull, 0xFFFFFF7FFFF7FDECull,
        0xFFFFFEFFFFF7FDECull, 0xFFFFFDFFFFEFFDECull, 0xFFFFFBFFFFEFFDECull, 0xFFFFF7FFFFDFFBECull,
        0xFFFFEFFFFFDFFBECull, 0xFFFFDFFFFFBFFBECull, 0xFFFFBFFFFFBFFBECull, 0xFFFF7FFFFF7FF7DAull,
        0xFFFEFFFFFF7FF7DAull, 0xFFFDFFFFFEFFF7DAull, 0xFFFBFFFFFEFFF7DAull, 0xFFF7FFFFFDFFEFDAull,
        0xFFEFFFFFFDFFEFDAull, 0xFFDFFFFFFBFFEFDAull, 0xFFBFFFFFFBFFEFDAull, 0xFF7FFFFFF7FFDFBAull,
        0xFEFFFFFFF7FFDFBAull, 0xFDFFFFFFEFFFDFBAull, 0xFBFFFFFFEFFFDFBAull, 0xF7FFFFFFDFFFBFBAull,
        0xEFFFFFFFDFFFBFBAull, 0xDFFFFFFFBFFFBFBAull, 0xBFFFFFFFBFFFBFBAull, 0x0000000000000000ull,
    };

    static constexpr u64 GUnsetMasks_[64] = {
        0x7FFFFFFFFFFFFFFFull, 0x00007FFF807F879Aull, 0x7FFF80007F807864ull, 0x0000007F80078188ull,
        0x00007F8000780610ull, 0x007F800007801820ull, 0x7F80000078006040ull, 0x0000000780018080ull,
        0x0000007800060100ull, 0x0000078000180200ull, 0x0000780000600400ull, 0x0007800001800800ull,
        0x0078000006001000ull, 0x0780000018002000ull, 0x7800000060004000ull, 0x0000000180008000ull,
        0x0000000600010000ull, 0x0000001800020000ull, 0x0000006000040000ull, 0x0000018000080000ull,
        0x0000060000100000ull, 0x0000180000200000ull, 0x0000600000400000ull, 0x0001800000800000ull,
        0x0006000001000000ull, 0x0018000002000000ull, 0x0060000004000000ull, 0x0180000008000000ull,
        0x0600000010000000ull, 0x1800000020000000ull, 0x6000000040000000ull, 0x0000000080000000ull,
        0x0000000100000000ull, 0x0000000200000000ull, 0x0000000400000000ull, 0x0000000800000000ull,
        0x0000001000000000ull, 0x0000002000000000ull, 0x0000004000000000ull, 0x0000008000000000ull,
        0x0000010000000000ull, 0x0000020000000000ull, 0x0000040000000000ull, 0x0000080000000000ull,
        0x0000100000000000ull, 0x0000200000000000ull, 0x0000400000000000ull, 0x0000800000000000ull,
        0x0001000000000000ull, 0x0002000000000000ull, 0x0004000000000000ull, 0x0008000000000000ull,
        0x0010000000000000ull, 0x0020000000000000ull, 0x0040000000000000ull, 0x0080000000000000ull,
        0x0100000000000000ull, 0x0200000000000000ull, 0x0400000000000000ull, 0x0800000000000000ull,
        0x1000000000000000ull, 0x2000000000000000ull, 0x4000000000000000ull, 0x0000000000000000ull,
    };

    static constexpr u64 GSizeMasks_[32] = {
        0x000000008000808Bull, 0x0000000100000000ull, 0x0000000200010000ull, 0x0000000400000000ull,
        0x0000000800020100ull, 0x0000001000000000ull, 0x0000002000040000ull, 0x0000004000000000ull,
        0x0000008000080210ull, 0x0000010000000000ull, 0x0000020000100000ull, 0x0000040000000000ull,
        0x0000080000200400ull, 0x0000100000000000ull, 0x0000200000400000ull, 0x0000400000000000ull,
        0x0000800000800824ull, 0x0001000000000000ull, 0x0002000001000000ull, 0x0004000000000000ull,
        0x0008000002001000ull, 0x0010000000000000ull, 0x0020000004000000ull, 0x0040000000000000ull,
        0x0080000008002040ull, 0x0100000000000000ull, 0x0200000010000000ull, 0x0400000000000000ull,
        0x0800000020004000ull, 0x1000000000000000ull, 0x2000000040000000ull, 0x4000000000000000ull,
    };

    static FORCE_INLINE u32 FirstBitSet_(u64 mask) {
        Assert(mask);
        return u32(FPlatformMaths::tzcnt64(mask));
    }
    static FORCE_INLINE u32 MipLevel_(size_t sizeInBytes) {
        Assert(sizeInBytes);
        return FPlatformMaths::FloorLog2(checked_cast<u32>(TopMipSize / sizeInBytes));
    }
    static FORCE_INLINE u32 MipOffset_(u32 level) {
        Assert_NoAssume(level < lengthof(GLevelMasks_));
        return ((u32(1) << level) - 1);
    }
    static FORCE_INLINE u32 ParentBit_(u32 index) {
        Assert(index);
        return (index - 1) / 2;
    }

};
PRAGMA_MSVC_WARNING_POP()
//----------------------------------------------------------------------------
template <typename _VMemTraits>
TMallocMipMap<_VMemTraits>::TMallocMipMap()
:   vmem_traits()
,   _vSpace(nullptr)
,   _numMipMaps(0)
,   _nextFreeMipMap(0) {
    STATIC_ASSERT(vmem_traits::ReservedSize > TopMipSize);
    STATIC_ASSERT(Meta::IsAligned(TopMipSize, vmem_traits::ReservedSize));
    STATIC_ASSERT(std::atomic<u64>::is_always_lock_free);

    VerifyRelease((_vSpace = vmem_traits::PageReserve(vmem_traits::ReservedSize)) != nullptr);
}
//----------------------------------------------------------------------------
template <typename _VMemTraits>
TMallocMipMap<_VMemTraits>::~TMallocMipMap() {
    Assert(_vSpace);

    const FReadWriteLock::FScopeLockWrite GClockWrite(_GClockRW);
    vmem_traits::PageRelease(_vSpace, vmem_traits::ReservedSize);
}
//----------------------------------------------------------------------------
template <typename _VMemTraits>
bool TMallocMipMap<_VMemTraits>::AliasesToMipMaps(const void* ptr) const {
    return ((uintptr_t)ptr >= (uintptr_t)_vSpace &&
            (uintptr_t)ptr < (uintptr_t)_vSpace + vmem_traits::ReservedSize );
}
//----------------------------------------------------------------------------
template <typename _VMemTraits>
size_t TMallocMipMap<_VMemTraits>::AllocationSize(const void* ptr) const {
    Assert(ptr);
    Assert_NoAssume(AliasesToMipMaps(ptr));

    const intptr_t vptr = ((u8*)ptr - (u8*)_vSpace);
    const u32 mipIndex = checked_cast<u32>(vptr / TopMipSize);
    const FMipMap& mipMap = _mipMaps[mipIndex];

    const u32 idx = checked_cast<u32>((vptr - mipIndex * TopMipSize) / BottomMipSize);
    Assert_NoAssume(FPlatformMaths::popcnt64(mipMap.SizeMask & GSizeMasks_[idx]) == 1);

    const u32 bit = FirstBitSet_(mipMap.SizeMask & GSizeMasks_[idx]);
    const u32 lvl = FPlatformMaths::FloorLog2(bit + 1);

    return (TopMipSize / (size_t(1) << lvl));
}
//----------------------------------------------------------------------------
template <typename _VMemTraits>
size_t TMallocMipMap<_VMemTraits>::TotalAllocatedSize() const {
    return (TotalCommittedSize() - TotalAvailableSize());
}
//----------------------------------------------------------------------------
template <typename _VMemTraits>
size_t TMallocMipMap<_VMemTraits>::TotalAvailableSize() const {
    u64 availableSize = 0;

    const u32 n = _numMipMaps;
    forrange(i, 0, n)
        availableSize += MipAvailableSizeInBytes_(i);

    return checked_cast<u32>(availableSize);
}
//----------------------------------------------------------------------------
template <typename _VMemTraits>
size_t TMallocMipMap<_VMemTraits>::TotalCommittedSize() const {
    return (_numMipMaps * NumBottomMips * BottomMipSize);
}
//----------------------------------------------------------------------------
template <typename _VMemTraits>
void* TMallocMipMap<_VMemTraits>::AllocateHintTLS(size_t sizeInBytes) {
    static THREAD_LOCAL size_t GHintTLS = 0;
    return Allocate(sizeInBytes, &GHintTLS);
}
//----------------------------------------------------------------------------
template <typename _VMemTraits>
void* TMallocMipMap<_VMemTraits>::Allocate(size_t sizeInBytes, size_t* hint) {
    Assert(sizeInBytes);
    Assert(hint);
    Assert_NoAssume(sizeInBytes <= TopMipSize);
    Assert_NoAssume(Meta::IsAligned(BottomMipSize, sizeInBytes));

    const u32 lvl = MipLevel_(sizeInBytes);
    const u32 fst = MipOffset_(lvl);
    const u64 msk = GLevelMasks_[lvl];

    const auto allocateFromMip = [this, lvl, fst, msk, hint](u32 mipIndex) -> void* {
        Assert(mipIndex < NumMipMaps);

        FMipMap& mipMap = _mipMaps[mipIndex];
        u64 mip = mipMap.MipMask.load(std::memory_order_relaxed);
        Assert_NoAssume(GDeletedMask_ != mip);

        for (;;) {
            if (const u64 avail = mip & msk) {
                const u32 bit = FirstBitSet_(avail);
                Assert_NoAssume(mip & ~GSetMasks_[bit]);
                Assert_NoAssume(mip != (mip & GSetMasks_[bit]));

                if (mipMap.MipMask.compare_exchange_weak(mip, mip & GSetMasks_[bit],
                    std::memory_order_release, std::memory_order_relaxed)) {
                    Assert_NoAssume((mipMap.MipMask & ~GSetMasks_[bit]) == 0);
                    Assert_NoAssume((mipMap.SizeMask & (u64(1) << bit)) == 0);
                    mipMap.SizeMask |= (u64(1) << bit);
                    *hint = mipIndex;
                    const u32 off = (bit - fst) * u32(TopMipSize / (size_t(1) << lvl));
                    Assert_NoAssume(FPlatformMaths::popcnt64(mipMap.SizeMask & GSizeMasks_[off / BottomMipSize]) == 1);
                    return ((u8*)_vSpace + (mipIndex * TopMipSize + off));
                }
            }
            else {
                return nullptr;
            }
        }
    };

    void* p;

    const FReadWriteLock::FScopeLockRead GClockRead(_GClockRW);

    const u32 n = _numMipMaps;
    u32 mipIndex = checked_cast<u32>(*hint);

    // 1) try from last mip map this thread allocated from :
    p = (mipIndex < n ? allocateFromMip(mipIndex) : nullptr);
    if (Likely(p))
        return p;

    // 2) try from next mip map with most of available space :
    mipIndex = _nextFreeMipMap;
    p = (mipIndex < n ? allocateFromMip(mipIndex) : nullptr);
    if (Likely(p))
        return p;

    // 3) fall back to linear search (assume there's space available almost 99% of the time) :
    forrange(c, 0, n) {
        if (++mipIndex >= n)
            mipIndex = 0;

        p = allocateFromMip(mipIndex);
        if (p)
            return p;
    }

    // 4) failed to allocate from mip maps, request to commit 32 new pages :
    {
        const FAtomicSpinLock::FScope scopeLock(_barrier);

        if (_numMipMaps < NumMipMaps) {
            // initialize and commit a new mip for each blocked thread (natural interleaving)
            mipIndex = _numMipMaps;

            vmem_traits::PageCommit((u8*)_vSpace + (mipIndex * TopMipSize), TopMipSize);

            _mipMaps[mipIndex].MipMask = GEmptyMipMask_;
            _mipMaps[mipIndex].SizeMask = GEmptySizeMask_;

            _numMipMaps = (mipIndex + 1); // increment last to be sure than no one touches it before it's fully initialized
            _nextFreeMipMap = mipIndex;
        }
        else {
            return nullptr; // out of virtual space, failed allocation
        }
    }

    return allocateFromMip(mipIndex);
}
//----------------------------------------------------------------------------
template <typename _VMemTraits>
void TMallocMipMap<_VMemTraits>::Free(void* ptr) {
    Assert(ptr);
    Assert_NoAssume(AliasesToMipMaps(ptr));

    const FReadWriteLock::FScopeLockRead GClockRead(_GClockRW);

    const intptr_t vptr = ((u8*)ptr - (u8*)_vSpace);
    const u32 mipIndex = checked_cast<u32>(vptr / TopMipSize);
    FMipMap& mipMap = _mipMaps[mipIndex];

    const u32 idx = checked_cast<u32>((vptr - mipIndex * TopMipSize) / BottomMipSize);
    Assert_NoAssume(FPlatformMaths::popcnt64(mipMap.SizeMask & GSizeMasks_[idx]) == 1);

    const u32 bit = FirstBitSet_(mipMap.SizeMask & GSizeMasks_[idx]);

    Assert_NoAssume((mipMap.MipMask & ~GSetMasks_[bit]) == 0);
    Assert_NoAssume(mipMap.SizeMask & (u64(1) << bit));

    // first clear the allocated bit from the SizeMask,
    // so we can't allocate from MipMask when SizeMask is still set
    mipMap.SizeMask &= ~(u64(1) << bit);

    u64 mip = mipMap.MipMask.load(std::memory_order_relaxed);
    for (;;) {
        Assert_NoAssume((mip & ~GSetMasks_[bit]) == 0);
        u64 upd = mip;
        u32 del = bit;

        for (;;) {
            upd |= GUnsetMasks_[del];
            const u64 sib = (u64(1) << (del & 1 ? del + 1 : del - 1));
            if (0 == del || 0 == (upd & sib))
                break;
            del = ParentBit_(del);
        }

        Assert_NoAssume(mip != upd);
        if (mipMap.MipMask.compare_exchange_strong(mip, upd,
            std::memory_order_release, std::memory_order_relaxed)) {

            // keep track of the block with most free chunks, not thread safe but it's not important
            const u32 nextFree = _nextFreeMipMap.load(std::memory_order_relaxed);
            if (nextFree < _numMipMaps &&
                ((upd & GLevelMasks_[lengthof(GLevelMasks_) - 1]) >
                (_mipMaps[nextFree].SizeMask & GLevelMasks_[lengthof(GLevelMasks_) - 1]))) {
                _nextFreeMipMap = mipIndex;
            }

            return;
        }
    }
}
//----------------------------------------------------------------------------
template <typename _VMemTraits>
void TMallocMipMap<_VMemTraits>::GarbageCollect() {
    // will release mip maps ATE of committed space IFP
    // this is locking all other threads attempting to allocate
    // sadly due to fragmentation blocks maybe too scattered to release a
    // substantial amount of virtual memory

    const FReadWriteLock::FScopeLockWrite GClockWrite(_GClockRW);

    u32 mipIndex = _numMipMaps;
    while (mipIndex) {
        FMipMap& mipMap = _mipMaps[--mipIndex];
        if (mipMap.MipMask.load(std::memory_order_relaxed) == GEmptyMipMask_) {
            Assert_NoAssume(0 == mipMap.SizeMask);

            --_numMipMaps;

            vmem_traits::PageDecommit((u8*)_vSpace + (mipIndex * TopMipSize), TopMipSize);

#if USE_PPE_DEBUG
            mipMap.MipMask = GDeletedMask_;
            mipMap.SizeMask = GDeletedMask_;
#endif
        }
        else {
            break;
        }
    }
}
//----------------------------------------------------------------------------
template <typename _VMemTraits>
size_t TMallocMipMap<_VMemTraits>::SnapSize(size_t sizeInBytes) {
    Assert(sizeInBytes);

    const u32 lvl = MipLevel_(sizeInBytes);
    const size_t snapped = (TopMipSize / (size_t(1) << lvl));

    Assert(snapped >= sizeInBytes);
    return snapped;
}
//----------------------------------------------------------------------------
template <typename _VMemTraits>
u64 TMallocMipMap<_VMemTraits>::MipAvailableSizeInBytes_(u32 mipIndex) const {
    Assert(mipIndex < _numMipMaps);

    const u64 numBottomMips = FPlatformMaths::popcnt(
        _mipMaps[mipIndex].MipMask & GLevelMasks_[lengthof(GLevelMasks_) - 1]);
    return (numBottomMips * BottomMipSize);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE