#pragma once

#include "Core_fwd.h"

#include "Allocator/AllocatorBase.h"
#include "Allocator/AllocatorBlock.h"
#include "Maths/SSEHelpers.h"

#if !USE_PPE_FINAL_RELEASE
#   include "IO/TextWriter_fwd.h"
#endif

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FAllocatorBinning {
    // made common so that external code can align on block sizes
    static CONSTEXPR const u8 NumBins = 47;
    static CONSTEXPR const u16 BinSizes[NumBins + 1] = {
        16,     32,     48,     64,     80,     96,     112,    128,
        144,    176,    208,    240,    272,    320,    368,    416,
        480,    544,    624,    704,    800,    928,    1056,   1216,
        1424,   1616,   1840,   2112,   2384,   2720,   3184,   3680,
        4240,   4912,   5696,   6432,   7360,   8480,   9552,   10912,
        12736,  15008,  16960,  19104,  21824,  24560,  27632,  INT16_MAX/* padding/dummy */
    };
    static CONSTEXPR const u32 MaxBinSize = BinSizes[NumBins - 1];

    static CONSTEXPR const u8 NumBins_4kPages = 32;
    static CONSTEXPR const u32 MaxBinSize_4kPages = BinSizes[NumBins_4kPages - 1];

    static CONSTEXPR u32 IndexFromSizeConst(u16 blockSize) NOEXCEPT {
        u32 cnt = 0;
        for (const u16 sz : BinSizes)
            cnt += (sz < blockSize);
        return cnt;
    }

    static u32 IndexFromSize(size_t blockSize) NOEXCEPT {
        return IndexFromSize(checked_cast<u16>(blockSize));
    }
    static u32 IndexFromSize(u16 blockSize) NOEXCEPT {
#ifdef __clang__ // code-gen should out-perform the explicit SSE version bellow
        return IndexFromSizeConst(blockSize);

#else
        const m128i_t key = ::_mm_set1_epi16(blockSize);

        m128i_t cnt = ::_mm_setzero_si128();
        cnt = ::_mm_subs_epi16(cnt, ::_mm_cmplt_epi16(::_mm_load_si128((m128i_t*)&BinSizes[ 0]), key));
        cnt = ::_mm_subs_epi16(cnt, ::_mm_cmplt_epi16(::_mm_load_si128((m128i_t*)&BinSizes[ 8]), key));
        cnt = ::_mm_subs_epi16(cnt, ::_mm_cmplt_epi16(::_mm_load_si128((m128i_t*)&BinSizes[16]), key));
        cnt = ::_mm_subs_epi16(cnt, ::_mm_cmplt_epi16(::_mm_load_si128((m128i_t*)&BinSizes[24]), key));
        cnt = ::_mm_subs_epi16(cnt, ::_mm_cmplt_epi16(::_mm_load_si128((m128i_t*)&BinSizes[32]), key));
        cnt = ::_mm_subs_epi16(cnt, ::_mm_cmplt_epi16(::_mm_load_si128((m128i_t*)&BinSizes[40]), key));

        cnt = ::_mm_sad_epu8(cnt, ::_mm_setzero_si128());
        cnt = ::_mm_adds_epi16(cnt, _mm_shuffle_epi32(cnt, _MM_SHUFFLE(1, 0, 3, 2)));

        return (::_mm_cvtsi128_si32(cnt) & 0xFFFFu);

#endif
    }

    static CONSTEXPR u32 IndexToBlockSize(u32 poolIndex) NOEXCEPT {
        Assert(poolIndex < NumBins);
        return BinSizes[poolIndex];
    }

    static u32 BoundSizeToBins(u32 sizeInBytes) NOEXCEPT {
        Assert_NoAssume(sizeInBytes <= MaxBinSize);
        return (sizeInBytes ? IndexToBlockSize(IndexFromSize(static_cast<u16>(sizeInBytes))) : 0);
    }
    static CONSTEXPR u32 LowestBoundIndex(size_t sizeInBytes) NOEXCEPT {
        if (sizeInBytes <= MaxBinSize) {
            u32 pool = IndexFromSize(sizeInBytes);
            if (BinSizes[pool] > sizeInBytes)
                --pool;
            Assert_NoAssume(BinSizes[pool] <= sizeInBytes);
            return pool;
        }
        return (NumBins - 1);
    }
};
//----------------------------------------------------------------------------
class PPE_CORE_API FMallocBinned2 {
public:
    static void*    Malloc(size_t size);
    static void     Free(void* const ptr);
    static void*    Realloc(void* const ptr, size_t size);

    static void*    AlignedMalloc(size_t size, size_t alignment) {
        void* const p = Malloc(Meta::RoundToNextPow2(size, alignment));
        Assert_NoAssume(Meta::IsAlignedPow2(alignment, p));
        return p;
    }
    static void     AlignedFree(void* const ptr) {
        return Free(ptr);
    }
    static void*    AlignedRealloc(void* const ptr, size_t size, size_t alignment) {
        void* const p = Realloc(ptr, Meta::RoundToNextPow2(size, alignment));
        Assert_NoAssume(0 == size || Meta::IsAlignedPow2(alignment, p));
        return p;
    }

    static FAllocatorBlock MallocForNew(size_t size);
    static FAllocatorBlock ReallocForNew(FAllocatorBlock blk, size_t size);
    static void FreeForDelete(FAllocatorBlock blk);

    static size_t   RegionSize(void* ptr) NOEXCEPT;
    static size_t   SnapSize(size_t size) NOEXCEPT;

    static void     ReleaseCacheMemory();
    static void     ReleasePendingBlocks();

#if !USE_PPE_FINAL_RELEASE
    static void     DumpMemoryInfo(FTextWriter& oss);
    static void     DumpMemoryInfo(FWTextWriter& oss);
#endif

public:
    // made public so that external code can align on block sizes
    static CONSTEXPR const u8 NumSmallBlockSizes = FAllocatorBinning::NumBins;
    static CONSTEXPR const auto& SmallBlockSizes = FAllocatorBinning::BinSizes;
    static CONSTEXPR const u32 MaxSmallBlockSize = FAllocatorBinning::MaxBinSize;

    static CONSTEXPR u32 SmallPoolIndexConst(u16 blockSize) NOEXCEPT {
        return FAllocatorBinning::IndexFromSizeConst(blockSize);
    }
    static u32 SmallPoolIndex(u16 blockSize) NOEXCEPT {
        return FAllocatorBinning::IndexFromSize(blockSize);
    }
    static CONSTEXPR u32 SmallPoolIndexToBlockSize(u32 poolIndex) NOEXCEPT {
        return FAllocatorBinning::IndexToBlockSize(poolIndex);
    }
    static u32 BoundSizeToSmallPool(u32 sizeInBytes) NOEXCEPT {
        return FAllocatorBinning::BoundSizeToBins(sizeInBytes);
    }

};
//----------------------------------------------------------------------------
class PPE_CORE_API FMallocatorBinned2 : private FAllocatorPolicy {
public:
    using propagate_on_container_copy_assignment = std::true_type;
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_swap = std::true_type;

    using is_always_equal = std::true_type;

    using has_maxsize = std::false_type;
    using has_owns = std::false_type;
    using has_reallocate = std::true_type;
    using has_acquire = std::true_type;
    using has_steal = std::true_type;

    STATIC_CONST_INTEGRAL(size_t, Alignment, ALLOCATION_BOUNDARY);

    FMallocatorBinned2() = default;

    size_t SnapSize(size_t s) const NOEXCEPT {
        return FMallocBinned2::SnapSize(s);
    }

    FAllocatorBlock Allocate(size_t s) const {
        return FMallocBinned2::MallocForNew(s);
    }

    void Deallocate(FAllocatorBlock b) const {
        FMallocBinned2::FreeForDelete(b);
    }

    void Reallocate(FAllocatorBlock& b, size_t s) const {
        b = FMallocBinned2::ReallocForNew(b, s);
    }

    bool Acquire(FAllocatorBlock b) NOEXCEPT {
        Unused(b); // nothing to do
        return true;
    }

    bool Steal(FAllocatorBlock b) NOEXCEPT {
        Unused(b); // nothing to do
        return true;
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
