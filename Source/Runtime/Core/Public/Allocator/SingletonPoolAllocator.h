#pragma once

#include "Core.h"

#include "Allocator/AllocatorBase.h"
#include "Allocator/PoolAllocatorTag.h"
#include "Memory/SegregatedMemoryPool.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, bool _ThreadLocal = false, typename _PoolTag = POOL_TAG(Default) >
class TSingletonPoolAllocator : public TAllocatorBase<T> {
public:
    typedef TAllocatorBase<T> base_type;
    typedef _PoolTag pooltag_type;
    enum : bool { FThreadLocal = _ThreadLocal };

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

    typedef TTypedSegregatedMemoryPool<_PoolTag, T, _ThreadLocal> segregatedpool_type;

    template<typename U>
    struct rebind {
        typedef TSingletonPoolAllocator<U, _ThreadLocal, _PoolTag > other;
    };

    TSingletonPoolAllocator() throw() {}

    TSingletonPoolAllocator(const TSingletonPoolAllocator&) throw() {}
    template <typename U>
    TSingletonPoolAllocator(const TSingletonPoolAllocator<U, _ThreadLocal, _PoolTag>&) throw() {}

    TSingletonPoolAllocator& operator =(const TSingletonPoolAllocator&) { return *this; }
    template <typename U>
    TSingletonPoolAllocator& operator =(const TSingletonPoolAllocator<U, _ThreadLocal, _PoolTag>&) { return *this; }

    pointer allocate(size_type n);
    pointer allocate(size_type n, const void* /*hint*/) { return allocate(n); }
    void deallocate(void* p, size_type n);

    template <typename U>
    friend bool operator ==(const TSingletonPoolAllocator&/* lhs */, const TSingletonPoolAllocator<U, _ThreadLocal, _PoolTag>&/* rhs */) {
        return true;
    }

    template <typename U>
    friend bool operator !=(const TSingletonPoolAllocator& lhs, const TSingletonPoolAllocator<U, _ThreadLocal, _PoolTag>& rhs) {
        return !operator ==(lhs, rhs);
    }
};
//----------------------------------------------------------------------------
template <typename T, bool _ThreadLocal, typename _PoolTag >
auto TSingletonPoolAllocator<T, _ThreadLocal, _PoolTag>::allocate(size_type n) -> pointer {
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
    if (n > 1)
        PPE_THROW_IT(std::length_error("TSingletonPoolAllocator<T, _ThreadLocal, _PoolTag>::allocate() - pool allocator does not support contiguous allocations."));

    // TSingletonPoolAllocator wraps TTypedSegregatedMemoryPool<>.
    void * const pv = segregatedpool_type::Allocate();

    // Allocators should throw std::bad_alloc in the case of memory allocation failure.
    if (pv == nullptr)
        PPE_THROW_IT(std::bad_alloc());

    return static_cast<T *>(pv);
}
//----------------------------------------------------------------------------
template <typename T, bool _ThreadLocal, typename _PoolTag >
void TSingletonPoolAllocator<T, _ThreadLocal, _PoolTag>::deallocate(void* p, size_type n) {
    UNUSED(n);
    Assert(1 >= n);
    // TSingletonPoolAllocator wraps TTypedSegregatedMemoryPool<>.
    segregatedpool_type::Deallocate(p);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define SINGLETON_POOL_ALLOCATOR(_Domain, T, _PoolTag) \
    DECORATE_ALLOCATOR(_Domain, ::Core::TSingletonPoolAllocator<COMMA_PROTECT(T) COMMA false COMMA _PoolTag >)
//----------------------------------------------------------------------------
#define THREAD_LOCAL_SINGLETON_POOL_ALLOCATOR(_Domain, T, _PoolTag) \
    DECORATE_ALLOCATOR(_Domain, ::Core::TSingletonPoolAllocator<COMMA_PROTECT(T) COMMA true  COMMA _PoolTag >)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, bool _ThreadLocal, typename _PoolTag >
size_t AllocatorSnapSize(const TSingletonPoolAllocator<T, _ThreadLocal, _PoolTag>&, size_t size) {
    Assert(1 == size);
    return 1;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
