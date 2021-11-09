#pragma once

#include "Core_fwd.h"

#include "Allocator/AllocatorBinning.h"

#if !USE_PPE_FINAL_RELEASE
#   include "IO/TextWriter_fwd.h"
#endif

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
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

    static void     ReleaseCacheMemory();
    static void     ReleasePendingBlocks();

    static size_t   RegionSize(void* ptr) NOEXCEPT;
    static size_t   SnapSize(size_t size) NOEXCEPT;

#if !USE_PPE_FINAL_RELEASE
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
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
