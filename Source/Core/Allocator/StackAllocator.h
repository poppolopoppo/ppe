#pragma once

#include "Core/Core.h"

#include "Core/Allocator/AllocatorBase.h"
#include "Core/Allocator/Alloca.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class TStackAllocator : public TAllocatorBase<T> {
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
        typedef TStackAllocator<U> other;
    };

    TStackAllocator() noexcept {}

    TStackAllocator(const TStackAllocator& ) noexcept {}
    template <typename U>
    TStackAllocator(const TStackAllocator<U>&) noexcept {}

    TStackAllocator& operator=(const TStackAllocator& ) noexcept { return *this; }
    template <typename U>
    TStackAllocator& operator=(const TStackAllocator<U>&) noexcept { return *this; }

    pointer allocate(size_type n);
    pointer allocate(size_type n, const void* /*hint*/) { return allocate(n); }
    void deallocate(void* p, size_type n);

    // see AllocatorRealloc()
    void* relocate(void* p, size_type newSize, size_type oldSize);
};
//----------------------------------------------------------------------------
template <typename T>
auto TStackAllocator<T>::allocate(size_type n) -> pointer {
    // The return value of allocate(0) is unspecified.
    // TStackAllocator returns NULL in order to avoid depending
    // on Alloca(0)'s implementation-defined behavior
    // (the implementation can define Alloca(0) to return NULL,
    // in which case the bad_alloc check below would fire).
    // All allocators can return NULL in this case.
    if (n == 0)
        return nullptr;

    // All allocators should contain an integer overflow check.
    // The Standardization Committee recommends that std::length_error
    // be thrown in the case of integer overflow.
    if (n > max_size())
        CORE_THROW_IT(std::length_error("TStackAllocator<T>::allocate() - Integer overflow."));

    // TStackAllocator wraps Alloca().
    void * const pv = Alloca(n * sizeof(T));
    Assert(Meta::IsAligned(16, pv));

    // Allocators should throw std::bad_alloc in the case of memory allocation failure.
    if (pv == nullptr)
        CORE_THROW_IT(std::bad_alloc());

    return static_cast<T *>(pv);
}
//----------------------------------------------------------------------------
template <typename T>
void TStackAllocator<T>::deallocate(void* p, size_type n) {
    UNUSED(n);
    // TStackAllocator wraps Alloca().
    if (p)
        FreeAlloca(p);
}
//----------------------------------------------------------------------------
template <typename T>
void* TStackAllocator<T>::relocate(void* p, size_type newSize, size_type oldSize) {
    UNUSED(oldSize);

    // TStackAllocator wraps Alloca()
    void* const newp = RelocateAlloca(p, newSize * sizeof(T), true);
    Assert(Meta::IsAligned(16, newp));

    if (nullptr == newp && newSize)
        CORE_THROW_IT(std::bad_alloc());

    return newp;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename U, typename V>
bool operator ==(const TStackAllocator<U>&/* lhs */, const TStackAllocator<V>&/* rhs */) {
    return true;
}
//----------------------------------------------------------------------------
template <typename U, typename V>
bool operator !=(const TStackAllocator<U>& lhs, const TStackAllocator<V>& rhs) {
    return not operator ==(lhs, rhs);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
size_t AllocatorSnapSize(const TStackAllocator<T>&, size_t size) {
    return AllocaSnapSize((size * sizeof(T) + sizeof(T) - 1) / sizeof(T));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
