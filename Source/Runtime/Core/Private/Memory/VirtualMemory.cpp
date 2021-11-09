#include "stdafx.h"

#include "Memory/VirtualMemory.h"

#include "Container/CompressedRadixTrie.h"
#include "HAL/PlatformMemory.h"
#include "Memory/MemoryDomain.h"
#include "Memory/MemoryTracking.h"

#if USE_PPE_MEMORYDOMAINS
#   define TRACKINGDATA_ARG_IFP , FMemoryTracking& trackingData
#   define TRACKINGDATA_ARG_FWD , trackingData
#else
#   define TRACKINGDATA_ARG_IFP
#   define TRACKINGDATA_ARG_FWD
#endif

#ifdef PLATFORM_WINDOWS
#   define USE_VMALLOC_SIZE_PTRIE          1// This is faster than ::VirtualQuery()
#else
#   define USE_VMALLOC_SIZE_PTRIE          1// No support on other platforms
#endif

#if (USE_PPE_ASSERT || USE_PPE_MEMORY_DEBUGGING)
#   define USE_VMCACHE_PAGE_PROTECT     1// Crash when using a VM cached block
#else
#   define USE_VMCACHE_PAGE_PROTECT     0
#endif

#if USE_VMALLOC_SIZE_PTRIE
#   include "Thread/AtomicSpinLock.h"
#endif

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_VMALLOC_SIZE_PTRIE
namespace {
//----------------------------------------------------------------------------
class FVMAllocSizePTrie_ {
public:
    static FVMAllocSizePTrie_& Get() {
        ONE_TIME_DEFAULT_INITIALIZE(FVMAllocSizePTrie_, GInstance);
        return GInstance;
    }

    void Register(void* ptr, size_t sizeInBytes) {
        _allocs.Insert(uintptr_t(ptr), uintptr_t(sizeInBytes));
    }

    size_t Fetch(void* ptr) const {
        return size_t(_allocs.Lookup(uintptr_t(ptr)));
    }

    size_t Erase(void* ptr) {
        return size_t(_allocs.Erase(uintptr_t(ptr)));
    }

private:
    FReadWriteCompressedRadixTrie _allocs;

#if USE_PPE_MEMORYDOMAINS
    FVMAllocSizePTrie_() : _allocs(MEMORYDOMAIN_TRACKING_DATA(SizePTrie)) {}
#else
    FVMAllocSizePTrie_() {}
#endif
    ~FVMAllocSizePTrie_() {}
};
//----------------------------------------------------------------------------
} //!namespace
#endif //!USE_VMALLOC_SIZE_PTRIE
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
size_t FVirtualMemory::SizeInBytes(void* ptr) NOEXCEPT {
    if (nullptr == ptr)
        return 0;

    Assert(Meta::IsAlignedPow2(FPlatformMemory::AllocationGranularity, ptr));

#if USE_VMALLOC_SIZE_PTRIE
    const size_t regionSize = FVMAllocSizePTrie_::Get().Fetch(ptr);
    Assert(Meta::IsAlignedPow2(FPlatformMemory::AllocationGranularity, regionSize));

    return regionSize;

#else
    return FPlatformMemory::RegionSize(ptr);

#endif
}
//----------------------------------------------------------------------------
bool FVirtualMemory::Protect(void* ptr, size_t sizeInBytes, bool read, bool write) {
    Assert(ptr);
    Assert(sizeInBytes);

    return FPlatformMemory::PageProtect(ptr, sizeInBytes, read, write);
}
//----------------------------------------------------------------------------
// Keep allocations aligned to OS granularity
void* FVirtualMemory::Alloc(size_t alignment, size_t sizeInBytes TRACKINGDATA_ARG_IFP) {
    Assert(sizeInBytes);
    Assert(Meta::IsAlignedPow2(FPlatformMemory::AllocationGranularity, sizeInBytes));
    Assert(Meta::IsAlignedPow2(FPlatformMemory::AllocationGranularity, alignment));
    Assert(Meta::IsPow2(alignment));

    void* const p = FPlatformMemory::VirtualAlloc(alignment, sizeInBytes, true);

#if USE_VMALLOC_SIZE_PTRIE
    FVMAllocSizePTrie_::Get().Register(p, sizeInBytes);
#endif
#if USE_PPE_MEMORYDOMAINS
    Assert_NoAssume(trackingData.IsChildOf(FMemoryTracking::ReservedMemory()));
    trackingData.AllocateSystem(sizeInBytes);
#endif

    return p;
}
//----------------------------------------------------------------------------
void FVirtualMemory::Free(void* ptr, size_t sizeInBytes TRACKINGDATA_ARG_IFP) {
    Assert(ptr);
    Assert(sizeInBytes);

#if USE_VMALLOC_SIZE_PTRIE
    const size_t regionSize = FVMAllocSizePTrie_::Get().Erase(ptr);
    Assert(regionSize == sizeInBytes);
#endif

#if USE_PPE_MEMORYDOMAINS
    Assert_NoAssume(trackingData.IsChildOf(FMemoryTracking::ReservedMemory()));
    trackingData.DeallocateSystem(sizeInBytes);
#endif

    FPlatformMemory::VirtualFree(ptr, sizeInBytes, true);
}
//----------------------------------------------------------------------------
void* FVirtualMemory::PageReserve(size_t sizeInBytes) {
    Assert(sizeInBytes);
    Assert(Meta::IsAlignedPow2(FPlatformMemory::AllocationGranularity, sizeInBytes));

    return FPlatformMemory::VirtualAlloc(sizeInBytes, false);
}
//----------------------------------------------------------------------------
void* FVirtualMemory::PageReserve(size_t alignment, size_t sizeInBytes) {
    Assert(sizeInBytes);
    Assert(Meta::IsAlignedPow2(FPlatformMemory::AllocationGranularity, sizeInBytes));

    return FPlatformMemory::VirtualAlloc(alignment, sizeInBytes, false);
}
//----------------------------------------------------------------------------
void FVirtualMemory::PageCommit(void* ptr, size_t sizeInBytes TRACKINGDATA_ARG_IFP) {
    Assert(ptr);
    Assert(sizeInBytes);
    Assert(Meta::IsAlignedPow2(PAGE_SIZE, ptr));
    Assert(Meta::IsAlignedPow2(PAGE_SIZE, sizeInBytes));

    FPlatformMemory::VirtualCommit(ptr, sizeInBytes);

#if 0 && USE_VMALLOC_SIZE_PTRIE // shouldn't be necessary
    FVMAllocSizePTrie_::Get().Register(ptr, sizeInBytes);
#endif
#if USE_PPE_MEMORYDOMAINS
    Assert_NoAssume(trackingData.IsChildOf(FMemoryTracking::ReservedMemory()));
    trackingData.AllocateSystem(sizeInBytes);
#endif
}
//----------------------------------------------------------------------------
void FVirtualMemory::PageDecommit(void* ptr, size_t sizeInBytes TRACKINGDATA_ARG_IFP) {
    Assert(ptr);
    Assert(sizeInBytes);
    Assert(Meta::IsAlignedPow2(PAGE_SIZE, ptr));
    Assert(Meta::IsAlignedPow2(PAGE_SIZE, sizeInBytes));

#if 0 && USE_VMALLOC_SIZE_PTRIE // shouldn't be necessary
    const size_t regionSize = FVMAllocSizePTrie_::Get().Erase(ptr);
    Assert(regionSize == sizeInBytes);
#endif

#if USE_PPE_MEMORYDOMAINS
    Assert_NoAssume(trackingData.IsChildOf(FMemoryTracking::ReservedMemory()));
    trackingData.DeallocateSystem(sizeInBytes);
#endif

    FPlatformMemory::VirtualFree(ptr, sizeInBytes, false);
}
//----------------------------------------------------------------------------
void FVirtualMemory::PageRelease(void* ptr, size_t sizeInBytes) {
    Assert(ptr);
    Assert(sizeInBytes);
    Assert(Meta::IsAlignedPow2(FPlatformMemory::AllocationGranularity, ptr));
    Assert(Meta::IsAlignedPow2(FPlatformMemory::AllocationGranularity, sizeInBytes));

    FPlatformMemory::VirtualFree(ptr, sizeInBytes, true);
}
//----------------------------------------------------------------------------
// Won't register in FVMAllocSizePTrie_
void* FVirtualMemory::InternalAlloc(size_t sizeInBytes TRACKINGDATA_ARG_IFP) {
    Assert(Meta::IsAlignedPow2(ALLOCATION_BOUNDARY, sizeInBytes));

    void* const ptr = FPlatformMemory::VirtualAlloc(sizeInBytes, true);
    AssertRelease(ptr);

#if USE_PPE_MEMORYDOMAINS
    Assert(trackingData.IsChildOf(FMemoryTracking::ReservedMemory()));
    trackingData.AllocateSystem(sizeInBytes);
#endif

    return ptr;
}
//----------------------------------------------------------------------------
// Won't register in FVMAllocSizePTrie_
void FVirtualMemory::InternalFree(void* ptr, size_t sizeInBytes TRACKINGDATA_ARG_IFP) {
    Assert(ptr);
    Assert_NoAssume(Meta::IsAlignedPow2(ALLOCATION_BOUNDARY, sizeInBytes));


#if USE_PPE_MEMORYDOMAINS
    Assert(trackingData.IsChildOf(FMemoryTracking::ReservedMemory()));
    trackingData.DeallocateSystem(sizeInBytes);
#endif

    FPlatformMemory::VirtualFree(ptr, sizeInBytes, true);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#undef TRACKINGDATA_ARG_IFP
#undef TRACKINGDATA_ARG_FWD
