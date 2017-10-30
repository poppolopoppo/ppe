#pragma once

#include "Core/Core.h"

#include "Core/Allocator/AllocatorBase.h"
#include "Core/Allocator/HeapAllocator.h"
#include "Core/Meta/ThreadResource.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
extern FHeap& GetThreadLocalHeap();
//----------------------------------------------------------------------------
namespace Heaps {
//----------------------------------------------------------------------------
struct FThreadLocal {
    static FHeap& Instance() { return Core::GetThreadLocalHeap(); }
};
//----------------------------------------------------------------------------
} //!namespace Heaps
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef WITH_CORE_THREADRESOURCE_CHECKS
//----------------------------------------------------------------------------
template <typename T>
class EMPTY_BASES TThreadLocalAllocator : public THeapAllocator<T, Heaps::FThreadLocal>, public Meta::FThreadResource {
public:
    typedef THeapAllocator<T, Heaps::FThreadLocal> base_type;

    using typename base_type::pointer;
    using typename base_type::size_type;

    using typename base_type::propagate_on_container_copy_assignment;
    using typename base_type::propagate_on_container_move_assignment;
    using typename base_type::propagate_on_container_swap;
    using typename base_type::is_always_equal;

    using base_type::address;
    using base_type::construct;
    using base_type::destroy;
    using base_type::max_size;

    template<typename U>
    struct rebind {
        typedef TThreadLocalAllocator<U> other;
    };

    TThreadLocalAllocator() throw() {}

    TThreadLocalAllocator(const TThreadLocalAllocator& other) throw() {}
    template <typename U>
    TThreadLocalAllocator(const TThreadLocalAllocator<U>& other) throw() {}

    TThreadLocalAllocator& operator =(const TThreadLocalAllocator& other) {
        THIS_THREADRESOURCE_CHECKACCESS();
        THREADRESOURCE_CHECKACCESS(&other);
        return *this;
    }
    template <typename U>
    TThreadLocalAllocator& operator =(const TThreadLocalAllocator<U>&) {
        THIS_THREADRESOURCE_CHECKACCESS();
        THREADRESOURCE_CHECKACCESS(&other);
        return *this;
    }

    pointer allocate(size_type n) {
        THIS_THREADRESOURCE_CHECKACCESS();
        return base_type::allocate(n);
    }

    pointer allocate(size_type n, const void* /*hint*/) { return allocate(n); }
    void deallocate(void* p, size_type n) {
        THIS_THREADRESOURCE_CHECKACCESS();
        return base_type::deallocate(p, n);
    }

    void* relocate(void* p, size_type newSize, size_type oldSize) {
        THIS_THREADRESOURCE_CHECKACCESS();
        return base_type::relocate(p, newSize, oldSize);
    }

    template <typename U>
    friend bool operator ==(const TThreadLocalAllocator& lhs, const TThreadLocalAllocator<U>& rhs) {
        return (lhs.ThreadId() == rhs.ThreadId());
    }

    template <typename U>
    friend bool operator !=(const TThreadLocalAllocator& lhs, const TThreadLocalAllocator<U>& rhs) {
        return !operator ==(lhs, rhs);
    }
};
//----------------------------------------------------------------------------
#else
//----------------------------------------------------------------------------
template <typename T>
using TThreadLocalAllocator = THeapAllocator<T, Heaps::FThreadLocal>;
//----------------------------------------------------------------------------
#endif //!WITH_CORE_THREADRESOURCE_CHECKS
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
