#include "stdafx.h"

#include "Heap.h"

#define WITH_CORE_USE_NATIVEHEAP 0 //%_NOCOMMIT%

#if WITH_CORE_USE_NATIVEHEAP

#ifdef PLATFORM_WINDOWS
#   include "Misc/Platform_Windows.h"
#   include <HeapApi.h>
#else
#   error "no support"
#endif

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static void* CreateHeap_() {
    const DWORD heapOpts =
        HEAP_GENERATE_EXCEPTIONS /* Throw exceptions when corrupted or out of memory */
        ;

    HANDLE const handle = ::HeapCreate(heapOpts, 0, 0);
    if (!handle)
        CORE_THROW_IT(std::bad_alloc());

#if _DEBUG
    // Enable heap terminate-on-corruption.
    // A correct application can continue to run even if this call fails,
    // so it is safe to ignore the return value and call the function as follows:
    // (void)HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);
    ::HeapSetInformation(handle, HeapEnableTerminationOnCorruption, NULL, 0);
#endif

    // Enable the low-fragmenation heap (LFH). Starting with Windows Vista,
    // the LFH is enabled by default but this call does not cause an error.
    ULONG  HeapFragValue = 2; // Low fragmentation heap
    ::HeapSetInformation(handle, HeapCompatibilityInformation, &HeapFragValue, sizeof(HeapFragValue));

    return handle;
}
//----------------------------------------------------------------------------
#ifdef USE_HEAP_VALIDATION
static void ValidateHeap_(void* handle, void* ptr = nullptr) {
    Assert(handle);
    if (!::HeapValidate(handle, 0, ptr))
        CORE_THROW_IT(std::bad_alloc());
}
#endif
//----------------------------------------------------------------------------
static void DestroyHeap_(void* handle) {
#ifdef USE_HEAP_VALIDATION
    ValidateHeap_(handle);
#endif

    if (!::HeapDestroy(handle))
        CORE_THROW_IT(std::bad_alloc());
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FHeap::FHeap()
:   _handle(nullptr) {
    _handle = CreateHeap_();
}
//----------------------------------------------------------------------------
FHeap::FHeap(current_process_t)
:   _handle(nullptr) {
    _handle = ::GetProcessHeap();
    Assert(_handle);
}
//----------------------------------------------------------------------------
FHeap::FHeap(FHeap&& rvalue)
:   _handle(nullptr) {
    std::swap(rvalue._handle, _handle);
}
//----------------------------------------------------------------------------
FHeap::~FHeap() {
    if (nullptr != _handle && GetProcessHeap() != _handle)
        DestroyHeap_(_handle);
}
//----------------------------------------------------------------------------
void* FHeap::Malloc(size_t size) {
    if (0 == size)
        return nullptr;

    return ::HeapAlloc(_handle, 0, size);
}
//----------------------------------------------------------------------------
void FHeap::Free(void *ptr) {
    if (nullptr == ptr)
        return;

    ::HeapFree(_handle, 0, ptr);
}
//----------------------------------------------------------------------------
void* FHeap::Calloc(size_t nmemb, size_t size) {
    if (nmemb*size == 0)
        return nullptr;

    return ::HeapAlloc(_handle, HEAP_ZERO_MEMORY, nmemb * size);
}
//----------------------------------------------------------------------------
void* FHeap::Realloc(void *ptr, size_t size) {
    if (nullptr == ptr)
        return this->Malloc(size);

    if (ptr)
        return ::HeapReAlloc(_handle, 0, ptr, size);
    else if (size)
        return ::HeapAlloc(_handle, 0, size);
    else
        return nullptr;
}
//----------------------------------------------------------------------------
void* FHeap::AlignedMalloc(size_t size, size_t alignment) {
    if (0 == size)
        return nullptr;

    void* const ptr = ::HeapAlloc(_handle, 0, size + alignment);
    void** const aligned = reinterpret_cast<void**>(((size_t)ptr + alignment) & (~(alignment - 1)));
    Assert((size_t)aligned - (size_t)ptr >= sizeof(size_t));
    aligned[-1] = ptr;
    return aligned;
}
//----------------------------------------------------------------------------
void FHeap::AlignedFree(void *ptr) {
    if (nullptr == ptr)
        return;

    void** const aligned = reinterpret_cast<void**>(ptr);
    void* const block = aligned[-1];

    ::HeapFree(_handle, 0, block);
}
//----------------------------------------------------------------------------
void* FHeap::AlignedCalloc(size_t nmemb, size_t size, size_t alignment) {
    if (nmemb*size == 0)
        return nullptr;

    void* const ptr = ::HeapAlloc(_handle, HEAP_ZERO_MEMORY, size * nmemb + alignment);
    void** const aligned = reinterpret_cast<void**>(((size_t)ptr + alignment) & (~(alignment - 1)));
    Assert((size_t)aligned - (size_t)ptr >= sizeof(size_t));
    aligned[-1] = ptr;
    return aligned;
}
//----------------------------------------------------------------------------
void* FHeap::AlignedRealloc(void *ptr, size_t size, size_t alignment) {
    if (nullptr == ptr)
        return this->AlignedMalloc(size, alignment);

    void** aligned = reinterpret_cast<void**>(ptr);
    void* const block = aligned[-1];

    void* const new_ptr = ::HeapReAlloc(_handle, 0, block, size + alignment);
    aligned = reinterpret_cast<void**>(((size_t)new_ptr + alignment) & (~(alignment - 1)));
    aligned[-1] = new_ptr;
    return new_ptr;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#else //WITH_CORE_USE_NATIVEHEAP

#include "Malloc.h"
#include "VirtualMemory.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
struct FHeapHandle_ {
    STATIC_ASSERT(PAGE_SIZE == 64 * 1024);
    STATIC_CONST_INTEGRAL(size_t, LargeAllocSize, 32736/* FMallocBinned */);

#ifdef WITH_CORE_ASSERT
    const size_t Canary0 = 0xBAADF00D;
#endif

    VIRTUALMEMORYCACHE(Heap, 16, 2 * 1024 * 1024) LocalVM; // 16 cached entries, max 2 mo

#ifdef WITH_CORE_ASSERT
    size_t NumAllocs = 0;
    const size_t Canary1 = 0xF00DBAAD;
    void CheckCanaries() const {
        Assert(Canary0 == 0xBAADF00D);
        Assert(Canary1 == 0xF00DBAAD);
    }
    ~FHeapHandle_() {
        ONLY_IF_ASSERT(CheckCanaries());
        Assert(0 == NumAllocs);
    };
#endif

    void* Allocate(size_t sizeInBytes) {
        ONLY_IF_ASSERT(CheckCanaries());
        Assert(sizeInBytes > LargeAllocSize);
        sizeInBytes = ROUND_TO_NEXT_64K(sizeInBytes);

        void* const result = LocalVM.Allocate(sizeInBytes);
        AssertRelease(result);
        Assert(Meta::IsAligned(PAGE_SIZE, result));
        ONLY_IF_ASSERT(NumAllocs++);

        return result;
    }

    void Free(void* ptr) {
        ONLY_IF_ASSERT(CheckCanaries());
        Assert(ptr);
        Assert(Meta::IsAligned(PAGE_SIZE, ptr));
        Assert(0 < NumAllocs);

        ONLY_IF_ASSERT(NumAllocs--);
        LocalVM.Free(ptr);
    }

    static void* Create() { return new FHeapHandle_{}; }
    static void Destroy(void* handle) { Assert(handle); delete(handle); }

    static FHeapHandle_* Get(void* handle) {
        Assert(handle);
        return reinterpret_cast<FHeapHandle_*>(handle);
    }
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FHeap::FHeap()
:   _handle(nullptr) {}
//----------------------------------------------------------------------------
FHeap::FHeap(current_process_t)
:   _handle(nullptr) {}
//----------------------------------------------------------------------------
FHeap::FHeap(FHeap&& rvalue)
:   _handle(nullptr) {
    std::swap(rvalue._handle, _handle);
}
//----------------------------------------------------------------------------
FHeap::~FHeap() {
    if (nullptr != _handle)
        FHeapHandle_::Destroy(_handle);
}
//----------------------------------------------------------------------------
void* FHeap::Malloc(size_t size) {
    if (0 == size)
        return nullptr;

    void* result;

    if (size <= FHeapHandle_::LargeAllocSize) {
        // there's probably already a thread local cache efficient for small blocks
        result = Core::malloc(size);

        Assert(not Meta::IsAligned(PAGE_SIZE, result));
    }
    else {
        // fallback to a local cache of large blocks
        if (nullptr == _handle)
            _handle = FHeapHandle_::Create();

        result = FHeapHandle_::Get(_handle)->Allocate(size);
    }

    return result;
}
//----------------------------------------------------------------------------
void FHeap::Free(void *ptr) {
    if (nullptr == ptr)
        return;

    if (not Meta::IsAligned(PAGE_SIZE, ptr)) {
        // return to standard allocator
        Core::free(ptr);
    }
    else {
        // release block to local cache
        FHeapHandle_::Get(_handle)->Free(ptr);
    }
}
//----------------------------------------------------------------------------
void* FHeap::Calloc(size_t nmemb, size_t size) {
    if (nmemb*size == 0)
        return nullptr;

    const size_t sizeInBytes = (nmemb * size);
    void* result = this->Malloc(sizeInBytes);
    ::memset(result, 0x00, sizeInBytes);

    return result;
}
//----------------------------------------------------------------------------
void* FHeap::Realloc(void *ptr, size_t size) {
    void* result;

    if (nullptr == ptr) {
        result = this->Malloc(size);
    }
    else if (0 == size) {
        this->Free(ptr);
        result = nullptr;
    }
    else if (not Meta::IsAligned(PAGE_SIZE, ptr)) {
        result = Core::realloc(ptr, size);
        Assert(not Meta::IsAligned(PAGE_SIZE, ptr));
    }
    else {
        const size_t oldSize = FVirtualMemory::AllocSizeInBytes(ptr);
        if (ROUND_TO_NEXT_64K(size) == oldSize)
            return ptr;

        result = this->Malloc(size);
        ::memcpy(result, ptr, Min(size, oldSize));

        FHeapHandle_::Get(_handle)->Free(ptr);
    }

    return result;
}
//----------------------------------------------------------------------------
void* FHeap::AlignedMalloc(size_t size, size_t alignment) {
    if (0 == size)
        return nullptr;

    void* result;

    if (size < FHeapHandle_::LargeAllocSize) {
        // there's probably already a thread local cache efficient for small blocks
        result = Core::aligned_malloc(size, alignment);

        Assert(not Meta::IsAligned(PAGE_SIZE, result));
    }
    else {
        // fallback to a local cache of large blocks
        if (nullptr == _handle)
            _handle = FHeapHandle_::Create();

        result = FHeapHandle_::Get(_handle)->Allocate(size);

        Assert(Meta::IsAligned(alignment, result));
    }

    return result;
}
//----------------------------------------------------------------------------
void FHeap::AlignedFree(void *ptr) {
    if (nullptr == ptr)
        return;

    if (not Meta::IsAligned(PAGE_SIZE, ptr)) {
        // return to standard allocator
        Core::aligned_free(ptr);
    }
    else {
        // release block to local cache
        FHeapHandle_::Get(_handle)->Free(ptr);
    }
}
//----------------------------------------------------------------------------
void* FHeap::AlignedCalloc(size_t nmemb, size_t size, size_t alignment) {
    if (nmemb*size == 0)
        return nullptr;

    const size_t sizeInBytes = (nmemb * size);
    void* const result = this->AlignedMalloc(sizeInBytes, alignment);
    ::memset(result, 0x00, sizeInBytes);

    return result;
}
//----------------------------------------------------------------------------
void* FHeap::AlignedRealloc(void *ptr, size_t size, size_t alignment) {
    void* result;

    if (nullptr == ptr) {
        result = this->AlignedMalloc(size, alignment);
    }
    else if (0 == size) {
        this->AlignedFree(ptr);
        result = nullptr;
    }
    else if (not Meta::IsAligned(PAGE_SIZE, ptr)) {
        result = Core::aligned_realloc(ptr, size, alignment);
    }
    else {
        const size_t oldSize = FVirtualMemory::AllocSizeInBytes(ptr);
        if (ROUND_TO_NEXT_64K(size) == oldSize)
            return ptr;

        result = this->AlignedMalloc(size, alignment);
        ::memcpy(result, ptr, Min(size, oldSize));

        FHeapHandle_::Get(_handle)->Free(ptr);
    }

    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#endif //!WITH_CORE_USE_NATIVEHEAP
