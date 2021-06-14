#pragma once

#include "Core_fwd.h"

#include "Allocator/Malloc.h"
#include "IO/TextWriter_fwd.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FMallocBitmap {
public:
    static const size_t Alignment;

    // 32k-2Mb
    static const size_t MediumMaxAllocSize;

    static void* MediumAlloc(size_t sz, size_t alignment);
    static void* MediumResize(void* ptr, size_t newSize, size_t oldSize) NOEXCEPT;
    static void MediumFree(void* ptr);

    static void MediumTrim();

    static bool AliasesToMediumHeap(void* ptr) NOEXCEPT;
    static size_t MediumSnapSize(size_t sz) NOEXCEPT;
    static size_t MediumRegionSize(void* ptr) NOEXCEPT;

    // 1Mb-64Mb
    static const size_t LargeMaxAllocSize;

    static void* LargeAlloc(size_t sz, size_t alignment);
    static void* LargeResize(void* ptr, size_t newSize, size_t oldSize) NOEXCEPT;
    static void LargeFree(void* ptr);

    static void LargeTrim();

    static bool AliasesToLargeHeap(void* ptr) NOEXCEPT;
    static size_t LargeSnapSize(size_t sz) NOEXCEPT;
    static size_t LargeRegionSize(void* ptr) NOEXCEPT;

    // large+medium
    static const size_t MaxAllocSize;

    static void* HeapAlloc(size_t sz, size_t alignment);
    static void* HeapResize(void* ptr, size_t newSize, size_t oldSize) NOEXCEPT;
    NODISCARD static bool HeapFree_ReturnIfAliases(void* ptr);
    static void HeapFree(void* ptr) { Verify(HeapFree_ReturnIfAliases(ptr)); }

    static void MemoryTrim();

    static bool AliasesToHeaps(void* ptr) NOEXCEPT;
    static size_t SnapSize(size_t sz) NOEXCEPT;
    static bool RegionSize_ReturnIfAliases(size_t* pSizeInBytes, void* ptr) NOEXCEPT;
    static size_t RegionSize(void* ptr) NOEXCEPT {
        size_t sizeInBytes;
        Verify(RegionSize_ReturnIfAliases(&sizeInBytes, ptr));
        return sizeInBytes;
    }

#if !USE_PPE_FINAL_RELEASE
    static void DumpMediumHeapInfo(FWTextWriter& oss) NOEXCEPT;
    static void DumpLargeHeapInfo(FWTextWriter& oss) NOEXCEPT;

    static void DumpHeapInfo(FWTextWriter& oss) NOEXCEPT {
        DumpMediumHeapInfo(oss);
        DumpLargeHeapInfo(oss);
    }
#endif

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
