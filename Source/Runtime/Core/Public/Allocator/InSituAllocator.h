#pragma once

#include "Core.h"

#include "Allocator/AllocatorBase.h"
#include "HAL/PlatformMemory.h"

#include <type_traits>

#define USE_PPE_INSITU_ALLOCATOR (!USE_PPE_MEMORY_DEBUGGING)

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t _SizeInBytes>
class TInSituStorage {
public:
    TInSituStorage() noexcept
        : _insituCount(0)
        , _insituOffset(0) {}

    ~TInSituStorage() {
        Assert(InSituEmpty());
        Assert(0 == _insituOffset);
    }

    TInSituStorage(const TInSituStorage& ) = delete;
    TInSituStorage& operator=(const TInSituStorage& ) = delete;

    TInSituStorage(TInSituStorage&& ) = delete;
    TInSituStorage& operator=(TInSituStorage&& ) = delete;

    FORCE_INLINE const u8* InsituData() const { return reinterpret_cast<const u8*>(&_insituData); }

    void* AllocateIFP(size_t sizeInBytes);
    bool DeallocateIFP(void* ptr, size_t sizeInBytes) noexcept;
    void* ReallocateIFP(void* ptr, size_t newSizeInBytes, size_t oldSizeInBytes);

    FORCE_INLINE bool InSituEmpty() const noexcept { return (0 == _insituCount); }

    FORCE_INLINE bool Contains(const void* ptr) const noexcept {
        return (InsituData() <= reinterpret_cast<const u8*>(ptr) &&
                reinterpret_cast<const u8*>(ptr) <= InsituData() + _SizeInBytes);
    }

private:
#ifdef ARCH_X86 // less padding -> pack to u16
    STATIC_ASSERT(_SizeInBytes < UINT16_MAX);
    typedef u16 size_type;
#else
    STATIC_ASSERT(_SizeInBytes < UINT32_MAX);
    typedef u32 size_type;
#endif

    typename std::aligned_storage<_SizeInBytes, ALLOCATION_BOUNDARY>::type _insituData;

    size_type _insituCount;
    size_type _insituOffset;
};
//----------------------------------------------------------------------------
template <size_t _SizeInBytes>
void* TInSituStorage<_SizeInBytes>::AllocateIFP(size_t sizeInBytes) {
#if USE_PPE_INSITU_ALLOCATOR
    if (_insituOffset + sizeInBytes > _SizeInBytes)
        return nullptr;

    void* p = (void*)(InsituData() + _insituOffset);

    _insituCount++;
    _insituOffset = checked_cast<size_type>(_insituOffset + sizeInBytes);

    return p;

#else
    UNUSED(sizeInBytes);
    return nullptr;

#endif
}
//----------------------------------------------------------------------------
template <size_t _SizeInBytes>
bool TInSituStorage<_SizeInBytes>::DeallocateIFP(void* ptr, size_t sizeInBytes) noexcept {
#if USE_PPE_INSITU_ALLOCATOR
    const uintptr_t offset = uintptr_t((const u8*)ptr - InsituData());
    if (offset > _insituOffset)
        return false;

    Assert(_insituCount > 0);
    Assert(offset + sizeInBytes <= _insituOffset);

    _insituCount--;

    if (offset + sizeInBytes == _insituOffset)
        _insituOffset = checked_cast<size_type>(offset);
    else
        Assert(0 < _insituCount); // TODO : memory just freed is definitively lost (bubble) ...

    return true;

#else
    UNUSED(ptr);
    UNUSED(sizeInBytes);
    return false;

#endif
}
//----------------------------------------------------------------------------
template <size_t _SizeInBytes>
void* TInSituStorage<_SizeInBytes>::ReallocateIFP(void* ptr, size_t newSizeInBytes, size_t oldSizeInBytes) {
#if USE_PPE_INSITU_ALLOCATOR
    Assert(ptr); // All previously handled :
    Assert(oldSizeInBytes);
    Assert(newSizeInBytes);

    const uintptr_t offset = uintptr_t((const u8*)ptr - InsituData());
    Assert(offset + oldSizeInBytes <= _insituOffset);
    Assert(_insituCount > 0);

    // Try realloc in-place ONLY IF ITS THE LAST BLOCK ALLOCATED
    if (offset + oldSizeInBytes == _insituOffset && offset + newSizeInBytes <= _SizeInBytes) {
        _insituOffset = checked_cast<size_type>(offset + newSizeInBytes);
        return ptr;
    }
    // Need to allocate externally and free this block afterwards
    else {
        return nullptr;
    }

#else
    UNUSED(ptr);
    UNUSED(newSizeInBytes);
    UNUSED(oldSizeInBytes);
    return nullptr;

#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _SizeInBytes, typename _Allocator>
class TInSituAllocator : public _Allocator {
public:
    template <typename U, size_t N, typename A>
    friend class TInSituAllocator;

    typedef _Allocator fallback_type;

    using typename fallback_type::value_type;
    using typename fallback_type::pointer;
    using typename fallback_type::size_type;

    typedef std::false_type propagate_on_container_copy_assignment;
    typedef std::false_type propagate_on_container_move_assignment;
    typedef std::false_type propagate_on_container_swap;
    typedef std::false_type is_always_equal;

    typedef TInSituStorage<_SizeInBytes> storage_type;

    STATIC_ASSERT((_SizeInBytes % sizeof(T)) == 0);
    STATIC_CONST_INTEGRAL(size_t, GInSituSize, (_SizeInBytes/sizeof(T)));

    template<typename U>
    struct rebind {
        typedef TInSituAllocator<
            U, _SizeInBytes,
            typename _Allocator::template rebind<U>::other
        >   other;
    };

    const storage_type& InSitu() const { return _insitu; }

    fallback_type& FallbackAllocator() { return static_cast<fallback_type&>(*this); }
    const fallback_type& FallbackAllocator() const { return static_cast<const fallback_type&>(*this); }

    TInSituAllocator(storage_type& insitu) noexcept : _insitu(insitu) {}
    template <typename U, typename A>
    TInSituAllocator(const TInSituAllocator<U, _SizeInBytes, A>& other) noexcept : _insitu(other._insitu) {}

    TInSituAllocator(const TInSituAllocator&) = delete;
    TInSituAllocator& operator=(const TInSituAllocator&) = delete;

    TInSituAllocator(TInSituAllocator&& rvalue) : TInSituAllocator(rvalue._insitu) {}
    TInSituAllocator& operator=(TInSituAllocator&&) = delete;

    pointer allocate(size_type n);
    pointer allocate(size_type n, const void* /*hint*/) { return allocate(n); }
    void deallocate(pointer p, size_type n) noexcept;

    // see Relocate()
    void* relocate(void* p, size_type newSize, size_type oldSize);

    template <typename U, size_t N>
    friend bool operator ==(const TInSituAllocator& lhs, const TInSituAllocator<U, N, _Allocator>& rhs) noexcept {
        return (((N == _SizeInBytes) && (&lhs._insitu == &rhs._insitu)) ||
                (lhs._insitu.InSituEmpty() && rhs._insitu.InSituEmpty()) );
    }

    template <typename U, size_t N>
    friend bool operator !=(const TInSituAllocator& lhs, const TInSituAllocator<U, N, _Allocator>& rhs) noexcept {
        return !operator ==(lhs, rhs);
    }

private:
    storage_type& _insitu;
};
//----------------------------------------------------------------------------
template <typename T, size_t _SizeInBytes, typename _Allocator>
auto TInSituAllocator<T, _SizeInBytes, _Allocator>::allocate(size_type n) -> pointer {
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
void TInSituAllocator<T, _SizeInBytes, _Allocator>::deallocate(pointer p, size_type n) noexcept {
    Assert(p);
    Assert(n > 0);
    Assert(n < fallback_type::max_size());

    if (false == _insitu.DeallocateIFP(p, n * sizeof(value_type)) )
        fallback_type::deallocate(p, n);
}
//----------------------------------------------------------------------------
template <typename T, size_t _SizeInBytes, typename _Allocator>
void* TInSituAllocator<T, _SizeInBytes, _Allocator>::relocate(void* p, size_type newSize, size_type oldSize) {
    STATIC_ASSERT(Meta::TIsPod<value_type>::value);
    Assert(nullptr == p || 0 < oldSize);

    if (Likely(0 == oldSize)) {
        Assert(nullptr == p);
        return allocate(newSize);
    }
    else if (Unlikely(0 == newSize)) {
        if (p) {
            Assert(0 < oldSize);
            deallocate(static_cast<pointer>(p), oldSize);
        }
        return nullptr;
    }
    else {
        Assert(nullptr != p);

        if (not _insitu.Contains(p))
            return Relocate_AssumePod(
                static_cast<fallback_type&>(*this),
                TMemoryView<value_type>(static_cast<pointer>(p), oldSize),
                newSize, oldSize );

        void* newp = _insitu.ReallocateIFP(p, newSize * sizeof(value_type), oldSize * sizeof(value_type));
        if (newp)
            return newp;

        newp = fallback_type::allocate(newSize);
        if (nullptr == newp)
            return nullptr;

        // This is a POD type, so this is safe :
        const size_t cpySize = Min(oldSize, newSize);
        FPlatformMemory::Memcpy(newp, p, cpySize * sizeof(value_type));

        if (not _insitu.DeallocateIFP(p, oldSize * sizeof(value_type)))
            AssertNotReached();

        return newp;
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _SizeInBytes, typename _Allocator>
size_t AllocatorSnapSize(const TInSituAllocator<T, _SizeInBytes, _Allocator>& allocator, size_t size) {
#if USE_PPE_INSITU_ALLOCATOR
    constexpr size_t GInSituSize = TInSituAllocator<T, _SizeInBytes, _Allocator>::GInSituSize;
    return (size <= GInSituSize ? GInSituSize : AllocatorSnapSize(allocator.FallbackAllocator(), size));
#else
    return AllocatorSnapSize(allocator.FallbackAllocator(), size);
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _SizeInBytes, typename _Allocator, typename U, typename _Allocator2>
struct allocator_can_steal_from<
    TInSituAllocator<T, _SizeInBytes, _Allocator>,
    TInSituAllocator<U, _SizeInBytes, _Allocator2>
>   : allocator_can_steal_from<_Allocator, _Allocator2> {};
//----------------------------------------------------------------------------
template <typename T, size_t _SizeInBytes, typename _Allocator, typename _Allocator2>
struct allocator_can_steal_from<
    TInSituAllocator<T, _SizeInBytes, _Allocator>,
    _Allocator2
>   : allocator_can_steal_from<_Allocator, _Allocator2> {};
//----------------------------------------------------------------------------
template <typename T, size_t _SizeInBytes, typename _Allocator, typename _Allocator2>
struct allocator_can_steal_from<
    _Allocator,
    TInSituAllocator<T, _SizeInBytes, _Allocator2>
>   : allocator_can_steal_from<_Allocator, _Allocator2> {};
//----------------------------------------------------------------------------
template <typename T, size_t _SizeInBytes, typename _Allocator>
auto/* inherited */AllocatorStealFrom(
    TInSituAllocator<T, _SizeInBytes, _Allocator>& alloc,
    typename TInSituAllocator<T, _SizeInBytes, _Allocator>::pointer ptr, size_t size ) {
    // checks that we not stealing from insitu storage, which can't be moved
#if USE_PPE_INSITU_ALLOCATOR
    Assert(TInSituAllocator<T, _SizeInBytes, _Allocator>::GInSituSize < size);
#endif
    Assert(not alloc.InSitu().Contains(ptr));
    return AllocatorStealFrom(alloc.FallbackAllocator(), ptr, size);
}
//----------------------------------------------------------------------------
template <typename T, size_t _SizeInBytes, typename _Allocator>
auto/* inherited */AllocatorAcquireStolen(
    TInSituAllocator<T, _SizeInBytes, _Allocator>& alloc,
    typename TInSituAllocator<T, _SizeInBytes, _Allocator>::pointer ptr, size_t size ) {
    // checks that the stolen block is larger than insitu storage, we can't break this predicate
#if USE_PPE_INSITU_ALLOCATOR
    Assert(TInSituAllocator<T, _SizeInBytes, _Allocator>::GInSituSize < size);
#endif
    return AllocatorAcquireStolen(alloc.FallbackAllocator(), ptr, size);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
