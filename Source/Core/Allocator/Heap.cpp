#include "stdafx.h"

#include "Heap.h"

#ifdef OS_WINDOWS
#   include <windows.h>
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
static void* CreateHeap_(bool/* locked */, size_t initialSize = 0, size_t maximumSize = 0) {
    const DWORD heapOpts =
        HEAP_GENERATE_EXCEPTIONS /* Throw exceptions when corrupted or out of memory */
        ;

    HANDLE const handle = ::HeapCreate(heapOpts, initialSize, maximumSize);
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
FHeap::FHeap(const char* nameForDebug, bool locked, size_t maximumSize/* = 0 */)
:   _handle(nullptr)
#ifdef USE_MEMORY_DOMAINS
,   _trackingData(nameForDebug)
#endif
    {
#ifndef USE_MEMORY_DOMAINS
    UNUSED(nameForDebug);
#endif
    _handle = CreateHeap_(locked, maximumSize, maximumSize);
}
//----------------------------------------------------------------------------
FHeap::FHeap(current_process_t)
:   _handle(nullptr)
#ifdef USE_MEMORY_DOMAINS
,   _trackingData("Heap__current_process_t")
#endif
    {
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
void* FHeap::Malloc(size_t size, FMemoryTrackingData& trackingData) {
    if (0 == size)
        return nullptr;

#ifdef USE_MEMORY_DOMAINS
    _trackingData.Allocate(1, size);
    trackingData.Allocate(1, size);
#else
    UNUSED(trackingData);
#endif
    return ::HeapAlloc(_handle, 0, size);
}
//----------------------------------------------------------------------------
void FHeap::Free(void *ptr, FMemoryTrackingData& trackingData) {
    if (nullptr == ptr)
        return;

#ifdef USE_MEMORY_DOMAINS
    const size_t blockCount = ptr ? 1 : 0;
    const size_t size = ::HeapSize(_handle, 0, ptr);
    _trackingData.Deallocate(blockCount, size);
    trackingData.Deallocate(blockCount, size);
#else
    UNUSED(trackingData);
#endif
    ::HeapFree(_handle, 0, ptr);
}
//----------------------------------------------------------------------------
void* FHeap::Calloc(size_t nmemb, size_t size, FMemoryTrackingData& trackingData) {
    if (nmemb*size == 0)
        return nullptr;

#ifdef USE_MEMORY_DOMAINS
    _trackingData.Allocate(nmemb, size);
    trackingData.Allocate(nmemb, size);
#else
    UNUSED(trackingData);
#endif
    return ::HeapAlloc(_handle, HEAP_ZERO_MEMORY, nmemb * size);
}
//----------------------------------------------------------------------------
void* FHeap::Realloc(void *ptr, size_t size, FMemoryTrackingData& trackingData) {
    if (nullptr == ptr)
        return this->Malloc(size, trackingData);

#ifdef USE_MEMORY_DOMAINS
    if (ptr) {
        const size_t oldSize = HeapSize(_handle, 0, ptr);
        _trackingData.Deallocate(1, oldSize);
        trackingData.Deallocate(1, oldSize);
    }
    if (size) {
        _trackingData.Allocate(1, size);
        trackingData.Allocate(1, size);
    }
#else
    UNUSED(trackingData);
#endif
    if (ptr)
        return ::HeapReAlloc(_handle, 0, ptr, size);
    else if (size)
        return ::HeapAlloc(_handle, 0, size);
    else
        return nullptr;
}
//----------------------------------------------------------------------------
void* FHeap::AlignedMalloc(size_t size, size_t alignment, FMemoryTrackingData& trackingData) {
    if (0 == size)
        return nullptr;

#ifdef USE_MEMORY_DOMAINS
    _trackingData.Allocate(1, size + alignment);
    trackingData.Allocate(1, size + alignment);
#else
    UNUSED(trackingData);
#endif
    void* const ptr = ::HeapAlloc(_handle, 0, size + alignment);
    void** const aligned = reinterpret_cast<void**>(((size_t)ptr + alignment) & (~(alignment - 1)));
    Assert((size_t)aligned - (size_t)ptr >= sizeof(size_t));
    aligned[-1] = ptr;
    return aligned;
}
//----------------------------------------------------------------------------
void FHeap::AlignedFree(void *ptr, FMemoryTrackingData& trackingData) {
    if (nullptr == ptr)
        return;

    void** const aligned = reinterpret_cast<void**>(ptr);
    void* const block = aligned[-1];
#ifdef USE_MEMORY_DOMAINS
    const size_t blockCount = block ? 1 : 0;
    const size_t size = HeapSize(_handle, 0, block);
    _trackingData.Deallocate(blockCount, size);
    trackingData.Deallocate(blockCount, size);
#else
    UNUSED(trackingData);
#endif
    ::HeapFree(_handle, 0, block);
}
//----------------------------------------------------------------------------
void* FHeap::AlignedCalloc(size_t nmemb, size_t size, size_t alignment, FMemoryTrackingData& trackingData) {
    if (nmemb*size == 0)
        return nullptr;

#ifdef USE_MEMORY_DOMAINS
    _trackingData.Allocate(1, size * nmemb + alignment);
    trackingData.Allocate(1, size * nmemb + alignment);
#else
    UNUSED(trackingData);
#endif
    void* const ptr = ::HeapAlloc(_handle, HEAP_ZERO_MEMORY, size * nmemb + alignment);
    void** const aligned = reinterpret_cast<void**>(((size_t)ptr + alignment) & (~(alignment - 1)));
    Assert((size_t)aligned - (size_t)ptr >= sizeof(size_t));
    aligned[-1] = ptr;
    return aligned;
}
//----------------------------------------------------------------------------
void* FHeap::AlignedRealloc(void *ptr, size_t size, size_t alignment, FMemoryTrackingData& trackingData) {
    if (nullptr == ptr)
        return this->AlignedMalloc(size, alignment, trackingData);

    void** aligned = reinterpret_cast<void**>(ptr);
    void* const block = aligned[-1];
#ifdef USE_MEMORY_DOMAINS
    const size_t blockCount = block ? 1 : 0;
    const size_t oldSize = HeapSize(_handle, 0, block);
    _trackingData.Deallocate(blockCount, oldSize);
    trackingData.Deallocate(blockCount, oldSize);
    _trackingData.Allocate(1, size);
    trackingData.Allocate(1, size);
#else
    UNUSED(trackingData);
#endif
    void* const new_ptr = ::HeapReAlloc(_handle, 0, block, size + alignment);
    aligned = reinterpret_cast<void**>(((size_t)new_ptr + alignment) & (~(alignment - 1)));
    aligned[-1] = new_ptr;
    return new_ptr;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
