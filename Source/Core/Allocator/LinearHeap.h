#pragma once

#include "Core/Core.h"

#include "Core/Allocator/AllocatorBase.h"
#include "Core/Memory/MemoryDomain.h"
#include "Core/Memory/MemoryTracking.h"

#include <atomic>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define VECTOR_LINEARHEAP(T) ::Core::TVector<T, ::Core::TLinearHeapAllocator<T>>
//----------------------------------------------------------------------------
#ifdef USE_MEMORY_DOMAINS
#   define LINEARHEAP_DOMAIN_TRACKINGDATA(_DOMAIN) &MEMORY_DOMAIN_TRACKING_DATA(Json)
#else
#   define LINEARHEAP_DOMAIN_TRACKINGDATA(_DOMAIN)
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class CORE_API FLinearHeap {
public:
#ifdef USE_MEMORY_DOMAINS
    explicit FLinearHeap(FMemoryTracking* parent);
#else
    FLinearHeap();
#endif

    ~FLinearHeap();

    FLinearHeap(const FLinearHeap&) = delete;
    FLinearHeap& operator =(const FLinearHeap&) = delete;

    FLinearHeap(FLinearHeap&&) = delete;
    FLinearHeap& operator =(FLinearHeap&&) = delete;

#ifdef USE_MEMORY_DOMAINS
    const FMemoryTracking& TrackingData() const { return _trackingData; }
#endif

    void* Allocate(size_t size, size_t alignment = ALLOCATION_BOUNDARY);
    void* Relocate(void* ptr, size_t newSize, size_t oldSize, size_t alignment = ALLOCATION_BOUNDARY);
    void  Release(void* ptr, size_t size);

    void ReleaseAll();

    static const size_t MaxBlockSize;

private:
    void* _blocks;

#ifdef USE_MEMORY_DOMAINS
    FMemoryTracking _trackingData;
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Alignment = sizeof(ptrdiff_t) >
class TLinearHeapAllocator : public TAllocatorBase<T> {
public:
    template <typename U, size_t A>
    friend class TLinearHeapAllocator;

    typedef TAllocatorBase<T> base_type;

    using typename base_type::pointer;
    using typename base_type::size_type;

    typedef std::true_type propagate_on_container_copy_assignment;
    typedef std::true_type propagate_on_container_move_assignment;
    typedef std::true_type propagate_on_container_swap;
    typedef std::false_type is_always_equal;

    using base_type::address;
    using base_type::construct;
    using base_type::destroy;
    using base_type::max_size;

    template<typename U>
    struct rebind {
        typedef TLinearHeapAllocator<U> other;
    };

    TLinearHeapAllocator(FLinearHeap& heap) noexcept
        : _heap(&heap) {
        Assert(_heap);
    }

    TLinearHeapAllocator(const TLinearHeapAllocator& other) noexcept
        : TLinearHeapAllocator(*other._heap) {}

    template <typename U>
    TLinearHeapAllocator(const TLinearHeapAllocator<U>& other) noexcept
        : TLinearHeapAllocator(*other._heap) {}

    TLinearHeapAllocator& operator=(const TLinearHeapAllocator& other) noexcept {
        _heap = other._heap;
        return *this;
    }

    template <typename U>
    TLinearHeapAllocator& operator=(const TLinearHeapAllocator<U>& other) noexcept {
        _heap = other._heap;
        return *this;
    }

    FLinearHeap* Heap() const { return _heap; }

    pointer allocate(size_type n);
    pointer allocate(size_type n, const void* /*hint*/) { return allocate(n); }
    void deallocate(void* p, size_type n);

    // see AllocatorRealloc()
    void* relocate(void* p, size_type newSize, size_type oldSize);

private:
    FLinearHeap* _heap;
};
//----------------------------------------------------------------------------
template <typename T, size_t _Alignment>
auto TLinearHeapAllocator<T, _Alignment>::allocate(size_type n) -> pointer {
    // The return value of allocate(0) is unspecified.
    // TLinearHeapAllocator returns NULL in order to avoid depending
    // on malloc(0)'s implementation-defined behavior
    // (the implementation can define malloc(0) to return NULL,
    // in which case the bad_alloc check below would fire).
    // All allocators can return NULL in this case.
    if (n == 0)
        return nullptr;

    // All allocators should contain an integer overflow check.
    // The Standardization Committee recommends that std::length_error
    // be thrown in the case of integer overflow.
    if (n > max_size())
        CORE_THROW_IT(std::length_error("TLinearHeapAllocator<T>::allocate() - Integer overflow."));

    // TLinearHeapAllocator wraps FLinearHeap
    Assert(_heap);
    void * const pv = _heap->Allocate(n * sizeof(T), _Alignment);

    // Allocators should throw std::bad_alloc in the case of memory allocation failure.
    if (pv == nullptr)
        CORE_THROW_IT(std::bad_alloc());

    return static_cast<T *>(pv);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Alignment>
void TLinearHeapAllocator<T, _Alignment>::deallocate(void* p, size_type n) {
    Assert(_heap);

    if (p) {
        Assert(n);
        // TLinearHeapAllocator wraps FLinearHeap
        _heap->Release(p, n * sizeof(T));
    }
    else {
        Assert(0 == n);
    }
}
//----------------------------------------------------------------------------
template <typename T, size_t _Alignment>
void* TLinearHeapAllocator<T, _Alignment>::relocate(void* p, size_type newSize, size_type oldSize) {
    // TLinearHeapAllocator wraps FLinearHeap
    Assert(_heap);
    void* const newp = _heap->Relocate(p, newSize, oldSize, _Alignment);
    if (nullptr == newp && newSize)
        CORE_THROW_IT(std::bad_alloc());

    return newp;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t A, typename U, size_t B>
bool operator ==(const TLinearHeapAllocator<T, A>& lhs, const TLinearHeapAllocator<U, B>& rhs) {
    return (lhs.Heap() == rhs.Heap());
}
//----------------------------------------------------------------------------
template <typename T, size_t A, typename U, size_t B>
bool operator !=(const TLinearHeapAllocator<T, A>& lhs, const TLinearHeapAllocator<U, B>& rhs) {
    return !operator ==(lhs, rhs);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Alignment>
size_t AllocatorSnapSize(const TLinearHeapAllocator<T, _Alignment>&, size_t size) {
    return (ROUND_TO_NEXT_16(size * sizeof(T)) / sizeof(T)); // align on 16 bytes boundary
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename U, size_t _Alignment, typename V>
struct allocator_can_steal_from<
    TLinearHeapAllocator<U, _Alignment>,
    TLinearHeapAllocator<V, _Alignment>
> : Meta::TIntegralConstant<bool, true> {};
//----------------------------------------------------------------------------
template <typename T, size_t _Alignment>
std::true_type/* enabled */AllocatorStealFrom(
    TLinearHeapAllocator<T, _Alignment>&,
    typename TLinearHeapAllocator<T, _Alignment>::pointer, size_t) {
    return std::true_type{}; // nothing to do, everything is already checked statically
}
//----------------------------------------------------------------------------
template <typename T, size_t _Alignment>
std::true_type/* enabled */AllocatorAcquireStolen(
    TLinearHeapAllocator<T, _Alignment>&,
    typename TLinearHeapAllocator<T, _Alignment>::pointer, size_t) {
    return std::true_type{}; // nothing to do, everything is already checked statically
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

//----------------------------------------------------------------------------
// Use FLinearHeap as an inplace allocator :
//----------------------------------------------------------------------------
template <typename T>
void* operator new(size_t sizeInBytes, Core::FLinearHeap& heap) {
    Assert(sizeInBytes == sizeof(T));
    return heap.Allocate(sizeInBytes, std::alignment_of_v<T>);
}
