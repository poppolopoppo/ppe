#pragma once

#include "Core_fwd.h"

#include "Allocator/AllocatorBase.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Wraps our own allocator with std::allocator<> interface for stl-compliance
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
class TStlAllocator : protected _Allocator {
public:
    using ppe_traits = TAllocatorTraits<_Allocator>;

    using propagate_on_container_copy_assignment = typename ppe_traits::propagate_on_container_copy_assignment;
    using propagate_on_container_move_assignment = typename ppe_traits::propagate_on_container_move_assignment;
    using propagate_on_container_swap = typename ppe_traits::propagate_on_container_swap;

    using is_always_equal = typename ppe_traits::is_always_equal;

    using value_type = T;
    using pointer = Meta::TAddPointer<T>;
    using const_pointer = Meta::TAddPointer<const T>;
    using reference = Meta::TAddReference<T>;
    using const_reference = Meta::TAddReference<const T>;

    using difference_type = ptrdiff_t;
    using size_type = size_t;

    template <typename U>
    struct rebind {
        typedef TStlAllocator<U, _Allocator> other;
    };

    TStlAllocator() = default;

    explicit TStlAllocator(_Allocator&& ralloc) NOEXCEPT
        : _Allocator(TAllocatorTraits<_Allocator>::SelectOnMove(std::move(ralloc))) {}
    explicit TStlAllocator(const _Allocator& alloc) NOEXCEPT
        : _Allocator(TAllocatorTraits<_Allocator>::SelectOnCopy(alloc)) {}

    template <typename U>
    TStlAllocator(const TStlAllocator<U, _Allocator>& other)
        : _Allocator(ppe_traits::SelectOnCopy(other.InnerAlloc())) {}

    template <typename U>
    TStlAllocator& operator =(const TStlAllocator<U, _Allocator>& other) {
        ppe_traits::Copy(this, other.InnerAlloc());
        return (*this);
    }

    template <typename U>
    TStlAllocator(TStlAllocator<U, _Allocator>&& rvalue) NOEXCEPT
        : _Allocator(ppe_traits::SelectOnMove(std::move(rvalue.InnerAlloc()))) {}

    template <typename U>
    TStlAllocator& operator =(TStlAllocator<U, _Allocator>&& rvalue) NOEXCEPT {
        ppe_traits::Move(this, std::move(rvalue.InnerAlloc()));
        return (*this);
    }

    pointer address(reference x) const NOEXCEPT {
        return std::addressof(x);
    }

    const_pointer address(const_reference x) const NOEXCEPT {
        return std::addressof(x);
    }

    pointer allocate(size_type n) {
        return static_cast<pointer>(ppe_traits::Allocate(*this, n * sizeof(T)).Data);
    }

    pointer allocate(size_type n, const void* hint) {
        UNUSED(hint);
        return static_cast<pointer>(ppe_traits::Allocate(*this, n * sizeof(T)).Data);
    }

    void deallocate(pointer p, size_type n) {
        ppe_traits::Deallocate(*this, FAllocatorBlock{p, n * sizeof(T)});
    }

    // ** this one is *NOT* in STL, but can be still very useful **
    void reallocate(pointer p, size_type newn, size_t oldn) {
        ppe_traits::Reallocate(*this, FAllocatorBlock{p, oldn * sizeof(T)}, newn * sizeof(T));
    }

    size_type max_size() const NOEXCEPT {
        return (ppe_traits::MaxSize(*this) / sizeof(T));
    }

    void construct(pointer p, const_reference val) {
        Meta::Construct(p, val);
    }

    template <typename U, typename... _Args>
    void construct(U* p, _Args&& ... args) {
        Meta::Construct(p, std::forward<_Args>(args)...);
    }

    void destroy(pointer p) {
        Meta::Destroy(p);
    }

    template <typename U>
    void destroy(U* p) {
        Meta::Destroy(p);
    }

    TStlAllocator select_on_container_copy_construction() const {
        _Allocator alloc;
        ppe_traits::Copy(&alloc, *this);
        return TStlAllocator{std::move(alloc)};
    }

    friend bool operator ==(const TStlAllocator& lhs, const TStlAllocator& rhs) NOEXCEPT {
        return ppe_traits::Equals(lhs, rhs);
    }

    friend bool operator !=(const TStlAllocator& lhs, const TStlAllocator& rhs) NOEXCEPT {
        return (not operator ==(lhs, rhs));
    }

public: // PPE allocators :
    _Allocator& InnerAlloc() NOEXCEPT {
        return ppe_traits::Get(*this);
    }

    const _Allocator& InnerAlloc() const NOEXCEPT {
        return ppe_traits::Get(*this);
    }
};
//----------------------------------------------------------------------------
class FStdMallocator : private FGenericAllocator {
public:
    using propagate_on_container_copy_assignment = std::true_type;
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_swap = std::true_type;

    using is_always_equal = std::true_type; // STL allocators can't have a state

    using has_maxsize = std::false_type;
    using has_owns = std::false_type;
    using has_reallocate = std::true_type;
    using has_acquire = std::true_type;
    using has_steal = std::true_type;

    STATIC_CONST_INTEGRAL(size_t, Alignment, ALLOCATION_BOUNDARY);

    FStdMallocator() = default;

    static size_t SnapSize(size_t s) NOEXCEPT {
        return Meta::RoundToNextPow2(s, Alignment);
    }

    FAllocatorBlock Allocate(size_t s) const {
        return FAllocatorBlock{ std::malloc(s), s };
    }

    void Deallocate(FAllocatorBlock b) const {
        std::free(b.Data);
    }

    void Reallocate(FAllocatorBlock& b, size_t s) const {
        b.Data = std::realloc(b.Data, s);
        b.SizeInBytes = s;
    }

    bool Acquire(FAllocatorBlock b) NOEXCEPT {
        UNUSED(b); // nothing to do
        return true;
    }

    bool Steal(FAllocatorBlock b) NOEXCEPT {
        UNUSED(b); // nothing to do
        return true;
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
