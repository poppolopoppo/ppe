#pragma once

#include "Core_fwd.h"

#include "HAL/PlatformMemory.h"
#include "Memory/MemoryDomain.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FVirtualMemory {
public:
    FORCE_INLINE static size_t SnapSize(size_t sizeInBytes) NOEXCEPT {
        return Meta::RoundToNext(sizeInBytes, FPlatformMemory::AllocationGranularity);
    }

    static size_t   SizeInBytes(void* ptr) NOEXCEPT;
    static bool     Protect(void* ptr, size_t sizeInBytes, bool read, bool write);

#if USE_PPE_MEMORYDOMAINS
#   define TRACKINGDATA_ARG_IFP , FMemoryTracking& trackingData
#   define TRACKINGDATA_FWD_IFP , trackingData
#   define TRACKINGDATA_PRM_IFP(...) , MEMORYDOMAIN_TRACKING_DATA(__VA_ARGS__)
#else
#   define TRACKINGDATA_ARG_IFP
#   define TRACKINGDATA_FWD_IFP
#   define TRACKINGDATA_PRM_IFP(...)
#endif

    static void*    Alloc(size_t sizeInBytes TRACKINGDATA_ARG_IFP) { return Alloc(FPlatformMemory::AllocationGranularity, sizeInBytes TRACKINGDATA_FWD_IFP); }
    static void*    Alloc(size_t alignment, size_t sizeInBytes TRACKINGDATA_ARG_IFP);
    static void     Free(void* ptr, size_t sizeInBytes TRACKINGDATA_ARG_IFP);

    static void*    PageReserve(size_t sizeInBytes);
    static void*    PageReserve(size_t alignment, size_t sizeInBytes);
    static void     PageCommit(void* ptr, size_t sizeInBytes TRACKINGDATA_ARG_IFP);
    static void     PageDecommit(void* ptr, size_t sizeInBytes TRACKINGDATA_ARG_IFP);
    static void     PageRelease(void* ptr, size_t sizeInBytes);

    static void*    InternalAlloc(size_t sizeInBytes TRACKINGDATA_ARG_IFP);
    static void     InternalFree(void* ptr, size_t sizeInBytes TRACKINGDATA_ARG_IFP);

#undef TRACKINGDATA_ARG_IFP
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
