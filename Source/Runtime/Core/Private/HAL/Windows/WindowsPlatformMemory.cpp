#include "stdafx.h"

#include "HAL/Windows/WindowsPlatformMemory.h"

#ifdef PLATFORM_WINDOWS

#include "Diagnostic/Logger.h"
#include "HAL/PlatformMaths.h"

#include "HAL/Windows/LastError.h"
#include "HAL/Windows/WindowsPlatformIncludes.h"

#include <Psapi.h>

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
    AssertRelease(FWindowsPlatformMemory::AllocationGranularity == result.AllocationGranularity); // some code is dependent on this assumption

    return result;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
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
void* FWindowsPlatformMemory::PageAlloc(size_t sizeInBytes) {
    Assert(sizeInBytes);
    Assert(Meta::IsAligned(PAGE_SIZE, sizeInBytes));

    void* const ptr = ::VirtualAlloc(nullptr, sizeInBytes, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    Assert(Meta::IsAligned(PAGE_SIZE, ptr));
    return ptr;
}
//----------------------------------------------------------------------------
void FWindowsPlatformMemory::PageFree(void* ptr, size_t sizeInBytes) {
    Assert(ptr);
    Assert(Meta::IsAligned(PAGE_SIZE, ptr));
    Assert(Meta::IsAligned(PAGE_SIZE, sizeInBytes));

    NOOP(sizeInBytes);

    if (0 == ::VirtualFree(ptr, 0, MEM_RELEASE))
        PPE_THROW_IT(FLastErrorException("VirtualFree"));
}
//----------------------------------------------------------------------------
void* FWindowsPlatformMemory::VirtualAlloc(size_t sizeInBytes) {
    return FWindowsPlatformMemory::VirtualAlloc(ALLOCATION_GRANULARITY, sizeInBytes);
}
//----------------------------------------------------------------------------
void* FWindowsPlatformMemory::VirtualAlloc(size_t alignment, size_t sizeInBytes) {
    Assert(sizeInBytes);
    Assert(Meta::IsAligned(ALLOCATION_GRANULARITY, sizeInBytes));
    Assert(Meta::IsPow2(alignment));

    // Optimistically try mapping precisely the right amount before falling back to the slow method :
    void* p = ::VirtualAlloc(nullptr, sizeInBytes, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    if (not Meta::IsAligned(alignment, p)) {

        // Fill "bubbles" (reserve unaligned regions) at the beginning of virtual address space, otherwise there will be always falling back to the slow method
        if ((uintptr_t)p < 16 * 1024 * 1024)
            ::VirtualAlloc(p, alignment - ((uintptr_t)p & (alignment - 1)), MEM_RESERVE, PAGE_NOACCESS);

        do {
            p = ::VirtualAlloc(NULL, sizeInBytes + alignment - ALLOCATION_GRANULARITY, MEM_RESERVE, PAGE_NOACCESS);
            if (nullptr == p)
                goto RETURN_ALLOC;

            ::VirtualFree(p, 0, MEM_RELEASE);// Unfortunately, WinAPI doesn't support release a part of allocated region, so release a whole region

            p = ::VirtualAlloc(
                (void*)(((uintptr_t)p + (alignment - 1)) & ~(alignment - 1)),
                sizeInBytes,
                MEM_RESERVE | MEM_COMMIT,
                PAGE_READWRITE);

        } while (nullptr == p);
    }

RETURN_ALLOC:
    CLOG(nullptr == p, HAL, Fatal, L"VirtualAlloc({0}, {1}) failed with = {2}", alignment, sizeInBytes, FLastError());
    Assert(Meta::IsAligned(ALLOCATION_GRANULARITY, p));
    return p;
}
//----------------------------------------------------------------------------
void FWindowsPlatformMemory::VirtualFree(void* ptr, size_t sizeInBytes) {
    Assert(ptr);
    Assert(Meta::IsAligned(ALLOCATION_GRANULARITY, ptr));
    Assert(Meta::IsAligned(ALLOCATION_GRANULARITY, sizeInBytes));

    NOOP(sizeInBytes);

    if (0 == ::VirtualFree(ptr, 0, MEM_RELEASE))
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
    NOOP(sizeInBytes, alignment);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_WINDOWS