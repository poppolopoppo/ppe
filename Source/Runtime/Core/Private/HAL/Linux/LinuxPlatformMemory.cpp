#include "stdafx.h"

#include "HAL/Linux/LinuxPlatformMemory.h"

#ifdef PLATFORM_LINUX

#include "HAL/Linux/Errno.h"
#include "HAL/Linux/LinuxPlatformIncludes.h"

#include "HAL/PlatformMaths.h"
#include "HAL/TargetPlatform.h"

#include "Diagnostic/Logger.h"

#include <pthread.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/sysinfo.h>
#include <sys/time.h>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static FILE* ProcFS_Open_(const char* fname) {
    if (FILE* f = ::fopen(fname, "r"))
        return f;

    LOG(HAL, Error, L"fopen({0}) failed with errno: {1}", MakeCStringView(fname), FErrno{});
    return nullptr;
}
//----------------------------------------------------------------------------
static bool ProcFS_ParseULLONG_kB(u64* pValue, const char* name, char* ln) {
    char* start = ln;
    for (const char *key = name; *key; ++key, ++start)
        if (*key != *start)
            return false;

    for (; *start && !::isdigit(*start); ++start);
    if (!*start)
        return false;

    char* stop = start;
    for (; *stop && ::isdigit(*stop); ++stop);
    if (start == stop)
        return false;

    *stop =  '\0';
    *pValue = ::strtoull(start, nullptr, 10) * 1024/* assuming kB */;

    return true;
}
//----------------------------------------------------------------------------
static FLinuxPlatformMemory::FConstants FetchConstants_() {
    // Gather platform memory constants once

    FLinuxPlatformMemory::FConstants cst;

    struct sysinfo sysInfo;
    if (0 == sysinfo(&sysInfo)) {
        cst.TotalPhysical = static_cast< unsigned long long >( sysInfo.mem_unit ) * static_cast< unsigned long long >( sysInfo.totalram );
        cst.TotalVirtual = static_cast< unsigned long long >( sysInfo.mem_unit ) * static_cast< unsigned long long >( sysInfo.totalswap );
    }
    else {
        cst.TotalPhysical = 0;
        cst.TotalVirtual = 0;

        LOG(HAL, Error, L"sysinfo() failed with errno: {0}", FErrno{});
    }

    ::rlimit addressSpace;
    if (0 == ::getrlimit(RLIMIT_AS, &addressSpace)) {
        cst.AddressLimit = addressSpace.rlim_cur;
    }
    else {
        cst.AddressLimit = 0;

        LOG(HAL, Error, L"getrlimit() failed with errno: {0}", FErrno{});
    }

    cst.CacheLineSize = ::sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    cst.PageSize = ::sysconf(_SC_PAGE_SIZE);

    // fetch huge page size (no C clean API for this :/)
    if (FILE* globalMemStats = ProcFS_Open_("/proc/meminfo")) {
        char buf[256];

        size_t numParsed = 0;
        while (char* ln = ::fgets(buf, lengthof(buf), globalMemStats)) {
            numParsed += size_t(
                ProcFS_ParseULLONG_kB(&cst.AllocationGranularity, "Hugepagesize:", ln) );

            if (numParsed)
                break; // early out to avoid parsing all lines
        }

        if (!numParsed)
            cst.AllocationGranularity = cst.PageSize;

        Verify(0 == ::fclose(globalMemStats));
    }
    else {
        cst.AllocationGranularity = cst.PageSize;
    }

    return cst;
}
//----------------------------------------------------------------------------
static void* VMPageAlloc_(size_t sizeInBytes, bool commit) {
    int protectionMode = (commit ? PROT_WRITE | PROT_READ : PROT_NONE);
    void* const ptr = ::mmap(nullptr, sizeInBytes, protectionMode, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

    if (ptr != MAP_FAILED) {
        Assert(Meta::IsAligned(PAGE_SIZE, ptr));
        return ptr;
    }
    else {
        LOG(HAL, Error, L"mmap() failed with errno: {0}", FErrno{});
        return nullptr;
    }
}
//----------------------------------------------------------------------------
static bool VMPageProtect_(void* ptr, size_t sizeInBytes, bool read, bool write) {
    int protectMode = (int(read) * PROT_READ) | (int(write) * PROT_READ);
    Assert((read|write)||(protectMode==PROT_NONE));

    if (0 != ::mprotect(ptr, sizeInBytes, protectMode) == 0) {
        return true;
    }
    else {
        LOG(HAL, Error, L"mprotect() failed with errno: {0}", FErrno{});
        return false;
    }
}
//----------------------------------------------------------------------------
static void VMPageFree_(void* ptr, size_t sizeInBytes, bool release) {
    if (release) {
        if (::munmap(ptr, sizeInBytes) != 0)
            LOG(HAL, Fatal, L"::munmap() failed with errno: {0}", FErrno{});
    }
    else {
        Verify(VMPageProtect_(ptr, sizeInBytes, false, false));
    }
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
auto FLinuxPlatformMemory::Constants() -> const FConstants& {
    static const FConstants GConstants = FetchConstants_();
    return GConstants;
}
//----------------------------------------------------------------------------
auto FLinuxPlatformMemory::Stats() -> FStats {
    FStats st;
    char buf[200];

    // no clean C API for this, can't escape ProcFS :'(
    if (FILE* globalMemStats = ProcFS_Open_("/proc/meminfo")) {
        st.AvailablePhysical = 0;
        u64 memFree = 0, cached = 0;

        size_t numParsed = 0;
        while (char* ln = ::fgets(buf, lengthof(buf), globalMemStats)) {
            numParsed += size_t(
                // if we have MemAvailable, favor that (see http://git.kernel.org/cgit/linux/kernel/git/torvalds/linux.git/commit/?id=34e431b0ae398fc54ea69ff85ec700722c9da773)
                ProcFS_ParseULLONG_kB(&st.AvailablePhysical, "MemAvailable:", ln) ||
                ProcFS_ParseULLONG_kB(&st.AvailableVirtual, "SwapFree:", ln) ||
                ProcFS_ParseULLONG_kB(&memFree, "MemFree:", ln) ||
                ProcFS_ParseULLONG_kB(&cached, "Cached:", ln) );

            if (4 == numParsed)
                break; // early out to avoid parsing all lines
        }

        // if we didn't have MemAvailable (kernels < 3.14 or CentOS 6.x), use memFree + cached as a (bad) approximation
        if (st.AvailablePhysical == 0)
            st.AvailablePhysical= memFree + cached;

        Assert(numParsed >= 3);

        Verify(0 == ::fclose(globalMemStats));
    }
    else {
        st.AvailablePhysical = 0;
        st.AvailableVirtual = 0;
    }

    // ProcFS, again :'(
    if (FILE* processMemStats = ProcFS_Open_("/proc/self/status")) {
        size_t numParsed = 0;
        while (char* ln = ::fgets(buf, lengthof(buf), processMemStats)) {
            numParsed += size_t(
                ProcFS_ParseULLONG_kB(&st.PeakUsedVirtual, "VmPeak:", ln) ||
                ProcFS_ParseULLONG_kB(&st.UsedVirtual, "VmSize:", ln) ||
                ProcFS_ParseULLONG_kB(&st.PeakUsedPhysical, "VmHWM:", ln) ||
                ProcFS_ParseULLONG_kB(&st.PeakUsedVirtual, "VmRSS:", ln) );

            if (4 == numParsed)
                break; // early out to avoid parsing all lines
        }

        Assert(numParsed == 4);

        // sanitize stats as sometimes peak < used for some reason
        st.PeakUsedPhysical = Max(st.PeakUsedPhysical, st.UsedPhysical);
        st.PeakUsedVirtual = Max(st.PeakUsedVirtual, st.UsedVirtual);

        Verify(0 == ::fclose(processMemStats));
    }
    else {
        st.PeakUsedPhysical = 0;
        st.PeakUsedVirtual = 0;
        st.UsedPhysical = 0;
        st.UsedVirtual = 0;
    }

    return st;
}
//----------------------------------------------------------------------------
auto FLinuxPlatformMemory::StackUsage() -> FStackUsage {
    FStackUsage stk;

    ::rusage rusage;
    if (::getrusage(RUSAGE_THREAD, &rusage) == 0) {
        stk.Reserved = rusage.ru_isrss;
    }
    else {
        stk.Reserved = 0;
        LOG(HAL, Error, L"getrusage() failed with errno: {0}", FErrno{});
    }

    ::pthread_attr_t threadAttr;
    if (0 == ::pthread_getattr_np(::pthread_self(), &threadAttr)) {
        size_t stackSize;
        if (::pthread_attr_getstack(&threadAttr, &stk.BaseAddr, &stackSize) == 0) {
            stk.Committed = stackSize;
        }
        else {
            stk.BaseAddr = nullptr;
            stk.Committed = 0;

            LOG(HAL, Error, L"pthread_get_stack() failed with errno: {0}", FErrno{});
        }

        size_t guardSize;
        if (::pthread_attr_getguardsize(&threadAttr, &guardSize) == 0) {
            stk.Guard = guardSize;
        }
        else {
            stk.Guard = 0;

            LOG(HAL, Error, L"pthread_getguardsize() failed with errno: {0}", FErrno{});
        }
    }
    else {
        stk.Committed = 0;
        stk.Guard = 0;
        stk.BaseAddr = nullptr;

        LOG(HAL, Error, L"pthread_getattr_np() failed with errno: {0}", FErrno{});
    }

    return stk;
}
//----------------------------------------------------------------------------
void* FLinuxPlatformMemory::AddressOfReturnAddress() {
    // https://stackoverflow.com/questions/15944119/addressofreturnaddress-equivalent-in-clang-llvm/38799865
    return ((i64*)__builtin_frame_address (0) + 1);
}
//----------------------------------------------------------------------------
void* FLinuxPlatformMemory::PageAlloc(size_t sizeInBytes) {
    Assert(sizeInBytes);
    Assert(Meta::IsAligned(PAGE_SIZE, sizeInBytes));

    return VMPageAlloc_(sizeInBytes, true);
}
//----------------------------------------------------------------------------
void FLinuxPlatformMemory::PageFree(void* ptr, size_t sizeInBytes) {
    Assert(ptr);
    Assert(Meta::IsAligned(PAGE_SIZE, ptr));
    Assert(Meta::IsAligned(PAGE_SIZE, sizeInBytes));

    VMPageFree_(ptr, sizeInBytes, true);
}
//----------------------------------------------------------------------------
void* FLinuxPlatformMemory::VirtualAlloc(size_t sizeInBytes, bool commit) {
    return FLinuxPlatformMemory::VirtualAlloc(ALLOCATION_GRANULARITY, sizeInBytes, commit);
}
//----------------------------------------------------------------------------
void* FLinuxPlatformMemory::VirtualAlloc(size_t alignment, size_t sizeInBytes, bool commit) {
    Assert(sizeInBytes);
    Assert(Meta::IsAligned(ALLOCATION_GRANULARITY, sizeInBytes));
    Assert(Meta::IsPow2(alignment));

    void* p = VMPageAlloc_(sizeInBytes, commit);//optimistically try mapping precisely the right amount before falling back to the slow method

    if (not Meta::IsAligned(alignment, p)) {
        VMPageFree_(p, sizeInBytes, true);

        const FConstants cst = Constants();
        p = VMPageAlloc_(sizeInBytes + alignment - cst.AllocationGranularity, commit);

        if (p) {
            uintptr_t ap = ((uintptr_t)p + (alignment - 1)) & ~(alignment - 1);
            uintptr_t diff = ap - (uintptr_t)p;
            if (diff)
                VMPageFree_(p, diff, true);

            diff = alignment - cst.AllocationGranularity - diff;
            Assert((intptr_t)diff >= 0);

            if (diff)
                VMPageFree_((void*)(ap + sizeInBytes), diff, true);

            return (void*)ap;
        }

    }

    return p;
}
//----------------------------------------------------------------------------
void FLinuxPlatformMemory::VirtualCommit(void* ptr, size_t sizeInBytes) {
    Assert(ptr);
    Assert(sizeInBytes);
    Assert(Meta::IsAligned(PAGE_SIZE, ptr));
    Assert(Meta::IsAligned(PAGE_SIZE, sizeInBytes));

    // Remember : memory must be reserved first with VirtualAlloc(sizeInBytes, false)

    Verify(VMPageProtect_(ptr, sizeInBytes, true, true));
}
//----------------------------------------------------------------------------
void FLinuxPlatformMemory::VirtualFree(void* ptr, size_t sizeInBytes, bool release) {
    Assert(ptr);
    Assert(Meta::IsAligned(ALLOCATION_GRANULARITY, ptr));
    Assert(Meta::IsAligned(ALLOCATION_GRANULARITY, sizeInBytes));

    VMPageFree_(ptr, sizeInBytes, release);
}
//----------------------------------------------------------------------------
size_t FLinuxPlatformMemory::RegionSize(void* ptr) {
    Assert(ptr);
    // relying on FVirtualMemory implementation
    AssertNotImplemented();
    return 0;
}
//----------------------------------------------------------------------------
bool FLinuxPlatformMemory::PageProtect(void* ptr, size_t sizeInBytes, bool read, bool write) {
    Assert(ptr);
    Assert(sizeInBytes);

    return VMPageProtect_(ptr, sizeInBytes, read, write);
}
//----------------------------------------------------------------------------
void FLinuxPlatformMemory::OnOutOfMemory(size_t sizeInBytes, size_t alignment) {
    // #TODO add SubmitErrorReport() ?
    UNUSED(sizeInBytes);
    UNUSED(alignment);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_LINUX