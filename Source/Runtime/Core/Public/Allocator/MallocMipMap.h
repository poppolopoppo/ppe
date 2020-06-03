#pragma once

#include "Core_fwd.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FMallocMipMap {
public:
    // 64k-2Mb
    static const size_t MediumTopMipSize;

    static void* MediumAlloc(size_t sz, size_t alignment);
    static void MediumFree(void* ptr);

    static void MediumTrim();

    static bool AliasesToMediumMips(void* ptr) NOEXCEPT;
    static size_t MediumSnapSize(size_t sz) NOEXCEPT;
    static size_t MediumRegionSize(void* ptr) NOEXCEPT;

    // 2Mb-64Mb
    static const size_t LargeTopMipSize;

    static void* LargeAlloc(size_t sz, size_t alignment);
    static void LargeFree(void* ptr);

    static void LargeTrim();

    static bool AliasesToLargeMips(void* ptr) NOEXCEPT;
    static size_t LargeSnapSize(size_t sz) NOEXCEPT;
    static size_t LargeRegionSize(void* ptr) NOEXCEPT;

    // large+medium
    static void* MipAlloc(size_t sz, size_t alignment);
    static void MipFree(void* ptr);

    static void MemoryTrim();

    static bool AliasesToMips(void* ptr) NOEXCEPT;
    static size_t SnapSize(size_t sz) NOEXCEPT;
    static size_t RegionSize(void* ptr) NOEXCEPT;

#if !USE_PPE_FINAL_RELEASE
    // used for memory diagnostics only
    static void MediumMips(
        void** vspace,
        size_t* numCommitted,
        size_t* numReserved,
        size_t* mipSizeInBytes,
        TMemoryView<const u32>* mipMasks );
    static void LargeMips(
        void** vspace,
        size_t* numCommitted,
        size_t* numReserved,
        size_t* mipSizeInBytes,
        TMemoryView<const u32>* mipMasks );
#endif

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
