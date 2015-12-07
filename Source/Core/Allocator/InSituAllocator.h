#pragma once

#include "Core/Core.h"

#include "Core/Allocator/AllocatorBase.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t _SizeInBytes>
class InSituStorage : Meta::ThreadResource {
public:
    InSituStorage() noexcept : _ptr(_data) {}
    ~InSituStorage() { Assert(InSituEmpty()); _ptr = nullptr; }

    InSituStorage(const InSituStorage& ) = delete;
    InSituStorage& operator=(const InSituStorage& ) = delete;

    void* AllocateIFP(size_t sizeInBytes);
    bool DeallocateIFP(void* ptr, size_t sizeInBytes) noexcept;

    bool InSituEmpty() const noexcept { return (_data == _ptr); }

    bool Contains(const void* ptr) const noexcept {
        THIS_THREADRESOURCE_CHECKACCESS();
        const u8* const p = reinterpret_cast<const u8*>(ptr);
        return _data <= p && p <= _data + _SizeInBytes;
    }

private:
    u8* _ptr;
    ALIGN(16) u8 _data[_SizeInBytes];
};
//----------------------------------------------------------------------------
template <size_t _SizeInBytes>
void* InSituStorage<_SizeInBytes>::AllocateIFP(size_t sizeInBytes) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Contains(_ptr));
    if (_data + _SizeInBytes - _ptr >= sizeInBytes) {
        u8* const r = _ptr;
        _ptr += sizeInBytes;
        return r;
    }
    else {
        return nullptr;
    }
}
//----------------------------------------------------------------------------
template <size_t _SizeInBytes>
bool InSituStorage<_SizeInBytes>::DeallocateIFP(void* ptr, size_t sizeInBytes) noexcept {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Contains(_ptr));
    u8* p = reinterpret_cast<u8*>(ptr);
    if (Contains(p)) {
        if (p + sizeInBytes == _ptr)
            _ptr = p;
        return true;
    }
    else {
        return false;
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _SizeInBytes, typename _Allocator>
class InSituAllocator : public _Allocator {
public:
    template <typename U, size_t N, typename A>
    friend class InSituAllocator;

    typedef _Allocator fallback_type;

    using typename fallback_type::value_type;
    using typename fallback_type::pointer;
    using typename fallback_type::size_type;

    typedef InSituStorage<_SizeInBytes> storage_type;

    template<typename U>
    struct rebind {
        typedef InSituAllocator<
            U, _SizeInBytes,
            typename _Allocator::template rebind<U>::other
        >   other;
    };

    InSituAllocator(storage_type& insitu) noexcept : _insitu(insitu) {}
    template <typename U, typename A>
    InSituAllocator(const InSituAllocator<U, _SizeInBytes, A>& other) noexcept : _insitu(other._insitu) {}
    InSituAllocator(const InSituAllocator&) = default;
    InSituAllocator& operator=(const InSituAllocator&) = delete;

    pointer allocate(size_type n);
    pointer allocate(size_type n, const void* /*hint*/) { return allocate(n); }
    void deallocate(pointer p, size_type n) noexcept;

    template <typename U, size_t N>
    friend bool operator ==(const InSituAllocator& lhs, const InSituAllocator<U, N, _Allocator>& rhs) noexcept {
        return ( (N == _SizeInBytes) && (&lhs._insitu == &rhs._insitu) );
    }

    template <typename U, size_t N>
    friend bool operator !=(const InSituAllocator& lhs, const InSituAllocator<U, N, _Allocator>& rhs) noexcept {
        return !operator ==(lhs, rhs);
    }

private:
    storage_type& _insitu;
};
//----------------------------------------------------------------------------
template <typename T, size_t _SizeInBytes, typename _Allocator>
auto InSituAllocator<T, _SizeInBytes, _Allocator>::allocate(size_type n) -> pointer {
    pointer p = reinterpret_cast<pointer>(_insitu.AllocateIFP(n * sizeof(value_type)));
    if (nullptr == p)
        p = fallback_type::allocate(n);
    Assert(p); // fallback_type should have thrown a std::bad_alloc() exception
    return p;
}
//----------------------------------------------------------------------------
template <typename T, size_t _SizeInBytes, typename _Allocator>
void InSituAllocator<T, _SizeInBytes, _Allocator>::deallocate(pointer p, size_type n) noexcept {
    Assert(p);
    if (false == _insitu.DeallocateIFP(p, n * sizeof(value_type)) )
        fallback_type::deallocate(p, n);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
