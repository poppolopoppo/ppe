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
    InSituStorage() noexcept
        : _ptr(_data), _count(0) {}

    ~InSituStorage() {
        Assert(0 == _count);
        Assert(InSituEmpty());
        _ptr = nullptr;
    }

    InSituStorage(const InSituStorage& ) = delete;
    InSituStorage& operator=(const InSituStorage& ) = delete;

    InSituStorage(InSituStorage&& ) = delete;
    InSituStorage& operator=(InSituStorage&& ) = delete;

    void* AllocateIFP(size_t sizeInBytes);
    bool DeallocateIFP(void* ptr, size_t sizeInBytes) noexcept;
    void* ReallocateIFP(void* ptr, size_t newSizeInBytes, size_t oldSizeInBytes);

    bool InSituEmpty() const noexcept {
        Assert((_data == _ptr) == (0 == _count));
        return (_data == _ptr);
    }

    bool Contains(const void* ptr) const noexcept {
        THIS_THREADRESOURCE_CHECKACCESS();
        const u8* const p = reinterpret_cast<const u8*>(ptr);
        return _data <= p && p <= _data + _SizeInBytes;
    }

private:
    size_t _count;
    u8* _ptr;
    ALIGN(16) u8 _data[_SizeInBytes];
};
//----------------------------------------------------------------------------
template <size_t _SizeInBytes>
void* InSituStorage<_SizeInBytes>::AllocateIFP(size_t sizeInBytes) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Contains(_ptr));
    if (Contains(_ptr + sizeInBytes)) {
        Assert(0 != _count || _data == _ptr);
        ++_count;
        u8* const p = _ptr;
        _ptr += sizeInBytes;
        return p;
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
        Assert(0 < _count);
        --_count;
        Assert(Contains(p + sizeInBytes));
        if (0 == _count) {
            Assert(p + sizeInBytes == _ptr);
            _ptr = _data;
        }
        else if (p + sizeInBytes == _ptr) {
            _ptr = p;
        }
        else {
            Assert(0 < _count);
        }
        return true;
    }
    else {
        return false;
    }
}
//----------------------------------------------------------------------------
template <size_t _SizeInBytes>
void* InSituStorage<_SizeInBytes>::ReallocateIFP(void* ptr, size_t newSizeInBytes, size_t oldSizeInBytes) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Contains(_ptr));
    u8* p = reinterpret_cast<u8*>(ptr);
    if (Contains(p)) {
        Assert(Contains(p + oldSizeInBytes));
        Assert(0 < _count);
        if (p + oldSizeInBytes == _ptr && Contains(p + newSizeInBytes)) {
            _ptr = p + newSizeInBytes;
            return p;
        }
        else {
            return nullptr;
        }
    }
    else if (nullptr == ptr) {
        Assert(0 == oldSizeInBytes);
        return AllocateIFP(newSizeInBytes);
    }
    else {
        return nullptr;
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

    typedef std::false_type propagate_on_container_copy_assignment;
    typedef std::false_type propagate_on_container_move_assignment;
    typedef std::false_type propagate_on_container_swap;
    typedef std::false_type is_always_equal;

    typedef InSituStorage<_SizeInBytes> storage_type;

    enum : size_t { InSitu = (_SizeInBytes/sizeof(value_type)) };

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

    InSituAllocator(const InSituAllocator&) = delete;
    InSituAllocator& operator=(const InSituAllocator&) = delete;

    InSituAllocator(InSituAllocator&& rvalue) : InSituAllocator(rvalue._insitu) {}
    InSituAllocator& operator=(InSituAllocator&&) = delete;

    pointer allocate(size_type n);
    pointer allocate(size_type n, const void* /*hint*/) { return allocate(n); }
    void deallocate(pointer p, size_type n) noexcept;

    // see Relocate()
    void* relocate(void* p, size_type newSize, size_type oldSize);

    template <typename U, size_t N>
    friend bool operator ==(const InSituAllocator& lhs, const InSituAllocator<U, N, _Allocator>& rhs) noexcept {
        return (((N == _SizeInBytes) && (&lhs._insitu == &rhs._insitu)) ||
                (lhs._insitu.InSituEmpty() && rhs._insitu.InSituEmpty()) );
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
    Assert(n > 0);
    Assert(n < fallback_type::max_size());

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
    Assert(n > 0);
    Assert(n < fallback_type::max_size());

    if (false == _insitu.DeallocateIFP(p, n * sizeof(value_type)) )
        fallback_type::deallocate(p, n);
}
//----------------------------------------------------------------------------
template <typename T, size_t _SizeInBytes, typename _Allocator>
void* InSituAllocator<T, _SizeInBytes, _Allocator>::relocate(void* p, size_type newSize, size_type oldSize) {
    STATIC_ASSERT(std::is_pod<value_type>::value);
    Assert(nullptr == p || 0 < oldSize);

    if (0 == newSize) {
        if (p) {
            Assert(0 < oldSize);
            deallocate(static_cast<pointer>(p), oldSize);
        }
        return nullptr;
    }
    else {
        void* result = _insitu.ReallocateIFP(p, newSize * sizeof(value_type), oldSize * sizeof(value_type));
        if (nullptr != result)
            return result;

        return (_insitu.Contains(p))
            ? Relocate_AssumeNoRealloc(*this, MemoryView<value_type>(static_cast<pointer>(p), oldSize), newSize, oldSize)
            : Relocate_AssumePod(static_cast<fallback_type&>(*this), MemoryView<value_type>(static_cast<pointer>(p), oldSize), newSize, oldSize);
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
