#include "stdafx.h"

#include "VirtualMemory.h"

#include "Core/Diagnostic/LastError.h"
#include "Core/Misc/TargetPlatform.h"
#include "Core/Memory/MemoryDomain.h"
#include "Core/Memory/MemoryTracking.h"

#if     defined(PLATFORM_WINDOWS)
#   include "Misc/Platform_Windows.h"
#   define VMALLOC(_SizeInBytes) ::VirtualAlloc(nullptr, (_SizeInBytes), MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE)
#   define VMFREE(_Ptr, _SizeInBytes) ::VirtualFree((_Ptr), 0, MEM_RELEASE)
#elif   defined(PLATFORM_LINUX)
#   include "Misc/Platform_Linux.h"
#   define VMALLOC(_SizeInBytes) (void*)(((uintptr_t)mmap(NULL, (_SizeInBytes), PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0)+1)&~1)//with the conversion of MAP_FAILED to 0
#   define VMFREE(_Ptr, _SizeInBytes) munmap(_Ptr), (_SizeInBytes))
#else
#   error "unsupported platform"
#endif

#ifdef USE_MEMORY_DOMAINS
#   define TRACKINGDATA_ARG_IFP , FMemoryTracking& trackingData
#   define TRACKINGDATA_ARG_FWD , trackingData
#else
#   define TRACKINGDATA_ARG_IFP
#   define TRACKINGDATA_ARG_FWD
#endif

#define USE_VMALLOC_SIZE_PTRIE // This is faster than ::VirtualQuery()

#ifdef USE_VMALLOC_SIZE_PTRIE
#   include "Core/Thread/AtomicSpinLock.h"
    PRAGMA_INITSEG_COMPILER
#endif

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef USE_VMALLOC_SIZE_PTRIE
namespace {
//----------------------------------------------------------------------------
// Compressed radix trie method from :
// https://github.com/r-lyeh/ltalloc/blob/4ad53ea91c359a07f97de65d93fb8a7d279354bd/ltalloc.cc
//----------------------------------------------------------------------------
PRAGMA_MSVC_WARNING_PUSH()
PRAGMA_MSVC_WARNING_DISABLE(6001) // Using uninitialized memory 'XXX'.
PRAGMA_MSVC_WARNING_DISABLE(6011) // Dereferencing NULL pointer 'XXX'.
//----------------------------------------------------------------------------
#if     defined(PLATFORM_WINDOWS)
#   define BSR(r, v) CODE3264(_BitScanReverse, _BitScanReverse64)((unsigned long*)&r, v)
#elif   defined(PLATFORM_LINUX)
#   define BSR(r, v) r = CODE3264(__builtin_clz(v) ^ 31, __builtin_clzll(v) ^ 63)//x ^ 31 = 31 - x, but gcc does not optimize 31 - __builtin_clz(x) to bsr(x), but generates 31 - (bsr(x) ^ 31)
#else
#   error "unsupported platform !"
#endif
//----------------------------------------------------------------------------
struct ALIGN(16) FVMPtrieNode_ {
    uintptr_t Keys[2];
    FVMPtrieNode_* Children[2];
    static FVMPtrieNode_* const Null;
};
STATIC_ASSERT(sizeof(FVMPtrieNode_) == 4 * sizeof(size_t));
FVMPtrieNode_* const FVMPtrieNode_::Null = (FVMPtrieNode_*)(uintptr_t)1;
//----------------------------------------------------------------------------
static uintptr_t VMPtrie_lookup_(const FVMPtrieNode_* root, uintptr_t key) {
    const FVMPtrieNode_* node = root;
    const uintptr_t* lastKey = NULL;

    while (!((uintptr_t)node & 1)) {
        int branch = (key >> (node->Keys[0] & 0xFF)) & 1;
        lastKey = &node->Keys[branch];
        node = node->Children[branch];
    }

    Assert(lastKey && (*lastKey & ~0xFF) == key);
    return ((uintptr_t)node & ~1); // can't fail, key is *ALWAYS* here
}
//----------------------------------------------------------------------------
static void VMPtrie_insert_(FVMPtrieNode_** root, uintptr_t key, uintptr_t value, FVMPtrieNode_* newNode) {
    FVMPtrieNode_** node = root, *n;
    uintptr_t* prevKey = nullptr, x, pkey;
    unsigned int index, b;
    Assert(!((value & 1) | (key & 0xFF)));//check constraints for key/value

    for (;;) {
        n = *node;

        if (!((uintptr_t)n & 1)) {//not a leaf
            int prefixEnd = n->Keys[0] & 0xFF;
            x = key ^ n->Keys[0];// & ~0xFF;
            if (!(x & (~(uintptr_t)1 << prefixEnd))) {//prefix matches, so go on
                int branch = (key >> prefixEnd) & 1;
                node = &n->Children[branch];
                prevKey = &n->Keys[branch];
            }
            else {//insert a new node before current
                pkey = n->Keys[0] & ~0xFF;
                break;
            }
        }
        else {//leaf
            if (*node == FVMPtrieNode_::Null) {
                *node = newNode;
                newNode->Keys[0] = key;//left prefixEnd = 0, so all following insertions will be before this node
                newNode->Children[0] = (FVMPtrieNode_*)(value | 1);
                newNode->Children[1] = FVMPtrieNode_::Null;
                return;
            }
            else {
                pkey = *prevKey & ~0xFF;
                x = key ^ pkey;
                Assert(x/*key != pkey*/ && "key already inserted");
                break;
            }
        }
    }

    BSR(index, x);

    b = (key >> index) & 1;
    newNode->Keys[b] = key;
    newNode->Keys[b ^ 1] = pkey;
    newNode->Keys[0] |= index;
    newNode->Children[b] = (FVMPtrieNode_*)(value | 1);
    newNode->Children[b ^ 1] = n;

    *node = newNode;
}
//----------------------------------------------------------------------------
static uintptr_t VMPtrie_erase_(FVMPtrieNode_** root, uintptr_t key, FVMPtrieNode_** freeList) {
    FVMPtrieNode_** node = root;
    uintptr_t *pkey = NULL;
    Assert(*root != FVMPtrieNode_::Null && "trie is empty!");

    for (;;) {
        FVMPtrieNode_* n = *node;
        int branch = (key >> (n->Keys[0] & 0xFF)) & 1;
        FVMPtrieNode_* cn = n->Children[branch];//current child node

        if ((uintptr_t)cn & 1) {//leaf
            FVMPtrieNode_* other = n->Children[branch ^ 1];

            Assert((n->Keys[branch] & ~0xFF) == key);
            Assert(cn != FVMPtrieNode_::Null && "node's key is probably broken");

            if (((uintptr_t)other & 1) && other != FVMPtrieNode_::Null)//if other node is not a pointer
                *pkey = (n->Keys[branch ^ 1] & ~0xFF) | ((*pkey) & 0xFF);

            *node = other;
            *(FVMPtrieNode_**)n = *freeList; *freeList = n;//free(n);

            return ((uintptr_t)cn & ~1);
        }
        pkey = &n->Keys[branch];
        node = &n->Children[branch];
    }
}
//----------------------------------------------------------------------------
static FAtomicSpinLock GVMPtrieLock;
static FVMPtrieNode_* GVMPtrieRoot = FVMPtrieNode_::Null;
static FVMPtrieNode_* GVMPtrieFreeList = nullptr;
static FVMPtrieNode_* GVMPtrieNewAllocatedPage = nullptr;
//----------------------------------------------------------------------------
static void VMRegisterBlockSize_(void* ptr, size_t sizeInBytes) {
    FVMPtrieNode_* newNode;
    GVMPtrieLock.Lock();

    if (GVMPtrieFreeList) {
        GVMPtrieFreeList = *(FVMPtrieNode_**)(newNode = GVMPtrieFreeList);
    }
    else if (GVMPtrieNewAllocatedPage) {
        newNode = GVMPtrieNewAllocatedPage;
        if (!((uintptr_t)++GVMPtrieNewAllocatedPage & (ALLOCATION_GRANULARITY - 1)))
            GVMPtrieNewAllocatedPage = ((FVMPtrieNode_**)GVMPtrieNewAllocatedPage)[-1];
    }
    else {
        GVMPtrieLock.Unlock();

        newNode = (FVMPtrieNode_*)VMALLOC(ALLOCATION_GRANULARITY);
        if (!newNode)
            CORE_THROW_IT(std::bad_alloc());

#ifdef USE_MEMORY_DOMAINS
        // Memory allocated here will never be freed !
        // But we're talking about never more than a few megabytes
        // 1024 * 1024 / 32 = 32768 *ALIVE* allocations for 1 mo on x64 architecture, 65536 for x86
        MEMORY_DOMAIN_TRACKING_DATA(Internal).Allocate(1, ALLOCATION_GRANULARITY);
#endif

        Assert(((char**)((char*)newNode + ALLOCATION_GRANULARITY))[-1] == 0);

        GVMPtrieLock.Lock();
        ((FVMPtrieNode_**)((char*)newNode + ALLOCATION_GRANULARITY))[-1] = GVMPtrieNewAllocatedPage;//in case if other thread also have just allocated a new page
        GVMPtrieNewAllocatedPage = newNode + 1;
    }

    VMPtrie_insert_(&GVMPtrieRoot, (uintptr_t)ptr, sizeInBytes, newNode);
    GVMPtrieLock.Unlock();
}
//----------------------------------------------------------------------------
static size_t VMFetchBlockSize_(void* ptr) {
    const FAtomicSpinLock::FScope scopeLock(GVMPtrieLock);
    return VMPtrie_lookup_(GVMPtrieRoot, (uintptr_t)ptr);
}
//----------------------------------------------------------------------------
static size_t VMReleaseBlockSize_(void* ptr) {
    const FAtomicSpinLock::FScope scopeLock(GVMPtrieLock);
    return size_t(VMPtrie_erase_(&GVMPtrieRoot, (uintptr_t)ptr, &GVMPtrieFreeList));
}
//----------------------------------------------------------------------------
#undef BSR
//----------------------------------------------------------------------------
PRAGMA_MSVC_WARNING_POP()
//----------------------------------------------------------------------------
} //!namespace
#endif //!USE_VMALLOC_SIZE_PTRIE
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
size_t FVirtualMemory::AllocSizeInBytes(void* ptr) {
    if (nullptr == ptr)
        return 0;

    Assert(Meta::IsAligned(ALLOCATION_GRANULARITY, ptr));

#ifdef USE_VMALLOC_SIZE_PTRIE
    const size_t regionSize = VMFetchBlockSize_(ptr);
    Assert(Meta::IsAligned(ALLOCATION_GRANULARITY, regionSize));

    return regionSize;

#else
#   if defined(PLATFORM_WINDOWS)
    //  https://msdn.microsoft.com/en-us/library/windows/desktop/aa366902(v=vs.85).aspx
    ::MEMORY_BASIC_INFORMATION info;
    if (0 == ::VirtualQuery(ptr, &info, sizeof(info)))
        AssertNotReached();

    Assert(ptr == info.BaseAddress);

    return info.RegionSize;
#   else
#       error "not implemented !"
#   endif
#endif
}
//----------------------------------------------------------------------------
void* FVirtualMemory::Alloc(size_t sizeInBytes) {
    Assert(sizeInBytes);

    void* const vmem = VMALLOC(sizeInBytes);

#ifdef USE_VMALLOC_SIZE_PTRIE
    VMRegisterBlockSize_(vmem, sizeInBytes);
#endif

    return vmem;
}
//----------------------------------------------------------------------------
void FVirtualMemory::Free(void* ptr, size_t sizeInBytes) {
    Assert(ptr);

#ifdef USE_VMALLOC_SIZE_PTRIE
    const size_t regionSize = VMReleaseBlockSize_(ptr);
    Assert(regionSize == sizeInBytes);
#endif

    VMFREE(ptr, sizeInBytes);
}
//----------------------------------------------------------------------------
bool FVirtualMemory::Protect(void* ptr, size_t sizeInBytes, bool read, bool write) {
    Assert(ptr);
    Assert(sizeInBytes);

#if     defined(PLATFORM_WINDOWS)
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

#else
#       error "Unsupported platform !"
#endif
}
//----------------------------------------------------------------------------
// Keep allocations aligned to OS granularity
// https://github.com/r-lyeh/ltalloc/blob/master/ltalloc.cc
PRAGMA_MSVC_WARNING_PUSH()
PRAGMA_MSVC_WARNING_DISABLE(6001) // Using uninitialized memory 'XXX'.
void* FVirtualMemory::AlignedAlloc(size_t alignment, size_t sizeInBytes) {
    Assert(sizeInBytes);
    Assert(Meta::IsAligned(ALLOCATION_GRANULARITY, sizeInBytes));
    Assert(Meta::IsPow2(alignment));


#if     defined(PLATFORM_WINDOWS)
    // Optimistically try mapping precisely the right amount before falling back to the slow method :
    void* p = ::VirtualAlloc(nullptr, sizeInBytes, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
#ifdef USE_MEMORY_DOMAINS
    if (p)
        MEMORY_DOMAIN_TRACKING_DATA(Reserved).Allocate(1, sizeInBytes);
#endif

    if (not Meta::IsAligned(alignment, p)) {
        const size_t allocationGranularity = FPlatformMisc::SystemInfo.AllocationGranularity;

        // Fill "bubbles" (reserve unaligned regions) at the beginning of virtual address space, otherwise there will be always falling back to the slow method
        if ((uintptr_t)p < 16 * 1024 * 1024)
            ::VirtualAlloc(p, alignment - ((uintptr_t)p & (alignment - 1)), MEM_RESERVE, PAGE_NOACCESS);

        do {
            p = ::VirtualAlloc(NULL, sizeInBytes + alignment - allocationGranularity, MEM_RESERVE, PAGE_NOACCESS);
            if (nullptr == p)
                return nullptr;

            ::VirtualFree(p, 0, MEM_RELEASE);// Unfortunately, WinAPI doesn't support release a part of allocated region, so release a whole region

            p = ::VirtualAlloc(
                (void*)(((uintptr_t)p + (alignment - 1)) & ~(alignment - 1)),
                sizeInBytes,
                MEM_RESERVE|MEM_COMMIT,
                PAGE_READWRITE );

        } while (nullptr == p);
    }

#elif   defined(PLATFORM_LINUX)
    void* p = (void*)(((uintptr_t)::mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0) + 1)&~1);//with the conversion of MAP_FAILED to 0

    if (not Meta::IsAligned(alignment, p)) {
        p = VMALLOC(size + alignment - ALLOCATION_GRANULARITY());
        if (p/* != MAP_FAILED*/) {
            uintptr_t ap = ((uintptr_t)p + (alignment - 1)) & ~(alignment - 1);
            uintptr_t diff = ap - (uintptr_t)p;
            if (diff) VMFREE(p, diff);
            diff = alignment - ALLOCATION_GRANULARITY() - diff;
            assert((intptr_t)diff >= 0);
            if (diff) VMFREE((void*)(ap + size), diff);
            return (void*)ap;
        }
    }

#else
#   error "unsupported platform"
#endif

#ifdef USE_VMALLOC_SIZE_PTRIE
    VMRegisterBlockSize_(p, sizeInBytes);
#endif

    return p;
}
PRAGMA_MSVC_WARNING_POP()
//----------------------------------------------------------------------------
void FVirtualMemory::AlignedFree(void* ptr, size_t sizeInBytes) {
    Assert(ptr);
    Assert(sizeInBytes);

#ifdef USE_VMALLOC_SIZE_PTRIE
    const size_t regionSize = VMReleaseBlockSize_(ptr);
    Assert(regionSize == sizeInBytes);
#endif

#ifdef USE_MEMORY_DOMAINS
    if (ptr)
        MEMORY_DOMAIN_TRACKING_DATA(Reserved).Deallocate(1, sizeInBytes);
#endif

#if     defined(PLATFORM_WINDOWS)
    UNUSED(sizeInBytes);
    if (0 == ::VirtualFree(ptr, 0, MEM_RELEASE))
        CORE_THROW_IT(FLastErrorException("VirtualFree"));

#elif   defined(PLATFORM_LINUX)

#else
#   error "unsupported platform"
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVirtualMemoryCache::FVirtualMemoryCache()
    : FreePageBlockCount(0)
    , TotalCacheSizeInBytes(0)
{}
//----------------------------------------------------------------------------
void* FVirtualMemoryCache::Allocate(size_t sizeInBytes, FFreePageBlock* first, size_t maxCacheSizeInBytes TRACKINGDATA_ARG_IFP) {
    const size_t alignment = FPlatformMisc::SystemInfo.AllocationGranularity;
    Assert(Meta::IsAligned(alignment, sizeInBytes));

    if (FreePageBlockCount && (sizeInBytes <= maxCacheSizeInBytes / 4)) {
        Assume(first);

        FFreePageBlock* cachedBlock = nullptr;

        FFreePageBlock* const last = first + FreePageBlockCount;
        for (FFreePageBlock* block = first; block != last; ++block) {
            // look for exact matches first, these are aligned to the page size, so it should be quite common to hit these on small pages sizes
            if (block->SizeInBytes == sizeInBytes) {
                cachedBlock = block;
                break;
            }
        }

#if 0 // this is better to release the chunks which are too large, since the client can't use the extra memory
        if (nullptr == cachedBlock) {
            const size_t sizeTimes4 = sizeInBytes * 4;

            for (FFreePageBlock* block = first; block != last; ++block) {
                // is it possible (and worth i.e. <25% overhead) to use this block
                if (block->SizeInBytes >= sizeInBytes && block->SizeInBytes * 3 <= sizeTimes4) {
                    cachedBlock = block;
                    break;
                }
            }
        }
#endif

        if (nullptr != cachedBlock) {
            void* result = cachedBlock->Ptr;
            Assert(nullptr != result);

            FreePageBlockCount--;
            TotalCacheSizeInBytes -= cachedBlock->SizeInBytes;

#ifdef USE_MEMORY_DOMAINS
            // Only track overhead due to cached memory, actual blocks in use should be logged in their own domain
            trackingData.Deallocate(1, cachedBlock->SizeInBytes);
#endif

            if (cachedBlock + 1 != last)
                ::memmove(cachedBlock, cachedBlock + 1, sizeof(FFreePageBlock) * (last - cachedBlock - 1));

            Assert(Meta::IsAligned(alignment, result));
            return result;
        }

        if (void* result = FVirtualMemory::AlignedAlloc(alignment, sizeInBytes)) {
            Assert(Meta::IsAligned(alignment, result));
            return result;
        }

        // Are we holding on to much mem? Release it all.
        ReleaseAll(first TRACKINGDATA_ARG_FWD);
    }

    void* result = FVirtualMemory::AlignedAlloc(alignment, sizeInBytes);
    Assert(Meta::IsAligned(alignment, result));
    Assert(FVirtualMemory::AllocSizeInBytes(result) == sizeInBytes);

    return result;
}
//----------------------------------------------------------------------------
void FVirtualMemoryCache::Free(void* ptr, size_t sizeInBytes, FFreePageBlock* first, size_t cacheBlocksCapacity, size_t maxCacheSizeInBytes TRACKINGDATA_ARG_IFP) {
    if (0 == sizeInBytes)
        sizeInBytes = FVirtualMemory::AllocSizeInBytes(ptr);

    Assert(Meta::IsAligned(FPlatformMisc::SystemInfo.AllocationGranularity, sizeInBytes));

    if (sizeInBytes > maxCacheSizeInBytes / 4) {
        FVirtualMemory::AlignedFree(ptr, sizeInBytes);
        return;
    }

    while (FreePageBlockCount >= cacheBlocksCapacity || TotalCacheSizeInBytes + sizeInBytes > maxCacheSizeInBytes) {
        Assert(FreePageBlockCount);

        FVirtualMemory::AlignedFree(first->Ptr, first->SizeInBytes);

        Assert(TotalCacheSizeInBytes >= first->SizeInBytes);
        TotalCacheSizeInBytes -= first->SizeInBytes;

#ifdef USE_MEMORY_DOMAINS
        // Only track overhead due to cached memory, actual blocks in use should be logger in their owning domain
        trackingData.Deallocate(1, first->SizeInBytes);
#endif

#ifdef _DEBUG
        first->Ptr = nullptr;
        first->SizeInBytes = 0;
#endif

        if (--FreePageBlockCount)
            ::memmove(first, first + 1, sizeof(FFreePageBlock) * FreePageBlockCount);
    }

    ONLY_IF_ASSERT(::memset(ptr, 0xDD, sizeInBytes)); // trash the memory block before caching

    first[FreePageBlockCount] = FFreePageBlock{ ptr, sizeInBytes };
    TotalCacheSizeInBytes += sizeInBytes;
    FreePageBlockCount++;

#ifdef USE_MEMORY_DOMAINS
    // Only track overhead due to cached memory, actual blocks in use should be logger in their owning domain
    trackingData.Allocate(1, sizeInBytes);
#endif
}
//----------------------------------------------------------------------------
void FVirtualMemoryCache::ReleaseAll(FFreePageBlock* first TRACKINGDATA_ARG_IFP) {
    for (   FFreePageBlock* const last = (first + FreePageBlockCount);
            first != last;
            ++first ) {
        Assert(Meta::IsAligned(FPlatformMisc::SystemInfo.AllocationGranularity, first->Ptr));

        FVirtualMemory::AlignedFree(first->Ptr, first->SizeInBytes);

        Assert(TotalCacheSizeInBytes >= first->SizeInBytes);
        TotalCacheSizeInBytes -= first->SizeInBytes;

#ifdef USE_MEMORY_DOMAINS
        // Only track overhead due to cached memory, actual blocks in use should be logger in their owning domain
        trackingData.Deallocate(1, first->SizeInBytes);
#endif

#ifdef _DEBUG
        first->Ptr = nullptr;
        first->SizeInBytes = 0;
#endif
    }
    Assert(0 == TotalCacheSizeInBytes);
    FreePageBlockCount = 0;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#undef TRACKINGDATA_ARG_IFP
