﻿// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "HAL/Windows/WindowsPlatformMemory.h"

#ifdef PLATFORM_WINDOWS

#include "Diagnostic/Logger.h"
#include "HAL/PlatformMaths.h"
#include "HAL/PlatformMemory.h"
#include "HAL/Windows/LastError.h"
#include "HAL/Windows/WindowsPlatformIncludes.h"

#include <intrin.h> // _AddressOfReturnAddress()
#include <Psapi.h> // GetProcessMemoryInfo()

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static FWindowsPlatformMemory::FConstants FetchConstants_() {
    // Gather platform memory constants once

    ::MEMORYSTATUSEX mem;
    ::ZeroMemory(&mem, sizeof(mem));
    mem.dwLength = sizeof(mem);
    Verify(::GlobalMemoryStatusEx(&mem));

    ::SYSTEM_INFO sys;
    ::ZeroMemory(&sys, sizeof(sys));
    ::GetSystemInfo(&sys);

    int cpu[4];
    ::__cpuid(cpu, 0x80000006);

    FWindowsPlatformMemory::FConstants result;
    ::ZeroMemory(&result, sizeof(result));

    result.TotalPhysical = checked_cast<u64>(mem.ullTotalPhys);
    result.TotalVirtual = checked_cast<u64>(mem.ullTotalVirtual);
    result.AllocationGranularity = checked_cast<u64>(sys.dwAllocationGranularity);	// VirtualAlloc cannot allocate memory less than that
    result.PageSize = checked_cast<u64>(sys.dwPageSize);
    result.AddressLimit = FPlatformMaths::NextPow2(result.TotalPhysical);
    result.CacheLineSize = checked_cast<u64>(cpu[2] & 0xFF);

    AssertRelease(FWindowsPlatformMemory::PageSize == result.PageSize);
    AssertRelease(FWindowsPlatformMemory::CacheLineSize == result.CacheLineSize);
    AssertRelease(ALLOCATION_GRANULARITY == result.AllocationGranularity); // some code is dependent on this assumption

    return result;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const size_t FWindowsPlatformMemory::AllocationGranularity{
    checked_cast<size_t>(Constants().AllocationGranularity)
};
//----------------------------------------------------------------------------
auto FWindowsPlatformMemory::Constants() -> const FConstants& {
    static const FConstants GConstants = FetchConstants_();
    return GConstants;
}
//----------------------------------------------------------------------------
auto FWindowsPlatformMemory::Stats() -> FStats {
    /**
    *	GlobalMemoryStatusEx
    *	MEMORYSTATUSEX
    *		ullTotalPhys
    *		ullAvailPhys
    *		ullTotalVirtual
    *		ullAvailVirtual
    *
    *	GetProcessMemoryInfo
    *	PROCESS_MEMORY_COUNTERS
    *		WorkingSetSize
    *		UsedVirtual
    *		PeakUsedVirtual
    *
    *	GetPerformanceInfo
    *		PPERFORMANCE_INFORMATION
    *		PageSize
    */

    // This method is slow, do not call it too often.
    // #TODO Should be executed only on the background thread.

    // Gather platform memory stats.

    ::MEMORYSTATUSEX mem;
    ::ZeroMemory(&mem, sizeof(mem));
    mem.dwLength = sizeof(mem);
    ::GlobalMemoryStatusEx(&mem);

    ::PROCESS_MEMORY_COUNTERS counters;
    ::ZeroMemory(&counters, sizeof(counters));
    ::GetProcessMemoryInfo(::GetCurrentProcess(), &counters, sizeof(counters));

    FStats stats;
    ::ZeroMemory(&stats, sizeof(stats));

    stats.AvailablePhysical = checked_cast<u64>(mem.ullAvailPhys);
    stats.AvailableVirtual = checked_cast<u64>(mem.ullAvailVirtual);
    stats.UsedPhysical = checked_cast<u64>(counters.WorkingSetSize);
    stats.UsedVirtual = checked_cast<u64>(counters.PagefileUsage);
    stats.PeakUsedPhysical = checked_cast<u64>(counters.PeakWorkingSetSize);
    stats.PeakUsedVirtual = checked_cast<u64>(counters.PeakPagefileUsage);

    return stats;
}
//----------------------------------------------------------------------------
auto FWindowsPlatformMemory::StackUsage() -> FStackUsage {
    FStackUsage usage;

    ::MEMORY_BASIC_INFORMATION mbi;
    ::VirtualQuery(&mbi, &mbi, sizeof(mbi));
    // now mbi.AllocationBase = reserved stack memory base address

    ::VirtualQuery(mbi.AllocationBase, &mbi, sizeof(mbi));
    // now (mbi.BaseAddress, mbi.RegionSize) describe reserved (uncommitted) portion of the stack
    usage.Reserved = checked_cast<u64>(mbi.RegionSize);

    ::VirtualQuery((char*)mbi.BaseAddress + mbi.RegionSize, &mbi, sizeof(mbi));
    // now (mbi.BaseAddress, mbi.RegionSize) describe the guard page
    usage.Guard = checked_cast<u64>(mbi.RegionSize);

    ::VirtualQuery((char*)mbi.BaseAddress + mbi.RegionSize, &mbi, sizeof(mbi));
    // now (mbi.BaseAddress, mbi.RegionSize) describe the committed (i.e. accessed) portion of the stack
    usage.Committed = checked_cast<u64>(mbi.RegionSize);

    usage.BaseAddr = mbi.BaseAddress; // base of committed portion of the stack

    return usage;
}
//----------------------------------------------------------------------------
void* FWindowsPlatformMemory::PageAlloc(size_t sizeInBytes) {
    Assert(sizeInBytes);
    Assert(Meta::IsAlignedPow2(PAGE_SIZE, sizeInBytes));

    // /!\ BEWARE /!\
    // #TODO : refactor this into a less wasteful page allocator
    // You're wasting virtual address space with this method
    // Allocation have a 64k granularity, but this is allocating on 4k boundary, wasting 60k for each alloc
    // A more efficient way would be to reserve 64k, and then commit 4k pages : in this scenario VirtualAlloc
    // has a page boundary instead of dwAllocationGranularity boundary
    // Note : at the time writing this comment only FMallocStomp uses this, but it's critical client since
    // it already has a massive memory overhead
    void* const ptr = ::VirtualAlloc(nullptr, sizeInBytes, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    Assert(Meta::IsAlignedPow2(PAGE_SIZE, ptr));
    return ptr;
}
//----------------------------------------------------------------------------
void FWindowsPlatformMemory::PageFree(void* ptr, size_t sizeInBytes) {
    Assert(ptr);
    Assert(Meta::IsAlignedPow2(PAGE_SIZE, ptr));
    Assert(Meta::IsAlignedPow2(PAGE_SIZE, sizeInBytes));

    NOOP(sizeInBytes);

    if (0 == ::VirtualFree(ptr, 0, MEM_RELEASE))
        PPE_THROW_IT(FLastErrorException("VirtualFree"));
}
//----------------------------------------------------------------------------
void* FWindowsPlatformMemory::VirtualAlloc(size_t sizeInBytes, bool commit) {
    return FWindowsPlatformMemory::VirtualAlloc(FPlatformMemory::AllocationGranularity, sizeInBytes, commit);
}
//----------------------------------------------------------------------------
PRAGMA_MSVC_WARNING_PUSH()
PRAGMA_MSVC_WARNING_DISABLE(6001) // Using uninitialized memory 'p'. (no need to worry, we just use the virtual addr)
void* FWindowsPlatformMemory::VirtualAlloc(size_t alignment, size_t sizeInBytes, bool commit) {
    Assert(sizeInBytes);
    Assert(Meta::IsAlignedPow2(FPlatformMemory::AllocationGranularity, sizeInBytes));
    Assert(Meta::IsPow2(alignment));

    const ::DWORD flAllocationType = (commit ? MEM_RESERVE | MEM_COMMIT : MEM_RESERVE);
    const ::DWORD flProtect = (commit ? PAGE_READWRITE : PAGE_NOACCESS);

    // Optimistically try mapping precisely the right amount before falling back to the slow method :
    void* p = ::VirtualAlloc(nullptr, sizeInBytes, flAllocationType, flProtect);

    if (not Meta::IsAlignedPow2(alignment, p)) {

        // Fill "bubbles" (reserve unaligned regions) at the beginning of virtual address space, otherwise there will be always falling back to the slow method
        if (bit_cast<uintptr_t>(p) < 16 * 1024 * 1024)
            ::VirtualAlloc(p, alignment - (bit_cast<uintptr_t>(p) & (alignment - 1)), MEM_RESERVE, PAGE_NOACCESS);

        do {
            p = ::VirtualAlloc(NULL, sizeInBytes + alignment - FPlatformMemory::AllocationGranularity, MEM_RESERVE, PAGE_NOACCESS);
            if (nullptr == p)// if OOM
                return nullptr;

            ::VirtualFree(p, 0, MEM_RELEASE);// Unfortunately, WinAPI doesn't support release a part of allocated region, so release a whole region

            p = ::VirtualAlloc(
                (void*)((bit_cast<uintptr_t>(p) + (alignment - 1)) & ~(alignment - 1)),
                sizeInBytes, flAllocationType, flProtect );

        } while (nullptr == p);
    }

    Assert(Meta::IsAlignedPow2(FPlatformMemory::AllocationGranularity, p));
    return p;
}
PRAGMA_MSVC_WARNING_POP()
//----------------------------------------------------------------------------
void FWindowsPlatformMemory::VirtualCommit(void* ptr, size_t sizeInBytes) {
    Assert(ptr);
    Assert(sizeInBytes);
    Assert(Meta::IsAlignedPow2(PAGE_SIZE, ptr));
    Assert(Meta::IsAlignedPow2(PAGE_SIZE, sizeInBytes));

    // Remember : memory must be reserved first with VirtualAlloc(sizeInBytes, false)

    Verify(::VirtualAlloc(ptr, sizeInBytes, MEM_COMMIT, PAGE_READWRITE) == ptr);
}
//----------------------------------------------------------------------------
void FWindowsPlatformMemory::VirtualFree(void* ptr, size_t sizeInBytes, bool release) {
    Assert(ptr);
    Assert(Meta::IsAlignedPow2(release ? FPlatformMemory::AllocationGranularity : PAGE_SIZE, ptr));
    Assert(Meta::IsAlignedPow2(release ? FPlatformMemory::AllocationGranularity : PAGE_SIZE, sizeInBytes));

    ::DWORD dwFreeType;
    if (release) {
        sizeInBytes = 0;
        dwFreeType = MEM_RELEASE;
    }
    else {
        dwFreeType = MEM_DECOMMIT;
    }

    if (0 == ::VirtualFree(ptr, sizeInBytes, dwFreeType))
        PPE_THROW_IT(FLastErrorException("VirtualFree"));
}
//----------------------------------------------------------------------------
size_t FWindowsPlatformMemory::RegionSize(void* ptr) {
    Assert(ptr);

    //  https://msdn.microsoft.com/en-us/library/windows/desktop/aa366902(v=vs.85).aspx

    ::MEMORY_BASIC_INFORMATION info;
    Verify(::VirtualQuery(ptr, &info, sizeof(info)));
    Assert(ptr == info.BaseAddress);

    return checked_cast<size_t>(info.RegionSize);
}
//----------------------------------------------------------------------------
bool FWindowsPlatformMemory::PageProtect(void* ptr, size_t sizeInBytes, bool read, bool write) {
    Assert(ptr);
    Assert(sizeInBytes);

    ::DWORD oldProtect, newProtect;

    if (read && write)
        newProtect = PAGE_READWRITE;
    else if (write)
        newProtect = PAGE_READWRITE;
    else if (read)
        newProtect = PAGE_READONLY;
    else
        newProtect = PAGE_NOACCESS;

    return (0 != ::VirtualProtect(ptr, sizeInBytes, newProtect, &oldProtect));
}
//----------------------------------------------------------------------------
void FWindowsPlatformMemory::OnOutOfMemory(size_t sizeInBytes, size_t alignment) {
    // #TODO add SubmitErrorReport() ?
    Unused(sizeInBytes);
    Unused(alignment);
}
//----------------------------------------------------------------------------
// system allocator
//----------------------------------------------------------------------------
void* FWindowsPlatformMemory::SystemMalloc(size_t s) {
    void* const result = ::malloc(s);

#if USE_PPE_MEMORYDOMAINS
    const size_t sys = ::_msize(result);
    MEMORYDOMAIN_TRACKING_DATA(SystemMalloc).Allocate(sys, sys);
#endif

    return result;
}
//----------------------------------------------------------------------------
NODISCARD void* FWindowsPlatformMemory::SystemRealloc(void* p, size_t s) {
#if USE_PPE_MEMORYDOMAINS
    if (p) {
        const size_t sys = ::_msize(p);
        MEMORYDOMAIN_TRACKING_DATA(SystemMalloc).Deallocate(sys, sys);
    }
#endif

    void* const result = ::realloc(p, s);
    Assert(0 == s || !!result);

#if USE_PPE_MEMORYDOMAINS
    if (result) {
        const size_t sys = ::_msize(result);
        MEMORYDOMAIN_TRACKING_DATA(SystemMalloc).Allocate(sys, sys);
    }
#endif

    return result;
}
//----------------------------------------------------------------------------,
void FWindowsPlatformMemory::SystemFree(void* p) {
    Assert(p);

#if USE_PPE_MEMORYDOMAINS
    const size_t sys = ::_msize(p);
    MEMORYDOMAIN_TRACKING_DATA(SystemMalloc).Deallocate(sys, sys);
#endif

    ::free(p);
}
//----------------------------------------------------------------------------
// system allocator aligned
//----------------------------------------------------------------------------
NODISCARD void* FWindowsPlatformMemory::SystemAlignedMalloc(size_t s, size_t boundary) {
    void* const result = ::_aligned_malloc(s, boundary);
    Assert(result);

#if USE_PPE_MEMORYDOMAINS
    const size_t sys = ::_aligned_msize(result, boundary, 0);
    MEMORYDOMAIN_TRACKING_DATA(SystemMalloc).Allocate(sys, sys);
#endif

    return result;
}
//----------------------------------------------------------------------------
NODISCARD void* FWindowsPlatformMemory::SystemAlignedRealloc(void* p, size_t s, size_t boundary) {
#if USE_PPE_MEMORYDOMAINS
    if (p) {
        const size_t sys = ::_aligned_msize(p, boundary, 0);
        MEMORYDOMAIN_TRACKING_DATA(SystemMalloc).Deallocate(sys, sys);
    }
#endif

    void* const result = ::_aligned_realloc(p, s, boundary);
    Assert(0 == s || !!result);

#if USE_PPE_MEMORYDOMAINS
    if (result) {
        const size_t sys = ::_aligned_msize(result, boundary, 0);
        MEMORYDOMAIN_TRACKING_DATA(SystemMalloc).Allocate(sys, sys);
    }
#endif

    return result;
}
//----------------------------------------------------------------------------
void FWindowsPlatformMemory::SystemAlignedFree(void* p, size_t boundary) {
    Assert(p);

#if USE_PPE_MEMORYDOMAINS
    const size_t sys = ::_aligned_msize(p, boundary, 0);
    MEMORYDOMAIN_TRACKING_DATA(SystemMalloc).Deallocate(sys, sys);
#else
    Unused(boundary);
#endif

    ::_aligned_free(p);
}
//----------------------------------------------------------------------------
#if !USE_PPE_FINAL_RELEASE
size_t FWindowsPlatformMemory::SystemAlignedRegionSize(void* p, size_t boundary) {
    Assert(p);
    return ::_aligned_msize(p, boundary, 0);
}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_WINDOWS
