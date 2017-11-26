#pragma once

#include "Core/Core.h"

#include "Core/Allocator/AllocatorBase.h"
#include "Core/Allocator/Malloc.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Alignment = sizeof(ptrdiff_t) >
class TMallocator : public TAllocatorBase<T> {
public:
    typedef TAllocatorBase<T> base_type;

    using typename base_type::pointer;
    using typename base_type::size_type;

    typedef std::true_type propagate_on_container_copy_assignment;
    typedef std::true_type propagate_on_container_move_assignment;
    typedef std::true_type propagate_on_container_swap;
    typedef std::true_type is_always_equal;

    using base_type::address;
    using base_type::construct;
    using base_type::destroy;
    using base_type::max_size;

    template<typename U>
    struct rebind {
        typedef TMallocator<U> other;
    };

    TMallocator() noexcept {}

    TMallocator(const TMallocator& ) noexcept {}
    template <typename U>
    TMallocator(const TMallocator<U>&) noexcept {}

    TMallocator& operator=(const TMallocator& ) noexcept { return *this; }
    template <typename U>
    TMallocator& operator=(const TMallocator<U>&) noexcept { return *this; }

    pointer allocate(size_type n);
    pointer allocate(size_type n, const void* /*hint*/) { return allocate(n); }
    void deallocate(void* p, size_type n);

    // see AllocatorRealloc()
    void* relocate(void* p, size_type newSize, size_type oldSize);
};
//----------------------------------------------------------------------------
template <typename T, size_t _Alignment>
auto TMallocator<T, _Alignment>::allocate(size_type n) -> pointer {
    // The return value of allocate(0) is unspecified.
    // TMallocator returns NULL in order to avoid depending
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
        CORE_THROW_IT(std::length_error("TMallocator<T>::allocate() - Integer overflow."));

    // TMallocator wraps malloc().
    void * const pv = Core::malloc<_Alignment>(n * sizeof(T));

    // Allocators should throw std::bad_alloc in the case of memory allocation failure.
    if (pv == nullptr)
        CORE_THROW_IT(std::bad_alloc());

    return static_cast<T *>(pv);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Alignment>
void TMallocator<T, _Alignment>::deallocate(void* p, size_type n) {
    UNUSED(n);
    // TMallocator wraps malloc().
    Core::free<_Alignment>(p);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Alignment>
void* TMallocator<T, _Alignment>::relocate(void* p, size_type newSize, size_type oldSize) {
    UNUSED(oldSize);

    // TMallocator wraps malloc()
    void* const newp = Core::realloc<_Alignment>(p, newSize * sizeof(T));
    if (nullptr == newp && newSize)
        CORE_THROW_IT(std::bad_alloc());

    return newp;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t A, typename U, size_t B>
bool operator ==(const TMallocator<T, A>&/* lhs */, const TMallocator<U, B>&/* rhs */) {
    return A == B;
}
//----------------------------------------------------------------------------
template <typename T, size_t A, typename U, size_t B>
bool operator !=(const TMallocator<T, A>& lhs, const TMallocator<U, B>& rhs) {
    return !operator ==(lhs, rhs);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Alignment>
size_t AllocatorSnapSize(const TMallocator<T, _Alignment>&, size_t size) {
    return (Core::malloc_snap_size(size * sizeof(T)) / sizeof(T));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
