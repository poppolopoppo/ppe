#pragma once

#include "Core/Core.h"

#include "Core/Allocator/AllocatorBase.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _SizeInBytes, typename _Allocator>
class InSituAllocator : public _Allocator {
public:
    template <typename, size_t, typename>
    friend class InSituAllocator;

    typedef _Allocator fallback_type;

    typedef T *pointer;
    typedef std::size_t size_type;

    typedef typename std::aligned_storage< _SizeInBytes, 16 >::type
        storage_type;

    template<typename U>
    struct rebind {
        typedef InSituAllocator<
            U, _SizeInBytes,
            typename _Allocator::template rebind<U>::other
        >   other;
    };

    InSituAllocator(storage_type& insitu) throw() : _pinsitu(&insitu) {}

    template <typename U, typename A>
    InSituAllocator(const InSituAllocator<U, _SizeInBytes, A>& other)
        : _pinsitu(other._pinsitu) {}

    InSituAllocator& operator =(const InSituAllocator& ) { return *this; }

    pointer allocate(size_type n);
    pointer allocate(size_type n, const void* /*hint*/) { return allocate(n); }
    void deallocate(pointer p, size_type n);

    template <typename U, typename A>
    friend bool operator ==(const InSituAllocator& lhs, const InSituAllocator<U, _SizeInBytes, A>& rhs) {
        return lhs._pinsitu == rhs._pinsitu;
    }

    template <typename U, typename A>
    friend bool operator !=(const InSituAllocator& lhs, const InSituAllocator<U, _SizeInBytes, A>& rhs) {
        return !operator ==(lhs, rhs);
    }

private:
    storage_type *_pinsitu;
};
//----------------------------------------------------------------------------
template <typename T, size_t _SizeInBytes, typename _Allocator>
auto InSituAllocator<T, _SizeInBytes, _Allocator>::allocate(size_type n) -> pointer {
    if (n * sizeof(T) == _SizeInBytes) {
        Assert(_pinsitu);
        return reinterpret_cast<T *>(_pinsitu);
    }
    else {
        return fallback_type::allocate(n);
    }
}
//----------------------------------------------------------------------------
template <typename T, size_t _SizeInBytes, typename _Allocator>
void InSituAllocator<T, _SizeInBytes, _Allocator>::deallocate(pointer p, size_type n) {
    if ((void *)p != (void *)_pinsitu)
        fallback_type::deallocate(p, n);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
