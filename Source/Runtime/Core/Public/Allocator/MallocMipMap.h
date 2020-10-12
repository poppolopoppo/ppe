#pragma once

#include "Core_fwd.h"

#include "Allocator/Malloc.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FMallocMipMap {
public:
    // 64k-2Mb
    static const size_t MediumMaxAllocSize;

    static void* MediumAlloc(size_t sz, size_t alignment);
    static void* MediumResize(void* ptr, size_t newSize, size_t oldSize) NOEXCEPT;
    static void MediumFree(void* ptr);

    static void MediumTrim();

    static bool AliasesToMediumMips(void* ptr) NOEXCEPT;
    static size_t MediumSnapSize(size_t sz) NOEXCEPT;
    static size_t MediumRegionSize(void* ptr) NOEXCEPT;

    // 2Mb-64Mb
    static const size_t LargeMaxAllocSize;

    static void* LargeAlloc(size_t sz, size_t alignment);
    static void* LargeResize(void* ptr, size_t newSize, size_t oldSize) NOEXCEPT;
    static void LargeFree(void* ptr);

    static void LargeTrim();

    static bool AliasesToLargeMips(void* ptr) NOEXCEPT;
    static size_t LargeSnapSize(size_t sz) NOEXCEPT;
    static size_t LargeRegionSize(void* ptr) NOEXCEPT;

    // large+medium
    static const size_t MipMaxAllocSize;

    static void* MipAlloc(size_t sz, size_t alignment);
    static void* MipResize(void* ptr, size_t newSize, size_t oldSize) NOEXCEPT;
    static void MipFree(void* ptr);

    static void MemoryTrim();

    static bool AliasesToMips(void* ptr) NOEXCEPT;
    static size_t SnapSize(size_t sz) NOEXCEPT;
    static size_t RegionSize(void* ptr) NOEXCEPT;

#if !USE_PPE_FINAL_RELEASE
    // used for memory diagnostics only
    using FMipmapInfo = FMallocDebug::FMipmapInfo;

    // return the number of mipmap, call twice to know how much to allocate for infos
    static u32 FetchMediumMipsInfo(FMipmapInfo* pinfo) NOEXCEPT;
    static u32 FetchLargeMipsInfo(FMipmapInfo* pinfo) NOEXCEPT;
#endif

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
